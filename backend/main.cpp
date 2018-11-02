//
// Created by lotus mile on 02/11/2018.
//

#include <iostream>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <boost/program_options.hpp>
#include <milecsa_queue.h>

#include <boost/asio/ssl/stream.hpp>
#include <boost/program_options.hpp>

#include "db.hpp"
#include "jsonrpc/rpc.hpp"
#include "http/listener.hpp"
#include "http/session.hpp"

static std::string opt_config_file = "";

namespace po = boost::program_options;
using namespace milecsa::explorer;
using namespace milecsa::rpc;

static bool parse_cmdline(int ac, char *av[]);

int main(int argc, char *argv[]) {

    setlocale(LC_ALL, "");

    if (!parse_cmdline(argc, argv))
        return -1;

    auto db = milecsa::explorer::Db::Open();

    if (!db)
        exit(-1);

    auto echo = [&](server::context &ctx) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1234));
            ctx.response.result = true;
    };

    auto router = server::router::Create("v1", "api");

    router->add("ping", echo);

    boost::asio::io_context ioc{10};

    auto address = boost::asio::ip::make_address(config::http_bind_address);

    std::make_shared<server::http::Listener>(
            ioc,
            boost::asio::ip::tcp::endpoint{address, config::http_port},
            router
    )->run();

    ioc.run();

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

        config::db_host = config_nodes["db_host"].as<string>();
        config::db_port = config_nodes["db_port"].as<unsigned short>();
        config::db_name = config_nodes["db_name"].as<string>();

        config::http_bind_address = config_nodes["http_bind_address"].as<string>();
        config::http_port = config_nodes["http_port"].as<unsigned short>();

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
