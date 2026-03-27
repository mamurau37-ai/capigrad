#include "rl_env.hpp"

#include <stdexcept>

namespace capigrad {

RLEnv::RLEnv(const RLEnvConfig& cfg)
    : cfg_(cfg) {}

void RLEnv::load_rows(const std::vector<RLReplayRow>& rows) {
    rows_ = rows;
    idx_ = 0;
}

bool RLEnv::empty() const {
    return rows_.empty();
}

int RLEnv::size() const {
    return static_cast<int>(rows_.size());
}

int RLEnv::index() const {
    return idx_;
}

RLObservation RLEnv::make_obs(const RLReplayRow& row) const {
    RLObservation obs;

    // Compact state vector
    obs.values = {
        row.p_down,
        row.p_flat,
        row.p_up,
        row.offline_conf,

        row.spread_bps,
        row.imbalance_top3,
        row.imbalance_top10,
        row.micro_gap_bps,
        row.signed_flow_short,
        row.signed_flow_medium,
        row.micro_score
    };

    return obs;
}

RLObservation RLEnv::reset() {
    if (rows_.empty()) {
        throw std::runtime_error("RLEnv::reset called on empty replay rows");
    }

    idx_ = 0;
    return make_obs(rows_[idx_]);
}

RLObservation RLEnv::current_observation() const {
    if (rows_.empty()) {
        throw std::runtime_error("RLEnv::current_observation called on empty replay rows");
    }
    if (idx_ < 0 || idx_ >= (int)rows_.size()) {
        throw std::runtime_error("RLEnv::current_observation invalid index");
    }

    return make_obs(rows_[idx_]);
}

double RLEnv::compute_reward(const RLReplayRow& row, RLAction action) const {
    const double total_cost = cfg_.round_trip_fee + cfg_.slippage;

    // HOLD
    if (action == RLAction::HOLD) {
        return -cfg_.no_trade_penalty;
    }

    // LONG
    if (action == RLAction::LONG) {
        double pnl = row.future_return - total_cost;
        if (pnl < 0.0) pnl -= cfg_.wrong_trade_penalty;
        pnl -= cfg_.turnover_penalty;
        return pnl;
    }

    // SHORT
    if (action == RLAction::SHORT) {
        double pnl = -row.future_return - total_cost;
        if (pnl < 0.0) pnl -= cfg_.wrong_trade_penalty;
        pnl -= cfg_.turnover_penalty;
        return pnl;
    }

    return 0.0;
}

RLStepResult RLEnv::step(RLAction action) {
    if (rows_.empty()) {
        throw std::runtime_error("RLEnv::step called on empty replay rows");
    }
    if (idx_ < 0 || idx_ >= (int)rows_.size()) {
        throw std::runtime_error("RLEnv::step invalid index");
    }

    const RLReplayRow& row = rows_[idx_];

    RLStepResult out;
    out.timestamp = row.timestamp;
    out.reward = compute_reward(row, action);

    idx_++;
    if (idx_ >= (int)rows_.size()) {
        out.done = true;
        out.obs.values.clear();
        return out;
    }

    out.done = false;
    out.obs = make_obs(rows_[idx_]);
    return out;
}

} // namespace capigrad
