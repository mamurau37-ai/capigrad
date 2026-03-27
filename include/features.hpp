#ifndef CAPIGRAD_FEATURES_HPP
#define CAPIGRAD_FEATURES_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace capigrad {

// Raw OHLCV candle
struct Candle {
    std::int64_t timestamp = 0;   // candle start timestamp, UTC ms
    double open   = 0.0;
    double high   = 0.0;
    double low    = 0.0;
    double close  = 0.0;
    double volume = 0.0;
};

// Engineered features for one candle
struct FeatureVector {
    std::int64_t timestamp = 0;
    std::vector<double> values;
};

// Optional feature validity flags
struct FeatureValidity {
    bool has_ret_1         = false;
    bool has_vol_z         = false;
    bool has_volatility_16 = false;
    bool has_ma_gap_16     = false;

    bool all_valid() const {
        return has_ret_1 &&
               has_vol_z &&
               has_volatility_16 &&
               has_ma_gap_16;
    }
};

struct FeatureConfig {
    bool use_ret_1         = true;
    bool use_body          = true;
    bool use_range         = true;
    bool use_upper_wick    = true;
    bool use_lower_wick    = true;
    bool use_vol_z         = true;
    bool use_volatility_16 = true;
    bool use_ma_gap_16     = true;

    int vol_z_window       = 16;
    int volatility_window  = 16;
    int ma_window          = 16;
};

struct FeatureSchema {
    std::vector<std::string> names;

    int dim() const {
        return static_cast<int>(names.size());
    }
};

FeatureSchema build_feature_schema(const FeatureConfig& cfg);

double safe_div(double x, double y, double eps = 1e-12);
double safe_log_return(double prev_close, double curr_close);

struct RollingStats {
    double mean = 0.0;
    double std  = 0.0;
};

RollingStats compute_window_stats(const std::vector<double>& values,
                                  int start_idx,
                                  int end_idx);

std::vector<FeatureVector> build_features(
    const std::vector<Candle>& candles,
    const FeatureConfig& cfg);

std::vector<FeatureValidity> build_feature_validity(
    const std::vector<Candle>& candles,
    const FeatureConfig& cfg);

} // namespace capigrad

#endif // CAPIGRAD_FEATURES_HPP
