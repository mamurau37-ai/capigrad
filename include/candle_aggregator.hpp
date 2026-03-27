#ifndef CAPIGRAD_CANDLE_AGGREGATOR_HPP
#define CAPIGRAD_CANDLE_AGGREGATOR_HPP

#include <cstdint>
#include <optional>
#include <vector>

#include "features.hpp"
#include "live_features.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// Generic time-bucket candle aggregator
// Builds OHLCV candles from TradeTick stream.
// -----------------------------------------------------------------------------
class CandleAggregator {
public:
    explicit CandleAggregator(std::int64_t interval_ms);

    // Reset internal state
    void reset();

    // Feed one trade tick.
    // If this tick closes the previous candle, returns that closed candle.
    // Otherwise returns std::nullopt.
    std::optional<Candle> on_trade(const TradeTick& tr);

    // Force flush current unfinished candle (optional use on shutdown)
    std::optional<Candle> flush();

    bool has_open_candle() const;
    std::int64_t interval_ms() const;

private:
    std::int64_t interval_ms_;
    bool has_current_ = false;
    Candle current_;

    std::int64_t bucket_start(std::int64_t ts) const;
    Candle make_new_candle(std::int64_t bucket_ts,
                           double price,
                           double size) const;
};

} // namespace capigrad

#endif // CAPIGRAD_CANDLE_AGGREGATOR_HPP
