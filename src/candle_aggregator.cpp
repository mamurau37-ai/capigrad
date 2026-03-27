#include "candle_aggregator.hpp"

#include <stdexcept>

namespace capigrad {

CandleAggregator::CandleAggregator(std::int64_t interval_ms)
    : interval_ms_(interval_ms) {
    if (interval_ms_ <= 0) {
        throw std::runtime_error("CandleAggregator interval must be positive");
    }
}

void CandleAggregator::reset() {
    has_current_ = false;
    current_ = Candle{};
}

bool CandleAggregator::has_open_candle() const {
    return has_current_;
}

std::int64_t CandleAggregator::interval_ms() const {
    return interval_ms_;
}

std::int64_t CandleAggregator::bucket_start(std::int64_t ts) const {
    return (ts / interval_ms_) * interval_ms_;
}

Candle CandleAggregator::make_new_candle(std::int64_t bucket_ts,
                                         double price,
                                         double size) const {
    Candle c;
    c.timestamp = bucket_ts;
    c.open = price;
    c.high = price;
    c.low = price;
    c.close = price;
    c.volume = size;
    return c;
}

std::optional<Candle> CandleAggregator::on_trade(const TradeTick& tr) {
    if (tr.timestamp <= 0 || tr.price <= 0.0 || tr.size <= 0.0) {
        return std::nullopt;
    }

    const std::int64_t bucket_ts = bucket_start(tr.timestamp);

    // First trade starts first candle
    if (!has_current_) {
        current_ = make_new_candle(bucket_ts, tr.price, tr.size);
        has_current_ = true;
        return std::nullopt;
    }

    // Same bucket -> update current candle
    if (current_.timestamp == bucket_ts) {
        if (tr.price > current_.high) current_.high = tr.price;
        if (tr.price < current_.low)  current_.low  = tr.price;
        current_.close = tr.price;
        current_.volume += tr.size;
        return std::nullopt;
    }

    // New bucket -> close previous candle, start new one
    if (bucket_ts > current_.timestamp) {
        Candle closed = current_;
        current_ = make_new_candle(bucket_ts, tr.price, tr.size);
        has_current_ = true;
        return closed;
    }

    // Old/out-of-order tick: ignore for now
    return std::nullopt;
}

std::optional<Candle> CandleAggregator::flush() {
    if (!has_current_) return std::nullopt;
    Candle out = current_;
    has_current_ = false;
    current_ = Candle{};
    return out;
}

} // namespace capigrad
