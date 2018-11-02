//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include <boost/asio/ssl/stream.hpp>
#include <boost/program_options.hpp>

#include <milecsa_queue.h>
#include <optional>
#include <string>
#include "jsonrpc/http/listener.hpp"
#include "jsonrpc/http/session.hpp"
#include "jsonrpc/router.hpp"

#include "logger.hpp"
#include "config.hpp"

namespace milecsa::rpc {

    using namespace milecsa::explorer;
    using namespace std;

    class Server: public Logger {

        typedef shared_ptr<boost::asio::io_context> IoContext;
        typedef shared_ptr<server::http::Listener>  HttpListener;

    public:
        static optional<Server> Create(
                const optional<milecsa::explorer::Db>& db,
                const string bind_address,
                unsigned short port,
                int threads = config::rpc_pool_size,
                const string &version = "v1",
                const string &target  = "api",
                const milecsa::ErrorHandler& error_handler = Server::error_handler);

        const server::Router &get_router();
        void set_router(const server::Router &router) { router_ = std::move(router);};

        void run();

        ~Server(){}
        Server(const Server &);

    private:

        Server(
                const optional<milecsa::explorer::Db> &db,
                const boost::asio::ip::address &address,
                unsigned short port,
                const IoContext &ioc,
                const HttpListener &listener,
                const server::Router router,
                const milecsa::ErrorHandler& error_handler);

        boost::asio::ip::address address_;
        unsigned short port_;
        milecsa::ErrorHandler error_handler_;

        IoContext      ioc_;
        HttpListener   listener_;
        server::Router router_;
        optional<milecsa::explorer::Db> db_;
    };
}
