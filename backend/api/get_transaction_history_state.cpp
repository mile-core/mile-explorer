//
// Created by denn on 03/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    auto state = db->get_transaction_history_state();
    ctx.response.result["count"] = state;
    ctx.response.result["first"] = 0;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-transaction-history-state",method);
