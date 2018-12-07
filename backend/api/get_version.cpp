//
// Created by lotus mile on 2018-12-07.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    auto state = db->get_version();
    ctx.response.result = state;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-version",method,"{}");
