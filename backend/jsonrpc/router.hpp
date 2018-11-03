#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/container/flat_map.hpp>
#include <set>

#include <milecsa.hpp>
#include "jsonrpc/context.hpp"
#include "db.hpp"

namespace milecsa::rpc::server {

    using method = std::function<void(context&, const std::optional<milecsa::explorer::Db>&)>;

    template<template<class...> class Container>
    class RpcBase final {
    public:

        using storage_type = Container<std::string,method>;

        RpcBase(const std::optional<milecsa::explorer::Db>& db,
                const std::string &version,
                const std::string &target,
                const milecsa::ErrorHandler &error_handler)
                :
                db(db),
                version(version),
                target(target),
                path("/"+version+"/"+target),
                error_handler(error_handler)
        {

        }

        ~RpcBase() = default;

        bool apply(const std::string& name, context& ctx){
            try {
                auto it = storage.find(name);
                if( it == storage.end() ){
                    return false;
                } else {
                    it->second(ctx,db);
                    return true;
                }
            }
            catch(nlohmann::json::parse_error& e) {
                error_handler(milecsa::result::EXCEPTION, ErrorFormat("rpc::Server: parse error: %s", e.what()));
                make_response_parse_error(ctx, e.what());
            }
            catch(nlohmann::json::invalid_iterator& e){
                error_handler(milecsa::result::EXCEPTION, ErrorFormat("rpc::Server: invalid iterator error: %s", e.what()));
                make_response_parse_error(ctx, e.what());
            } catch(nlohmann::json::type_error & e){
                error_handler(milecsa::result::EXCEPTION, ErrorFormat("rpc::Server: type error: %s", e.what()));
                make_response_parse_error(ctx, e.what());
            } catch(nlohmann::json::out_of_range& e){
                error_handler(milecsa::result::EXCEPTION, ErrorFormat("rpc::Server: out of range error: %s", e.what()));
                make_response_parse_error(ctx, e.what());
            } catch(nlohmann::json::other_error& e){
                error_handler(milecsa::result::EXCEPTION, ErrorFormat("rpc::Server: other error: %s", e.what()));
                make_response_parse_error(ctx, e.what());
            }
            catch (...) {
                error_handler(milecsa::result::EXCEPTION, ErrorFormat("rpc::Server: router method %s: unknown error", name.c_str()));
                ctx.response.result = nlohmann::json::array();
            }
            return false;
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
        std::optional<milecsa::explorer::Db> db;
        std::string  version;
        std::string  target;
        std::string  path;
        storage_type storage;
        milecsa::ErrorHandler error_handler;
    };

    using flat_map   = RpcBase<boost::container::flat_map>;
    using shared_rpc = std::shared_ptr<flat_map>;

    typedef shared_rpc Router;

    namespace router {
        static inline Router Create(
                const std::optional<milecsa::explorer::Db>& db,
                const std::string &version,
                const std::string &target,
                const milecsa::ErrorHandler &error_handler
        ) {
            return Router(new flat_map(db, version, target, error_handler));
        }
    }
}