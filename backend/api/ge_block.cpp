//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check(ctx, api::params::id)) return;

    auto id = ctx.request.params.at(api::params::id).get<std::string>();
    uint256_t block_id;

    if(!StringToUInt256(id, block_id, false)){
        make_response_parse_error(ctx, "block couldn't be converted to uint256");
        return;
    }

    ctx.response.result = db->get_block(block_id);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-block",method);
