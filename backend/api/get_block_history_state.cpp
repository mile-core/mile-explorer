//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const Db &db) {
    auto state = db->get_block_history_state();
    ctx.response.result["count"] = state["block-id"];
    ctx.response.result["first-id"] = 0;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-block-history-state",method);
