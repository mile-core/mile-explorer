//
// Created by lotus mile on 29/10/2018.
//

#include <any>
#include <milecsa_queue.h>
#include <milecsa.hpp>

#include "db.hpp"
#include "table.hpp"
#include "names.hpp"
#include "statistics.hpp"

namespace milecsa::explorer::db {
    void migration(optional<Db> db);
}

using namespace milecsa::explorer;
using namespace std;

dispatch::Queue Db::common_processing_queue_ = dispatch::Queue(config::block_processin_queue_size);

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

unsigned int Db::get_version() const {
    try {
        return (unsigned int)open_table(table::name::meta)->cursor()
                .max("id")
                .field("version")
                .get_number();
    }
    catch (db::Error & e)
    {
        Db::err->error("Db: {} get_version error {}", db_name_.c_str(), e.message);
    }
    return 0;
}

void Db::update_version(unsigned int version) {
    try {
        unsigned int restart_number = 0;
        try {
            restart_number = (unsigned int)open_table(table::name::meta)->cursor()
                    .max("id")
                    .field("id")
                    .get_number();
            ++restart_number;
        }
        catch (db::Error & e)
        {
            Db::err->error("Db: {} get_version error {}", db_name_.c_str(), e.message);
        }

        unsigned int previous_version = get_version();

        Db::log->info("Db: {} restarts: {}, new version {}, previuos; {}",
                db_name_.c_str(), restart_number, version, previous_version);

        db::Data meta = {
                {"id",               restart_number},
                {"version",          version},
                {"previous_version", previous_version},
                {"timestamp", time(0)}
        };
        open_table(table::name::meta)->insert(meta);
    }
    catch (db::Error & e)
    {
        Db::err->error("Db: {} update_version error {}", db_name_.c_str(), e.message);
    }
}

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
        return open_table(name);
    }
    catch (db::Error &e) {
        return open_table(name);
    }
}

void Db::delete_table(const std::string &name) {
    try {
        auto connection = get_connection();
        query().table_drop(name).run(*connection);
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} delete_table {} error {}", db_name_.c_str(), name, e.message);
    }
}

Db::Table Db::open_table(const std::string &name) const {
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

        db::migration(*this);

        Db::log->info("Db: {} is opened ...", db_name_.c_str());

        blocks_processing();

        transactions_processing();

        common_processing_queue_.async([&]{
            while (common_processing_queue_.is_running()) {
                std::this_thread::sleep_for(std::chrono::seconds(config::update_timeout));

                //
                // process statistics

                auto state  = get_block_history_state();
                time_t last = state["timestamp"];

                for (auto &[name, method]: statistic::Registry::Instance().get_statistics()){
                    try {

                        method(*this, last);
                        Logger::log->info("Statistic: {} processing done", name);
                    }
                    catch (db::Error &e) {
                        Logger::err->error("Statistic: {} processing error {}", name, e.message);
                    }
                    catch(nlohmann::json::parse_error& e) {
                        Logger::err->error("Statistic: {} parse json error {}", name, e.what());
                    }
                    catch(nlohmann::json::invalid_iterator& e){
                        Logger::err->error("Statistic: {} invalid iterator error {}", name, e.what());
                    } catch(nlohmann::json::type_error & e){
                        Logger::err->error("Statistic: {} type error {}", name, e.what());
                    } catch(nlohmann::json::out_of_range& e){
                        Logger::err->error("Statistic: {} out of range error {}", name, e.what());
                    } catch(nlohmann::json::other_error& e){
                        Logger::err->error("Statistic: {} other error {}", name, e.what());
                    }
                    catch (...) {
                        Logger::err->error("Statistic: {} turnovers processing unknown error ... ", name);
                    }
                }
            }
        });

    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error initializing {}", db_name_.c_str(), e.message);
    }

    return init_db;
}
