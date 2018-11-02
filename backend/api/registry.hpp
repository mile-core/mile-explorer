//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include "jsonrpc/context.hpp"
#include "jsonrpc/router.hpp"
#include "db.hpp"

#include <optional>
#include <string>
#include <iostream>
#include <thread>

namespace milecsa::rpc::server {

    template <typename T>
    class Singleton
    {
    public:
        static T& Instance()
        {
            static T instance;
            return instance;
        }

    protected:
        Singleton() {}
        ~Singleton() {}
    public:
        Singleton(Singleton const &) = delete;
        Singleton& operator=(Singleton const &) = delete;
    };

    struct registry {

        void set_router(const server::Router &router);

        template <typename F>
        bool add(std::string name, F&&f){
            storage_.emplace(std::move(name), std::forward<F>(f));
            return true;
        }

    private:
        server::RpcBase<boost::container::flat_map>::storage_type storage_;
    };

    class Registry:public Singleton<registry>
    {
        friend class Singleton<registry>;
    private:
        Registry(){};
    };
}

using namespace std;
using namespace milecsa::rpc;
using Db = optional<milecsa::explorer::Db>;

#define MILECSA_JSONRPC_REGESTRY_METHOD(name,method) static auto v = server::Registry::Instance().add((name),(method))
