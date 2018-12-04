//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/chrono.hpp>

#include <iostream>

#include "jsonrpc/context.hpp"
#include "logger.hpp"

namespace milecsa::rpc::server {

    using namespace milecsa::explorer;

    inline auto fail(boost::system::error_code ec, char const *what) -> void {
        Logger::err->error("rpc::Server: error[{}] {} / {}", ec.value(), ec.message(), what);
        std::this_thread::sleep_for(std::chrono::milliseconds(config::request_timeout));
    }

    namespace request {

        namespace http = boost::beast::http;

        template<class T>
        void fill_cors(http::response<T> &res){
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods,
                    http::to_string(http::verb::post).to_string() +
                    ", " +
                    http::to_string(http::verb::options).to_string());
            res.set(http::field::access_control_allow_headers, "DNT,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Range");
            res.set(http::field::access_control_expose_headers, "Content-Length,Content-Range");
        }

        template<
                class Body,
                class Allocator,
                class Send,
                class RPC>
        auto handle(
                http::request<Body, http::basic_fields<Allocator>> &&req,
                Send &&send,
                RPC &&rpc
        ) -> void {

            auto tick = boost::posix_time::microsec_clock::local_time();

            std::string http_method = http::to_string(req.method()).to_string();
            auto path = req.target().to_string();

            if (path.length() > 0) {
                auto it = path.end() - 1;
                if (*(it) == '/')
                    path.erase(it);
            }

            if( req.target().empty()  || path != rpc->get_path()) {
                Logger::err->error("rpc::Server: path {} is forbidden", rpc->get_path());
                return send(std::move(http::response<http::string_body>{http::status::forbidden, req.version()}));
            }

            if (req.method() == http::verb::post) {

                http::response<http::string_body> res;
                res.version(req.version());
                fill_cors(res);

                context ctx;
                ctx.from_string(req.body());

                if (rpc->apply(ctx.request.method, ctx)) {

                    res.body() = ctx.to_json()+"\n";
                    res.prepare_payload();

                    auto diff = (float)
                                        (boost::posix_time::microsec_clock::local_time()
                                         - tick).total_milliseconds()/1000.0f;

                    Logger::log->debug("rpc::Server: {} <- \"{}\" params:{} {:03.4f}s.",
                            path,
                            ctx.request.method,
                            ctx.request.params.dump(),
                            diff);

                    return send(std::move(res));

                } else {

                    res.result(http::status::not_found);

                    if (!ctx.response.error) {
                        make_response_error_non_method(ctx);
                    }

                    res.body() = ctx.to_json();
                    res.prepare_payload();


                    Logger::err->error("rpc::Server: {} <- request error[{}] {} <- {}",
                            path, ctx.response.error ? ctx.response.error->code : -1,
                            ctx.response.error ? ctx.response.error->message : "",
                            req.body());

                    return send(std::move(res));
                }

            }
            else if (req.method() == http::verb::options) {
                http::response<http::string_body> res;
                res.version(req.version());
                fill_cors(res);
                return send(std::move(res));
            }

            Logger::err->error("rpc::Server: {} HTTP method {} not allowed ", path, http_method);

            return send(std::move( http::response<http::empty_body> {http::status::method_not_allowed,req.version()}));
        }
    }
}