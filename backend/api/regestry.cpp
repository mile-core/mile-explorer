//
// Created by lotus mile on 02/11/2018.
//

#include "registry.hpp"
#include <iostream>

using namespace milecsa::rpc::server;

void registry::set_router(const server::Router &router){
    for (auto &[name, method]: storage_){
        router->add(name,method);
    }
}

void registry::set_error_handler(milecsa::ErrorHandler &error_handler) {
    error_handler_ = std::make_optional(error_handler);
}

std::optional<milecsa::ErrorHandler> registry::error_handler_ = std::nullopt;
