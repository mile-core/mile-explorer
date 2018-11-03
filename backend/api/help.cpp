//
// Created by lotus mile on 02/11/2018.
//

#include "api/registry.hpp"

static auto method = [](server::context &ctx, const ctxDb &db) {
    ctx.response.result = {
            //
            // block id is uint256, so json-rpc requests use string conversion
            // transaction id is uint64, json-rpc requests use json int
            //
            "help",                              // {}
            "ping",                              // {}
            "get-network-state",                 // {}
            "get-nodes",                         // {"first":"0", "limit":42}
            "get-block-history-state",           // {}
            "get-block-history",                 // {"first":"0", "limit":42}
            "get-block",                         // {"id": "0"}
            "get-wallet-history-state",          // {}
            "get-wallet-history-blocks",         // {"first":0, "limit":42}
            "get-wallet-history-transactions",   // {"first":0, "limit":42}
            "get-transaction-history-state",     // {}
            "get-transaction-history",           // {"first":0, "limit":42}
            "get-transaction"                    // {"id":42}
    };
};

MILECSA_JSONRPC_REGESTRY_METHOD("help",method);