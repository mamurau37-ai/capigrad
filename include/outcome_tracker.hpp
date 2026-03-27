#ifndef CAPIGRAD_OUTCOME_TRACKER_HPP
#define CAPIGRAD_OUTCOME_TRACKER_HPP

#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

#include "features.hpp"
#include "live_logger.hpp"
#include "signal_fusion.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// Pending trade opened from a live decision
// -----------------------------------------------------------------------------
struct PendingOutcome {
    std::int64_t open_timestamp = 0;
    double open_price = 0.0;

    // -1 short, 0 no-trade, +1 long
    int action = 0;

    // number of closed M15 bars to wait before evaluation
    int horizon_m15 = 4;

    // copy of original logged row
    LiveLogRow row;

    // how many M15 bars have passed since open
    int bars_elapsed = 0;
};

// -----------------------------------------------------------------------------
// Finalized outcome row
// -----------------------------------------------------------------------------
struct FinalizedOutcome {
    LiveLogRow row;
};

// -----------------------------------------------------------------------------
// Tracks pending decisions and finalizes them after horizon passes
// using M15 closed candles.
// -----------------------------------------------------------------------------
class OutcomeTracker {
public:
    OutcomeTracker(int horizon_m15 = 4,
                   double round_trip_fee = 0.0004,
                   double slippage = 0.0002);

    void reset();

    // open new tracked outcome from a live decision
    void on_decision(const LiveLogRow& row, double current_price);

    // call on every newly closed M15 candle
    // returns zero or more finalized outcomes
    std::vector<FinalizedOutcome> on_m15_close(const Candle& closed_candle);

    int pending_count() const;

private:
    int horizon_m15_;
    double round_trip_fee_;
    double slippage_;

    std::deque<PendingOutcome> pending_;
};

} // namespace capigrad

#endif // CAPIGRAD_OUTCOME_TRACKER_HPP
