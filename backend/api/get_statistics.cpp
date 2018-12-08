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

    else {
        make_response_parse_error(ctx, "statistics not found");
        return false;
    }

};

MILECSA_JSONRPC_REGESTRY_METHOD("get-statistics",method,"{name:<method-name>}");
