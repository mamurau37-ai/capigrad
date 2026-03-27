#ifndef CAPIGRAD_RL_ENV_HPP
#define CAPIGRAD_RL_ENV_HPP

#include <cstdint>
#include <vector>

#include "sequence_builder.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// RL action space
// We keep it small and safe:
// 0 = HOLD / NO TRADE
// 1 = LONG
// 2 = SHORT
// -----------------------------------------------------------------------------
enum class RLAction : int {
    HOLD  = 0,
    LONG  = 1,
    SHORT = 2
};

// -----------------------------------------------------------------------------
// RL observation:
// a compact state built from existing model stack outputs + live-like features
// -----------------------------------------------------------------------------
struct RLObservation {
    std::vector<double> values;
};

// -----------------------------------------------------------------------------
// Step result
// -----------------------------------------------------------------------------
struct RLStepResult {
    RLObservation obs;
    double reward = 0.0;
    bool done = false;
    std::int64_t timestamp = 0;
};

// -----------------------------------------------------------------------------
// Config for sandbox env
// -----------------------------------------------------------------------------
struct RLEnvConfig {
    // transaction costs
    double round_trip_fee = 0.0004;
    double slippage = 0.0002;

    // reward penalties
    double no_trade_penalty = 0.0;
    double wrong_trade_penalty = 0.0;
    double turnover_penalty = 0.0;

    // if true, HOLD action gets no pnl
    bool allow_hold = true;
};

// -----------------------------------------------------------------------------
// Replay record for RL environment
// This is the "supervised signal row" that RL will consume.
// It is built from your existing pipeline outputs.
//
// Fields:
// - timestamp
// - offline probs/confidence
// - live micro score / spread / imbalance / flow
// - realized future return
// -----------------------------------------------------------------------------
struct RLReplayRow {
    std::int64_t timestamp = 0;

    // offline model outputs
    double p_down = 0.0;
    double p_flat = 0.0;
    double p_up   = 0.0;
    double offline_conf = 0.0;

    // live micro features
    double spread_bps = 0.0;
    double imbalance_top3 = 0.0;
    double imbalance_top10 = 0.0;
    double micro_gap_bps = 0.0;
    double signed_flow_short = 0.0;
    double signed_flow_medium = 0.0;
    double micro_score = 0.0;

    // label / realized return
    int true_label = 1; // 0=DOWN,1=FLAT,2=UP
    double future_return = 0.0;
};

// -----------------------------------------------------------------------------
// Simple replay RL environment
// The agent does NOT talk to live exchange.
// It only walks over replay rows and receives reward.
// -----------------------------------------------------------------------------
class RLEnv {
public:
    explicit RLEnv(const RLEnvConfig& cfg = RLEnvConfig());

    // load replay rows
    void load_rows(const std::vector<RLReplayRow>& rows);

    // reset episode to first row
    RLObservation reset();

    // current observation without stepping
    RLObservation current_observation() const;

    // one action step
    RLStepResult step(RLAction action);

    // info
    bool empty() const;
    int size() const;
    int index() const;

private:
    RLEnvConfig cfg_;
    std::vector<RLReplayRow> rows_;
    int idx_ = 0;

    RLObservation make_obs(const RLReplayRow& row) const;
    double compute_reward(const RLReplayRow& row, RLAction action) const;
};

} // namespace capigrad

#endif // CAPIGRAD_RL_ENV_HPP
