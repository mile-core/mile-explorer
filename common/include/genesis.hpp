//
// Created by lotus mile on 2018-12-12.
//

#pragma once

#include "milecsa_http.hpp"
#include "db.hpp"
#include <optional>

namespace milecsa::explorer {

    class Genesis {
    public:
        static std::optional<Genesis> Parser(const std::string &buffer);

        const db::Data &get_transactions() const { return transactions_; }

    private:
        Genesis(){};

        db::Data transactions_;
    };

}