#ifndef DATA_HPP
#define DATA_HPP

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