//
// Created by lotus mile on 2018-12-13.
//

#include "statistics.hpp"
#include "utils.hpp"
#include <algorithm>
#include <string>
#include <array>

static std::string table_name = "wallets_top";
typedef std::pair<std::string,std::string>  mpair;
typedef std::vector<mpair> mvector;

template <typename T>
std::vector<T> mslice(std::vector<T> v, int start=0, int end=-1) {

    std::sort(v.begin(), v.end(),
              [](const T & a, const T & b) -> bool
              {
                  return std::stof(a.second)>std::stof(b.second);
              });

    auto it = std::unique(v.begin(), v.end(), [](const T& a, const T& b) -> bool
    {
        return a.first == b.first;
    });

    v.erase(it,v.end());

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

static void filter(
        const db::Data &items,
        mvector &xdr,
        mvector &mile) {

    std::for_each(items.begin(), items.end(), [&xdr,&mile](db::Data a) {
        if (a.count("balance") == 0)
            return;

        for (auto coin:  a["balance"]) {

            if (coin.count("code") == 0)
                return;

            if (coin.count("amount") == 0)
                return;

            auto x = coin["code"].get<std::string>();

            unsigned short code = std::stoi(x);
            auto id = a["id"].get<std::string>();
            auto amount = coin["amount"].get<std::string>();

            if ( code == milecsa::assets::XDR.code ) {
                xdr.push_back(mpair(id,amount));
            }
            else if ( code == milecsa::assets::MILE.code ) {
                mile.push_back(mpair(id,amount));
            }
        }
    });
};

static void update(
        const ctxDb &db,
        const milecsa::token& token,
        const mvector &slice){

    uint64_t count = 0;

    for (auto &item: slice) {

        std::string id = std::to_string(count);
        id.append(":");
        id.append(std::to_string(token.code));

        db::Data trx = {
                {"id", id},
                {"position", count},
                {"public-key", item.first},
                {"amount", item.second},
                {"asset-code", token.code}
        };

        db->open_table(table_name)->update(trx);

        count++;
    }
};


static auto method = [](const ctxDb &db, time_t last) {

    mvector xdr;
    mvector mile;

    if (!db->has_table(table_name))
        db->create_table(table_name);

    auto table = db->open_table(table_name);

    if (!table->has_index("position"))
        table->create_index("position");

    db::Data items = db->open_table(table::name::wallets_state)
            ->cursor()
            .get_data();

    filter(items, xdr, mile);

    xdr  = mslice(xdr, 0, 1024);
    mile = mslice(mile,0, 1024);

    update(db, milecsa::assets::XDR, xdr);
    update(db, milecsa::assets::MILE, mile);
};

MILECSA_REGESTRY_STAT_METHOD("wallets_top",method);

