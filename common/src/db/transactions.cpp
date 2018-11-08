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

        add_transactions(trx, id);

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

uint64_t Db::add_stream_transaction(const db::Data &input_trx, uint256_t block_id, db::Data &output_trx) { //, uint64_t idx){

    std::string id = UInt256ToDecString(block_id);

    db::Data trx = input_trx;

    uint64_t count = 0;

    for(const auto &entry: find_public_keys(trx)) {

        std::string pk = entry.second;

        db::Data row = db::Table::Open(*this, table::name::transactions_processing)
                ->cursor().get(id).get_data();

        if (row.count("id")>0) {
            Db::log->trace("Processing: stream transaction already is in table {}", row.dump());
            continue;
        }

        trx["id"] = pk;
        trx["block-id"] = std::stoull(id);

        if (trx["transaction-id"].is_string()){
            string trx_it = trx["transaction-id"];
            trx["transaction-id"] = std::stoull(trx_it);
        }

        for (const auto &[from, to]: table::get_replacement_keys()) {
            replace_keys(from, to, trx);
        }

        output_trx.push_back(trx);

        Db::log->trace("Processing: stream transactions {}", trx.dump());

        count ++;
    }

    return count;
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

    if(transactions.is_array()) {
        transactions_queue_.async([=] {

            uint64_t count = 0 ;

            db::Data stream;

            for ( auto trx: transactions ) {
                count += add_stream_transaction(trx, block_id, stream);
                add_wallet_transaction(trx, block_id);
            }

            db::Table::Open(*this, table::name::transactions_processing)->insert(stream);

            Db::log->info("Processing: {} transactions are processed, block-id: {}", count, UInt256ToDecString(block_id));
        });
    }
    else {

        db::Data stream;
        std::string id = UInt256ToDecString(block_id);
        auto _block_id = std::stoull(id);

        stream["id"] = id;
        stream["block-id"] = std::stoull(id);
        stream["transaction-type"] = "__processing__";

        db::Table::Open(*this, table::name::transactions_processing)->insert(stream);

        Db::log->trace("Processing: block-id: {} is empty", id);
    }
}
