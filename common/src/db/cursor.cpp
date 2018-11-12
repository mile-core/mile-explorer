//
// Created by lotus mile on 04/11/2018.
//

#include <rethinkdb.h>
#include <any>

#include <milecsa_queue.h>
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

db::Data db::Cursor::get_data()  {
    try {

        if (!db_) return db::Data::array();

        auto connection = db_->get_connection();

        auto result = cursor_.run(*connection);

        auto object = result.to_datum();

        db::Data data;
        if (object.is_nil()) {
            data = db::Data::array();
        }
        else {
            data = db::Data::parse(object.as_json());
        }

        Db::log->trace("Table: {} get_data(): {}", db_->get_name(), data.dump());
        return data;
    }
    catch (db::Error &e) {}
    return db::Data::array();
}


double db::Cursor::get_number()  {
    try {

        if (!db_) return 0;

        auto connection = db_->get_connection();

        auto result = cursor_.run(*connection);

        auto object = result.to_datum();

        if (object.is_number()) {
            return *object.get_number();
        }

        return 0;
    }
    catch (db::Error &e) {}
    return 0;
}

db::Cursor db::Cursor::max(const string &id) const {
    try {
        return  db::Cursor(
                cursor_.max(db::Driver::optargs("index", id)).copy(),
                std::move(db_));
    }
    catch (db::Error &e) {}
    return db::Cursor();
}

db::Cursor db::Cursor::min(const string &id) const {
    try {
        return  db::Cursor(
                cursor_.min(db::Driver::optargs("index", id)).copy(),
                std::move(db_));
    }
    catch (db::Error &e) {}
    return db::Cursor();
}

db::Cursor db::Cursor::get(const string &id)const {
    try {
        return  db::Cursor(
                cursor_.get(id),
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get({}): {}", db_->get_name(), id, e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::remove(const string &id) {
    try {
        auto connection = db_->get_connection();

        auto r = cursor_.get(id);
        r.delete_().run(*connection);
        return  db::Cursor(
                r,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} remove {}: {}", db_->get_name(), id, e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::remove() {
    try {
        auto connection = db_->get_connection();
        cursor_.delete_().run(*connection);
        return  db::Cursor(
                cursor_,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} remove: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::count()const {
    try {
        return  db::Cursor(
                cursor_.count(),
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error count: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::field(const string &field_name) const {
    try {
        auto result = cursor_.get_field(field_name);
        return  db::Cursor(
                result,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error get field: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::slice(uint64_t first_id, uint64_t limit)const {
    try {

        auto result = cursor_.slice(first_id,first_id+limit);//skip(first_id).limit(limit);

        return  db::Cursor(
                result,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error slice: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::between(uint64_t first_id, uint64_t limit, const string &id)const {
    try {

        auto result = cursor_
                .order_by(db::Driver::optargs("index", id))
                .between(first_id, first_id + limit, db::Driver::optargs("index", id));

        return  db::Cursor(
                result,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error slice: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::sort(const string &index)const {
    try {

        auto result = cursor_.order_by(db::Driver::optargs("index", index));

        return  db::Cursor(
                result,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error sort: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}

db::Cursor db::Cursor::sort_field(const string &index)const {
    try {

        auto result = cursor_.order_by(index);

        return  db::Cursor(
                result,
                std::move(db_));
    }
    catch (db::Error &e) {
        Db::err->error("Table: {} error sort: {}", db_->get_name(), e.message);
    }
    return db::Cursor();
}