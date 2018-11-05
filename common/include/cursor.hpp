//
// Created by lotus mile on 04/11/2018.
//

#pragma once

#include <optional>
#include <any>
#include <string>
#include <vector>

#include <stdlib.h>

#include <milecsa.hpp>
#include <milecsa_jsonrpc.hpp>

#include <rethinkdb.h>

#include "table.hpp"
#include "logger.hpp"
#include "config.hpp"

namespace milecsa::explorer {
    using namespace std;

    namespace db {

        class Cursor;

        typedef Driver::Term TableCursor;

        /**
         * Specific methodes for data selecting
         */
        class Cursor {
        public:
            Cursor():db_(nullopt),cursor_(db::Driver::Datum(db::Driver::Nil())){};
            Cursor(const Cursor& other) = default;
            Cursor(Cursor&& other) = default;
            Cursor& operator= (const Cursor& other) = default;
            Cursor& operator= (Cursor&& other) = default;

            Cursor(const TableCursor &c, const optional<Db> &db)
                    :cursor_(c), db_(db) {}

            db::Data           get_data();

            Cursor max(const string &id)const;
            Cursor field(const string &field_name)const;
            Cursor get(const string &id)const;
            Cursor count()const;

            Cursor slice(uint64_t first_id, uint64_t limit)const;
            Cursor between(uint64_t first_id, uint64_t limit, const string &id)const;
            Cursor sort(const string &order_by_field_name)const;

        private:
            optional<Db> db_;
            TableCursor cursor_;
        };

    }
}