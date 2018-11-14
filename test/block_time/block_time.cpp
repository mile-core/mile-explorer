//
// Created by lotus mile on 14/11/2018.
//


#define BOOST_TEST_MODULE block_time

#include "utils.hpp"
#include <boost/test/included/unit_test.hpp>

// 2018-Nov-13 07:01:20 => %Y-%h-%d %H:%m:%S

using namespace std;
BOOST_AUTO_TEST_CASE(block_time){

    setlocale(LC_ALL, "");

    std::string timestamp = "2018-Nov-13 07:01:20 UTC";
    std::tm ltm = milecsa::explorer::getTime(timestamp);
    std::time_t t = milecsa::explorer::getEpochTime(timestamp);

    std::cout << "UTC:       " << milecsa::explorer::utcToString(t) << '\n';
    std::cout << "LOC:       " << std::put_time(std::localtime(&t), "%Y-%h-%d %H:%m:%S %Z") << '\n';

    std::cout << "TIM:       " << t << '\n';
    ltm.tm_year -= 1900;
    std::cout << "TIM:       " << timegm(&ltm) << '\n';

    cout << "Year  : "<< ltm.tm_year<<endl;
    cout << "Month : "<< ltm.tm_mon<< endl;
    cout << "Day   : "<< ltm.tm_mday << endl;
    cout << "Time  : "<< ltm.tm_hour << ":";
    cout << ltm.tm_min << ":";
    cout << ltm.tm_sec << endl;

    cout << "Z    : "<< ltm.tm_gmtoff<<endl;

    std::time_t lt = std::time(nullptr);
    std::tm *gtm = std::gmtime(&lt);

    cout << "UTC  : "<< lt << "  :  " << timegm(gtm) <<endl;

    BOOST_CHECK(timegm(&ltm)==t);
}

