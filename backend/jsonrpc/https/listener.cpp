//
// Created by lotus mile on 02/11/2018.
//

#include "jsonrpc/https/listener.hpp"
#include "jsonrpc/request.hpp"

namespace milecsa::rpc::server::https {

    void Listener::do_accept() {
        acceptor_.async_accept(
                socket_,
                std::bind(
                        &Listener::on_accept,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void Listener::on_accept(boost::system::error_code ec) {
        if (ec) {
            fail(ec, "accept");
        } else {
            // Create the Session and run it
            std::make_shared<Session>(std::move(socket_), ctx_, rpc)->run();
        }

        // Accept another connection
        do_accept();
    }

    Listener::Listener(boost::asio::io_context &ioc,
                       ssl::context &ctx,
                       tcp::endpoint endpoint,
                       rpc::server::Router rpc) :
            ctx_(ctx),
            acceptor_(ioc),
            socket_(ioc),
            rpc(rpc) {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
                boost::asio::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }
    }
}

