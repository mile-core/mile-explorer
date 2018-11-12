//
// Created by lotus mile on 10/11/2018.
//

#include "migration.hpp"
#include "names.hpp"
#include "db.hpp"
#include "table.hpp"
#include "cursor.hpp"
#include "version.hpp"
#include <optional>

using namespace milecsa::explorer;
using namespace std;

namespace milecsa::explorer::db {

    void migration(std::optional<Db> db) {
        unsigned int v = indexerVersion();
        unsigned int dbv = db->get_version();

        Logger::log->info("Logger: previous db[{}] version: {}, indexer version: {}", db->get_name().c_str(), dbv, v);

        try {
            auto old = db->open_table(table::name::meta)->cursor()
                    .get("version")
                    .field("current")
                    .get_number();

            if (old <= 122) {
                db->open_table(table::name::meta)->cursor()
                        .get("version").remove();
            }

        }
        catch (db::Error & e)
        {
            Db::err->warn("Db: {} get_version error {}", db->get_name().c_str(), e.message);
        }


        if (v > db->get_version()) {
            //
            // Do migrations
            //
            db->update_version(v);
        }

        uint64_t state_block_id = static_cast<uint64_t>(db->get_last_processed_block_id());
        uint64_t processing_block_id = 0;

        try {

            auto id = db
                    ->open_table(table::name::transactions_processing)
                    ->cursor()
                    .min("block-id")
                    .field("block-id")
                    .get_number();

            processing_block_id = static_cast<uint64_t>(id);

        } catch (db::Error &e) {
            Db::err->trace("Db: {} error reading last block id from transactions_processing {}", db->get_name().c_str(),
                           e.message);
        }

        Db::log->debug("Db: {} last state block-id: {}, processing block-id: {}",
                       db->get_name().c_str(),
                       state_block_id,
                       processing_block_id);

        if (state_block_id>processing_block_id && processing_block_id > 0) {

            db->open_table(table::name::transactions_processing)
                    ->cursor()
                    .between(processing_block_id, state_block_id, "block-id")
                    .remove();

            Db::log->info("Db: {} last state block-id: {}, processing block-id: {}, processing table flushed ... ",
                           db->get_name().c_str(),
                           state_block_id,
                           processing_block_id);
        }
    }
}
