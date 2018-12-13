//
// Created by lotus mile on 30/10/2018.
//
#pragma once

#include <string>
#include <vector>
#include "version.hpp"

namespace milecsa::explorer {

    using namespace std;

    struct config {

        ///
        /// Common
        ///

        /**
         * Rethink db host
         */
        static string         db_host;

        /**
         * Rethink db port
         */
        static unsigned short db_port;

        /**
        * Rethink db name
        */
        static string         db_name;


        ///
        /// Indexer
        ///

        /**
         * Scanning nodes
         */
        static vector<string> node_urls;

        /**
         * Json rpc task queue size, i.e. concurrency threads
         * to process requests
         */
        static int rpc_queue_size;

        /**
         * Block processing concurrency task
         */
        static int block_processin_queue_size;

        /**
         * Refresh time
         */
        static time_t update_timeout;  // sec.

        /**
         * Wait between request
         */
        static time_t request_timeout; // msec.

        static time_t rpc_connection_timeout; // sec.

        static time_t procesing_refresh_time;  // sec.

        ///
        /// Backend
        ///

        /**
         * Base http binding address and port
         */
        static string         http_bind_address;
        static unsigned short http_port;

        /**
         * Concurrency requests handling pool size
         */
        static int            rpc_pool_size;

        static std::string genesis_url;
    };

    template <typename T>
    class Singleton
    {
    public:
        static T& Instance()
        {
                static T instance;
                return instance;
        }

    protected:
        Singleton() {}
        ~Singleton() {}
    public:
        Singleton(Singleton const &) = delete;
        Singleton& operator=(Singleton const &) = delete;
    };
}

