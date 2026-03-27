#include "live_engine.hpp"

#include <stdexcept>

namespace capigrad {

LiveEngine::LiveEngine(const OfflineDualTFModel& offline_template,
                       const LiveFeatureConfig& live_cfg,
                       const FusionConfig& fusion_cfg,
                       const LiveEngineConfig& engine_cfg)
    : offline_model_(offline_template),
      live_engine_(live_cfg),
      fusion_engine_(fusion_cfg),
      engine_cfg_(engine_cfg) {}

void LiveEngine::reset() {
    live_engine_.reset();
    offline_model_.reset_state();
    has_context_ = false;
    current_context_ = OfflineSample{};
}

void LiveEngine::set_offline_context(const OfflineSample& sample) {
    current_context_ = sample;
    has_context_ = true;
}

void LiveEngine::on_event(const LiveEvent& ev) {
    if (ev.type == LiveEventType::ORDERBOOK) {
        live_engine_.on_orderbook(ev.orderbook);
    } else if (ev.type == LiveEventType::TRADE) {
        live_engine_.on_trade(ev.trade);
    }
}

OfflineSignal LiveEngine::infer_offline_signal() {
    if (!has_context_) {
        throw std::runtime_error("LiveEngine::infer_offline_signal called without offline context");
    }

    offline_model_.reset_state();

    std::vector<double> logits = offline_model_.forward(current_context_);
    std::vector<double> probs  = offline_model_.softmax(logits);

    int pred = 0;
    for (int i = 1; i < (int)probs.size(); ++i) {
        if (probs[i] > probs[pred]) pred = i;
    }

    OfflineSignal sig;
    sig.timestamp = current_context_.timestamp;
    sig.pred_class = pred;
    sig.probs = probs;
    sig.confidence = probs[pred];

    return sig;
}

FusionDecision LiveEngine::decide() {
    if (!has_context_) {
        FusionDecision d;
        d.reason = "NO_OFFLINE_CONTEXT";
        return d;
    }

    if (engine_cfg_.require_live_ready && !live_engine_.ready()) {
        FusionDecision d;
        d.reason = "LIVE_NOT_READY";
        return d;
    }

    OfflineSignal off = infer_offline_signal();
    LiveFeatureVector lv = live_engine_.build();
    return fusion_engine_.decide(off, lv);
}

LiveDecisionLog LiveEngine::step() {
    LiveDecisionLog log;

    FusionDecision d = decide();

    log.timestamp = d.timestamp;
    log.action = d.action;
    log.reason = d.reason;
    log.micro_score = d.micro_score;
    log.final_score = d.final_score;
    log.offline_confidence = d.offline_confidence;

    if (has_context_) {
        OfflineSignal off = infer_offline_signal();
        log.offline_pred = off.pred_class;
        log.offline_confidence = off.confidence;
    }

    return log;
}

LiveFeatureVector LiveEngine::current_live_features() const {
    return live_engine_.build();
}

bool LiveEngine::has_context() const {
    return has_context_;
}

bool LiveEngine::live_ready() const {
    return live_engine_.ready();
}

} // namespace capigrad
