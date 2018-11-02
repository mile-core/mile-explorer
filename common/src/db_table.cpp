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

db::Data db::Table::get_state(const string &table_name, const string &id) const {

    auto connection = db_->get_connection();
    db::Driver::Term q = db_->query();

    auto result = q
            .table(table_name)
            .max(db::Driver::optargs("index", id))
            .run(*connection);

    return nlohmann::json::parse(result.to_datum().as_json());
}

db::Data db::Table::get_range(
        const string &table_name,
        uint64_t first_id,
        uint64_t limit,
        const string &id,
        bool ordered
) const {
    auto connection = db_->get_connection();
    db::Driver::Term q = db_->query();

    if (ordered ) {
        auto  result = q
                .table(table_name)
                .between(first_id, first_id + limit, db::Driver::optargs("index", id))
                .order_by(db::Driver::optargs("index", id))
                .run(*connection);
        return nlohmann::json::parse(result.to_datum().as_json());
    }
    else {
        auto result = q
                .table(table_name)
                .between(first_id, first_id + limit, db::Driver::optargs("index", id))
                .run(*connection);
        return nlohmann::json::parse(result.to_datum().as_json());
    }

}

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
                       const string &id,
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