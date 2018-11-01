//
// Created by lotus mile on 01/11/2018.
//


#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "tables.hpp"

using namespace milecsa::explorer;
using namespace std;

void db::Table::update(
        const string &table_name,
        const milecsa::explorer::db::Data &data) {

    auto connection = db_->get_connection();
    db::Driver::Term q = db_->query();

    q.table(table_name)
            .insert(db::Driver::json(data.dump()))
            .run(*connection);
}

void db::Table::update(const string &table_name,
                       const string id,
                       const map<string,db::Data> &data) {

    auto connection = db_->get_connection();
    db::Driver::Term q = db_->query();

    auto result = q
            .table(table::name::wallets)
            .get_all(id, R"({"index":"id"})").is_empty().run(*connection);

    auto status = *(result.to_datum().get_boolean());

    if (status) {

        db::Data query;

        query["id"] = id;

        for (const auto &entry : data) {
            query[entry.first] = entry.second;
        }

        q.table(table::name::wallets)
                .insert(db::Driver::json(query.dump()))
                .run(*connection);
    } else {

        for (const auto &entry : data) {
            auto d = [&](db::Driver::Var ff) {
                return db::Driver::object(
                        entry.first,
                        (*ff)[entry.first]
                        .coerce_to("array").set_union(db::Driver::json(entry.second.dump()))
                );
            };

            q.table(table::name::wallets)
                    .get(id)
                    .update(d, db::Driver::optargs("non_atomic", true))
                    .run(*connection);
        }
    }
}