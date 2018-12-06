//
// Created by lotus mile on 2018-12-05.
//

#include "statistics.hpp"

using namespace milecsa::explorer::statistic;

void registry::set_error_handler(milecsa::ErrorHandler &error_handler) {
    error_handler_ = std::make_optional(error_handler);
}

std::optional<milecsa::ErrorHandler> registry::error_handler_ = std::nullopt;
