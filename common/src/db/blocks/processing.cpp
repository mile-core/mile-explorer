//
// Created by lotus mile on 2018-12-06.
//

#include <any>
#include <milecsa_queue.h>
#include <milecsa.hpp>

#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using namespace milecsa::explorer;
using namespace std;

dispatch::Queue Db::blocks_processing_queue_ = dispatch::Queue(1);

void Db::blocks_processing() {

    blocks_processing_queue_.async([&]{

        auto get_prev_bid = [&]{
            return open_table(table::name::blocks)
                    ->cursor(false)
                    .max("block-id")
                    .field("block-id")
                    .get_number();
        };

        uint64_t prev_bid = 0;

        while (blocks_processing_queue_.is_running()) {

            try {

                db::Data items = open_table(table::name::blocks_processing)
                        ->cursor()
                        .sort("block-id")
                        .get_data();

                Db::log->info("Block processing: {} blocks are ready", items.size());

                for (auto &item: items) {

                    uint64_t bid = item["block-id"];
                    prev_bid     = get_prev_bid();

                    if ( (prev_bid > 0) && ((bid-prev_bid) > 1) && (bid > prev_bid)) {
                        Db::log->debug("Block processing: block {} is in a forward when current is {}", bid, prev_bid);
                        break;
                    }

                    std:string id = std::to_string(bid);
                    db::Data row = open_table(table::name::blocks)
                            ->cursor().get(id).get_data();

                    block_changes(item, bid, item["timestamp"]);

                    if (row.count("id")>0){
                        Db::log->debug("Block processing: block {} already is in table",  bid);
                    } else {
                        open_table(table::name::blocks)->insert(item);
                    }

                    open_table(table::name::blocks_processing)->cursor().remove(id);

                    Db::log->info("Block processing: block {} has been updated", bid);
                }
            }

            catch (db::Error &e) {
                Db::err->error("Block processing: {} error block update {}", db_name_.c_str(), e.message);
            }
            catch (...) {
                Db::err->error("Block processing: transactions updating index unknown error ... ");
            }

            std::this_thread::sleep_for(std::chrono::seconds(config::procesing_refresh_time));
        }
    });
}
