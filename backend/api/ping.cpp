//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    ctx.response.result = true;
};

MILECSA_JSONRPC_REGESTRY_METHOD("ping",method);