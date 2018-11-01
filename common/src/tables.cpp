//
// Created by lotus mile on 31/10/2018.
//

#include "tables.hpp"

namespace milecsa::explorer::table {
    using namespace std;

    const vector<string>& get_names() {
        static vector<string> names = {
                name::nodes_state,
                name::blockchain_state,
                name::blocks,
                name::wallets,
                name::transactions,
        };

        return names;
    }

    const vector<table::index::description>& get_indices() {
        static vector<index::description> indices = {
                {name::blocks, "block-id"},
                {name::blockchain_state, "block-id"},
                {name::nodes_state, "block-id"},
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

