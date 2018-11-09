//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    uint256_t block_id = api::params::get_block_id(ctx, api::params::id);
    if (block_id == uint256_t(-1)) return;

    ctx.response.result = db->get_block_by_id(block_id);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-block",method,"{id:0} or {id:0}");
