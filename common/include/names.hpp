//
// Created by lotus mile on 31/10/2018.
//
#pragma once

#include <optional>
#include <any>
#include <string>
#include <vector>
#include "logger.hpp"
#include "config.hpp"

namespace milecsa::explorer{

    using namespace std;

    namespace table {

        namespace name {
            constexpr const char *meta               = "meta";
            constexpr const char *blockchain_info    = "blockchain_info";
            constexpr const char *blockchain_state   = "blockchain_state";
            constexpr const char *node_states        = "node_states";
            constexpr const char *node_wallets       = "node_wallets";
            constexpr const char *blocks             = "blocks";
            constexpr const char *wallets            = "wallets";
            constexpr const char *transactions       = "transactions";
            constexpr const char *transactions_state = "transactions_state";
            constexpr const char *transactions_processing = "transactions_processing";
        }

        namespace index {
            struct description {
                string table;
                string field;
            };
        }

        const map<string,string>& get_replacement_keys();
        const vector<string>& get_names();
        const vector<index::description>& get_indices();
    }
}