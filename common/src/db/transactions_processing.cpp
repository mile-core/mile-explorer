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

dispatch::Queue Db::transaction_processing_queue_ = dispatch::Queue(1);

void Db::transactions_processing() {

    transaction_processing_queue_.async([&]{

        uint64_t prev_bid = 0;

        while (transaction_processing_queue_.is_running()) {

            try{

                auto update_state = [=](Db *db, uint64_t count, uint64_t block_id){
                    db::Data state = {
                            {"id", "state"},
                            {"count", count},
                            {"block-id", block_id}
                    };
                    open_table(table::name::transactions_state)->update(state);
                };

                db::Data items = open_table(table::name::transactions_processing)
                        ->cursor()
                        .sort("block-id")
                        .get_data();

                uint64_t last_count = 0 ;

                try {
                    last_count = get_transaction_history_state();
                }
                catch(...){
                    update_state(this, 0, 0);
                }

                uint64_t count = 0;

                Db::log->info("Transaction processing: {} transactions processing queue reading items: {}",
                              db_name_.c_str(), items.size());

                for (auto &item: items) {

                    uint64_t    bid   = item["block-id"];
                    std::string trxid = item["id"];

                    if (prev_bid>0) {

                        if (item.at("transaction-type") == "__processing__") {
                            Db::log->trace("Transaction processing: {} processing type, will be skipped", trxid);
                            prev_bid = bid;
                            open_table(table::name::transactions_processing)->cursor().remove(trxid);
                            continue;
                        }

                        if ((bid-prev_bid)>1) {
                            Db::log->debug("Transaction processing: {} is in a forward block {} while current is {}", trxid, bid, prev_bid);
                            prev_bid = bid;
                            break;
                        }

                    }
                    else if (item.at("transaction-type") == "__processing__") {
                        prev_bid = bid;
                        open_table(table::name::transactions_processing)->cursor().remove(trxid);
                        continue;
                    }

                    prev_bid = bid;

                    db::Data row = db::Table::Open(*this, table::name::transactions)
                            ->cursor().get(trxid).get_data();

                    if (row.count("id")>0) {
                        Db::log->trace("Transaction processing: {} already is in table {}", trxid, row.dump());
                        open_table(table::name::transactions_processing)->cursor().remove(trxid);
                        continue;
                    }

                    item["serial"] = last_count++;

                    open_table(table::name::transactions)->insert(item);

                    update_state(this, last_count, bid);

                    open_table(table::name::transactions_processing)->cursor().remove(trxid);

                    Db::log->debug("Db: {} transaction serial number updated: {}, block-id: {}",
                                   db_name_.c_str(), last_count, bid);

                    count++;
                }
                if (count>0)
                    Db::log->info("Transaction processing: {} transactions are updated: {}", db_name_.c_str(), count);
            }
            catch (db::Error &e) {
                Db::err->error("Transaction processing: {} transactions processing error {}", db_name_.c_str(), e.message);
            }
            catch (...) {
                Db::err->error("Transaction processing: transactions processing unknown error ... ");
            }

            std::this_thread::sleep_for(std::chrono::seconds(config::procesing_refresh_time));
        }
    });
}
