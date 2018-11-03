//
// Created by lotus mile on 03/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check(ctx, api::params::public_key)) return;
    if (!api::params::check_limit(ctx)) return;

    auto public_key = ctx.request.params.at(api::params::public_key).get<std::string>();
    auto limit = ctx.request.params.at(api::params::limit).get<uint64_t>();

    uint256_t block_id = api::params::get_block_id(ctx, api::params::first);
    if (block_id == uint256_t(-1)) return;

    uint64_t first_id = static_cast<uint64_t>(block_id);

    ctx.response.result = db->get_wallet_history_blocks(public_key,first_id,limit);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-wallet-history-blocks",method);
