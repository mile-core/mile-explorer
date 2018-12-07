//
// Created by lotus mile on 2018-12-05.
//

#include "statistics.hpp"
#include "utils.hpp"

static auto method = [](const ctxDb &db, time_t last) {

    time_t first = last - 86400;

    db::Data items = db->open_table(table::name::blocks)
            ->cursor()
            .between(first, last, "timestamp")
            .field("transactions")
            .get_data();


    long double xdr  = 0;
    long double mile = 0;
    uint64_t count_xdr = 0;
    uint64_t count_mile = 0;

    uint64_t count = 0;
    for (auto &item: items) {
        if (item.is_array()) {
            count += enumerate_assets(
                    item,
                    [&](double amount, time_t) {
                        xdr += amount;
                        count_xdr++;
                    },
                    [&](double amount, time_t) {
                        mile += amount;
                        count_mile++;
                    });
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
};

MILECSA_REGESTRY_STAT_METHOD("turnover24",method);
