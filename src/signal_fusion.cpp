#include "signal_fusion.hpp"

#include <stdexcept>

namespace capigrad {

SignalFusionEngine::SignalFusionEngine(const FusionConfig& cfg)
    : cfg_(cfg) {}

double SignalFusionEngine::get_feature(const LiveFeatureVector& live, int idx) const {
    if (idx < 0 || idx >= (int)live.values.size()) return 0.0;
    return live.values[idx];
}

double SignalFusionEngine::compute_micro_score(const LiveFeatureVector& live) const {
    // Positive => bullish micro pressure
    // Negative => bearish micro pressure

    const double spread_bps         = get_feature(live, LiveFeatureSchema::SPREAD_BPS);
    const double micro_gap_bps      = get_feature(live, LiveFeatureSchema::MICROPRICE_GAP_BPS);
    const double imb_top3           = get_feature(live, LiveFeatureSchema::IMBALANCE_TOP3);
    const double imb_top10          = get_feature(live, LiveFeatureSchema::IMBALANCE_TOP10);
    const double signed_flow_short  = get_feature(live, LiveFeatureSchema::SIGNED_FLOW_SHORT);
    const double signed_flow_medium = get_feature(live, LiveFeatureSchema::SIGNED_FLOW_MEDIUM);
    const double book_pressure_s    = get_feature(live, LiveFeatureSchema::BOOK_PRESSURE_SHORT);
    const double book_pressure_m    = get_feature(live, LiveFeatureSchema::BOOK_PRESSURE_MEDIUM);

    // scale down raw flows
    const double flow_short_scaled  = signed_flow_short  * 1e-7;
    const double flow_medium_scaled = signed_flow_medium * 1e-7;
    const double spread_penalty     = spread_bps * 0.01;

    double score = 0.0;
    score += 0.30 * imb_top3;
    score += 0.18 * imb_top10;
    score += 0.15 * (micro_gap_bps / 10.0);
    score += 0.08 * flow_short_scaled;
    score += 0.06 * flow_medium_scaled;
    score += 0.12 * book_pressure_s;
    score += 0.11 * book_pressure_m;
    score -= 0.05 * spread_penalty;

    return score;
}

FusionDecision SignalFusionEngine::decide(const OfflineSignal& offline,
                                          const LiveFeatureVector& live) const {
    FusionDecision d;
    d.timestamp = (live.timestamp != 0) ? live.timestamp : offline.timestamp;
    d.offline_confidence = offline.confidence;

    if (offline.probs.size() != 3) {
        d.reason = "BAD_OFFLINE_PROBS";
        return d;
    }

    if (live.values.size() < LiveFeatureSchema::DIM) {
        d.reason = "BAD_LIVE_FEATURES";
        return d;
    }

    const double spread_bps        = get_feature(live, LiveFeatureSchema::SPREAD_BPS);
    const double imb_top3          = get_feature(live, LiveFeatureSchema::IMBALANCE_TOP3);
    const double signed_flow_short = get_feature(live, LiveFeatureSchema::SIGNED_FLOW_SHORT);
    const double micro_gap_bps     = get_feature(live, LiveFeatureSchema::MICROPRICE_GAP_BPS);

    d.micro_score = compute_micro_score(live);

    // -----------------------------------------------------------------
    // Market quality gate
    // -----------------------------------------------------------------
    if (spread_bps <= cfg_.max_spread_bps) {
        d.passed_spread_gate = true;
    } else {
        d.reason = "SPREAD_TOO_WIDE";
        return d;
    }

    // -----------------------------------------------------------------
    // 1) Strong directional LONG
    // -----------------------------------------------------------------
    if (offline.pred_class == 2 && offline.confidence >= cfg_.offline_long_threshold) {
        d.passed_offline_gate = true;

        bool micro_ok =
            d.micro_score >= cfg_.min_micro_long_score &&
            imb_top3 >= cfg_.min_imbalance_top3_long &&
            signed_flow_short >= cfg_.min_signed_flow_short_long &&
            micro_gap_bps >= cfg_.min_microprice_gap_bps_long;

        if (micro_ok) {
            d.passed_micro_gate = true;
            d.action = TradeAction::LONG;
            d.final_score = 0.55 * offline.confidence + 0.45 * d.micro_score;
            d.reason = "LONG_CONFIRMED";
            return d;
        } else {
            d.reason = "LONG_MICRO_REJECT";
            return d;
        }
    }

    // -----------------------------------------------------------------
    // 2) Strong directional SHORT
    // -----------------------------------------------------------------
    if (offline.pred_class == 0 && offline.confidence >= cfg_.offline_short_threshold) {
        d.passed_offline_gate = true;

        bool micro_ok =
            d.micro_score <= cfg_.min_micro_short_score &&
            imb_top3 <= cfg_.max_imbalance_top3_short &&
            signed_flow_short <= cfg_.max_signed_flow_short_short &&
            micro_gap_bps <= cfg_.max_microprice_gap_bps_short;

        if (micro_ok) {
            d.passed_micro_gate = true;
            d.action = TradeAction::SHORT;
            d.final_score = 0.55 * offline.confidence + 0.45 * (-d.micro_score);
            d.reason = "SHORT_CONFIRMED";
            return d;
        } else {
            d.reason = "SHORT_MICRO_REJECT";
            return d;
        }
    }

    // -----------------------------------------------------------------
    // 3) FLAT override for scalp
    // -----------------------------------------------------------------
    if (offline.pred_class == 1) {
        bool weak_long =
            offline.probs[2] >= 0.30 &&
            d.micro_score >= 0.18 &&
            imb_top3 >= 0.04 &&
            micro_gap_bps >= -1.0;

        if (weak_long) {
            d.passed_offline_gate = true;
            d.passed_micro_gate   = true;
            d.action = TradeAction::LONG;
            d.final_score = 0.30 * offline.probs[2] + 0.70 * d.micro_score;
            d.reason = "FLAT_OVERRIDE_LONG";
            return d;
        }

        bool weak_short =
            offline.probs[0] >= 0.17 &&
            d.micro_score <= -0.18 &&
            imb_top3 <= -0.04 &&
            micro_gap_bps <= 1.0;

        if (weak_short) {
            d.passed_offline_gate = true;
            d.passed_micro_gate   = true;
            d.action = TradeAction::SHORT;
            d.final_score = 0.30 * offline.probs[0] + 0.70 * (-d.micro_score);
            d.reason = "FLAT_OVERRIDE_SHORT";
            return d;
        }
    }

    // -----------------------------------------------------------------
    // 4) PURE MICRO SCALP TRIGGER
    // This is the new part:
    // offline model becomes soft prior, not hard veto
    // -----------------------------------------------------------------

    // Strong bullish micro
    bool pure_micro_long =
        spread_bps <= 3.0 &&
        imb_top3 >= 0.20 &&
        micro_gap_bps >= 0.0 &&
        d.micro_score >= 0.35;

    if (pure_micro_long) {
        d.passed_micro_gate = true;
        d.action = TradeAction::LONG;
        d.final_score = 0.20 * offline.probs[2] + 0.80 * d.micro_score;
        d.reason = "PURE_MICRO_LONG";
        return d;
    }

    // Strong bearish micro
    bool pure_micro_short =
        spread_bps <= 3.0 &&
        imb_top3 <= -0.20 &&
        micro_gap_bps <= 0.0 &&
        d.micro_score <= -0.35;

    if (pure_micro_short) {
        d.passed_micro_gate = true;
        d.action = TradeAction::SHORT;
        d.final_score = 0.20 * offline.probs[0] + 0.80 * (-d.micro_score);
        d.reason = "PURE_MICRO_SHORT";
        return d;
    }

    // -----------------------------------------------------------------
    // fallback reasons
    // -----------------------------------------------------------------
    if (offline.pred_class == 1) {
        d.reason = "OFFLINE_FLAT";
    } else {
        d.reason = "OFFLINE_CONF_TOO_LOW";
    }

    return d;
}

} // namespace capigrad
