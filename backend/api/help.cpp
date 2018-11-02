//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto help = [](server::context &ctx, const Db &db) {
    ctx.response.result = {
            "get-network-state",
            "get-nodes",
            "get-block-history-state",
            "get-block-history",
            "get-block",
            "get-wallet-history-state",
            "get-wallet-history-blocks",
            "get-wallet-history-transactions",
            "get-transaction-history-state"
            "get-transaction-history",
            "get-transaction"
    };
};

MILECSA_JSONRPC_REGESTRY_METHOD("help",help);