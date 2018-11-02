//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const Db &db) {
    auto state = db->get_network_state();
    ctx.response.result["nodes"]["count"] = state["nodes"].size();
    ctx.response.result["nodes"]["first-id"] = 0;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-network-state",method);
