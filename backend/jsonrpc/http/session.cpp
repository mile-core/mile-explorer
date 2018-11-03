//
// Created by lotus mile on 02/11/2018.
//

#include "jsonrpc/http/session.hpp"

namespace milecsa::rpc::server::http {

    void Session::do_close() {
        // Send a TCP shutdown
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
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

    void Session::do_read() {
        // Read a request
        http::async_read(socket_, buffer_, req_,
                         boost::asio::bind_executor(
                                 strand_,
                                 std::bind(
                                         &Session::on_read,
                                         shared_from_this(),
                                         std::placeholders::_1,
                                         std::placeholders::_2)));
    }

    void Session::run() {
        do_read();
    }

    Session::send_lambda::send_lambda(Session &self) : self_(self) {
    }
}