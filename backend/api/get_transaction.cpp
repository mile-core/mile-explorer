//
// Created by lotus-mile on 03/11/2018.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static inline optional<string> get_block_id(server::context &ctx) {

    if (ctx.request.params.count(api::params::block_id)) {
        auto block_id_param = ctx.request.params.at(api::params::block_id);

        string block_id;

        if (block_id_param.is_number()) {
            block_id = std::to_string(block_id_param.get<std::uint64_t>());
        }
        else {
            block_id = block_id_param.get<std::string>();
        }

        return make_optional(block_id);
    }

    return nullopt;
}

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (ctx.request.params.count(api::params::digest)) {
        auto param = ctx.request.params.at(api::params::digest);
        ctx.response.result = db->get_transaction_by_digest(param.get<std::string>());
        return;
    }

    if (!api::params::check(ctx, api::params::id)) return;

    auto param = ctx.request.params.at(api::params::id);

    string id;
    if (param.is_number()) {

        if (!api::params::check(ctx, api::params::public_key)) return;

        auto public_key = ctx.request.params.at(api::params::public_key).get<std::string>();

        id = public_key
                .append(":");

        if (auto block_id = get_block_id(ctx)){
            id
                    .append(*block_id)
                    .append(":");
        }
        else {
            api::params::error(ctx,api::params::block_id);
            return;
        }

        id
                .append(std::to_string(param.get<std::uint64_t>()));
    }
    else {
        if (ctx.request.params.count(api::params::public_key)){

            auto public_key = ctx.request.params
                    .at(api::params::public_key)
                    .get<std::string>();

            id = public_key
                    .append(":");

            if (auto block_id = get_block_id(ctx)){
                id
                        .append(*block_id)
                        .append(":");
            }
            else if (param.is_number()) {
                api::params::error(ctx,api::params::block_id);
                return;
            }

            if (param.is_number())
                id = id
                        .append(std::to_string(param.get<std::uint64_t>()));
            else {
                id = id
                        .append(param.get<std::string>());
            }
        }
        else {
            id = param.get<std::string>();
        }
    }

    ctx.response.result = db->get_transaction_by_id(id);
};

MILECSA_JSONRPC_REGESTRY_METHOD(
        "get-transaction",
        method,
        "{'digest': <digest>} or {id: '<public-key>:<block-id>:<transaction-id>'} or {id:<transaction-id>, block-id:<block-id>, public-key: '<public-key>'}  or {id:'<block-id>:<transaction-id>', public-key: '<public-key>'}");
