//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const Db &db) {
    try{
        auto state = db->get_network_state();
        ctx.response.result["nodes"]["count"] = state["nodes"].size();
        ctx.response.result["nodes"]["first-id"] = 0;
    }
    catch(nlohmann::json::parse_error& e) {
        server::Registry::Instance().error(e.what());
        make_response_parse_error(ctx);
    }
    catch (...) {
        server::Registry::Instance().error("unknown error");
        ctx.response.result = nlohmann::json::array();
    }
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-network-state",method);
