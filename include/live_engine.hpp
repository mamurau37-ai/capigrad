#ifndef CAPIGRAD_LIVE_ENGINE_HPP
#define CAPIGRAD_LIVE_ENGINE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "offline_model.hpp"
#include "live_features.hpp"
#include "signal_fusion.hpp"
#include "sequence_builder.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// Replay/live event types
// -----------------------------------------------------------------------------
enum class LiveEventType : int {
    ORDERBOOK = 0,
    TRADE     = 1
};

struct LiveEvent {
    LiveEventType type = LiveEventType::ORDERBOOK;

    OrderBookSnapshot orderbook;
    TradeTick trade;
};

// -----------------------------------------------------------------------------
// Decision log output
// -----------------------------------------------------------------------------
struct LiveDecisionLog {
    std::int64_t timestamp = 0;

    int offline_pred = 1;
    double offline_confidence = 0.0;

    double micro_score = 0.0;
    double final_score = 0.0;

    TradeAction action = TradeAction::NO_TRADE;
    const char* reason = "NO_SIGNAL";
};

// -----------------------------------------------------------------------------
// Engine config
// -----------------------------------------------------------------------------
struct LiveEngineConfig {
    // reuse last offline sample as context source for demo/replay mode
    bool use_offline_replay_context = true;

    // only emit decision if live features ready
    bool require_live_ready = true;
};

// -----------------------------------------------------------------------------
// Main live engine
// -----------------------------------------------------------------------------
class LiveEngine {
public:
    LiveEngine(const OfflineDualTFModel& offline_template,
               const LiveFeatureConfig& live_cfg = LiveFeatureConfig(),
               const FusionConfig& fusion_cfg = FusionConfig(),
               const LiveEngineConfig& engine_cfg = LiveEngineConfig());

    // Reset internal state
    void reset();

    // Replace offline context sample used for bias inference
    void set_offline_context(const OfflineSample& sample);

    // Process one event
    void on_event(const LiveEvent& ev);

    // Build offline signal from current offline context
    OfflineSignal infer_offline_signal();

    // Build fused decision from current state
    FusionDecision decide();

    // Convenience: infer + fuse + log
    LiveDecisionLog step();

    // Access current feature vector
    LiveFeatureVector current_live_features() const;

    bool has_context() const;
    bool live_ready() const;

private:
    OfflineDualTFModel offline_model_;
    LiveFeatureEngine live_engine_;
    SignalFusionEngine fusion_engine_;
    LiveEngineConfig engine_cfg_;

    bool has_context_ = false;
    OfflineSample current_context_;
};

} // namespace capigrad

#endif // CAPIGRAD_LIVE_ENGINE_HPP
