//
// Created by lotus mile on 14/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    auto state = db->get_turnovers_24();
    ctx.response.result = db->get_turnovers_24();
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-turnovers",method,"{}");
