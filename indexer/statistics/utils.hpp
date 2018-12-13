//
// Created by lotus mile on 2018-12-07.
//
#pragma once
#include "statistics.hpp"
#include "fetcher.hpp"
#include <map>

template <typename T>
struct interval: public std::pair<T,T> {
    interval(const T &l, const T &u) : std::pair<T, T>(l, u) {};
};

template <typename T>
bool operator<(const interval<T>& l, const interval<T>& r )
{
    return l.second <= r.first && ( l.first < r.first && l.second < r.second);
}

template <typename T, typename C>
struct interval_map: public std::map<interval<T>,C> {

    interval_map(const T& begin, const T& end, const T& step){
        for (T i = begin; i < end ; i += step) {
            auto c = interval<T>(i,i+step);
            (*this)[c] = C();
        }
    };

    C& emplace(const T& key, const C& value) {
        auto it = std::find_if(this->begin(), this->end(),
                               [key](const std::pair<interval<T>,C> & t) -> bool {
                                   return t.first.first <= key && t.first.second >= key;
                               }
        );

        (*this)[it->first] = value;

        return (*this)[it->first];
    }

    C& operator[]( const interval<T>& key ) {
        return std::map<interval<T>,C>::operator[](key);
    }

    C& operator[]( const T& key ) {
        auto it = std::find_if(this->begin(), this->end(),
                               [key](const std::pair<interval<T>,C> & t) -> bool {
                                   return t.first.first <= key && t.first.second >= key;
                               }
        );
        return (*this)[it->first];
    };
};

using asset_amount = std::function<void(double, time_t)>;
auto enumerate_assets = [](const db::Data &items, asset_amount&&xdr, asset_amount&&mile) {

    uint64_t count = 0;

    if (items.is_array()) {

        for (auto &trx: items) {

            time_t t = (time_t)-1;

            if (trx.count("timestamp"))
                t = trx["timestamp"].get<time_t>();

            if (trx.count("asset")){
                for(auto &asset: trx["asset"]){
                    std::string code = asset["code"];
                    std::string amount = asset["amount"];
                    unsigned short asset_code = (unsigned short )std::stoi(code);
                    if (asset_code == milecsa::assets::XDR.code){
                        xdr(std::stold(amount), t);
                    }
                    else if (asset_code == milecsa::assets::MILE.code){
                        mile(std::stold(amount), t);
                    }
                    count++;
                }
            }
        }
    }

    return count;
};
