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

void Db::update_info(const milecsa::explorer::db::Data &_blockchain_info) {
    db::Data blockchain_info = _blockchain_info;
    auto &assets = blockchain_info.at("supported-assets");

    if (assets.is_array()) {
        for (uint i = 0; i < assets.size(); i++) {
            if (assets[i].count("code")) {
                auto code = assets[i]["code"].get<string>();
                if ("0" == code)
                    assets[i]["ticker-name"] = "XDR";
                else if ("1" == code)
                    assets[i]["ticker-name"] = "MILE";
            }
        }
    }

    db::Data info = {
            {"info", blockchain_info},
            {"id", blockchain_info["version"]}
    };

    open_table(table::name::blockchain_info)->update(info);
    Db::log->info("Db: {} blockchain info snapshot: ", db_name_, info.dump());
}

void Db::add_node_states(const db::Data &nodes_state, uint256_t block_id){
    std::string id = UInt256ToDecString(block_id);

    Db::log->debug("Db: {} check node states for block-id: {}", db_name_, id);

    db::Data states = {
            {"nodes", nodes_state},
            {"id", id},
    };

    open_table(table::name::node_states)->insert(states);

    Db::log->trace("Db: {} node states: {} block id {}", db_name_, states.dump(), id);

    for (auto &item: nodes_state) {
        std::string pid = item["public-key"];

        db::Data node = {
                {"address", item["address"]},
                {"node-id", item["node-id"]},
                {"timestamp", time(0)},
                {"block-id", id},
                {"id", pid}
        };

        open_table(table::name::node_wallets)->update(node);

        Db::log->trace("Db: {} node wallet[{}]: {}", db_name_, pid, node.dump());
    }

}
