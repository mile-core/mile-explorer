//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const Db &db) {
    try{
        auto state = db->get_block_history_state();
        ctx.response.result["count"] = state["block-id"];
        ctx.response.result["first-id"] = 0;
    }
    catch(nlohmann::json::parse_error& e) {
        server::Registry::Instance().error(e.what());
        make_response_parse_error(ctx);
    }
    catch (...) {
        server::Registry::Instance().error("get-block-history-state: unknown error");
        ctx.response.result = nlohmann::json::array();
    }
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-block-history-state",method);
