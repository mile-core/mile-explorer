//
// Created by lotus mile on 29/10/2018.
//

#pragma once

#include <vector>
#include <milecsa.hpp>
#include <milecsa_jsonrpc.hpp>
#include <milecsa_queue.h>

#include "db.hpp"
#include "config.hpp"
#include "logger.hpp"

namespace milecsa::explorer {

    using namespace std;
    using namespace milecsa::rpc;
    typedef optional<Client> Client;

    struct Task: public Logger{
        Task(const std::string &name, size_t size);

        bool  is_running() const;
        const std::string &get_name() const;
        void async(const dispatch::function&);
        bool is_active() const {return is_active_;};

    private:
        dispatch::Queue queue_;
        std::string name_;
        bool is_active_;
    };

    class Fetcher: public Logger {
    public:

        /**
         * Create connected block fetcher
         *
         * @param urls - nodes publishes json-rpc API
         * @param update_timeout - connection time out
         * @param db - index db
         * @return optional fetcher
         */
        static optional<Fetcher> Connect(
                const std::vector<std::string> &urls = config::node_urls,
                int update_timeout = (int)config::update_timeout,
                const optional<Db> &db = std::nullopt);

        /**
         * Run fetching process
         *
         * @param block_id - start from bloc id
         */
        void run(uint256_t block_id);

        /**
         * Get current index db
         *
         * @return db connection
         */
         optional<Db> &get_db();

        optional<milecsa::rpc::Client> get_rpc();

    private:
        Fetcher(const std::vector<std::string> &urls,
                int read_timeout,
                const optional<Db> &db);

        std::vector<std::string> urls_;
        int                      update_timeout_;
        optional<Db>             db_;

        shared_ptr<Task> main_fetching_task_;
        shared_ptr<Task> block_fetcher_task_;
        shared_ptr<Task> block_processing_task_;
        shared_ptr<Task> rpc_fetching_block_task_;

        shared_ptr<Task> utility_task_;

        void fetch_blocks(uint256_t from, uint256_t to);

        void fetch_states();

        void fetch_genesis();
    };
}

