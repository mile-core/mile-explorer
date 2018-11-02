//
// Created by lotus mile on 01/11/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "tables.hpp"

using namespace milecsa::explorer;
using namespace std;

void Db::add_nodes_state(const db::Data &nodes_state, uint256_t block_id){
    std::string id = UInt256ToDecString(block_id);

    db::Data query = {
            {"nodes", nodes_state},
            {"id", id},
    };

    db::Table::Open(*this)->update(table::name::nodes_state, query);

    Db::log->trace("Db: ... nodes: {} block id {}", query.dump(), id);
}
