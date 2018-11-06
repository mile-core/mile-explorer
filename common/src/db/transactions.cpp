//
// Created by lotus mile on 31/10/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

void Db::block_changes(const db::Data &block, uint256_t id) {

    try {

        db::Data trx = block.at("transactions");
        Db::log->trace("Db: get transactions {}... {} ", db_name_.c_str(), trx.dump());

        if (trx.is_array()) {
            add_transactions(trx, id);
        }
    }
    catch (db::Timeout &e) {
        Db::err->warn("Db: {} timeout get changes {}", db_name_.c_str(), e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Db: {} error processing get changes {}", db_name_.c_str(), e.message);
    }
}

inline void replace_keys(const string &key_from, const string &key_to,  db::Data &trx){
    if (trx.count(key_from)>0){
        trx[key_to] = trx[key_from];
        trx.erase(key_from);
    }
}

inline map<string,string> find_public_keys(const db::Data &trx) {

    vector<string> keys = {"from", "to", "public-key"};

    map<string,string> ret;

    for (auto address_name: keys) {

        if (trx.count(address_name) <= 0)
            continue;

        string address = trx[address_name].get<std::string>();
        string trx_id(address);

        trx_id.append(":");
        trx_id.append(trx["transaction-id"].get<std::string>());

        ret[address] = trx_id;
    }

    return ret;
}

void Db::add_stream_transaction(const db::Data &input_trx, uint256_t block_id){

    std::string id = UInt256ToDecString(block_id);

    db::Data trx = input_trx;

    for(const auto &entry: find_public_keys(trx)) {

        trx["id"] = entry.second;
        trx["block-id"] = std::stoull(id);

        if (trx["transaction-id"].is_string()){
            string trx_it = trx["transaction-id"];
            trx["transaction-id"] = std::stoull(trx_it);
        }

        for (const auto &[from, to]: table::get_replacement_keys()) {
            replace_keys(from, to, trx);
        }

        db::Table::Open(*this, table::name::transactions)->insert(trx);
        Db::log->trace("Processing: stream transactions {}", trx.dump());
    }
}

void Db::add_wallet_transaction(const db::Data &trx, uint256_t block_id){

    std::string id = UInt256ToDecString(block_id);
    auto _block_id = std::stoull(id);

    db::Data blocks;
    blocks.push_back(_block_id);

    for(const auto &entry: find_public_keys(trx)) {

        db::Data transaction_raw = {
                {"id",   entry.second},
                {"block-id",   _block_id},
                {"transaction-type", trx["transaction-name"]}
        };

        db::Data transactions_col;
        transactions_col.push_back(transaction_raw);

        std::map<string, db::Data> query = {
                {"blocks",       blocks},
                {"transactions", transactions_col}
        };

        db::Table::Open(*this, table::name::wallets)->update(entry.first, query);

        Db::log->trace("Processing: wallet transactions {}", transactions_col.dump());
    }
}

void Db::add_transactions(const db::Data &transactions, uint256_t block_id) {

    static bool first_time_update = true;

    auto udpate_state = [&](){

        uint64_t last_count = db::Table::Open(*this, table::name::transactions)
                ->cursor()
                .count()
                .get_data();

        db::Data state = {
                {"id", "state"},
                {"count", last_count}
        };

        db::Table::Open(*this, table::name::transactions_state)->update(state);

        Db::log->info("Processing: transactions_state are: {}", last_count);

        first_time_update = false;
    };

    if (first_time_update){
        udpate_state();
    }

    if(transactions.is_array()) {
        uint64_t count = 0 ;
        for ( auto trx: transactions ) {
            add_stream_transaction(trx, block_id);
            add_wallet_transaction(trx, block_id);
            count++;
        }

        udpate_state();

        Db::log->info("Processing: {} transactions are processed", count);
    }
}
