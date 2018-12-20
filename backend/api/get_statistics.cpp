//
// Created by lotus mile on 2018-12-08.
//

#include "api/registry.hpp"
#include "utils.hpp"
#include "names.hpp"
#include "table.hpp"

using namespace milecsa::explorer;

/**
 * TODO: automate statistics API methods adding process
 */

static auto method = [](server::context &ctx, const ctxDb &db) {

    if (!api::params::check(ctx, api::params::name)) return false;

    auto name = ctx.request.params.at(api::params::name).get<std::string>();

    if (name == "turnover-24") {
        ctx.response.result = db
                        ->open_table(table::name::turnovers)
                        ->cursor()
                        .get("turnovers-24")
                        .get_data();
        return true;
    }

    else if (name == "transaction-turnover") {
        ctx.response.result = db
                        ->open_table(table::name::transaction_turnover)
                        ->cursor()
                        .get_data();
        return true;
    }

    else if (name == "wallets-top") {

        if (!api::params::check(ctx, "asset-code")) return false;
        if (!api::params::check(ctx, "limit")) return false;

        auto code  = ctx.request.params.at("asset-code").get<unsigned short>();
        auto limit = ctx.request.params.at("limit").get<uint64_t>();

        std::string table = "wallets_top";
        milecsa::token asset = milecsa::assets::TokenFromCode(code);

        if (asset.code != milecsa::assets::XDR.code && asset.code != milecsa::assets::MILE.code) {
            make_response_parse_error(ctx, "token not found");
            return false;
        }

        ctx.response.result = db
                ->open_table(table)
                ->cursor()
                .between(0,limit,"position")
                .filter("asset-code",asset.code)
                .get_data();

        return true;
    }

    else {
        make_response_parse_error(ctx, "statistics not found");
        return false;
    }

};

MILECSA_JSONRPC_REGESTRY_METHOD("get-statistics",method,"{name:<method-name>}");
