//
// Created by lotus mile on 10/11/2018.
//
#pragma once

#include <optional>
namespace milecsa::explorer {
    class Db;
    namespace db {
        void migration(std::optional<Db> db);
    }
}