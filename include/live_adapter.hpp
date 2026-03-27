#ifndef CAPIGRAD_LIVE_ADAPTER_HPP
#define CAPIGRAD_LIVE_ADAPTER_HPP

#include <string>
#include "live_engine.hpp"

namespace capigrad {

// Raw text line -> LiveEvent parser
// Supported formats:
//
// Trade:
//   T,<ts>,<price>,<qty>,<m>
//   where m = 1 if market maker was seller side (Binance aggTrade style)
//   We convert this into is_buy = !m
//
// Orderbook:
//   D,<ts>,bid1_p,bid1_q,...,bid10_p,bid10_q,ask1_p,ask1_q,...,ask10_p,ask10_q
//
class LiveAdapter {
public:
    LiveAdapter() = default;

    // Returns true if line parsed successfully into ev
    bool parse_line(const std::string& line, LiveEvent& ev) const;

private:
    bool parse_trade(const std::string& line, LiveEvent& ev) const;
    bool parse_orderbook(const std::string& line, LiveEvent& ev) const;
};

} // namespace capigrad

#endif // CAPIGRAD_LIVE_ADAPTER_HPP
