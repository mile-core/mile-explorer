//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include "jsonrpc/message.hpp"

namespace milecsa::rpc::server {

    struct context final {

        context() = default;
        ~context() = default;

        auto from_string(const std::string &name) -> void;

        auto to_json() const -> std::string;

        message::request_message request;
        message::response_message response;
    };

    inline auto make_response_parse_error(context &ctx, const std::string &_error = "") -> void {
        if (_error.empty())
            ctx.response.error = message::response_error(message::response_error::error_code::ParseError, "ParseError");
        else
            ctx.response.error = message::response_error(message::response_error::error_code::ParseError, _error);
    }

    inline auto make_response_internal_error(context &ctx) -> void {
        ctx.response.error =  message::response_error(message::response_error::error_code::InternalError, "InternalError");
    }

    inline auto make_response_error_non_method(context &ctx) -> void {
        ctx.response.error = message::response_error(message::response_error::error_code::MethodNotFound, "MethodNotFound");
    }
}

