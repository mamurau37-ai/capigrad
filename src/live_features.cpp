#include "live_features.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace capigrad {

LiveFeatureEngine::LiveFeatureEngine(const LiveFeatureConfig& cfg)
    : cfg_(cfg) {}

void LiveFeatureEngine::reset() {
    last_book_ = OrderBookSnapshot{};
    has_book_ = false;

    prev_mid_ = 0.0;
    has_prev_mid_ = false;

    signed_flow_short_.clear();
    signed_flow_medium_.clear();
    trade_times_short_.clear();

    book_pressure_short_.clear();
    book_pressure_medium_.clear();
}

double LiveFeatureEngine::safe_div(double x, double y, double eps) {
    if (std::abs(y) < eps) return 0.0;
    return x / y;
}

double LiveFeatureEngine::topk_depth(const std::vector<BookLevel>& side, int k) {
    double s = 0.0;
    int n = std::min((int)side.size(), k);
    for (int i = 0; i < n; ++i) s += side[i].size;
    return s;
}

double LiveFeatureEngine::topk_notional(const std::vector<BookLevel>& side, int k) {
    double s = 0.0;
    int n = std::min((int)side.size(), k);
    for (int i = 0; i < n; ++i) s += side[i].price * side[i].size;
    return s;
}

double LiveFeatureEngine::best_bid(const OrderBookSnapshot& ob) {
    if (ob.bids.empty()) return 0.0;
    return ob.bids.front().price;
}

double LiveFeatureEngine::best_ask(const OrderBookSnapshot& ob) {
    if (ob.asks.empty()) return 0.0;
    return ob.asks.front().price;
}

double LiveFeatureEngine::mid_price(const OrderBookSnapshot& ob) {
    double bb = best_bid(ob);
    double ba = best_ask(ob);
    if (bb <= 0.0 || ba <= 0.0) return 0.0;
    return 0.5 * (bb + ba);
}

double LiveFeatureEngine::micro_price(const OrderBookSnapshot& ob) {
    if (ob.bids.empty() || ob.asks.empty()) return 0.0;

    double bb = ob.bids.front().price;
    double ba = ob.asks.front().price;
    double bs = ob.bids.front().size;
    double as = ob.asks.front().size;

    double denom = bs + as;
    if (denom <= 1e-12) return 0.0;

    // Standard microprice
    return (ba * bs + bb * as) / denom;
}

double LiveFeatureEngine::imbalance_k(const OrderBookSnapshot& ob, int k) {
    double bid_d = topk_depth(ob.bids, k);
    double ask_d = topk_depth(ob.asks, k);
    return safe_div(bid_d - ask_d, bid_d + ask_d);
}

double LiveFeatureEngine::spread_bps(const OrderBookSnapshot& ob) {
    double bb = best_bid(ob);
    double ba = best_ask(ob);
    double mid = mid_price(ob);
    if (bb <= 0.0 || ba <= 0.0 || mid <= 0.0) return 0.0;
    return 10000.0 * (ba - bb) / mid;
}

double LiveFeatureEngine::microprice_gap_bps(const OrderBookSnapshot& ob) {
    double mid = mid_price(ob);
    double micro = micro_price(ob);
    if (mid <= 0.0 || micro <= 0.0) return 0.0;
    return 10000.0 * (micro - mid) / mid;
}

double LiveFeatureEngine::signed_trade_value(const TradeTick& tr) {
    double sgn = tr.is_buy ? 1.0 : -1.0;
    return sgn * tr.price * tr.size;
}

void LiveFeatureEngine::push_bounded(std::deque<double>& dq, double x, int max_size) {
    dq.push_back(x);
    while ((int)dq.size() > max_size) dq.pop_front();
}

void LiveFeatureEngine::push_time_bounded(std::deque<std::int64_t>& dq,
                                          std::int64_t ts,
                                          int max_size) {
    dq.push_back(ts);
    while ((int)dq.size() > max_size) dq.pop_front();
}

double LiveFeatureEngine::sum_deque(const std::deque<double>& dq) {
    double s = 0.0;
    for (double x : dq) s += x;
    return s;
}

