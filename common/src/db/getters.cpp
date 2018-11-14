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

uint256_t Db::get_last_processed_block_id() const {
    try {

        auto id =  db::Table::Open(*this, table::name::transactions_state)
                ->cursor()
                .get("state")
                .field("block-id")
                .get_number();

        return static_cast<uint256_t>(id);

    } catch (db::Error &e) {
        Db::err->error("Db: {} error reading last processed block id {}", db_name_.c_str(), e.message);
    }

    return 0;
}

uint256_t Db::get_last_block_id() const {

    uint256_t processing_block_id = 0;

    try {
        auto id =  db::Table::Open(*this, table::name::transactions_processing)
                ->cursor()
                .max("block-id")
                .field("block-id")
                .get_number();

        processing_block_id = static_cast<uint256_t>(id);

    } catch (db::Error &e) {
        Db::err->trace("Db: {} error reading processing block id from transactions_processing {}", db_name_.c_str(), e.message);
    }

    uint256_t last_block_id = 0;

    try {
        auto id =  db::Table::Open(*this, table::name::blocks)
                ->cursor()
                .max("block-id")
                .field("block-id")
                .get_number();

        last_block_id = static_cast<uint256_t>(id);

    } catch (db::Error &e) {
        Db::err->trace("Db: {} error reading last block id from blocks {}", db_name_.c_str(), e.message);
    }

    uint256_t state_block_id = get_last_processed_block_id();

    Db::log->debug("Db: {} state block-id: {}, processing block-id: {}, last block-id: {}",
                   db_name_.c_str(),
                   UInt256ToDecString(state_block_id),
                   UInt256ToDecString(processing_block_id),
                   UInt256ToDecString(last_block_id)
    );

    if (processing_block_id == 0) {
        return last_block_id;
    }

    return state_block_id > processing_block_id ? state_block_id : processing_block_id;
}

db::Data Db::get_network_state() const {
    return db::Table::Open(*this, table::name::node_states)
            ->cursor()
            .max("id")
            .get_data();
}

db::Data Db::get_blockchain_info() const {
    return db::Table::Open(*this, table::name::blockchain_info)
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
    return db::Table::Open(*this, table::name::blocks)
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
            .sort_field("block-id")
            .field("block-id")
            .slice(first_id, limit)
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
    return db::Table::Open(*this, table::name::transactions_state)
            ->cursor()
            .get("state")
            .field("count")
            .get_data();
}

db::Data Db::get_transaction_history(uint64_t first_id, uint64_t limit) const {
    return db::Table::Open(*this, table::name::transactions)
            ->cursor()
            .between(first_id, limit, "serial")
            .get_data();
}

db::Data Db::get_transaction_by_id(const string &id) const {
    return db::Table::Open(*this, table::name::transactions)
            ->cursor()
            .get(id)
            .get_data();
}

db::Data Db::get_turnovers_24() const {
    return db::Table::Open(*this, table::name::turnovers)
            ->cursor()
            .get("turnovers-24")
            .get_data();
}