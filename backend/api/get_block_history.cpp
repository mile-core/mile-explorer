//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const Db &db) {

    if (ctx.request.params.count("first-id") == 0) {
        make_response_parse_error(ctx, "first-id params must be set");
        return;
    }

    if (ctx.request.params.count("limit") == 0) {
        make_response_parse_error(ctx, "limit params must be set");
        return;
    }

    auto limit = ctx.request.params.at("limit").get<uint64_t>();

    if (limit>1000) {
        make_response_parse_error(ctx, "limit params must be less then 1000");
        return;
    }

    auto first_id = ctx.request.params.at("first-id").get<uint64_t>();

    ctx.response.result = db->get_block_history(first_id, limit);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-block-history",method);
