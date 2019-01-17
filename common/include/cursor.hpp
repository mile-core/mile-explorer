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
                    :cursor_(c), db_(std::move(db)) {}

            db::Data          get_data();
            double            get_number();

            Cursor filter(const db::Driver::Term &term)const;
            Cursor order_by(const db::Driver::Term &term)const;
            Cursor concatMap(std::function<db::Driver::Term(db::Driver::Var)> f)const;
            Cursor concatMap(const db::Driver::Term &term)const;
            Cursor group(const string &field)const;
            Cursor limit_func(const int &amount)const;
            Cursor ungroup()const;
            template <typename T>
            Cursor fold(const T& base_value,
                        std::function<db::Driver::Term(db::Driver::Var, db::Driver::Var)> reduce_value_f,
                        db::Driver::OptArgs&& opt_args) const {
                try {

                    auto result = cursor_.fold(base_value, reduce_value_f, std::move(opt_args));

                    return  db::Cursor(
                    result,
                    std::move(db_));
                }
                catch (db::Error &e) {
                    Db::err->error("Table: {} error fold: {}", db_->get_name(), e.message);
                }
                return db::Cursor();
            }

            Cursor max(const string &id)const;
            Cursor min(const string &id)const;
            Cursor field(const string &field_name)const;
            Cursor get(const string &id, const optional<std::string> &index=nullopt)const;
            Cursor remove(const string &id);
            Cursor remove();
            Cursor count()const;

            Cursor slice(uint64_t first_id, uint64_t limit)const;
            Cursor between(uint64_t first_id, uint64_t limit, const string &id)const;
            Cursor sort(const string &index)const;
            Cursor sort_field(const string &index)const;
            template <typename T>
            Cursor filter(const string &index, T value)const{
                try {

                    auto result = cursor_
                            .filter(db::Driver::optargs(index.c_str(), value));

                    return  db::Cursor(
                            result,
                            std::move(db_));
                }
                catch (db::Error &e) {
                    Db::err->error("Table: {} error slice: {}", db_->get_name(), e.message);
                }
                return db::Cursor();
            }

        private:
            optional<Db> db_;
            TableCursor cursor_;
        };
    }
}