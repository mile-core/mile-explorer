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
    return db::Table::Open(*this)->get_state(table::name::node_states);
}

db::Data Db::get_nodes(uint64_t first_id, uint64_t limit) const {

    auto connection = get_connection();
    db::Driver::Term q = query();

    auto result = q
            .table(table::name::node_states)
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

db::Data Db::get_block_by_id(uint256_t block_id) const {
    std::string id = UInt256ToDecString(block_id);
    return db::Table::Open(*this)->get_by_id(table::name::blocks, id);

}

db::Data Db::get_wallet_node(const string &public_key) const {
    return db::Table::Open(*this)->get_by_id(table::name::node_wallets, public_key);
}

std::pair<uint64_t,uint64_t> Db::get_wallet_history_state(const string &public_key) const {

    try {
        db::Data block = db::Table::Open(*this)->get_count(table::name::wallets, public_key, "blocks");
        db::Data trx = db::Table::Open(*this)->get_count(table::name::wallets, public_key, "transactions");
        return std::pair(block, trx);
    }
    catch (...) {
        return std::pair(0, 0);
    }

}

db::Data Db::get_wallet_history_blocks(const string &public_key, uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this)->get_slice(
            table::name::wallets,
            public_key,
            "transactions",
            first_id, limit,
            "block-id", "block-id");
}

db::Data Db::get_wallet_history_transactions(const string &public_key, uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this)->get_slice(table::name::wallets, public_key, "transactions", first_id, limit);
}

uint64_t Db::get_transaction_history_state() const {
    return db::Table::Open(*this)->get_count(table::name::transactions);
}

db::Data Db::get_transaction_history(uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this)->get_slice(
            table::name::transactions,
            "",
            "",
            first_id, limit,
            "block-id", "");
}

db::Data Db::get_transaction_by_id(const string &id) const {
    return db::Table::Open(*this)->get_by_id(table::name::transactions, id);
}
