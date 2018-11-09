//
// Created by lotus-mile on 03/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check(ctx, api::params::id)) return;

    auto param = ctx.request.params.at(api::params::id);

    string id;
    if (param.is_number()) {
        if (!api::params::check(ctx, api::params::public_key)) return;
        auto public_key = ctx.request.params.at(api::params::public_key).get<std::string>();
        id = public_key.append(":").append(std::to_string(param.get<std::uint64_t>()));
    }
    else {
        if (ctx.request.params.count(api::params::public_key)){
            auto public_key = ctx.request.params.at(api::params::public_key).get<std::string>();
            id = public_key.append(":");
            if (param.is_number())
                id = id.append(std::to_string(param.get<std::uint64_t>()));
            else {
                id = id.append(param.get<std::string>());
            }
        }
        else {
            id = param.get<std::string>();
        }
    }

    ctx.response.result = db->get_transaction_by_id(id);
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-transaction",method,"{id: 'xxxxxxx:0000000'} or {id:0, public-key: 'xxxxxxx'");
