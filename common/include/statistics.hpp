//
// Created by denn on 2018-12-05.
//

#pragma once
#include "utils.hpp"
#include "db.hpp"
#include "table.hpp"
#include "names.hpp"

using ctxDb = std::optional<milecsa::explorer::Db>;

namespace milecsa::explorer {

    using statistic_method = std::function<void(const ctxDb&)>;

    auto exec_statistic = [](const std::string &name, const ctxDb &db, const statistic_method &method) {
        try {
            method(db);
            Logger::log->info("Statistic: {} processing done", name);
        }
        catch (db::Error &e) {
            Logger::err->error("Statistic: {} processing error {}", name, e.message);
        }
        catch(nlohmann::json::parse_error& e) {
            Logger::err->error("Statistic: {} parse json error {}", name, e.what());
        }
        catch(nlohmann::json::invalid_iterator& e){
            Logger::err->error("Statistic: {} invalid iterator error {}", name, e.what());
        } catch(nlohmann::json::type_error & e){
            Logger::err->error("Statistic: {} type error {}", name, e.what());
        } catch(nlohmann::json::out_of_range& e){
            Logger::err->error("Statistic: {} out of range error {}", name, e.what());
        } catch(nlohmann::json::other_error& e){
            Logger::err->error("Statistic: {} other error {}", name, e.what());
        }
        catch (...) {
            Logger::err->error("Statistic: {} turnovers processing unknown error ... ", name);
        }
    };

    namespace statistic {

        using storage_type = map<std::string,statistic_method>;

        struct registry {

            void set_error_handler(milecsa::ErrorHandler& error_handler);

            void error(const std::string &error) {
                if(error_handler_){
                    (*error_handler_)(milecsa::result::EXCEPTION, error);
                }
            }

            const storage_type &get_statistics() const { return storage_; }

            template <typename F>
            bool add(std::string name, F&&f){
                storage_.emplace(name, std::forward<F>(f));
                return true;
            }

        private:
            storage_type storage_;
            static std::optional<milecsa::ErrorHandler> error_handler_;
        };

        class Registry:public milecsa::explorer::Singleton<registry>
        {
            friend class milecsa::explorer::Singleton<registry>;
        private:
            Registry(){};
        };
    }
}

using namespace milecsa::explorer;

#define MILECSA_REGESTRY_STAT_METHOD(name, method) static auto v = milecsa::explorer::statistic::Registry::Instance().add((name),(method))