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
#include <milecsa_queue.h>

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

    /**
     * Common db interface
     */
    class Db: public Logger {

    public:

        friend class db::Table;
        friend class db::Cursor;

        typedef shared_ptr<db::Table> Table;

        /**
         * Open MILE indexed blockchain db
         * @param db_name
         * @param host
         * @param port
         * @return
         */
        static optional<Db> Open(
                const std::string &db_name = config::db_name,
                const std::string &host = config::db_host,
                int port = config::db_port);
        ~Db();

        bool is_exist();

        /**
         * First initialization process
         * @return is the first time db or not
         */
        bool init();

        /**
         * Chek table in db
         * @param name table name
         * @return false if table is not exist
         */
        bool has_table(const std::string &name);

        /**
         * Create table
         * @param name table name
         * @return true if table is created
         */
        Db::Table create_table(const std::string &name);

        /**
         * Open or create new table
         * @param name - table name
         * @return table object
         */
        Db::Table open_table(const std::string &name) const;

        void delete_table(const std::string &name);

        unsigned int get_version() const;
        void update_version(unsigned int version);

        /**
         * Db name
         * @return string
         */
        const string get_name() const {return db_name_;};

        /**
         * Process block
         * @param block - block data serialized in json
         * @param id - block id
         */
        void add_block(const db::Data &block, uint256_t id);

        /**
         *
         * Update blockchain info on indexer start
         *
         * @param blockchain_info
         */
        void update_info(const db::Data &blockchain_info);

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

        /**
         * Get the last block id indexed in DB
         * @return
         */
        uint256_t get_last_block_id() const;

        uint256_t get_last_processed_block_id() const;

        /**
         * Get current indexed network state
         * @return
         */
        db::Data get_network_state() const ;

        /**
         * Get current consensus nodes
         * @param first_id
         * @param limit
         * @return
         */
        db::Data get_nodes(uint64_t first_id, uint64_t limit) const ;

        db::Data get_blockchain_info() const ;

        db::Data get_block_history_state() const ;
        db::Data get_block_history(uint64_t first_id, uint64_t limit) const;
        db::Data get_block_by_id(uint256_t id) const;

        /**
         * Get count blocks
         * @param public_key
         * @return pair of count blocks contains wallet transactions and total wallet transactions count
         */
        std::pair<uint64_t,uint64_t> get_wallet_history_state(const string &public_key) const;
        db::Data get_wallet_history_blocks(const string &public_key, uint64_t first_id, uint64_t limit) const;
        db::Data get_wallet_history_transactions(const string &public_key, uint64_t first_id, uint64_t limit) const;
        db::Data get_wallet_node(const string &public_key) const;

        /**
         * Get transactions stream
         * @return
         */
        uint64_t get_transaction_history_state() const;
        db::Data get_transaction_history(uint64_t first_id, uint64_t limit) const;
        db::Data get_transaction_by_id(const string &id) const;


    protected:
        const db::Connection get_connection() const;

    private:
        Db():db_name_(""),host_(), port_(0), last_block_id_(0), exists_(false) {}
        Db(const std::string &db_name, const std::string &ip, unsigned short port, bool exists);

        void block_changes(const db::Data &block, uint256_t id);

        uint64_t add_stream_transaction(const db::Data &imput_trx, uint256_t block_id, db::Data &output_trx);//, uint64_t idx);
        void add_wallet_transaction(const db::Data &transactions, uint256_t block_id);

        db::Result query() const {return db::Driver::db(db_name_);};

        void transactions_processing();

        std::string    host_;
        unsigned short port_;
        std::string    db_name_;
        uint256_t last_block_id_;

        bool exists_;

        static dispatch::Queue transactions_queue_;
        static dispatch::Queue transactions_update_index_queue_;

    };
}
