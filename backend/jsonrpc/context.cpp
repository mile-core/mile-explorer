//
// Created by lotus mile on 02/11/2018.
//

#include "jsonrpc/context.hpp"

using namespace milecsa::rpc::server;

auto context::from_string(const std::string &name) -> void {
    try {
        auto json = nlohmann::json::parse(name);

        if(!message::is_valid_version(json)){
            request.id = json.at("id");
            make_response_parse_error(*this,"WrongVersion");
            return;
        }

        if(!message::parse(json, request)){
            make_response_invalid_request(*this);
            return;
        }

        response.id = request.id;

        if(!message::is_jsonrpc(json)){
            make_response_invalid_request(*this);
            return;
        }

        if(message::is_notify(json)){
            make_response_invalid_request(*this);
            return;
        }

    }
    catch(nlohmann::json::parse_error& e) {
        make_response_parse_error(*this, e.what());
    }
    catch (...) {
        make_response_parse_error(*this,"json-rpc: unknown parse error");
    }
}

auto context::to_json() const -> std::string {
    return message::serialize(response);
}
