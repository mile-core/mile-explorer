//
// Created by lotus mile on 2018-12-07.
//

#include "statistics.hpp"
#include "utils.hpp"

static auto method = [](const ctxDb &db, time_t last) {

    static time_t day = 86400;
    static time_t two_week = day * 14;

    time_t first = last - two_week;

    auto xdr_weeks = interval_map<time_t,std::pair<double,uint64_t>>(first, last, day);
    auto mile_weeks = interval_map<time_t,std::pair<double,uint64_t>>(first, last, day);

    db::Data items = db->open_table(table::name::transactions)
            ->cursor()
            .between(first, last, "timestamp")
            .get_data();

    uint64_t count = enumerate_assets(
            items,
            [&](double amount, time_t t){
                xdr_weeks[t].first = xdr_weeks[t].first + amount;
                xdr_weeks[t].second++;
            },
            [&](double amount, time_t t){
                mile_weeks[t].first = mile_weeks[t].first + amount;
                mile_weeks[t].second++;
            });

    db::Data xdr_trx;
    db::Data mile_trx;

    auto trx_update = [](
            interval_map<time_t, std::pair<double,uint64_t>> &week,
            db::Data &trx){

        for (auto &[key, value]: week) {
            trx.push_back({
                                       {"begin",  key.first},
                                       {"end",    key.second},
                                       {"amount", value.first},
                                       {"count",  value.second},
                               });
        }

    };

    trx_update(xdr_weeks,xdr_trx);
    trx_update(mile_weeks,mile_trx);

    db::Data result  = {{"id", "transaction-turnover"},
                        {"assets", {
                                           {std::to_string(milecsa::assets::XDR.code), xdr_trx},
                                           {std::to_string(milecsa::assets::MILE.code), mile_trx},
                                   }
                        }
    };

    db->open_table(table::name::transaction_turnover)->update(result);

};

MILECSA_REGESTRY_STAT_METHOD("transaction_turnover",method);
