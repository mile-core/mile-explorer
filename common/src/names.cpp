//
// Created by lotus mile on 31/10/2018.
//

#include "names.hpp"

namespace milecsa::explorer::table {
    using namespace std;

    const vector<string>& get_names() {
        static vector<string> names = {
                name::node_states,
                name::node_wallets,
                name::blockchain_state,
                name::blocks,
                name::wallets,
                name::transactions,
                name::transactions_state
        };

        return names;
    }

    const vector<table::index::description>& get_indices() {
        static vector<index::description> indices = {
                {name::blocks, "block-id"},
                {name::blockchain_state, "block-id"},
                {name::node_states, "block-id"},
                {name::transactions, "block-id"}
        };
        return indices;
    }

    const map<string,string>& get_replacement_keys(){
        static map<string,string> repl_keys = {
            {"transaction-name","transaction-type"}
        };

        return repl_keys;
    }
}

