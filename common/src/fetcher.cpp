//
// Created by lotus mile on 29/10/2018.
//

#include "milecsa_http.hpp"
#include "fetcher.hpp"
#include "db.hpp"
#include "utils.hpp"
#include "genesis.hpp"

using namespace milecsa::explorer;
using namespace std;

optional<Fetcher> Fetcher::Connect(
        const std::vector<std::string>  &urls,
        int update_timeout,
        const optional<Db> &db) {

    int count = 0;

    for(auto u: urls) {
        auto client = milecsa::rpc::Client::Connect(
                u,
                true,
                response_fail_handler,
                error_handler);

        if (!client)
            continue;

        count++;
    }

    if (count <= 0 )
        return nullopt;

    return make_optional(Fetcher(urls, update_timeout, db));
}

optional<milecsa::rpc::Client> Fetcher::get_rpc() {
    return milecsa::explorer::get_rpc(urls_);
}

optional<Db> &Fetcher::get_db() {
    return db_;
}

Fetcher::Fetcher(
        const std::vector<std::string> &urls,
        int read_timeout,
        const optional<Db> &db)
        :urls_(urls),
         update_timeout_(read_timeout),
         db_(db),
         main_fetching_task_(new Task("main-task", config::rpc_queue_size)),
         block_fetcher_task_(new Task("block-fetcher", config::block_processin_queue_size)),
         block_processing_task_(new Task("block-processing", config::block_processin_queue_size)),
         rpc_fetching_block_task_(new Task("rpc-fetching-block", config::rpc_queue_size)),
         utility_task_(new Task("utility-processing", 3))
{
    srand(time(0));
}

void Fetcher::run(uint256_t block_id) {

    main_fetching_task_->async([this, block_id]{

        uint256_t first = block_id;

        Fetcher::log->info("Fetcher: starting ... {}, block-id: {}",
                main_fetching_task_->is_running(), UInt256ToDecString(block_id));

        while( true ) {

            try {
                auto client = this->get_rpc();

                if (!client){
                    Logger::err->warn("Fetcher: main request failed, retrying");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }

                //
                // prepare fetching block
                //
                optional<uint256_t> next_block_id = client->get_current_block_id();

                if (next_block_id) {

                    uint256_t last = *next_block_id;

                    Logger::log->info("Fetcher: current block: {}, processing from: {}", UInt256ToDecString(last), UInt256ToDecString(first));

                    Fetcher::log->debug(
                            "Getting block ids: [{}:{}], block fetcher queue is busy: {}",
                            UInt256ToDecString(first),
                            UInt256ToDecString(last),
                            (this->block_fetcher_task_->is_active()));

                    //
                    // Start block fetching
                    //
                    if (!this->block_fetcher_task_->is_active() && (first<last)) {
                        block_fetcher_task_->async([this, &first, last] {
                            this->fetch_blocks(first,last);
                            first = last;
                        });
                    }
                }

            }
            catch (std::exception &e) {
                Fetcher::err->critical("Fetcher:  next block error: {} ", e.what());
            }
            catch (...) {
                Logger::log->info("Fetcher: next block Unknown error");
            }

            std::this_thread::sleep_for(std::chrono::seconds(this->update_timeout_));
        }

    });

    //
    // Utility...
    //
    if (!this->utility_task_->is_active()) {
        utility_task_->async([this] {
            Fetcher::log->info("Fetcher: states fetching started ...");
            this->fetch_states();
        });

        utility_task_->async([this] {
            Fetcher::log->info("Fetcher: genesis fetching started ...");
            this->fetch_genesis();
        });
    }

    Fetcher::log->info("Fetcher: ... started");
}

