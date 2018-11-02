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
