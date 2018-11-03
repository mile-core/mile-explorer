//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include <boost/beast/http.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "jsonrpc/https/session.hpp"
#include "jsonrpc/router.hpp"

namespace milecsa::rpc::server::https {

    using tcp = boost::asio::ip::tcp;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;

    class Listener final : public std::enable_shared_from_this<Listener> {
        ssl::context &ctx_;
        tcp::acceptor acceptor_;
        tcp::socket socket_;
        rpc::server::Router rpc;

    public:
        Listener(
                boost::asio::io_context &ioc,
                ssl::context &ctx,
                tcp::endpoint endpoint,
                rpc::server::Router rpc
        );

        // Start accepting incoming connections
        void
        run() {
            if (!acceptor_.is_open())
                return;
            do_accept();
        }

        void do_accept();

        void on_accept(boost::system::error_code ec);
    };
}