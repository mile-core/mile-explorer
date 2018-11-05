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

    db::Data states = {
            {"nodes", nodes_state},
            {"id", id},
    };

    db::Table::Open(*this, table::name::node_states)->update(states);
    Db::log->trace("Db: ... node states: {} block id {}", states.dump(), id);

    for (auto &item: nodes_state) {
        db::Data node = {
                {"address", item["address"]},
                {"node-id", item["node-id"]}
        };

        std::string id = item["public-key"];
        map<string,db::Data> ms;
        ms[id] = node;
        db::Table::Open(*this, table::name::node_wallets)->update(id, node);

        Db::log->trace("Db: ... node wallet[{}]: {}", id, node.dump());
    }

}
