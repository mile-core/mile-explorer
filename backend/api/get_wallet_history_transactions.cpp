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
    auto first_id = api::params::get_transaction_id(ctx, api::params::first);

    ctx.response.result = db->get_wallet_history_transactions(public_key,first_id,limit);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-wallet-history-transactions",method);
