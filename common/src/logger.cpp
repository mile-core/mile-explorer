//
// Created by lotus mile on 29/10/2018.
//

#include "logger.hpp"

using namespace milecsa::explorer;
using namespace std;

const std::shared_ptr<spdlog::logger>  Logger::log = spdlog::stdout_color_mt("explorer log");
const std::shared_ptr<spdlog::logger>  Logger::err = spdlog::stderr_color_mt("explorer error");

const  milecsa::ErrorHandler  Logger::error_handler = [](milecsa::result code, const std::string &error){
    Logger::err->critical("[code:{}] {}", code, error.c_str());
};

const milecsa::http::ResponseHandler Logger::response_fail_handler = [](
        const milecsa::http::status code,
        const std::string &method,
        const milecsa::http::response &http){
    std::string  message = http.body().data();
    Logger::err->error("rpc response:[code:{}, method:{}] {}", (int)code, method, message);
};