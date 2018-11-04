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

#include "db.hpp"
#include "cursor.hpp"
#include "logger.hpp"
#include "config.hpp"

namespace milecsa::explorer {
    using namespace std;

    namespace db{

        class Table{

        public:

            friend class db::Cursor;

            static const shared_ptr<Table> Open(const optional<Db> &db, const string &name=""){
                return shared_ptr<Table>(new Table(db, name));
            }

            Table(const optional<Db> &db, const string &name):db_(db), name_(name){};

            void update(
                    const string &table_name,
                    const db::Data &data);

            void update(
                    const string &table_name,
                    const string &id,
                    const std::map<string,db::Data> &data);

            Cursor cursor() const;

        private:
            optional<Db> db_;
            string name_;
        };
    }
}