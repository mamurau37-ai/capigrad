#ifndef CAPIGRAD_LABELER_HPP
#define CAPIGRAD_LABELER_HPP

#include <cstdint>
#include <vector>
#include "features.hpp"

namespace capigrad {

// IMPORTANT fixed order for neural model:
// 0 = DOWN, 1 = FLAT, 2 = UP
enum class DirectionLabel : int {
    DOWN = 0,
    FLAT = 1,
    UP   = 2
};

struct LabelTarget {
    std::int64_t timestamp = 0;
    DirectionLabel label   = DirectionLabel::FLAT;

    double future_return = 0.0;
    double threshold     = 0.0;

    bool valid = false;
};

struct LabelConfig {
    int horizon_m15       = 4;    // future bars ahead
    int vol_window        = 32;   // rolling std window
    double threshold_k    = 0.8;  // threshold = k * rolling std
    double min_threshold  = 0.0;  // optional threshold floor
};

std::vector<double> compute_close_returns(const std::vector<Candle>& candles);

std::vector<LabelTarget> build_direction_labels(
    const std::vector<Candle>& m15_candles,
    const LabelConfig& cfg);

int label_to_index(DirectionLabel label);
const char* to_string(DirectionLabel label);

} // namespace capigrad

#endif // CAPIGRAD_LABELER_HPP
