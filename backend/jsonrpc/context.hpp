#pragma once

#include "message.hpp"

namespace milecsa::rpc::server {

    struct context final {

        context() = default;
        ~context() = default;

        auto from_string(const std::string &name) -> void;

        auto to_json() const -> std::string;

        message::request_message request;
        message::response_message response;
    };

    inline auto make_response_parse_error(context &ctx) -> void {
        message::response_error error(message::response_error::error_code::ParseError, "ParseError");
        ctx.response.error = error;
    }

    inline auto make_response_internal_error(context &ctx) -> void {
        message::response_error error(message::response_error::error_code::InternalError, "InternalError");
        ctx.response.error = error;
    }

    inline auto make_response_error_non_method(context &ctx) -> void {
        message::response_error error(message::response_error::error_code::MethodNotFound, "MethodNotFound");
        ctx.response.error = error;
    }
}

