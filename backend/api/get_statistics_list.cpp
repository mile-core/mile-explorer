//
// Created by lotus-mile on 2018-12-08.
//

#include "api/registry.hpp"

using namespace milecsa::explorer;

static auto method = [](server::context &ctx, const ctxDb &db) {
    ctx.response.result["turnover-24"]= R"({"params":{}})"_json;
    ctx.response.result["transaction-turnover"] = R"({"params":{}})"_json;
    ctx.response.result["wallets-top"] = R"({"params":{"asset-code":0, "limit":10}})"_json;
};

MILECSA_JSONRPC_REGESTRY_METHOD("get-statistics-list", method, "{}");
