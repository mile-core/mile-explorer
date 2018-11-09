//
// Created by lotus mile on 09/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    auto state = db->get_blockchain_info();
    ctx.response.result = state;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-blockchain-info",method,"{}");
