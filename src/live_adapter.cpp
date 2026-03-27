#include "live_adapter.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace capigrad {

namespace {

std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> out;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        out.push_back(item);
    }
    return out;
}

} // anonymous namespace

bool LiveAdapter::parse_line(const std::string& line, LiveEvent& ev) const {
    if (line.empty()) return false;
    if (line[0] == 'T') return parse_trade(line, ev);
    if (line[0] == 'D') return parse_orderbook(line, ev);
    return false;
}

bool LiveAdapter::parse_trade(const std::string& line, LiveEvent& ev) const {
    std::vector<std::string> f = split_csv(line);
    if (f.size() != 5) return false;

    try {
        // T, ts, price, qty, m
        std::int64_t ts = std::stoll(f[1]);
        double price    = std::stod(f[2]);
        double qty      = std::stod(f[3]);
        int m           = std::stoi(f[4]);

        TradeTick tr;
        tr.timestamp = ts;
        tr.price     = price;
        tr.size      = qty;

        // Binance aggTrade:
        // m=true means buyer was maker => aggressive side was sell
        tr.is_buy = (m == 0);

        ev = LiveEvent{};
        ev.type = LiveEventType::TRADE;
        ev.trade = tr;
        return true;
    }
    catch (...) {
        return false;
    }
}

bool LiveAdapter::parse_orderbook(const std::string& line, LiveEvent& ev) const {
    std::vector<std::string> f = split_csv(line);

    // D, ts, 10*(bid_p,bid_q), 10*(ask_p,ask_q)
    // => 1 + 1 + 20 + 20 = 42 fields
    if (f.size() != 42) return false;

    try {
        std::int64_t ts = std::stoll(f[1]);

        OrderBookSnapshot ob;
        ob.timestamp = ts;
        ob.bids.reserve(10);
        ob.asks.reserve(10);

        int idx = 2;

        for (int i = 0; i < 10; ++i) {
            double p = std::stod(f[idx++]);
            double q = std::stod(f[idx++]);
            if (p > 0.0 && q >= 0.0) {
                ob.bids.push_back(BookLevel{p, q});
            }
        }

        for (int i = 0; i < 10; ++i) {
            double p = std::stod(f[idx++]);
            double q = std::stod(f[idx++]);
            if (p > 0.0 && q >= 0.0) {
                ob.asks.push_back(BookLevel{p, q});
            }
        }

        ev = LiveEvent{};
        ev.type = LiveEventType::ORDERBOOK;
        ev.orderbook = ob;
        return true;
    }
    catch (...) {
        return false;
    }
}

} // namespace capigrad
