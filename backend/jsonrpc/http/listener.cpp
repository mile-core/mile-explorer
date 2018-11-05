//
// Created by lotus mile on 02/11/2018.
//

#include "jsonrpc/http/listener.hpp"

namespace milecsa::rpc::server::http {

    void Listener::do_accept() {
        acceptor_.async_accept(
                socket_,
                std::bind(
                        &Listener::on_accept,
                        shared_from_this(),
                        std::placeholders::_1));
    }


    void Listener::on_accept(boost::system::error_code ec) {
        if(ec) {
            fail(ec, "accept");
            exit(EXIT_FAILURE);
        } else {
            // Create the Session and run it
            std::make_shared<Session>(std::move(socket_), rpc)->run();
        }

        // Accept another connection
        do_accept();
    }

    void Listener::run() {
        if (!acceptor_.is_open())
            return;
        do_accept();
    }

    Listener::Listener(boost::asio::io_context &ioc,
            tcp::endpoint endpoint,
            rpc::server::Router rpc) :
            acceptor_(ioc),
            socket_(ioc),
            rpc(rpc) {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if(ec) {
            fail(ec, "listen");
            return;
        }
    }
}
