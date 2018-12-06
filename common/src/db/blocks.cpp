//
// Created by lotus mile on 05/11/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"
#include "utils.hpp"

using namespace milecsa::explorer;
using namespace std;

void Db::add_block(const db::Data &_block, uint256_t block_id) {

    try {

        std::string id = UInt256ToDecString(block_id);
        db::Data block = _block;

        {
            db::Data row = open_table(table::name::blocks_processing)
                    ->cursor().get(id).get_data();

            if (row.count("id")>0){
                Db::log->trace("Processing: block {} already is in table",  id);
                return;
            }

            block["block-id"]  = std::stoull(id);
            std::time_t tm = std::time(nullptr);
            if (block.count("timestamp"))
                block["timestamp"] = getEpochTime(block["timestamp"]);
            else
                block["timestamp"] = tm;

            block["index-timestamp"] = tm;

            open_table(table::name::blocks_processing)->insert(block);
        }
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error processing add block {}", db_name_.c_str(), e.message);
    }
    catch (std::exception &e) {
        Db::err->error("Db: {} error processing add block {}", db_name_.c_str(), e.what());
    }
}

void Db::block_changes(const db::Data &block, uint256_t id, time_t t) {

    try {
        db::Data trx = block.at("transactions");
        Db::log->trace("Db: get transactions {}... {} ", db_name_.c_str(), trx.dump());

        add_transactions(trx, id, t);

    }
    catch (db::Timeout &e) {
        Db::err->warn("Db: {} timeout get changes {}", db_name_.c_str(), e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error processing get changes {}", db_name_.c_str(), e.message);
    }
}