void Fetcher::fetch_blocks(uint256_t from, uint256_t to) {

    Fetcher::log->info("Fetcher: pocessing blocks: [{}:{}] is in progress ...",
            UInt256ToDecString(from), UInt256ToDecString(to));

    for (auto i = from; i <= to; i++){

        rpc_fetching_block_task_->async([i, from, to, this]{

            milecsa::rpc::response response;
            db::Data               block;

            std::string cu;

            auto tick = boost::posix_time::microsec_clock::local_time();

            while (!response) {

                std::this_thread::sleep_for(std::chrono::milliseconds(config::request_timeout));

                try{
                    {
                        //
                        // auto destroy
                        //
                        auto client = this->get_rpc();

                        if (!client) {
                            Fetcher::err->warn("Fetcher: block {} getting request failed, retrying",
                                               UInt256ToDecString(i));
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            response.reset();
                            continue;
                        }

                        cu = client->get_url().get_host();

                        response = client->get_block(i);
                    }

                    if (!response){
                        Fetcher::err->warn("Fetcher: response {} is empty, retrying", UInt256ToDecString(i));
                        response.reset();
                        continue;
                    }

                    if (response->count("block-data")==0){
                        Fetcher::err->warn("Fetcher: block {} getting not data, retrying", UInt256ToDecString(i));
                        response.reset();
                        continue;
                    }

                    block = response->at("block-data");
                }
                catch (std::exception &e) {
                    Fetcher::err->error("Fetcher: fetching block id: {}, error: {}", UInt256ToDecString(i), e.what());
                    response.reset();
                }
                catch (...) {
                    Fetcher::err->error("Fetcher: something wrong ...");
                    response.reset();
                }
            }

            block_processing_task_->async([block, i, tick, cu, this]{
                try{

                    this->get_db()->add_block(block, i);

                    auto diff = (float)
                            (boost::posix_time::microsec_clock::local_time()
                            - tick).total_milliseconds()/1000.0f;

                    Fetcher::log->trace("Fetcher: total processing time from {} takes: {}s.", cu, diff);
                } catch (db::Error &e) {
                    Db::err->error("Db: {} error reading last block id {}", " = ", e.message);
                }
            });
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(config::request_timeout));
    }
}

void Fetcher::fetch_states(){

    uint256_t block_id = 0;

    bool bch_info_got = false;

    while(utility_task_->is_running()) {

        try {
            auto client = this->get_rpc();

            if (!client){
                Logger::err->warn("Fetcher: utility request failed, retrying");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            if (!bch_info_got) {
                if (auto info = client->get_blockchain_info()){
                    this->get_db()->update_info(*info);
                    bch_info_got = true;
                }
            }

            optional<uint256_t> next_block_id = client->get_current_block_id();

            if (!next_block_id)
                continue;

            auto nodes = client->get_nodes();

            if(!nodes || *next_block_id<=block_id)
                continue;

            block_id = *next_block_id;

            if(!nodes->empty()){
                this->get_db()->add_node_states(*nodes, block_id);
            }

        }
        catch (std::exception &e) {
            Fetcher::err->error("Fetcher: {} error fetching network meta data: {}", " = ", e.what());
        }
        catch (...) {
            Fetcher::err->error("Fetcher: something wrong ...");
        }

        std::this_thread::sleep_for(std::chrono::seconds(this->update_timeout_));
    }
}

void Fetcher::fetch_genesis() {

    while(utility_task_->is_running()) {

        try {
            auto client = milecsa::http::Client::Connect(config::genesis_url, true, Fetcher::error_handler);

            if (!client) {
                Logger::err->warn("Fetcher: genesis request failed, retrying");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            if (auto body = client->get()) {
                auto genesis = milecsa::explorer::Genesis::Parser(body.value());
                this->get_db()->update_genesis(genesis->get_transactions());
            }
        }

        catch (std::exception &e) {
            Fetcher::err->error("Fetcher: {} error fetching genesis data: {}", " = ", e.what());
            continue;
        }
        catch (...) {
            Fetcher::err->error("Fetcher: genesis something wrong ...");
            continue;
        }

        break;
    }
}

Task::Task(const std::string &name, size_t size)
        :name_(name),
         queue_(size),
         is_active_(false)
{}

bool Task::is_running() const {
    return queue_.is_running();
}

void Task::async(const dispatch::function &op) {
    queue_.async([=]{
        this->is_active_ = true;
        op();
        this->is_active_ = false;
    });
}