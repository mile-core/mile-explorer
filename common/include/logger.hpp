//
// Created by lotus mile on 29/10/2018.
//

#pragma once

#include <milecsa.hpp>
#include <milecsa_jsonrpc.hpp>
#include "spdlog.h"
#include "sinks/stdout_color_sinks.h"
#include "config.hpp"

namespace milecsa::explorer {

    class Logger {
    public:
        /**
         * Loggers
         */

        const static std::shared_ptr<spdlog::logger> log;
        const static std::shared_ptr<spdlog::logger> err;
        const static milecsa::http::ResponseHandler response_fail_handler;
        const static milecsa::ErrorHandler error_handler;

    };
}

