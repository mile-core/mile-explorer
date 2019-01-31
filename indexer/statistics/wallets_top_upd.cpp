#include "statistics.hpp"

namespace wallets_top_upd {
    static std::string table_name = "wallets_top";

    static auto method = [](const ctxDb &db, time_t last) {

        if (!db->has_table(table_name))
            db->create_table(table_name);

        auto table = db->open_table(table_name);

        if (!table->has_index("position"))
            table->create_index("position");

        int each_type_amount = 1024;

        auto xdr_code = std::to_string(milecsa::assets::XDR.code);
        auto mile_code = std::to_string(milecsa::assets::MILE.code);

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

        auto position_increaser = [](db::Driver::Var acc, db::Driver::Var row) {
            return (*acc).add(1);
        };

        auto get_resulted_object = [](db::Driver::Var acc, db::Driver::Var row, db::Driver::Var new_acc) {
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
                .fold(0, position_increaser, db::Driver::optargs("emit", get_resulted_object))
                .ungroup()
                .concatMap(db::Driver::row["reduction"])
                .get_data();
        table->insert_with_replace(items);
    };
    MILECSA_REGESTRY_STAT_METHOD("wallets_top_upd", method);
}