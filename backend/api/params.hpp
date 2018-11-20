//
// Created by lotus mile on 03/11/2018.
//

#pragma once

#include "jsonrpc/context.hpp"
#include "jsonrpc/router.hpp"

namespace milecsa::explorer {

    namespace api::params {

        using namespace milecsa::rpc::server;

        constexpr const char *id = "id";
        constexpr const char *limit = "limit";
        constexpr const char *first = "first";
        constexpr const char *public_key = "public-key";
        constexpr const char *block_id = "block-id";
        constexpr const char *digest = "digest";

        inline auto error(context &ctx, const char *param) -> void {
            return make_response_parse_error(ctx, ErrorFormat("param %s has not found", param));
        }

        inline bool check(context &ctx, const char *param) {
            if (ctx.request.params.count(param) == 0) {
                api::params::error(ctx,param);
                return false;
            }
            return true;
        }

        inline bool check_limit(context &ctx) {
            if (!api::params::check(ctx, api::params::first)) return false;
            if (!api::params::check(ctx, api::params::limit)) return false;

            auto limit = ctx.request.params.at(api::params::limit).get<uint64_t>();

            if (limit>1000) {
                make_response_parse_error(ctx, "limit params must be less then 1000");
                return false;
            }

            return true;
        }

        static inline uint256_t get_block_id(context &ctx, const char *param_name) {

            if (!api::params::check(ctx, param_name)) return (uint256_t)(-1);

            auto param = ctx.request.params.at(param_name);

            if (param.is_number()) {
                return param.get<std::uint64_t>();
            }

            auto id = param.get<std::string>();
            uint256_t block_id;

            if(!StringToUInt256(id, block_id, false)){
                make_response_parse_error(ctx, "block couldn't be converted to uint256");
                return uint256_t(-1);
            }
            return block_id;
        }

        static inline uint64_t get_transaction_id(context &ctx, const char *param_name) {
            auto param = ctx.request.params.at(param_name);

            if (param.is_number()) {
                return param.get<std::uint64_t>();
            }
            else {
                return std::stoull(param.get<std::string>());
            }

        }
    }
}
