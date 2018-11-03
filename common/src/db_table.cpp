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

db::Data db::Table::get_slice(
        const string &table_name,
        const string &id,
        const string &with,
        uint64_t first_id,
        uint64_t limit,
        const string order_by,
        const string order_slice) const {

    try {
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        auto table = id.empty() ? q.table(table_name) : q.table(table_name).get(id);

        auto set   = with.empty() ? table : table[with];

        auto order = order_by.empty() ? set : set.order_by(order_by);

        auto slice = order_slice.empty() ? order : order[order_slice];

        auto result = slice
                .skip(first_id)
                .limit(limit).run(*connection);

        return db::Data::parse(result.to_datum().as_json());
    }
    catch (db::Timeout &e) {
        Db::err->warn("Table: {} timeout {}", table_name, e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get_slice: {}", table_name, e.message);
    }
    return db::Data::array();

}

uint64_t db::Table::get_count(const string &table_name, const string &id, const string with) const {
    try{
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        auto table = id.empty() ? q.table(table_name) : q.table(table_name).get(id);

        db::Driver::Datum datum;

        if (with.empty()){
            datum = table.count().run(*connection).to_datum();
        }
        else {
            datum = table[with].count().run(*connection).to_datum();
        }

        return db::Data::parse(datum.as_json());
    }
    catch (db::Timeout &e) {
        Db::err->warn("Table: {} timeout {}", table_name, e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get_count: {}", table_name, e.message);
    }
    return 0;
}

db::Data db::Table::get_by_id(const string &table_name, const string &id) const{
    try{
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        auto result = q
                .table(table_name)
                .get(id)
                .run(*connection);

        return db::Data::parse(result.to_datum().as_json());
    }
    catch (db::Timeout &e) {
        Db::err->warn("Table: {} timeout {}", table_name, e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get_by_id: {}", table_name, e.message);
    }
    return db::Data::array();
}


db::Data db::Table::get_state(const string &table_name, const string &id) const {

    try {
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        auto result = q
                .table(table_name)
                .max(db::Driver::optargs("index", id))
                .run(*connection);

        return db::Data::parse(result.to_datum().as_json());
    }
    catch (db::Timeout &e) {
        Db::err->warn("Table: {} timeout {}", table_name, e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get_state: {}", table_name, e.message);
    }
    return db::Data::array();

}

db::Data db::Table::get_range(
        const string &table_name,
        uint64_t first_id,
        uint64_t limit,
        const string &id,
        bool ordered
) const {

    try {
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        auto table = q.table(table_name);
        auto order = ordered ? table.order_by(db::Driver::optargs("index", id)) : table;

        auto result = order.between(first_id, first_id + limit, db::Driver::optargs("index", id)).run(*connection);

        return db::Data::parse(result.to_datum().as_json());

    }
    catch (db::Timeout &e) {
        Db::err->warn("Table: {} timeout {}", table_name, e.what());
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get_range: {}", table_name, e.message);
    }
    return db::Data::array();
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
            .table(table_name)
            .get_all(id, R"({"index":"id"})").is_empty().run(*connection);

    auto status = *(result.to_datum().get_boolean());

    if (status) {

        db::Data query;

        query["id"] = id;

        for (const auto &entry : data) {
            query[entry.first] = entry.second;
        }

        q.table(table_name)
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