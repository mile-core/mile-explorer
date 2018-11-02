#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/container/flat_map.hpp>
#include <set>

#include "jsonrpc/context.hpp"
#include "db.hpp"

namespace milecsa::rpc::server {

    using method = std::function<void(context&, const std::optional<milecsa::explorer::Db>&)>;

    template<template<class...> class Container>
    class RpcBase final {
    public:

        using storage_type = Container<std::string,method>;

        RpcBase(const std::optional<milecsa::explorer::Db>& db,
                const std::string version,
                const std::string target)
                :
                db(db),
                version(version),
                target(target),
                path("/"+version+"/"+target)
        {

        }

        ~RpcBase() = default;

        bool apply(const std::string&name, context& ctx){
            auto it = storage.find(name);
            if( it == storage.end() ){
                return false;
            } else {
                it->second(ctx,db);
                return true;
            }
        }

        template <typename F>
        void add(std::string name, F&&f){
            storage.emplace(std::move(name), std::forward<F>(f));
        }

        std::set<std::string> keys() const {
            std::set<std::string> tmp;
            for(auto&i:storage){
                tmp.emplace(i.first);
            }
            return  tmp;
        }

        const std::string &get_version() const { return version; }
        const std::string &get_target() const { return target; }
        const std::string &get_path() const { return path; }

    private:
        std::string  version;
        std::string  target;
        std::string  path;
        storage_type storage;
        std::optional<milecsa::explorer::Db> db;
    };

    using flat_map   = RpcBase<boost::container::flat_map>;
    using shared_rpc = std::shared_ptr<flat_map>;

    typedef shared_rpc Router;

    namespace router {
        static inline Router Create(
                const std::optional<milecsa::explorer::Db>& db,
                const std::string version,
                const std::string target
        ) {
            return Router(new flat_map(db, version,target));
        }
    }
}