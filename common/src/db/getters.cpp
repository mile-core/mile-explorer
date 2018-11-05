//
// Created by lotus mile on 02/11/2018.
//

#include "db.hpp"

#include <any>
#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

uint256_t Db::get_last_block_id() const {
    try {
        auto connection = get_connection();

        auto cursor = query()
                .table(table::name::blockchain_state)
                .max(db::Driver::optargs("index", "block-id"))["block-id"]
                .run(*connection);

        return static_cast<uint256_t>((*cursor.to_datum().get_number()));

    } catch (db::Error &e) {
        Db::err->error("Db: {} error reading last block id {}", db_name_.c_str(), e.message);
    }

    return 0;
}

db::Data Db::get_network_state() const {
    return db::Table::Open(*this, table::name::node_states)
            ->cursor()
            .max("id")
            .get_data();
}

db::Data Db::get_nodes(uint64_t first_id, uint64_t limit) const {

    return db::Table::Open(*this, table::name::node_states)
            ->cursor()
            .max("id")
            .field("nodes")
            .slice(first_id,limit)
            .get_data();
}

db::Data Db::get_wallet_node(const string &public_key) const {
    return db::Table::Open(*this, table::name::node_wallets)
            ->cursor()
            .get(public_key)
            .get_data();
}

db::Data Db::get_block_history_state() const {
    return db::Table::Open(*this, table::name::blockchain_state)
            ->cursor()
            .max("block-id")
            .get_data();
}

db::Data Db::get_block_history(uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this, table::name::blocks)
            ->cursor()
            .between(first_id,limit,"block-id")
            .get_data();
}

db::Data Db::get_block_by_id(uint256_t block_id) const {
    return db::Table::Open(*this, table::name::blocks)
            ->cursor()
            .get(UInt256ToDecString(block_id))
            .get_data();
}

std::pair<uint64_t,uint64_t> Db::get_wallet_history_state(const string &public_key) const {

    try {
        db::Data block = db::Table::Open(*this, table::name::wallets)
                ->cursor()
                .get(public_key)
                .field("blocks")
                .count()
                .get_data();

        db::Data trx = db::Table::Open(*this, table::name::wallets)
                ->cursor()
                .get(public_key)
                .field("transactions")
                .count()
                .get_data();

        return std::pair(block, trx);
    }
    catch (...) {
        return std::pair(0, 0);
    }

}

db::Data Db::get_wallet_history_blocks(const string &public_key, uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this, table::name::wallets)
            ->cursor()
            .get(public_key)
            .field("transactions")
            .sort("block-id")
            .slice(first_id, limit)
            .field("block-id")
            .get_data();
}

db::Data Db::get_wallet_history_transactions(const string &public_key, uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this, table::name::wallets)
            ->cursor()
            .get(public_key)
            .field("transactions")
            .slice(first_id, limit)
            .get_data();
}

uint64_t Db::get_transaction_history_state() const {
    return db::Table::Open(*this, table::name::transactions)
            ->cursor()
            .count()
            .get_data();
}

db::Data Db::get_transaction_history(uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this, table::name::transactions)
            ->cursor()
            .sort("block-id")
            .slice(first_id, limit)
            .get_data();
}

db::Data Db::get_transaction_by_id(const string &id) const {
    return db::Table::Open(*this, table::name::transactions)
            ->cursor()
            .get(id)
            .get_data();
}
