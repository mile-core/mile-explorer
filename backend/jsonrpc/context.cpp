#include "context.hpp"

using namespace milecsa::rpc::server;

auto context::from_string(const std::string &name) -> void {
    try {
        auto json = nlohmann::json::parse(name);
        message::parse(json, request);
        response.id = request.id;
    } catch (...) {
        response.error=  message::response_error(message::response_error::error_code::ParseError, "ParseError");
    }
}

auto context::to_json() const -> std::string {
    return message::serialize(response);
}
