#ifndef CAPIGRAD_SIGNAL_FUSION_HPP
#define CAPIGRAD_SIGNAL_FUSION_HPP

#include <cstdint>
#include <vector>
#include "live_features.hpp"

namespace capigrad {

// Final fused action
enum class TradeAction : int {
    NO_TRADE = 0,
    LONG     = 1,
    SHORT    = -1
};

// Offline bias input from offline model
struct OfflineSignal {
    std::int64_t timestamp = 0;

    int pred_class = 1; // 0=DOWN, 1=FLAT, 2=UP
    std::vector<double> probs; // size 3
    double confidence = 0.0;
};

// Output of fusion layer
struct FusionDecision {
    std::int64_t timestamp = 0;

    TradeAction action = TradeAction::NO_TRADE;

    double offline_confidence = 0.0;
    double micro_score = 0.0;
    double final_score = 0.0;

    bool passed_offline_gate = false;
    bool passed_spread_gate  = false;
    bool passed_micro_gate   = false;

    const char* reason = "NO_SIGNAL";
};

// Rule-based config
struct FusionConfig {
    // Offline model confidence gates
    double offline_long_threshold  = 0.55;
    double offline_short_threshold = 0.55;

    // Market quality gates
    double max_spread_bps = 8.0;

    // Microstructure confirmation
    double min_micro_long_score  = 0.05;
    double min_micro_short_score = -0.05;

    // Optional stricter raw feature gates
    double min_imbalance_top3_long  = 0.05;
    double max_imbalance_top3_short = -0.05;

    double min_signed_flow_short_long  = 0.0;
    double max_signed_flow_short_short = 0.0;

    double min_microprice_gap_bps_long  = -1.0;
    double max_microprice_gap_bps_short = 1.0;
};

// Main fusion engine
class SignalFusionEngine {
public:
    explicit SignalFusionEngine(const FusionConfig& cfg = FusionConfig());

    FusionDecision decide(const OfflineSignal& offline,
                          const LiveFeatureVector& live) const;

    // Helper to build micro score from live feature vector
    double compute_micro_score(const LiveFeatureVector& live) const;

private:
    FusionConfig cfg_;

    double get_feature(const LiveFeatureVector& live, int idx) const;
};

} // namespace capigrad

#endif // CAPIGRAD_SIGNAL_FUSION_HPP
