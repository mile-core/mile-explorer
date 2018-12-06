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

        /**
         * Specific table data manipulation methodes
         */
        class Table{

        public:

            friend class db::Cursor;

            /**
             * Open table
             * @param db  - data base index
             * @param name - table name
             * @return table shared pointer interface
             */
            static const Db::Table Open(const optional<Db> &db, const string &name){
                return Db::Table(new Table(std::move(db), name));
            }

            /**
             * Get table cursor to select data
             * @return
             */
            Cursor cursor(bool outdated = true) const;

            /**
             * Test index with field name
             * @param index_name
             * @return true if is exists
             */
            bool has_index(const string &index_name);

            /**
             * Create indices
             * @param indices
             */
            void create_indices(const vector<string> &indices);

            /**
             * Create new foreign index
             * @param name
             */
            void create_index(const string &name);

            /**
             * Update table with json object
             * @param data
             */
            void insert(const db::Data &data);

            void update(const db::Data &data);

            /**
             * Update table data in row with index
             * @param id
             * @param data
             */
            void update(const string &id, const std::map<string,db::Data> &data);

        private:
            Table(const optional<Db> &db, const string &name):db_(db), name_(name){};
            optional<Db> db_;
            string name_;
        };
    }
}