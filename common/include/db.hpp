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
        typedef Driver::Error Error;
        typedef RethinkDB::TimeoutException Timeout;
        typedef std::unique_ptr<Driver::Connection> Connection;
        typedef Driver::Term Query;
        typedef nlohmann::json Data;
        class Table;
    }

    class Db: public Logger {

    public:

        friend class db::Table;

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

        db::Query query() const {return db::Driver::db(db_name_);};

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
        void add_nodes_state(const db::Data &nodes_state, uint256_t block_id);

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

    namespace db{
        class Table{
        public:
            static const shared_ptr<Table> Open(const optional<Db> &db){
                return shared_ptr<Table>(new Table(db));
            }

            Table(const optional<Db> &db):db_(db){};

            void update(
                    const string &table_name,
                    const db::Data &data);

            void update(
                    const string &table_name,
                    const string id,
                    const std::map<string,db::Data> &data);

        private:
            optional<Db> db_;
        };
    }
}
