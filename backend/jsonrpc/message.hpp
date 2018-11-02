//
// Created by lotus mile on 02/11/2018.
//

#pragma once

#include <boost/optional.hpp>
#include "json.hpp"

namespace milecsa::rpc::server {

    namespace message {

        struct request_message final {
            request_message() {}

            request_message(std::string id, std::string method,
                            nlohmann::json params)
                    : id(std::move(id)),
                      method(std::move(method)),
                      params(std::move(params)) {}

            request_message(int id, std::string method,
                            nlohmann::json params = nlohmann::json())
                    : id(double(id)), method(std::move(method)), params(std::move(params)) {}

            nlohmann::json id;
            std::string method;
            nlohmann::json params;
        };

        struct response_error final {
            int code;
            std::string message;
            nlohmann::json data;

            response_error(int code, std::string message)
                    : code(code), message(std::move(message)) {}

            enum error_code {
                ParseError = -32700,
                InvalidRequest = -32600,
                MethodNotFound = -32601,
                InvalidParams = -32602,
                InternalError = -32603,
                serverErrorStart = -32099,
                serverErrorEnd = -32000,
                ServerNotInitialized,
                UnknownErrorCode = -32001
            };
        };

        struct response_message final {
            response_message() {}

            response_message(std::string id, nlohmann::json result = nlohmann::json())
                    : id(std::move(id)), result(std::move(result)) {}

            response_message(int id, nlohmann::json result = nlohmann::json())
                    : id(double(id)), result(std::move(result)) {}

            nlohmann::json id;
            nlohmann::json result;
            boost::optional<response_error> error;
        };

        struct notify_message final {
            notify_message(std::string method, nlohmann::json params = nlohmann::json())
                    : method(std::move(method)), params(std::move(params)) {}

            std::string method;
            nlohmann::json params;
        };

        inline bool contains(const nlohmann::json &msg,const std::string&key){
            return msg.find(key) != msg.end();
        }

        inline bool is_notify(const nlohmann::json &msg) {
            return msg.is_object() && !contains(msg,"id");
        }

        ////TODO FIX
        inline bool is_request(const nlohmann::json &msg) {
            return msg.is_object() /*&& contains(msg,"method")*/ && msg.at("method").is_string();
        }

        inline bool is_response(const nlohmann::json &msg) {
            return msg.is_object() && !contains(msg,"method") && contains(msg,"id");
        }

        inline bool parse(const nlohmann::json &message, request_message &request) {
            if (!is_request(message)) {
                return false;
            }
            request.id = message.at("id");

            request.method = message.at("method").get<std::string>();
            if (contains(message,"param")) {
                request.params = message["param"];
            } else {
                request.params = message["params"];
            }
            return true;
        }

        inline bool parse(const nlohmann::json &message, notify_message &notify) {
            if (!is_notify(message)) {
                return false;
            }
            notify.method = message.at("method").get<std::string>();
            if (contains(message,"param")) {
                notify.params = message.at("param");
            } else {
                notify.params = message.at("params");
            }
            return true;
        }

        inline bool parse(const nlohmann::json &message, response_message &response) {
            if (!is_response(message)) {
                return false;
            }
            response.id = message.at("id");
            response.result = message.at("result");
            return true;
        }

        inline std::string serialize(const request_message &msg) {
            nlohmann::json obj;
            obj["jsonrpc"] = nlohmann::json("2.0");

            obj["method"] = nlohmann::json(msg.method);
            if (!msg.params.is_null()) {
                obj["params"] = msg.params;
            }
            obj["id"] = msg.id;
            return nlohmann::json(obj).dump();
        }

        inline std::string serialize(const response_message &msg) {
            nlohmann::json obj;
            obj["jsonrpc"] = nlohmann::json("2.0");

            obj["result"] = msg.result;

            if (msg.error) {
                nlohmann::json error = {{"code",    nlohmann::json(double(msg.error->code))},
                                        {"message", nlohmann::json(msg.error->message)},
                                        {"data",    nlohmann::json(msg.error->data)}};
                obj["error"] = nlohmann::json(error);
            }

            obj["id"] = msg.id;
            return nlohmann::json(obj).dump();
        }

        inline std::string serialize(const notify_message &msg) {
            nlohmann::json obj;
            obj["jsonrpc"] = nlohmann::json("2.0");

            obj["method"] = nlohmann::json(msg.method);
            if (!msg.params.is_null()) {
                obj["params"] = msg.params;
            }
            return nlohmann::json(obj).dump();
        }

        inline const std::string &get_method(const nlohmann::json &msg) {
            static std::string method_tmp;
            if (!msg.is_object() || !contains(msg,"method")) {
                return method_tmp;
            }
            const nlohmann::json &method = msg.at("method");
            if (!method.is_string()) {
                return method_tmp;
            }

            method_tmp = method.get<std::string>();

            return method_tmp;
        }

        inline const nlohmann::json &get_param(const nlohmann::json &msg) {
            static nlohmann::json null;
            if (!msg.is_object() || !contains(msg,"params")) {
                return null;
            }
            return msg.at("params");
        }

        inline const nlohmann::json &get_id(const nlohmann::json &msg) {
            static nlohmann::json null;
            if (!msg.is_object() || !contains(msg,"id")) {
                return null;
            }
            return msg.at("id");
        }

        namespace request {
            inline std::string serialize(const nlohmann::json &id, const std::string &method,
                                         const nlohmann::json &param = nlohmann::json()) {
                nlohmann::json obj;
                obj["method"] = nlohmann::json(method);
                if (!param.is_null()) {
                    obj["params"] = param;
                }
                obj["id"] = id;
                return nlohmann::json(obj).dump();
            }

            inline std::string serialize(const nlohmann::json &id, const std::string &method,
                                         const std::string &param) {
                return serialize(id, method, nlohmann::json(param));
            }

            inline std::string serialize(double id, const std::string &method,
                                         const std::string &param) {
                return serialize(nlohmann::json(id), method, nlohmann::json(param));
            }

            inline std::string serialize(double id, const std::string &method,
                                         const nlohmann::json &param = nlohmann::json()) {
                return serialize(nlohmann::json(id), method, nlohmann::json(param));
            }

            inline std::string serialize(const std::string &id, const std::string &method,
                                         const std::string &param) {
                return serialize(nlohmann::json(id), method, nlohmann::json(param));
            }

            inline std::string serialize(const std::string &id, const std::string &method,
                                         const nlohmann::json &param = nlohmann::json()) {
                return serialize(nlohmann::json(id), method, nlohmann::json(param));
            }
        }
        namespace notify {
            inline std::string serialize(const std::string &method,
                                         const nlohmann::json &param = nlohmann::json()) {
                nlohmann::json obj;
                obj["method"] = nlohmann::json(method);
                if (!param.is_null()) {
                    obj["params"] = param;
                }
                return nlohmann::json(obj).dump();
            }

            inline std::string serialize(const std::string &method,
                                         const std::string &param) {
                return serialize(method, nlohmann::json(param));
            }
        }
        namespace responce {
            inline std::string serialize(const nlohmann::json &id,
                                         const nlohmann::json &result = nlohmann::json(),
                                         bool error = false) {
                nlohmann::json obj;
                if (error) {
                    obj["error"] = result;
                } else {
                    obj["result"] = result;
                }
                obj["id"] = id;
                return nlohmann::json (obj).dump();
            }

            inline std::string serialize(const nlohmann::json &id, const std::string &result,
                                         bool error = false) {
                return serialize(id, nlohmann::json(result), error);
            }
        }
    }

    using message::request_message;
    using message::response_message;
    using message::notify_message;
    using message::response_error;
}