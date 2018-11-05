//
// Created by lotus mile on 05/11/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

void Db::add_block(const db::Data &_block, uint256_t block_id) {

    try {

        std::string id = UInt256ToDecString(block_id);
        db::Data block = _block;

        {
            block["block-id"] = std::stoull(id);
            db::Table::Open(*this, table::name::blocks)->update(block);

            db::Data state;
            state["id"] = id;
            state["block-id"] = std::stoull(id);

            db::Table::Open(*this, table::name::blockchain_state)->update(state);
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
