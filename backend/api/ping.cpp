//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto ping = [](server::context &ctx, const Db &db) {
    ctx.response.result = true;
};

MILECSA_JSONRPC_REGESTRY_METHOD("ping",ping);