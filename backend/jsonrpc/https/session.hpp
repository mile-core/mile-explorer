//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

#include <memory>
#include <boost/asio/ssl/stream.hpp>

#include "jsonrpc/request.hpp"
#include "jsonrpc/router.hpp"

namespace milecsa::rpc::server::https {

    using tcp = boost::asio::ip::tcp;
    namespace http = boost::beast::http;
    namespace ssl = boost::asio::ssl;

    class Session final : public std::enable_shared_from_this<Session> {
        // This is the C++11 equivalent of a generic lambda.
        // The function object is used to send an HTTP message.
        struct send_lambda {
            Session &self_;

            explicit send_lambda(Session &self) : self_(self) {
            }

            template<bool isRequest, class Body, class Fields>
            void
            operator()(http::message<isRequest, Body, Fields> &&msg) const {
                // The lifetime of the message has to extend
                // for the duration of the async operation so
                // we use a shared_ptr to manage it.
                auto
                        sp = std::make_shared<
                        http::message<isRequest, Body, Fields >>(std::move(msg));

                // Store a type-erased version of the shared
                // pointer in the class to keep it alive.
                self_.res_ = sp;

                // Write the response
                http::async_write(
                        self_.stream_,
                        *sp,
                        boost::asio::bind_executor(
                                self_.strand_,
                                std::bind(
                                        &Session::on_write,
                                        self_.shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        sp->need_eof())));
            }
        };

        tcp::socket socket_;
        ssl::stream<tcp::socket &> stream_;
        boost::asio::strand<boost::asio::io_context::executor_type> strand_;
        boost::beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        std::shared_ptr<void> res_;
        send_lambda lambda_;
        shared_rpc rpc;

    public:

        Session(
                tcp::socket socket,
                ssl::context &ctx,
                rpc::server::Router rpc
        );

        // Start the asynchronous operation
        void run();

        void on_handshake(boost::system::error_code ec);

        void do_read();

        void on_read(
                boost::system::error_code ec,
                std::size_t bytes_transferred);

        void on_write(
                boost::system::error_code ec,
                std::size_t bytes_transferred,
                bool close);

        void do_close();

        void on_shutdown(boost::system::error_code ec);
    };
}