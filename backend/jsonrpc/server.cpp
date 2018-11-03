//
// Created by lotus mile on 02/11/2018.
//

#include "jsonrpc/server.hpp"
#include <milecsa.hpp>

using namespace milecsa::rpc;

optional<Server> Server::Create(
        const optional<milecsa::explorer::Db>& db,
        const string bind_address,
        unsigned short port,
        int threads,
        const string &version,
        const string &target,
        const milecsa::ErrorHandler &error_handler) {

    try {

        auto address = boost::asio::ip::make_address(bind_address);

        auto context = IoContext(new boost::asio::io_context(threads));

        auto router = server::router::Create(db, version, target, error_handler);

        auto listener = std::make_shared<server::http::Listener>(
                *context,
                boost::asio::ip::tcp::endpoint{address, port},
                router
        );

        return make_optional(Server(db,address,port,context,listener,router,error_handler));

    }
    catch (boost::system::system_error &e){
        error_handler(milecsa::result::FAIL, ErrorFormat("rpc::Server create : %s", e.what()));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return nullopt;
}

Server::Server(
        const optional<milecsa::explorer::Db>& db,
        const boost::asio::ip::address &address,
        unsigned short port,
        const Server::IoContext &ioc,
        const Server::HttpListener &listener,
        const server::Router router,
        const milecsa::ErrorHandler &error_handler):
        db_(db),
        address_(address),
        port_(port),
        ioc_(ioc),
        listener_(listener),
        router_(router),
        error_handler_(error_handler)
{

}

Server::Server(const Server &s):
        db_(s.db_),
        address_(s.address_),
        port_(s.port_),
        ioc_(s.ioc_),
        listener_(s.listener_),
        router_(s.router_),
        error_handler_(s.error_handler_)
{
}

void Server::run() {
    try {
        Logger::log->info("rpc::Server binding on: {}:{:d}...", address_.to_string(), port_);
        listener_->run();
        Logger::log->info("rpc::Server started on: {}:{:d}", address_.to_string(), port_);
        ioc_->run();
    }
    catch (boost::system::system_error &e){
        error_handler(milecsa::result::FAIL, ErrorFormat("rpc::Server: run %s", e.what()));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

const server::Router& Server::get_router() {
    return router_;
}