#ifndef CAPIGRAD_RL_REPLAY_BUILDER_HPP
#define CAPIGRAD_RL_REPLAY_BUILDER_HPP

#include <string>
#include <vector>

#include "rl_env.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// Raw CSV row parsed from live_signals.csv
// -----------------------------------------------------------------------------
struct LiveSignalCsvRow {
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

    int action = 0;
    std::string reason;

    int has_realized = 0;
    double realized_return = 0.0;
    double realized_net_return = 0.0;
};

// -----------------------------------------------------------------------------
// Config for building RL replay rows from logged csv
// -----------------------------------------------------------------------------
struct RLReplayBuilderConfig {
    bool use_realized_net_return = true;
    bool skip_unrealized_rows = true;
};

// -----------------------------------------------------------------------------
// Parse CSV rows from live_signals.csv
// -----------------------------------------------------------------------------
std::vector<LiveSignalCsvRow> load_live_signal_csv(const std::string& path);

// -----------------------------------------------------------------------------
// Convert live csv rows into RLReplayRows
// -----------------------------------------------------------------------------
std::vector<RLReplayRow> build_rl_replay_rows(
    const std::vector<LiveSignalCsvRow>& rows,
    const RLReplayBuilderConfig& cfg = RLReplayBuilderConfig());

// Convenience wrapper: load + convert
std::vector<RLReplayRow> load_rl_replay_rows(
    const std::string& csv_path,
    const RLReplayBuilderConfig& cfg = RLReplayBuilderConfig());

} // namespace capigrad

#endif // CAPIGRAD_RL_REPLAY_BUILDER_HPP
