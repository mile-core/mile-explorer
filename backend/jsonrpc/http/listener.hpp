#pragma once

#include <boost/beast/http.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include "jsonrpc/http/session.hpp"

#include "jsonrpc/request.hpp"
#include "jsonrpc/router.hpp"

namespace milecsa::rpc::server::http {

    using tcp = boost::asio::ip::tcp;
    namespace http = boost::beast::http;

    class Listener final : public std::enable_shared_from_this<Listener> {
        tcp::acceptor       acceptor_;
        tcp::socket         socket_;
        rpc::server::Router rpc;

    public:
        Listener(
                boost::asio::io_context &ioc,
                tcp::endpoint           endpoint,
                rpc::server::Router     rpc
        );

        void run();

        void do_accept();

        void on_accept(boost::system::error_code ec);
    };
}