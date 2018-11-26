//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    auto state = db->get_block_history_state();
    ctx.response.result["count"] = state["block-id"].get<uint64_t>()+1;
    ctx.response.result["timestamp"] = state["timestamp"];
    ctx.response.result["first"] = 0;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-block-history-state",method,"{}");
