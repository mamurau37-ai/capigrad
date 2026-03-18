#ifndef DATA_HPP
#define DATA_HPP

#pragma once
#include <vector>
#include <string>

// GCC warningni o'chirish
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"

// MANA SHU Candle STRUKTURASI ENG MUHIMI
struct Candle {
    long long timestamp=0;
    double open=0,high=0,low=0,close=0,volume=0;
};

#pragma GCC diagnostic pop

std::vector<Candle> load_csv(const std::string& fn);

#endif // DATA_HPP
// Yangi struktura: Live orderbook va meta ma'lumotlar uchun
struct LiveTick {
    long long ts;
    double price;
    double spread_bps;
    double imb;
    double ti60;
    double cvd60;
    double cls_up;
    double cls_dn;
    double cls_fl;
    double clarity;
    std::string regime;
    std::string gate;
    std::string action;
    double meta_prob;
};

// Yangi funksiya e'loni
std::vector<LiveTick> load_live_log(const std::string& fn);