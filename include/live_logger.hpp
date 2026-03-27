#ifndef CAPIGRAD_LIVE_LOGGER_HPP
#define CAPIGRAD_LIVE_LOGGER_HPP

#include <fstream>
#include <string>
#include <vector>

#include "live_engine.hpp"
#include "live_features.hpp"
#include "signal_fusion.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// Row to log one live/paper decision
// -----------------------------------------------------------------------------
struct LiveLogRow {
    std::int64_t timestamp = 0;

    int offline_pred = 1;
    double p_down = 0.0;
    double p_flat = 0.0;
    double p_up   = 0.0;
    double offline_conf = 0.0;

    double spread_bps = 0.0;
    double imbalance_top3 = 0.0;
    double imbalance_top10 = 0.0;
    double micro_gap_bps = 0.0;
    double signed_flow_short = 0.0;
    double signed_flow_medium = 0.0;
    double micro_score = 0.0;
    double final_score = 0.0;

    int action = 0; // -1 short, 0 no-trade, 1 long
    std::string reason;

    // filled later if realized outcome is known
    int has_realized = 0;
    double realized_return = 0.0;
    double realized_net_return = 0.0;
};

// -----------------------------------------------------------------------------
// CSV logger for live/paper decisions
// -----------------------------------------------------------------------------
class LiveLogger {
public:
    explicit LiveLogger(const std::string& csv_path);

    void log_row(const LiveLogRow& row);
    void flush();

    // helper to construct row from engine outputs
    static LiveLogRow make_row(const OfflineSignal& off,
                               const LiveFeatureVector& fv,
                               const FusionDecision& d);

private:
    std::ofstream file_;
    bool header_written_ = false;

    void write_header_if_needed();
};

} // namespace capigrad

#endif // CAPIGRAD_LIVE_LOGGER_HPP
