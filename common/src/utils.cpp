//
// Created by lotus mile on 31/10/2018.
//

#include "utils.hpp"
#include "config.hpp"
#include "fetcher.hpp"
#include <milecsa_jsonrpc.hpp>

namespace milecsa::explorer {

    std::tm getTime(const std::string& dateTime, bool adjust_year){

        static const std::string dateTimeFormat{ "%Y-%h-%d %H:%M:%S %Z" };
        std::istringstream ss{ dateTime };
        std::tm dt;
        ss >> std::get_time(&dt, dateTimeFormat.c_str());
        if (adjust_year)
            dt.tm_year += 1900;
        return dt;
    }

    std::time_t getEpochTime(const std::string& dateTime)
    {
        std::tm dt = milecsa::explorer::getTime(dateTime, false);
        return timegm(&dt);
    }

    std::string utcToString(std::time_t t)
    {
        std::ostringstream ss;
        ss << std::put_time(std::gmtime(&t), "%Y-%h-%d %H:%m:%S %Z");
        return ss.str();
    }

    int random(int min, int max){
        static bool first = true;
        if (first)
        {
            srand( time(0) );
            first = false;
        }
        return min + rand() % (( max + 1 ) - min);
    }

    std::optional<milecsa::rpc::Client> get_rpc(const std::vector<std::string> &urls) {

        static uint64_t counter = 0;

        auto url = urls[counter++ % urls.size()];

        auto c = milecsa::rpc::Client::Connect(
                url,
                true,
                milecsa::explorer::Fetcher::response_fail_handler,
                milecsa::explorer::Fetcher::error_handler);

        return std::move(c);
    }
}
