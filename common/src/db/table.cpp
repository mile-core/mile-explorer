//
// Created by lotus mile on 01/11/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

bool db::Table::has_index(const string &index_name) {
    try {
        auto connection = db_->get_connection();
        auto q = db_->query();
        return q.table(name_)
                .index_wait(index_name)
                .is_empty().run(*connection)
                .to_datum()
                .get_boolean();
    }
    catch (...) {
        return false;
    }
}

void db::Table::create_index(const string &name) {
    try {
        auto connection = db_->get_connection();
        db_->query().table(name_).index_create(name).run(*connection);
    }
    catch (...) {
        return ;
    }
}

void db::Table::create_indices(const vector<string> &indices) {
    auto connection = db_->get_connection();
    for(auto name: indices) {
        db_->query().table(name_).index_create(name).run(*connection);
    }
}

db::Cursor db::Table::cursor() const {
    try {
        db::Result q = db_->query();
        return  db::Cursor(
                q.table(name_, db::Driver::optargs("read_mode", "outdated")),
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get cursor: {}", name_, e.message);
    }
    return db::Cursor();
}

void db::Table::insert(const milecsa::explorer::db::Data &data) {
    try{
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        q.table(name_)
                .insert(db::Driver::json(data.dump()))
                .run(*connection);
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error insert: {}", db_->get_name(), e.message);
    }
}

void db::Table::update(const db::Data &data){
    try {
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        std::string id = data["id"];

        auto result = q
                .table(name_)
                .get_all(id, R"({"index":"id"})").is_empty().run(*connection);

        auto status = *(result.to_datum().get_boolean());

        if (status) {
            q.table(name_)
                    .insert(db::Driver::json(data.dump()))
                    .run(*connection);
        } else {
            db::Data _data = data;
            _data.erase(id);

            q.table(name_)
                    .get(id)
                    .update(db::Driver::json(_data.dump()))
                    .run(*connection);
        }
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error update: {}", db_->get_name(), e.message);
    }
}

void db::Table::update(const string &id,
                       const map<string,db::Data> &data) {

    try{
        auto connection = db_->get_connection();
        db::Driver::Term q = db_->query();

        auto result = q
                .table(name_)
                .get_all(id, R"({"index":"id"})").is_empty().run(*connection);

        auto status = *(result.to_datum().get_boolean());

        if (status) {

            db::Data query;

            query["id"] = id;

            for (const auto &entry : data) {
                query[entry.first] = entry.second;
            }

            q.table(name_)
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

                q.table(name_)
                        .get(id)
                        .update(d, db::Driver::optargs("non_atomic", true))
                        .run(*connection);
            }
        }
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error update rows by id: {}", db_->get_name(), e.message);
    }
}