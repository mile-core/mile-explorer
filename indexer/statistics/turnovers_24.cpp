//
// Created by lotus mile on 2018-12-05.
//

#include "statistics.hpp"

static auto method = [](const ctxDb &db) {

    exec_statistic("turnover24", db, [](const ctxDb &db){

        auto state = db->get_block_history_state();
        time_t last = state["timestamp"];
        time_t first = last - 86400;

        db::Data items = db->open_table(table::name::blocks)
                ->cursor()
                .between(first, last, "timestamp")
                .field("transactions")
                .get_data();


        long double xdr  = 0;
        long double mile = 0;
        uint64_t count = 0;
        uint64_t count_xdr = 0;
        uint64_t count_mile = 0;
        for (auto &item: items) {
            if (item.is_array()) {
                for (auto &trx: item) {
                    if (trx.count("asset")){
                        for(auto &asset: trx["asset"]){
                            std::string code = asset["code"];
                            std::string amount = asset["amount"];
                            unsigned short asset_code = (unsigned short )std::stoi(code);
                            if (asset_code == milecsa::assets::XDR.code){
                                xdr +=  std::stold(amount);
                                count_xdr ++;
                            }
                            else if (asset_code == milecsa::assets::MILE.code){
                                mile += std::stold(amount);
                                count_mile ++;
                            }
                            count++;
                        }
                    }
                }
            }
        }

        db::Data trnv = {
                {"id", "turnovers-24"},
                {"count", count},
                {"assets", {
                               {{"code",std::to_string(milecsa::assets::XDR.code)},{"amount",xdr},{"count",count_xdr}},
                               {{"code",std::to_string(milecsa::assets::MILE.code)},{"amount",mile},{"count",count_mile}},
                       }}
        };
        db->open_table(table::name::turnovers)->update(trnv);
    });
};

MILECSA_REGESTRY_STAT_METHOD("turnover24",method);
