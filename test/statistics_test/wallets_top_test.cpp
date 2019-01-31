#define BOOST_TEST_MODULE wallets_top_test
#include <boost/test/included/unit_test.hpp>

#include "statistics.hpp"
#include "milecsa.hpp"
#include "../../indexer/statistics/wallets_top.cpp"
#include "../../indexer/statistics/wallets_top_upd.cpp"

#include <chrono>
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;

BOOST_AUTO_TEST_CASE(wallets_top_test){
    auto db = milecsa::explorer::Db::Open();
    auto t0 = Time::now();
    wallets_top_legacy::method(db, 0);
    auto t1 = Time::now();

    fsec fs = t1 - t0;
    ms d = std::chrono::duration_cast<ms>(fs);
    std::cout << "TIME "<< std::endl;
    std::cout << fs.count() << std::endl;
    std::cout << d.count() << std::endl;
}

BOOST_AUTO_TEST_CASE(wallets_top_upd_test){
    auto db = milecsa::explorer::Db::Open();
    auto t0 = Time::now();
    method(db, 0);
    auto t1 = Time::now();

    fsec fs = t1 - t0;
    ms d = std::chrono::duration_cast<ms>(fs);
    std::cout << "TIME "<< std::endl;
    std::cout << fs.count() << std::endl;
    std::cout << d.count() << std::endl;
}