void LiveFeatureEngine::on_orderbook(const OrderBookSnapshot& ob) {
    if (ob.bids.empty() || ob.asks.empty()) return;

    double mid = mid_price(ob);
    if (mid <= 0.0) return;

    if (has_book_) {
        prev_mid_ = mid_price(last_book_);
        has_prev_mid_ = (prev_mid_ > 0.0);
    }

    last_book_ = ob;
    has_book_ = true;

    double pressure_short =
        imbalance_k(ob, cfg_.depth_top3) + microprice_gap_bps(ob) / 10000.0;

    double pressure_medium =
        imbalance_k(ob, cfg_.depth_top10) + microprice_gap_bps(ob) / 10000.0;

    push_bounded(book_pressure_short_, pressure_short, cfg_.book_window_short);
    push_bounded(book_pressure_medium_, pressure_medium, cfg_.book_window_medium);
}

void LiveFeatureEngine::on_trade(const TradeTick& tr) {
    if (tr.price <= 0.0 || tr.size <= 0.0) return;

    double signed_val = signed_trade_value(tr);
    push_bounded(signed_flow_short_, signed_val, cfg_.trade_window_short);
    push_bounded(signed_flow_medium_, signed_val, cfg_.trade_window_medium);
    push_time_bounded(trade_times_short_, tr.timestamp, cfg_.trade_window_short);
}

LiveFeatureVector LiveFeatureEngine::build() const {
    LiveFeatureVector fv;

    if (!has_book_) {
        fv.values.assign(LiveFeatureSchema::DIM, 0.0);
        return fv;
    }

    fv.timestamp = last_book_.timestamp;
    fv.values.resize(LiveFeatureSchema::DIM, 0.0);

    double mid = mid_price(last_book_);

    double mid_ret_1 = 0.0;
    if (has_prev_mid_ && prev_mid_ > 0.0 && mid > 0.0) {
        mid_ret_1 = std::log(mid / prev_mid_);
    }

    double bid_depth_top5 = topk_depth(last_book_.bids, cfg_.depth_top5);
    double ask_depth_top5 = topk_depth(last_book_.asks, cfg_.depth_top5);
    double depth_ratio_top5 = safe_div(bid_depth_top5, ask_depth_top5);

    double signed_flow_s = sum_deque(signed_flow_short_);
    double signed_flow_m = sum_deque(signed_flow_medium_);

    double trade_intensity_short = (double)trade_times_short_.size();

    double pressure_short  = sum_deque(book_pressure_short_);
    double pressure_medium = sum_deque(book_pressure_medium_);

    fv.values[LiveFeatureSchema::SPREAD_BPS]          = spread_bps(last_book_);
    fv.values[LiveFeatureSchema::MID_RETURN_1]        = mid_ret_1;
    fv.values[LiveFeatureSchema::MICROPRICE_GAP_BPS]  = microprice_gap_bps(last_book_);
    fv.values[LiveFeatureSchema::IMBALANCE_TOP1]      = imbalance_k(last_book_, cfg_.depth_top1);
    fv.values[LiveFeatureSchema::IMBALANCE_TOP3]      = imbalance_k(last_book_, cfg_.depth_top3);
    fv.values[LiveFeatureSchema::IMBALANCE_TOP10]     = imbalance_k(last_book_, cfg_.depth_top10);
    fv.values[LiveFeatureSchema::BID_DEPTH_TOP5]      = bid_depth_top5;
    fv.values[LiveFeatureSchema::ASK_DEPTH_TOP5]      = ask_depth_top5;
    fv.values[LiveFeatureSchema::DEPTH_RATIO_TOP5]    = depth_ratio_top5;
    fv.values[LiveFeatureSchema::SIGNED_FLOW_SHORT]   = signed_flow_s;
    fv.values[LiveFeatureSchema::SIGNED_FLOW_MEDIUM]  = signed_flow_m;
    fv.values[LiveFeatureSchema::TRADE_INTENSITY_SHORT] = trade_intensity_short;
    fv.values[LiveFeatureSchema::BOOK_PRESSURE_SHORT] = pressure_short;
    fv.values[LiveFeatureSchema::BOOK_PRESSURE_MEDIUM] = pressure_medium;

    return fv;
}

bool LiveFeatureEngine::ready() const {
    return has_book_;
}

double LiveFeatureEngine::current_mid() const {
    if (!has_book_) return 0.0;
    return mid_price(last_book_);
}

std::int64_t LiveFeatureEngine::current_timestamp() const {
    if (!has_book_) return 0;
    return last_book_.timestamp;
}

} // namespace capigrad
