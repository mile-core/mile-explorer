//
// Created by lotus mile on 03/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check(ctx, api::params::public_key)) return;

    auto public_key = ctx.request.params.at(api::params::public_key).get<std::string>();

    ctx.response.result = db->get_wallet_node(public_key);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-wallet-node",method);
