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
    auto connection = db::Driver::connect(host, port);
    if (!connection) {
        Db::error_handler(milecsa::result::FAIL, ErrorFormat("Could not connect to server %s:%i", host.c_str(), port));
        return nullopt;
    }
    return make_optional(Db(db_name,host,port));
}

Db::Db(const std::string &db_name,
       const string &ip, unsigned short port)
        :
        db_name_(db_name),
        host_(ip),
        port_(port){
    Db::log->trace("Db: {} connected to {}:{}", db_name_.c_str(), host_.c_str(), port_);
}

Db::~Db(){}

const db::Connection Db::get_connection() const {
    db::Connection c = db::Driver::connect(host_, port_);
    return std::move(c);
}

bool Db::init() {
    bool init_db = true;

    try {
        auto connection = get_connection();

        db::Driver::Cursor databases = db::Driver::db_list().run(*connection);

        for (db::Driver::Datum const &db : databases) {
            if (*db.get_string() == db_name_) {
                init_db = false;
                break;
            }
        }

        if (init_db) {

            /// create table
            db::Driver::db_create(db_name_).run(*connection);

            for(auto name: table::get_names()) {
                query().table_create(name).run(*connection);
            }

            Db::log->info("Db: {} is initialized ...", db_name_.c_str());
        }

        for(auto desc: table::get_indices()){
            try {
                bool index_is_not_created = *db::Driver::db(db_name_)
                        .table(desc.table)
                        .index_wait(desc.field)
                        .is_empty().run(*connection)
                        .to_datum()
                        .get_boolean();

                Db::log->trace("Db: {} index status [{}] {}:{}", db_name_.c_str(),
                        !index_is_not_created,
                        desc.table, desc.field);

                if(index_is_not_created){
                    query().table(desc.table).index_create(desc.field).run(*connection);
                }

            } catch (db::Error &e) {
                Db::err->trace("Db: {} waiting indices error {}", db_name_.c_str(), e.message);
                query().table(desc.table).index_create(desc.field).run(*connection);
            }
        }

        if(!init_db) {
            try {
                last_block_id_ = get_last_block_id();
                Db::log->trace("Db: {} start at block-id: {}", db_name_.c_str(),  UInt256ToDecString(last_block_id_));

            } catch (db::Error &e) {
                Db::err->error("Db: {} error reading last block id {}", db_name_.c_str(), e.message);
                last_block_id_ = 0;
            }
        }
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error initializing {}", db_name_.c_str(), e.message);
        last_block_id_ = 0;
    }

    Db::log->info("Db: {} is opened ...", db_name_.c_str());

    return init_db;
}

uint256_t Db::get_last_block_id() const {
    try {
        auto connection = get_connection();

        auto cursor = query()
                .table(table::name::blockchain_state)
                .max(db::Driver::optargs("index", "block-id"))["block-id"]
                .run(*connection);

        return static_cast<uint256_t>((*cursor.to_datum().get_number()));

    } catch (db::Error &e) {
        Db::err->error("Db: {} error reading last block id {}", db_name_.c_str(), e.message);
    }

    return 0;
}

void Db::add_block(const db::Data &_block, uint256_t block_id) {

    try {

        std::string id = UInt256ToDecString(block_id);
        db::Data block = _block;

        {

            block["block-id"] = std::stoull(id);
            db::Table::Open(*this)->update(table::name::blocks, block);

            db::Data state;
            state["id"] = id;
            state["block-id"] = std::stoull(id);

            db::Table::Open(*this)->update(table::name::blockchain_state, state);
        }

        block_changes(block, block_id);

        Db::log->trace("Db: {} block-id: {} processed", db_name_.c_str(),id);
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error processing add block {}", db_name_.c_str(), e.message);
    }
    catch (std::exception &e) {
        Db::err->error("Db: {} error processing add block {}", db_name_.c_str(), e.what());
    }
}
