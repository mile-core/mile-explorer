//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check_limit(ctx)) return;

    auto first_id = ctx.request.params.at(api::params::id).get<uint64_t>();
    auto limit = ctx.request.params.at(api::params::limit).get<uint64_t>();

    ctx.response.result = db->get_nodes(first_id, limit);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-nodes",method);
