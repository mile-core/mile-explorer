//
// Created by lotus mile on 03/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check(ctx, api::params::public_key)) return;

    auto public_key = ctx.request.params.at(api::params::public_key).get<std::string>();

    auto state = db->get_wallet_history_state(public_key);

    nlohmann::json blocks;
    blocks["count"] = state.first;
    blocks["first"] = 0;

    nlohmann::json trx;
    trx["count"] = state.second;
    trx["first"] = 0;

    ctx.response.result["block"] = blocks;
    ctx.response.result["transaction"] = trx;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-wallet-history-state",method,"{public-key:'xxxxx0000xxxxx'}");
