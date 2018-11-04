//
// Created by lotus mile on 29/10/2018.
//

#pragma once

#include <optional>
#include <any>
#include <string>
#include <vector>

#include <stdlib.h>

#include <milecsa.hpp>
#include <milecsa_jsonrpc.hpp>

#include <rethinkdb.h>

#include "logger.hpp"
#include "config.hpp"

namespace milecsa::explorer{

    using namespace std;

    class Db;

    namespace db {
        namespace Driver = RethinkDB;
        using   Args = RethinkDB::OptArgs;
        typedef Driver::Error Error;
        typedef RethinkDB::TimeoutException Timeout;
        typedef std::unique_ptr<Driver::Connection> Connection;
        typedef Driver::Term Result;
        typedef nlohmann::json Data;
        class Table;
        class Cursor;
    }

    class Db: public Logger {

    public:

        friend class db::Table;
        friend class db::Cursor;

        static optional<Db> Open(
                const std::string &db_name = config::db_name,
                const std::string &host = config::db_host,
                int port = config::db_port);
        ~Db();

        /**
         * First initializetion process
         * @return is the first time db or not
         */
        bool init();

        /**
         * Db name
         * @return string
         */
        const string get_name() const {return db_name_;};

        db::Result query() const {return db::Driver::db(db_name_);};

        /**
         * Get the last block id indexed in DB
         * @return block id
         */
        uint256_t get_start_block_id() const { return last_block_id_; }

        /**
         * Get the current last approved block in blockchain
         * @return
         */
        uint256_t get_last_block_id() const;

        /**
         * Process block
         * @param block - block data serialized in json
         * @param id - block id
         */
        void add_block(const db::Data &block, uint256_t id);

        /**
         * Add transactions from block
         * @param transactions - transactions array
         * @param block_id - block id
         */
        void add_transactions(const db::Data &transactions, uint256_t block_id);

        /**
         * Update current network state
         *
         * @param nodes_state
         * @param block_id - ignoring at this version
         */
        void add_node_states(const db::Data &nodes_state, uint256_t block_id);

        db::Data get_network_state() const ;
        db::Data get_nodes(uint64_t first_id, uint64_t limit) const ;

        db::Data get_block_history_state() const ;
        db::Data get_block_history(uint64_t first_id, uint64_t limit) const;
        db::Data get_block_by_id(uint256_t id) const;

        std::pair<uint64_t,uint64_t> get_wallet_history_state(const string &public_key) const;
        db::Data get_wallet_history_blocks(const string &public_key, uint64_t first_id, uint64_t limit) const;
        db::Data get_wallet_history_transactions(const string &public_key, uint64_t first_id, uint64_t limit) const;
        db::Data get_wallet_node(const string &public_key) const;

        uint64_t get_transaction_history_state() const;
        db::Data get_transaction_history(uint64_t first_id, uint64_t limit) const;
        db::Data get_transaction_by_id(const string &id) const;

    protected:
        const db::Connection get_connection() const;

    private:
        Db():db_name_(""),host_(), port_(0), last_block_id_(0) {}
        Db(const std::string &db_name, const std::string &ip, unsigned short port);

        void block_changes(const db::Data &block, uint256_t id);
        void add_stream_transaction(const db::Data &transactions, uint256_t block_id);
        void add_wallet_transaction(const db::Data &transactions, uint256_t block_id);

        std::string    host_;
        unsigned short port_;
        std::string    db_name_;
        uint256_t last_block_id_;
    };
}
