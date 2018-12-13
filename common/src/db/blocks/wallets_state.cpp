//
// Created by lotus mile on 2018-12-13.
//

#include "statistics.hpp"
#include "utils.hpp"
#include "db.hpp"
#include <milecsa_jsonrpc.hpp>
#include <time.h>

static dispatch::Queue state_queue = dispatch::Queue(config::rpc_queue_size);

void Db::update_wallet_state(const std::string &public_key){

    state_queue.async([=]{

        try {
            optional<milecsa::rpc::Client> client = std::nullopt;

            while (!client) {
                client = get_rpc();

                if (!client) {
                    Db::err->error("Db: {} wallet state retry...", public_key);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }

                if (auto _state = client->get_wallet_state(public_key))
                    try{

                        db::Data state = *_state;

                        state["timestamp"] = std::time(0);

                        state.erase("preferred-transaction-id");

                        this->open_table(table::name::wallets_state)->update(public_key, state);

                        Db::log->debug("Db: {} wallet state {}", public_key, state.dump());

                        break;

                    }
                    catch (db::Error &e) {
                        Db::err->error("Db: {} error processing wallet state  {}", db_name_.c_str(), e.message);
                    }
                    catch (std::exception &e) {
                        Db::err->error("Db: {} error processing wallet state {}", db_name_.c_str(), e.what());
                    }

                client.reset();

                Db::err->error("Db: {} wallet state retry...", public_key);

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        catch(...){
            Db::log->info("Wallets: state updating unknow error");
        }

        Db::log->debug("Db: {} wallet state done", public_key);

    });
}

void Db::update_wallets_state() {

    db::Data items = open_table(table::name::wallets)
            ->cursor()
            .field("id")
            .get_data();

    for (auto &item: items) {
        update_wallet_state(item.get<string>());
    }

};

