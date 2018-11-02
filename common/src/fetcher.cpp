//
// Created by lotus mile on 29/10/2018.
//

#include "fetcher.hpp"
#include "db.hpp"
#include "utils.hpp"

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
    return milecsa::rpc::Client::Connect(
            urls_[random(0,urls_.size()-1)],
            true,
            Fetcher::response_fail_handler,
            Fetcher::error_handler);
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
         block_processing_task_(new Task("block-processing", 1)),  // 1 db connection at time
         rpc_fetching_block_task_(new Task("rpc-fetching-block", config::rpc_queue_size)),
         utility_task_(new Task("utility-processing", 2))
{
    srand(time(0));
}

void Fetcher::run(uint256_t block_id) {

    Fetcher::log->info("Fetcher: starting ...");

    main_fetching_task_->async([this, block_id]{

        uint256_t first = block_id;

        while(main_fetching_task_->is_running()) {

            auto client = this->get_rpc();

            if (!client){
                Logger::err->warn("Fetcher: main request failed, retrying}");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            //
            // prepare fetching block
            //
            optional<uint256_t> next_block_id = client->get_current_block_id();

            if (next_block_id) {
                uint256_t last = *next_block_id;

                Fetcher::log->trace(
                        "Getting block id from {} to {}, fetcher queue state: {}",
                        UInt256ToDecString(first),
                        UInt256ToDecString(last),
                        this->block_fetcher_task_->is_active());

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

            std::this_thread::sleep_for(std::chrono::seconds(this->update_timeout_));
        }
    });

    //
    // Utility...
    //
    if (!this->utility_task_->is_active()) {
        utility_task_->async([this] {
            Fetcher::log->info("Fetcher: utility fetching started ...");
            this->fetch_states();
        });
    }

    Fetcher::log->info("Fetcher: ... started");
}

void Fetcher::fetch_blocks(uint256_t from, uint256_t to) {

    Fetcher::log->info("Fetcher: pocessing blocks: [{}:{}] is in progress ...",
            UInt256ToDecString(from), UInt256ToDecString(to));

    for (auto i = from; i < to; i++){

        rpc_fetching_block_task_->async([i, from, to, this]{

            milecsa::rpc::response response;
            db::Data               block;

            auto tick = boost::posix_time::microsec_clock::local_time();

            std::string cu;

            while (!response) {

                try{
                    auto client = this->get_rpc();

                    if (!client){
                        Fetcher::err->warn("Fetcher: block getting request failed, retrying");
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        return;
                    }

                    cu = client->get_url().get_host();

                    response = client->get_block(i);

                    if (response->count("result")==0){
                        Fetcher::err->warn("Fetcher: block getting nothing result, retrying");
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        continue;
                    }

                    if (response->at("result").count("block-data")==0){
                        Fetcher::err->warn("Fetcher: block getting not data, retrying");
                        continue;
                    }

                    block = response->at("result")["block-data"];

                }
                catch (std::exception &e) {
                    Fetcher::err->error("Fetcher: {} error fetching block id {}", " = ", e.what());
                    std::this_thread::sleep_for(std::chrono::milliseconds(this->update_timeout_));
                }
                catch (...) {
                    Fetcher::err->error("Fetcher: something wrong ...");
                    std::this_thread::sleep_for(std::chrono::milliseconds(this->update_timeout_));
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

    while(utility_task_->is_running()) {

        auto client = this->get_rpc();

        if (!client){
            Logger::err->warn("Fetcher: utility request failed, retrying");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        optional<uint256_t> next_block_id = client->get_current_block_id();

        if (!next_block_id)
            continue;

        auto nodes = client->get_nodes();

        if(!nodes || *next_block_id<=block_id)
            continue;

        block_id = *next_block_id;

        if(nodes->count("result")>0){
            this->get_db()->add_nodes_state(nodes->at("result"), block_id);
        }

        std::this_thread::sleep_for(std::chrono::seconds(this->update_timeout_));
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