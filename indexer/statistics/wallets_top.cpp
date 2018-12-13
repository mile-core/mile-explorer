//
// Created by lotus mile on 2018-12-13.
//

#include "statistics.hpp"
#include "utils.hpp"
#include <algorithm>
#include <string>
#include <array>

template <typename T>
std::vector<T> mslice(const std::vector<T>& v, int start=0, int end=-1) {
    int oldlen = v.size();
    int newlen;

    if (end == -1 or end >= oldlen){
        newlen = oldlen-start;
    } else {
        newlen = end-start;
    }

    std::vector<T> nv(newlen);

    for (int i=0; i<newlen; i++) {
        nv[i] = v[start+i];
    }
    return nv;
}


static auto method = [](const ctxDb &db, time_t last) {

    static std::string xdr_table = "wallets_top_xdr";
    static std::string mile_table = "wallets_top_mile";

    if (!db->has_table(xdr_table))
        db->create_table(xdr_table);

    if (!db->has_table(mile_table))
        db->create_table(mile_table);

    auto table = db->open_table(xdr_table);
    if (!table->has_index("position"))
        table->create_index("position");

    table = db->open_table(mile_table);
    if (!table->has_index("position"))
        table->create_index("position");

    db::Data items = db->open_table(table::name::wallets_state)
            ->cursor()
            .get_data();

    auto filter = [&items](const milecsa::token& token, std::vector<std::pair<std::string,std::string>> &result) {

        std::for_each(items.begin(), items.end(), [&result,token](db::Data a) {
            if (a.count("balance") == 0)
                return;

            for (auto coin:  a["balance"]) {

                if (coin.count("code") == 0)
                    return;

                if (coin.count("amount") == 0)
                    return;

                auto x = coin["code"].get<std::string>();

                if (std::stoi(x) == token.code) {
                    result.push_back(std::pair<std::string, std::string>(a["id"], coin["amount"].get<std::string>()));
                }
            }
        });
    };

    std::vector<std::pair<std::string,std::string>> xdr;
    std::vector<std::pair<std::string,std::string>> mile;

    filter(milecsa::assets::XDR,xdr);
    filter(milecsa::assets::MILE,mile);

    std::sort(xdr.begin(), xdr.end(),
         [](const std::pair<std::string,std::string> & a, const std::pair<std::string,std::string> & b) -> bool
         {
             return std::stof(a.second)>std::stof(b.second);
         });

    std::sort(mile.begin(), mile.end(),
              [](const std::pair<std::string,std::string> & a, const std::pair<std::string,std::string> & b) -> bool
              {
                  return std::stof(a.second)>std::stof(b.second);
              });

    xdr  = mslice(xdr, 0, 512);
    mile = mslice(mile,0, 512);

    auto update = [&](std::vector<std::pair<std::string,std::string>> &slice, const std::string &table){
        uint64_t count = 0;

        for (auto &item: slice) {

            db::Data trx = {
                    {"id", std::to_string(count)},
                    {"position", count},
                    {"public-key", item.first},
                    {"amount", item.second}
            };

            db->open_table(table)->update(trx);

            count++;
        }
    };

    update(xdr,xdr_table);
    update(mile,mile_table);
};

MILECSA_REGESTRY_STAT_METHOD("wallets_top",method);
