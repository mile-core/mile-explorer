//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    ctx.response.result = server::Registry::Instance().get_help();
};

MILECSA_JSONRPC_REGESTRY_METHOD("help",method,"{}");