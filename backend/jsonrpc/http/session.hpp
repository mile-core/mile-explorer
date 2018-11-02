#pragma once

#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

#include <memory>
#include <boost/beast/core/flat_buffer.hpp>

#include "request.hpp"
#include "rpc.hpp"

namespace milecsa::rpc::server::http {

    using tcp = boost::asio::ip::tcp;
    namespace http = boost::beast::http;

    class Session final : public std::enable_shared_from_this<Session> {

        struct send_lambda {
            Session &self_;

            explicit send_lambda(Session &self);

            template<bool isRequest, class Body, class Fields>
            void operator()(http::message<isRequest, Body, Fields> &&msg) const {

                auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

                self_.res_ = sp;

                http::async_write(
                        self_.socket_,
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
        boost::asio::strand<boost::asio::io_context::executor_type> strand_;
        boost::beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        send_lambda lambda_;
        std::shared_ptr<void> res_;
        rpc::server::Router rpc;

    public:
        Session(
                tcp::socket socket,
                rpc::server::Router rpc) :
                socket_(std::move(socket)),
                strand_(socket_.get_executor()),
                lambda_(*this),
                rpc(rpc){
        }

        void run();

        void do_read();

        void on_read(
                boost::system::error_code ec,
                std::size_t bytes_transferred);

        void on_write(
                boost::system::error_code ec,
                std::size_t bytes_transferred,
                bool close);

        void do_close();
    };
}