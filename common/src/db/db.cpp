//
// Created by lotus mile on 29/10/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"
#include <milecsa.hpp>

namespace milecsa::explorer::db {
    void migration(optional<Db> db);
}

using namespace milecsa::explorer;
using namespace std;

dispatch::Queue Db::transactions_update_index_queue_ = dispatch::Queue(1);
dispatch::Queue Db::transactions_queue_ = dispatch::Queue(config::block_processin_queue_size);
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
        return db::Table::Open(*this, name);
    }
    catch (db::Error &e) {
        return db::Table::Open(*this, name);
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

        transactions_processing();

        //
        // TODO: create common processing registry
        //
        common_processing_queue_.async([&]{
            while (common_processing_queue_.is_running()) {
                std::this_thread::sleep_for(std::chrono::seconds(config::update_timeout));

                //
                // 1. turnovers processing
                //
                turnovers_processing();
            }
        });

    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error initializing {}", db_name_.c_str(), e.message);
    }

    return init_db;
}

void Db::turnovers_processing() {

    try {

        auto state = get_block_history_state();
        time_t last = state["timestamp"];
        time_t first = last - 86400;

        db::Data items = open_table(table::name::blocks)
                ->cursor()
                .between(first, last, "timestamp")
                .field("transactions")
                .get_data();

        Db::log->info("Db: {} turnovers processing items: {}", db_name_.c_str(), items.size());

        long double xdr  = 0;
        long double mile = 0;
        uint64_t count = 0;
        uint64_t count_xdr = 0;
        uint64_t count_mile = 0;
        for (auto &item: items) {
            if (item.is_array()) {
                for (auto &trx: item) {
                    if (trx.count("asset")){
                        for(auto &asset: trx["asset"]){
                            std::string code = asset["code"];
                            std::string amount = asset["amount"];
                            unsigned short asset_code = (unsigned short )std::stoi(code);
                            if (asset_code == milecsa::assets::XDR.code){
                                xdr +=  std::stold(amount);
                                count_xdr ++;
                            }
                            else if (asset_code == milecsa::assets::MILE.code){
                                mile += std::stold(amount);
                                count_mile ++;
                            }
                            count++;
                        }
                    }
                }
            }
        }

        db::Data trnv = {
                {"id", "turnovers-24"},
                {"count", count},
                {"assets", {
                                   {{"code",std::to_string(milecsa::assets::XDR.code)},{"amount",xdr},{"count",count_xdr}},
                                   {{"code",std::to_string(milecsa::assets::MILE.code)},{"amount",mile},{"count",count_mile}},
                }}
        };
        open_table(table::name::turnovers)->update(trnv);

        Db::log->trace("Db: {} turnovers processing done: {}", db_name_.c_str(), trnv.dump());
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} turnovers processing error {}", db_name_.c_str(), e.message);
    }
    catch (...) {
        Db::err->error("Db: turnovers processing unknown error ... ");
    }
}

void Db::transactions_processing() {

    transactions_update_index_queue_.async([&]{

        dispatch::Queue update_stream_q(8);

        uint64_t prev_bid = 0;

        while (transactions_update_index_queue_.is_running()) {

            try{

                auto update_state = [=](Db *db, uint64_t count, uint64_t block_id){
                    db::Data state = {
                            {"id", "state"},
                            {"count", count},
                            {"block-id", block_id}
                    };
                    open_table(table::name::transactions_state)->update(state);
                };

                db::Data items = open_table(table::name::transactions_processing)
                        ->cursor()
                        .sort("block-id")
                        .get_data();

                uint64_t last_count = 0 ;

                try {
                    last_count = get_transaction_history_state();
                }
                catch(...){
                    update_state(this, 0, 0);
                }

                uint64_t count = 0;

                Db::log->info("Db: {} transactions processing queue reading items: {}",
                        db_name_.c_str(), items.size());

                for (auto &item: items) {

                    uint64_t    bid   = item["block-id"];
                    std::string trxid = item["id"];

                    if (prev_bid>0) {

                        if (item.at("transaction-type") == "__processing__") {
                            Db::log->trace("Processing: transaction {} processing type, will be skipped", trxid);
                            prev_bid = bid;
                            open_table(table::name::transactions_processing)->cursor().remove(trxid);
                            continue;
                        }

                        if ((bid-prev_bid)>1) {
                            Db::log->debug("Processing: transaction {} is in a forward block {} while current is {}", trxid, bid, prev_bid);
                            prev_bid = bid;
                            break;
                        }

                    }
                    else if (item.at("transaction-type") == "__processing__") {
                        prev_bid = bid;
                        open_table(table::name::transactions_processing)->cursor().remove(trxid);
                        continue;
                    }

                    prev_bid = bid;

                    db::Data row = db::Table::Open(*this, table::name::transactions)
                            ->cursor().get(trxid).get_data();

                    if (row.count("id")>0) {
                        Db::log->trace("Processing: transaction {} already is in table {}", trxid, row.dump());
                        open_table(table::name::transactions_processing)->cursor().remove(trxid);
                        continue;
                    }

                    item["serial"] = last_count++;

                    open_table(table::name::transactions)->insert(item);

                    update_state(this, last_count, bid);

                    open_table(table::name::transactions_processing)->cursor().remove(trxid);

                    Db::log->debug("Db: {} transaction serial number updated: {}, block-id: {}",
                                   db_name_.c_str(), last_count, bid);

                    count++;
                }
                if (count>0)
                    Db::log->info("Db: {} transactions are updated: {}", db_name_.c_str(), count);
            }
            catch (db::Error &e) {
                Db::err->error("Db: {} error transactions update index {}", db_name_.c_str(), e.message);
            }
            catch (...) {
                Db::err->error("Db: transactions updating index unknown error ... ");
            }

            std::this_thread::sleep_for(std::chrono::seconds(config::update_timeout));
        }
    });
}
