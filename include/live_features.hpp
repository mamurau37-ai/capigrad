#ifndef CAPIGRAD_LIVE_FEATURES_HPP
#define CAPIGRAD_LIVE_FEATURES_HPP

#include <cstdint>
#include <deque>
#include <vector>

namespace capigrad {

// ─────────────────────────────────────────────────────────────
// Basic live market structs
// ─────────────────────────────────────────────────────────────
struct BookLevel {
    double price = 0.0;
    double size  = 0.0;
};

struct OrderBookSnapshot {
    std::int64_t timestamp = 0; // ms UTC

    std::vector<BookLevel> bids; // sorted best -> worse
    std::vector<BookLevel> asks; // sorted best -> worse
};

struct TradeTick {
    std::int64_t timestamp = 0; // ms UTC
    double price = 0.0;
    double size  = 0.0;

    // true  => buyer-initiated aggressive buy
    // false => seller-initiated aggressive sell
    bool is_buy = true;
};

// ─────────────────────────────────────────────────────────────
// Output live feature vector for fusion layer / decision engine
// ─────────────────────────────────────────────────────────────
struct LiveFeatureVector {
    std::int64_t timestamp = 0;
    std::vector<double> values;
};

// Fixed order schema for live features
struct LiveFeatureSchema {
    enum Index {
        SPREAD_BPS = 0,
        MID_RETURN_1,
        MICROPRICE_GAP_BPS,
        IMBALANCE_TOP1,
        IMBALANCE_TOP3,
        IMBALANCE_TOP10,
        BID_DEPTH_TOP5,
        ASK_DEPTH_TOP5,
        DEPTH_RATIO_TOP5,
        SIGNED_FLOW_SHORT,
        SIGNED_FLOW_MEDIUM,
        TRADE_INTENSITY_SHORT,
        BOOK_PRESSURE_SHORT,
        BOOK_PRESSURE_MEDIUM,
        DIM
    };
};

// ─────────────────────────────────────────────────────────────
// Config for rolling windows / top-k depth aggregation
// ─────────────────────────────────────────────────────────────
struct LiveFeatureConfig {
    int depth_top1   = 1;
    int depth_top3   = 3;
    int depth_top5   = 5;
    int depth_top10  = 10;

    int trade_window_short  = 32;
    int trade_window_medium = 128;

    int book_window_short   = 16;
    int book_window_medium  = 64;
};

// ─────────────────────────────────────────────────────────────
// Rolling live feature engine
// ─────────────────────────────────────────────────────────────
class LiveFeatureEngine {
public:
    explicit LiveFeatureEngine(const LiveFeatureConfig& cfg = LiveFeatureConfig());

    // Reset all rolling state
    void reset();

    // Update current book snapshot
    void on_orderbook(const OrderBookSnapshot& ob);

    // Update trade flow
    void on_trade(const TradeTick& tr);

    // Return current feature vector (14 dims)
    LiveFeatureVector build() const;

    // Helpers for fusion logic
    bool ready() const;
    double current_mid() const;
    std::int64_t current_timestamp() const;

private:
    LiveFeatureConfig cfg_;

    OrderBookSnapshot last_book_;
    bool has_book_ = false;

    double prev_mid_ = 0.0;
    bool has_prev_mid_ = false;

    // rolling signed trade flow
    std::deque<double> signed_flow_short_;
    std::deque<double> signed_flow_medium_;

    // rolling trade timestamps for intensity
    std::deque<std::int64_t> trade_times_short_;

    // rolling book pressure history
    std::deque<double> book_pressure_short_;
    std::deque<double> book_pressure_medium_;

    // internal helpers
    static double safe_div(double x, double y, double eps = 1e-12);

    static double topk_depth(const std::vector<BookLevel>& side, int k);
    static double topk_notional(const std::vector<BookLevel>& side, int k);

    static double best_bid(const OrderBookSnapshot& ob);
    static double best_ask(const OrderBookSnapshot& ob);
    static double mid_price(const OrderBookSnapshot& ob);
    static double micro_price(const OrderBookSnapshot& ob);

    static double imbalance_k(const OrderBookSnapshot& ob, int k);
    static double spread_bps(const OrderBookSnapshot& ob);
    static double microprice_gap_bps(const OrderBookSnapshot& ob);

    static double signed_trade_value(const TradeTick& tr);

    static void push_bounded(std::deque<double>& dq, double x, int max_size);
    static void push_time_bounded(std::deque<std::int64_t>& dq, std::int64_t ts, int max_size);

    static double sum_deque(const std::deque<double>& dq);
};

} // namespace capigrad

#endif // CAPIGRAD_LIVE_FEATURES_HPP
