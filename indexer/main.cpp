#include <iostream>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <boost/program_options.hpp>

#include "fetcher.hpp"
#include "db.hpp"

static std::string opt_config_file = "";

namespace po = boost::program_options;
using namespace milecsa::explorer;

static bool parse_cmdline(int ac, char *av[]);

int main(int argc, char *argv[]) {

    setlocale(LC_ALL, "");

    if (!parse_cmdline(argc, argv))
        return -1;

    auto db = milecsa::explorer::Db::Open();

    if (!db)
        exit(-1);

    db->init();

    auto fetcher = milecsa::explorer::Fetcher::Connect(
            milecsa::explorer::config::node_urls,
            milecsa::explorer::config::update_timeout,
            db);

    if (!fetcher)
        exit(-1);

    dispatch::Default::async([&]{
        fetcher->run(db->get_start_block_id());
    });

    dispatch::Default::loop::run();

    return EXIT_SUCCESS;
}

static bool parse_cmdline(int ac, char *av[]) {

    try {

        spdlog::set_level(spdlog::level::debug);

        po::options_description desc("Allowed options");

        std::string wallet_phrase="";

        desc.add_options()

                ("help", "produce help message")

                ("config,c", po::value<std::string>(&opt_config_file),
                 "config file");

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }

        if (opt_config_file.empty()) {
            std::cout << desc << "\n";
            exit(-1);
        }

        if (access(opt_config_file.c_str(), R_OK) !=0 ){
            Logger::err->critical("Configure: configure file {} not found", opt_config_file);
            exit(-1);
        }

        YAML::Node config_nodes = YAML::LoadFile(opt_config_file);

        YAML::Node urls = config_nodes["node_urls"];

        config::node_urls.clear();
        for (size_t i=0; i<urls.size(); i++) {
            auto url = urls[i].as<string>();
            config::node_urls.push_back(url);
            Logger::log->trace("Configure: add node: {}", url);
        }

        config::request_timeout  = config_nodes["request_timeout"].as<int>();
        config::update_timeout   = config_nodes["update_timeout"].as<int>();
        config::rpc_queue_size   = config_nodes["rpc_queue_size"].as<int>();
        config::block_processin_queue_size =  config_nodes["block_processin_queue_size"].as<int>();

        config::db_host = config_nodes["db_host"].as<string>();
        config::db_port = config_nodes["db_port"].as<unsigned short>();
        config::db_name = config_nodes["db_name"].as<string>();

        milecsa::rpc::detail::RpcSession::debug_on = config_nodes["json_rpc_debug"].as<bool>();

        spdlog::level::level_enum level = spdlog::level::level_enum(config_nodes["log_level"].as<int>());
        spdlog::set_level(level);

    }
    catch (std::exception &e) {
        Logger::err->critical("Configure: error {}", e.what());
        return false;
    }
    catch (...) {
        Logger::err->critical("Exception of unknown type!");
        return false;
    }

    return true;
}
