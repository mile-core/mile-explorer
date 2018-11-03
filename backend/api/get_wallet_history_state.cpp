//
// Created by lotus mile on 03/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const Db &db) {

    if (ctx.request.params.count("public-key") == 0) {
        make_response_parse_error(ctx, "params: public-key not found");
        return;
    }

    auto public_key = ctx.request.params.at("public-key").get<std::string>();

    auto state = db->get_wallet_history_state(public_key);

    nlohmann::json blocks;
    blocks["count"] = state.first;
    blocks["first-id"] = 0;

    nlohmann::json trx;
    trx["count"] = state.second;
    trx["first-id"] = 0;

    ctx.response.result["block"] = blocks;
    ctx.response.result["transaction"] = trx;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-wallet-history-state",method);
