#include "outcome_tracker.hpp"

#include <stdexcept>

namespace capigrad {

OutcomeTracker::OutcomeTracker(int horizon_m15,
                               double round_trip_fee,
                               double slippage)
    : horizon_m15_(horizon_m15),
      round_trip_fee_(round_trip_fee),
      slippage_(slippage) {}

void OutcomeTracker::reset() {
    pending_.clear();
}

void OutcomeTracker::on_decision(const LiveLogRow& row, double current_price) {
    if (row.action == 0) return; // no trade -> nothing to track
    if (current_price <= 0.0) return;

    PendingOutcome p;
    p.open_timestamp = row.timestamp;
    p.open_price = current_price;
    p.action = row.action;
    p.horizon_m15 = horizon_m15_;
    p.row = row;
    p.bars_elapsed = 0;

    pending_.push_back(std::move(p));
}

std::vector<FinalizedOutcome> OutcomeTracker::on_m15_close(const Candle& closed_candle) {
    std::vector<FinalizedOutcome> out;
    if (closed_candle.close <= 0.0) return out;

    for (auto& p : pending_) {
        p.bars_elapsed += 1;
    }

    while (!pending_.empty() && pending_.front().bars_elapsed >= pending_.front().horizon_m15) {
        PendingOutcome p = pending_.front();
        pending_.pop_front();

        double raw_ret = 0.0;

        // long
        if (p.action == 1) {
            raw_ret = (closed_candle.close / p.open_price) - 1.0;
        }
        // short
        else if (p.action == -1) {
            raw_ret = -((closed_candle.close / p.open_price) - 1.0);
        }

        double net_ret = raw_ret - round_trip_fee_ - slippage_;

        p.row.has_realized = 1;
        p.row.realized_return = raw_ret;
        p.row.realized_net_return = net_ret;

        FinalizedOutcome fin;
        fin.row = p.row;
        out.push_back(std::move(fin));
    }

    return out;
}

int OutcomeTracker::pending_count() const {
    return static_cast<int>(pending_.size());
}

} // namespace capigrad
