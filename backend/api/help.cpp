//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    ctx.response.result = {
            "help",                              // {}
            "ping",                              // {}
            "get-network-state",                 // {}
            "get-nodes",                         // {"first":0, "limit":42}
            "get-block-history-state",           // {}
            "get-block-history",                 // {"first":0, "limit":42}
            "get-block",                         // {"id": 0}
            "get-wallet-history-state",          // {}
            "get-wallet-history-blocks",         // {"first":0, "limit":42}
            "get-wallet-history-transactions",   // {"first":0, "limit":42}
            "get-transaction-history-state",
            "get-transaction-history",
            "get-transaction"
    };
};

MILECSA_JSONRPC_REGESTRY_METHOD("help",method);