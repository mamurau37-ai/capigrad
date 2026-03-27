#include "labeler.hpp"

#include <algorithm>
#include <cmath>

namespace capigrad {

std::vector<double> compute_close_returns(const std::vector<Candle>& candles) {
    std::vector<double> rets(candles.size(), 0.0);

    for (int i = 1; i < (int)candles.size(); ++i) {
        rets[i] = safe_log_return(candles[i - 1].close, candles[i].close);
    }

    return rets;
}

std::vector<LabelTarget> build_direction_labels(
    const std::vector<Candle>& m15_candles,
    const LabelConfig& cfg) {

    std::vector<LabelTarget> out(m15_candles.size());
    std::vector<double> rets = compute_close_returns(m15_candles);

    for (int i = 0; i < (int)m15_candles.size(); ++i) {
        LabelTarget t;
        t.timestamp = m15_candles[i].timestamp;
        t.valid = false;
        t.label = DirectionLabel::FLAT;

        if (i + cfg.horizon_m15 >= (int)m15_candles.size()) {
            out[i] = t;
            continue;
        }

        if (i < cfg.vol_window) {
            out[i] = t;
            continue;
        }

        const double c_now = m15_candles[i].close;
        const double c_fut = m15_candles[i + cfg.horizon_m15].close;

        if (c_now <= 0.0 || c_fut <= 0.0) {
            out[i] = t;
            continue;
        }

        t.future_return = (c_fut / c_now) - 1.0;

        auto rs = compute_window_stats(rets, i - cfg.vol_window + 1, i);
        t.threshold = std::max(cfg.min_threshold, cfg.threshold_k * rs.std);

        if (t.future_return > t.threshold) {
            t.label = DirectionLabel::UP;
        } else if (t.future_return < -t.threshold) {
            t.label = DirectionLabel::DOWN;
        } else {
            t.label = DirectionLabel::FLAT;
        }

        t.valid = true;
        out[i] = t;
    }

    return out;
}

int label_to_index(DirectionLabel label) {
    switch (label) {
        case DirectionLabel::DOWN: return 0;
        case DirectionLabel::FLAT: return 1;
        case DirectionLabel::UP:   return 2;
        default:                   return 1;
    }
}

const char* to_string(DirectionLabel label) {
    switch (label) {
        case DirectionLabel::DOWN: return "DOWN";
        case DirectionLabel::FLAT: return "FLAT";
        case DirectionLabel::UP:   return "UP";
        default:                   return "UNKNOWN";
    }
}

} // namespace capigrad
