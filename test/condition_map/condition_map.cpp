//
// Created by lotus mile on 2018-12-07.
//

#define BOOST_TEST_MODULE condition_test

#include <boost/test/included/unit_test.hpp>
#include "../../indexer/statistics/utils.hpp"

BOOST_AUTO_TEST_CASE(condition_test) {

    auto c = interval_map<time_t,double>(1542065422 - 86400 * 14, 1542065422, 86400);

    c.emplace(1541460622, 10);
    c.emplace(1541460623+1, 10);

    c[1541460622] += 11;
    c[1541460622+1] += 14;

    for (auto &[key, value]: c) {
        std::cout << "[" << key.first << ", "<< key.second << "] =  " << value << std::endl;
    }


}
