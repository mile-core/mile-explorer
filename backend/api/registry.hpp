//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include "jsonrpc/context.hpp"
#include "jsonrpc/router.hpp"
#include "api/params.hpp"
#include "db.hpp"
#include "config.hpp"

#include <milecsa.hpp>

#include <optional>
#include <string>
#include <iostream>
#include <thread>

namespace milecsa::rpc::server {

    struct registry {

        void set_router(const server::Router &router);
        void set_error_handler(milecsa::ErrorHandler& error_handler);

        void error(const std::string &error) {
            if(error_handler_){
                (*error_handler_)(milecsa::result::EXCEPTION, error);
            }
        }

        template <typename F>
        bool add(std::string name, F&&f, std::string params){
            storage_.emplace(name, std::forward<F>(f));
            try {
                params_.emplace(name, params);
            }
            catch (nlohmann::json::parse_error &e){
                if(error_handler_) {
                    std::cerr << "rpc::Regestry error: " <<  e.what() << " in method: " << name << std::endl;
                }
            }
            return true;
        }

        const explorer::db::Data &get_help() const { return params_; }

    private:
        explorer::db::Data params_;
        server::RpcBase<boost::container::flat_map>::storage_type storage_;
        static std::optional<milecsa::ErrorHandler> error_handler_;
    };

    class Registry:public milecsa::explorer::Singleton<registry>
    {
        friend class milecsa::explorer::Singleton<registry>;
    private:
        Registry(){};
    };
}

using namespace std;
using namespace milecsa::rpc;
using ctxDb = std::optional<milecsa::explorer::Db>;

#define MILECSA_JSONRPC_REGESTRY_METHOD(name, method, params) static auto v = milecsa::rpc::server::Registry::Instance().add((name),(method),(params))
