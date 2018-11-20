//
// Created by lotus mile on 31/10/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"
#include <milecsa.hpp>

using namespace milecsa::explorer;
using namespace std;

void Db::block_changes(const db::Data &block, uint256_t id, time_t t) {

    try {
        db::Data trx = block.at("transactions");
        Db::log->trace("Db: get transactions {}... {} ", db_name_.c_str(), trx.dump());

        add_transactions(trx, id, t);

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

inline map<string,string> find_public_keys(const db::Data &trx, std::string block_id) {

    vector<string> keys = {"from", "public-key"};

    map<string,string> ret;

    for (auto address_name: keys) {

        if (trx.count(address_name) <= 0)
            continue;

        string address = trx[address_name].get<std::string>();
        string trx_id(address);

        trx_id.append(":");
        trx_id.append(block_id);
        trx_id.append(":");
        trx_id.append(trx["transaction-id"].get<std::string>());

        ret[address] = trx_id;
    }

    return ret;
}

uint64_t Db::add_stream_transaction(const db::Data &input_trx,
        uint256_t block_id, time_t t,
        db::Data &output_trx) {

    std::string id = UInt256ToDecString(block_id);

    db::Data trx = input_trx;

    uint64_t count = 0;

    for(const auto &entry: find_public_keys(trx, id)) {

        std::string uniq_id = entry.second;

        db::Data row = db::Table::Open(*this, table::name::transactions_processing)
                ->cursor().get(id).get_data();

        if (row.count("id")>0) {
            Db::log->trace("Processing: stream transaction already is in table {}", row.dump());
            continue;
        }

        trx["id"] = uniq_id;
        trx["block-id"] = std::stoull(id);
        trx["timestamp"] = t;

        if (trx["transaction-id"].is_string()){
            trx["transaction-id"] = trx["transaction-id"].get<string>();
        }
        else if (trx["transaction-id"].is_number()) {
            trx["transaction-id"] = trx["transaction-id"].dump();
        }

        for (const auto &[from, to]: table::get_replacement_keys()) {
            replace_keys(from, to, trx);
        }

        if (trx.count("description")>0){
            std::string desc = trx["description"];
            size_t truncate_size = min(desc.length(),(size_t)milecsa::transaction::description_max_length);
            trx["description"] = desc.substr(0,truncate_size);
        }

        output_trx.push_back(trx);

        Db::log->trace("Processing: stream transactions {}", trx.dump());

        count ++;
    }

    return count;
}

void Db::add_wallet_transaction(const db::Data &trx, uint256_t block_id, time_t t){

    std::string id = UInt256ToDecString(block_id);
    auto _block_id = std::stoull(id);

    db::Data blocks;
    blocks.push_back(_block_id);

    for(const auto &entry: find_public_keys(trx,id)) {

        db::Data transactions_col;

        transactions_col.push_back({
                                           {"id",   entry.second},
                                           {"block-id",   _block_id},
                                           {"digest",     trx["digest"]},
                                           {"transaction-type", trx["transaction-type"]},
                                           {"timestamp", t}
                                   });

        std::map<string, db::Data> query = {
                {"blocks",       blocks},
                {"transactions", transactions_col}
        };

        db::Table::Open(*this, table::name::wallets)->update(entry.first, query);

        if (trx.count("to")>0){
            db::Table::Open(*this, table::name::wallets)->update(trx["to"], query);
        }
    }
}

void Db::add_transactions(const db::Data &transactions, uint256_t block_id, time_t t) {

    if(transactions.is_array()) {
        transactions_queue_.async([=] {
            uint64_t count = 0 ;
            for ( auto trx: transactions ) {
                add_wallet_transaction(trx, block_id, t);
                count++ ;
            }

            Db::log->info("Processing: {} wallet transactions are processed", count);
        });

        transactions_queue_.async([=] {

            uint64_t count = 0 ;

            db::Data stream;

            for ( auto trx: transactions ) {
                count += add_stream_transaction(trx, block_id, t, stream);
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
        stream["timestamp"] = t;

        db::Table::Open(*this, table::name::transactions_processing)->insert(stream);

        Db::log->trace("Processing: block-id: {} transactions is empty", id);
    }
}
