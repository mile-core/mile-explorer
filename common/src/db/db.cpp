//
// Created by lotus mile on 29/10/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

optional<Db> Db::Open(const std::string &db_name, const std::string &host, int port) {
    try {
        auto connection = db::Driver::connect(host, port);
        if (!connection) {
            Db::error_handler(milecsa::result::FAIL, ErrorFormat("Could not connect to server %s:%i", host.c_str(), port));
            return nullopt;
        }

        bool exists = false;
        db::Driver::Cursor databases = db::Driver::db_list().run(*connection);
        for (db::Driver::Datum const &db : databases) {
            if (*db.get_string() == db_name) {
                exists = true;
                break;
            }
        }

        return make_optional(Db(db_name,host,port, exists));
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} open error {}", db_name.c_str(), e.message);
    }
    return nullopt;
}

Db::Db(const std::string &db_name,
       const string &ip, unsigned short port, bool exists)
        :
        db_name_(db_name),
        host_(ip),
        port_(port),
        last_block_id_(0),
        exists_(exists){
    Db::log->trace("Db: {} connected to {}:{}", db_name_.c_str(), host_.c_str(), port_);
}

Db::~Db(){}

const db::Connection Db::get_connection() const {
    db::Connection c = db::Driver::connect(host_, port_);
    return std::move(c);
}

bool Db::is_exist() {
    return exists_;
}

bool Db::has_table(const std::string &name) {
    try {
        auto connection = get_connection();
        auto v = query().table_list().run(*connection).to_array();
        return (std::find(v.begin(), v.end(), name) != v.end());
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} has_table error {}", db_name_.c_str(), e.message);
    }
    return false;
}

Db::Table Db::create_table(const std::string &name) {
    try {
        auto connection = get_connection();
        query().table_create(name).run(*connection);
        return db::Table::Open(*this, name);
    }
    catch (db::Error &e) {
        return db::Table::Open(*this, name);
    }
}

Db::Table Db::open_table(const std::string &name) {
    return db::Table::Open(*this, name);
}

bool Db::init() {
    bool init_db = !is_exist();

    try {

        if (init_db) {
            auto connection = get_connection();
            db::Driver::db_create(db_name_).run(*connection);
            Db::log->info("Db: {} is initialized ...", db_name_.c_str());
        }

        for(auto name: table::get_names()) {
            if (!has_table(name))
                create_table(name);
        }

        for(auto desc: table::get_indices()){
            auto table = open_table(desc.table);
            if (!table->has_index(desc.field))
                table->create_index(desc.field);
        }

        if(is_exist()) {
            try {
                last_block_id_ = get_last_block_id();
                Db::log->trace("Db: {} start at block-id: {}", db_name_.c_str(),  UInt256ToDecString(last_block_id_));

            } catch (db::Error &e) {
                Db::err->error("Db: {} error reading last block id {}", db_name_.c_str(), e.message);
            }
        }
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error initializing {}", db_name_.c_str(), e.message);
    }

    Db::log->info("Db: {} is opened ...", db_name_.c_str());

    return init_db;
}
