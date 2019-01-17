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

static auto new_method = [](const ctxDb &db, time_t last) {

    if (!db->has_table(table_name))
        db->create_table(table_name);

    auto table = db->open_table(table_name);

    if (!table->has_index("position"))
        table->create_index("position");

    int each_type_amount = 1024;

    auto xdr_code = std::to_string(milecsa::assets::XDR.code);
    auto mile_code =std::to_string(milecsa::assets::MILE.code);

    auto get_raw_object = [](db::Driver::Var ff) {
        return (*ff)["balance"]
        .has_fields("code", "amount")
        .map([&ff](db::Driver::Var ss) {
            return db::Driver::object(
            "code", (*ss)["code"],
            "amount", (*ss)["amount"],
            "position", 1,
            "id", (*ff)["id"]);
        });
    };

    auto position_increaser = [](db::Driver::Var acc, db::Driver::Var row){
        return (*acc).add(1);
    };

    auto get_resulted_object = [](db::Driver::Var acc, db::Driver::Var row, db::Driver::Var new_acc){
        return db::Driver::array(db::Driver::object(
        "public-key", (*row)["id"],
        "amount", (*row)["amount"],
        "asset-code", (*row)["code"].coerce_to("NUMBER"),
        "position", (*acc),
        "id", (*acc).coerce_to("STRING").add(":").add((*row)["code"])
        ));
    };

    db::Data items = db->open_table(table::name::wallets_state)
    ->cursor()
    .filter(db::Driver::row.has_fields("balance")
            .and_(db::Driver::row["balance"]
                  .type_of().eq("ARRAY")))
    .concatMap(get_raw_object)
    .filter(db::Driver::row["code"].eq(xdr_code).or_(db::Driver::row["code"].eq(mile_code)))
    .group("code")
    .order_by(db::Driver::desc(db::Driver::row["amount"].coerce_to("NUMBER")))
    .limit_func(each_type_amount)
    .fold(0, position_increaser, get_resulted_object)
    .ungroup()
    .concatMap(db::Driver::row["reduction"])
    .get_data();
    table->insert_with_replace(items);
};

//MILECSA_REGESTRY_STAT_METHOD("wallets_top",method);
MILECSA_REGESTRY_STAT_METHOD("wallets_top",new_method);
