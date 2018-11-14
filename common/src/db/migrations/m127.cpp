//
// Created by lotus mile on 14/11/2018.
//

#include "db.hpp"
#include "table.hpp"

using namespace milecsa::explorer;
using namespace std;

namespace milecsa::explorer::db {

    void m127(std::optional<Db> db) {
        Logger::log->info("Migration[{}]: processing start...",
                "m127",
                db->get_name().c_str());

        Logger::log->info("Migration[{}]: processing end...",
                          "m127",
                          db->get_name().c_str());
    }
}
