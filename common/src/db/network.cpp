//
// Created by lotus mile on 01/11/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

void Db::add_node_states(const db::Data &nodes_state, uint256_t block_id){
    std::string id = UInt256ToDecString(block_id);

    Db::log->debug("Db: ... check node states for block-id: {}", id);

    db::Data states = {
            {"nodes", nodes_state},
            {"id", id},
    };

    db::Table::Open(*this, table::name::node_states)->insert(states);
    Db::log->trace("Db: ... node states: {} block id {}", states.dump(), id);

    for (auto &item: nodes_state) {
        std::string pid = item["public-key"];

        db::Data node = {
                {"address", item["address"]},
                {"node-id", item["node-id"]},
                {"timestamp", time(0)},
                {"block-id", id},
                {"id", pid}
        };

        db::Table::Open(*this, table::name::node_wallets)->update(node);

        Db::log->trace("Db: ... node wallet[{}]: {}", pid, node.dump());
    }

}
