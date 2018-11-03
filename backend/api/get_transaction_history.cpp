//
// Created by lotus mile on 03/11/2018.
//


#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check_limit(ctx)) return;

    auto first_id = api::params::get_transaction_id(ctx, api::params::first);
    auto limit = ctx.request.params.at(api::params::limit).get<uint64_t>();

    ctx.response.result = db->get_transaction_history(first_id,limit);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-transaction-history",method);
