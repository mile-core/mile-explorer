//
// Created by lotus mile on 31/10/2018.
//

#pragma once

#include <time.h>
#include <time.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace milecsa::explorer{
    int random(int min, int max);

    /**
     *  Converts UTC time string to a time_t value.
     *  return tm struct
     *  */
    std::time_t getEpochTime(const std::string& dateTime);

    /**
     *  Converts UTC time string to a tm struct.
     *  return tm struct, tm_year = tm_year+1900
     *  */
    std::tm getTime(const std::string& dateTime, bool adjust_year = true);

    std::string utcToString(std::time_t t);
};
