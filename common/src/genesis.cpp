//
// Created by lotus mile on 2018-12-12.
//

#include "genesis.hpp"
#include "db.hpp"
#include <boost/algorithm/string.hpp>
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

namespace milecsa::explorer {

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    std::optional<Genesis> Genesis::Parser(const std::string &buffer) {

        try {
            boost::char_separator<char> line("\n");
            boost::char_separator<char> space(" ");

            tokenizer lines(buffer, line);
            auto g = Genesis();

            uint64_t trx_id = 0;

            BOOST_FOREACH(std::string const &token, lines) {

                            tokenizer tline(token, space);
                            std::string transaction_type;
                            milecsa::token asset = milecsa::assets::NILL;
                            std::string pk;
                            std::string amount;
                            std::string address;

                            int i = 0;
                            db::Data trx;

                            BOOST_FOREACH(std::string const &t, tline) {

                                            std::string word = boost::to_lower_copy(t);

                                            if (word == "transfer")
                                                transaction_type = "TransferAssetsTransaction";

                                            else if (word == "amount-register-node")
                                                transaction_type = "RegisterNodeTransactionWithAmount";

                                            else if (word == "xdr")
                                                asset = milecsa::assets::XDR;

                                            else if (word == "mile")
                                                asset = milecsa::assets::MILE;

                                            else if (milecsa::keys::Pair::ValidatePublicKey(t))
                                                pk = t;

                                            if (transaction_type == "RegisterNodeTransactionWithAmount" && i == 2)
                                                address = t;

                                            if (i == 3)
                                                amount = asset.value_to_string(std::stof(t));

                                            i++;
                                        }

                            if (transaction_type == "RegisterNodeTransactionWithAmount") {

                                trx = {
                                        {"address",          address},
                                        {"amount",           milecsa::assets::XDR.value_to_string(std::stof(amount))},
                                        {"digest",           ""},
                                        {"public-key",       pk},
                                        {"signature",        ""},
                                        {"transaction-id",   std::to_string(trx_id)},
                                        {"transaction-type", transaction_type}
                                };

                            } else if (transaction_type == "TransferAssetsTransaction") {
                                trx = {
                                        {"asset",
                                         {{"amount", asset.value_to_string(std::stof(amount))},
                                          {"code", std::to_string(asset.code)}}},
                                        {"description",      ""},
                                        {"digest",           pk},
                                        {"fee",              asset.value_to_string(0.0f)},
                                        {"public-key",       pk},
                                        {"signature",        ""},
                                        {"transaction-id",   std::to_string(trx_id)},
                                        {"transaction-type", transaction_type},
                                };
                            }

                            trx_id++;

                            g.transactions_.push_back(trx);

                        }
            return g;
        }
        catch (...) {
            return std::nullopt;
        }
    }

}