//
// Created by lotus mile on 02/11/2018.
//

#include "jsonrpc/https/session.hpp"

namespace milecsa::rpc::server::https {

    Session::Session(tcp::socket socket,
                     ssl::context &ctx,
                     rpc::server::Router rpc) :
            socket_(std::move(socket)),
            stream_(socket_, ctx),
            strand_(socket_.get_executor()),
            lambda_(*this),
            rpc(rpc) {
    }

    void Session::run() {
        // Perform the SSL handshake
        stream_.async_handshake(
                ssl::stream_base::server,
                boost::asio::bind_executor(
                        strand_,
                        std::bind(
                                &Session::on_handshake,
                                shared_from_this(),
                                std::placeholders::_1)));
    }

    void Session::do_read() {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Read a request
        http::async_read(stream_, buffer_, req_,
                         boost::asio::bind_executor(
                                 strand_,
                                 std::bind(
                                         &Session::on_read,
                                         shared_from_this(),
                                         std::placeholders::_1,
                                         std::placeholders::_2)));
    }

    void Session::on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return do_close();

        if (ec)
            return fail(ec, "read");

        // Send the response
        request::handle(std::move(req_), lambda_, rpc);
    }

    void Session::on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void Session::do_close() {
        // Perform the SSL shutdown
        stream_.async_shutdown(
                boost::asio::bind_executor(
                        strand_,
                        std::bind(
                                &Session::on_shutdown,
                                shared_from_this(),
                                std::placeholders::_1)));
    }

    void Session::on_shutdown(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }

    void Session::on_handshake(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "handshake");

        do_read();
    }
}
