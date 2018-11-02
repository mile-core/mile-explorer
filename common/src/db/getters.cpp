//
// Created by lotus mile on 02/11/2018.
//

#include "db.hpp"

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "tables.hpp"

using namespace milecsa::explorer;
using namespace std;

db::Data Db::get_network_state() const {
    return db::Table::Open(*this)->get_state(table::name::nodes_state);
}

db::Data Db::get_nodes(uint64_t first_id, uint64_t limit) const {

    auto connection = get_connection();
    db::Driver::Term q = query();

    auto result = q
            .table("nodes_state")
            .max(db::Driver::optargs("index", "id"))["nodes"]
            .skip(first_id)
            .limit(limit).run(*connection);

    return nlohmann::json::parse(result.to_datum().as_json());
}

db::Data Db::get_block_history_state() const {
    return db::Table::Open(*this)->get_state(table::name::blockchain_state, "block-id");
}

db::Data Db::get_block_history(uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this)->get_range(table::name::blocks, first_id, limit, "block-id");
}

db::Data Db::get_block(uint256_t block_id) const {
    std::string id = UInt256ToDecString(block_id);
    return db::Table::Open(*this)->get_by_id(table::name::blocks, id);
}
