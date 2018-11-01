//
// Created by lotus mile on 30/10/2018.
//

#include "config.hpp"
//#include <spdlog.h>

using namespace milecsa::explorer;
using namespace std;

string config::db_host = "localhost";
unsigned short  config::db_port = 28015;
string config::db_name = "blockchain";

vector<string> config::node_urls = {"http://138.197.75.176:8080/v1/api",
                                    "https://104.248.7.135:8080/v1/api",
                                    "https://167.99.186.253:8080/v1/api"};

int config::rpc_queue_size             = 20;
int config::block_processin_queue_size = 12;

time_t config::update_timeout   = 20;
time_t config::request_timeout  = 20;
