//
// Created by lotus mile on 2018-12-12.
//

#define BOOST_TEST_MODULE genesis

#include "milecsa_http.hpp"
#include "genesis.hpp"
#include <boost/test/included/unit_test.hpp>

std::string url = "https://raw.githubusercontent.com/mile-core/mile-files/master/genesis_block.txt";

struct GenesisEval {

    using Url = milecsa::rpc::Url;

    milecsa::ErrorHandler errorHandler = [](milecsa::result code, std::string error){
        BOOST_TEST_MESSAGE("Http error: " + error);
    };

    bool test(const std::string &u = url) {

        if (auto client = milecsa::http::Client::Connect(u, true, errorHandler)) {
            if (auto body = client->get()) {
                auto genesis = milecsa::explorer::Genesis::Parser(body.value());
                std::cerr << genesis->get_transactions().dump(4);
                return true;
            }
        }

        return false;
    }
};


BOOST_FIXTURE_TEST_CASE( http, GenesisEval ){
    BOOST_CHECK(test());
}