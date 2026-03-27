#include "features.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace capigrad {

namespace {

double finite_or_zero(double x) {
    return std::isfinite(x) ? x : 0.0;
}

} // anonymous namespace

FeatureSchema build_feature_schema(const FeatureConfig& cfg) {
    FeatureSchema schema;

    if (cfg.use_ret_1)         schema.names.push_back("ret_1");
    if (cfg.use_body)          schema.names.push_back("body");
    if (cfg.use_range)         schema.names.push_back("range");
    if (cfg.use_upper_wick)    schema.names.push_back("upper_wick");
    if (cfg.use_lower_wick)    schema.names.push_back("lower_wick");
    if (cfg.use_vol_z)         schema.names.push_back("vol_z");
    if (cfg.use_volatility_16) schema.names.push_back("volatility_16");
    if (cfg.use_ma_gap_16)     schema.names.push_back("ma_gap_16");

    return schema;
}

double safe_div(double x, double y, double eps) {
    if (std::abs(y) < eps) return 0.0;
    return x / y;
}

double safe_log_return(double prev_close, double curr_close) {
    if (prev_close <= 0.0 || curr_close <= 0.0) return 0.0;
    return std::log(curr_close / prev_close);
}

RollingStats compute_window_stats(const std::vector<double>& values,
                                  int start_idx,
                                  int end_idx) {
    if (start_idx < 0 || end_idx < start_idx || end_idx >= (int)values.size()) {
        throw std::runtime_error("compute_window_stats: invalid window");
    }

    double mean = 0.0;
    double m2   = 0.0;
    int n = 0;

    for (int i = start_idx; i <= end_idx; ++i) {
        double x = values[i];
        ++n;
        double delta = x - mean;
        mean += delta / n;
        double delta2 = x - mean;
        m2 += delta * delta2;
    }

    RollingStats out;
    out.mean = mean;
    out.std  = (n > 0) ? std::sqrt(std::max(0.0, m2 / n)) : 0.0;
    return out;
}

std::vector<FeatureVector> build_features(
    const std::vector<Candle>& candles,
    const FeatureConfig& cfg) {

    std::vector<FeatureVector> out;
    out.reserve(candles.size());

    std::vector<double> closes;
    std::vector<double> volumes;
    closes.reserve(candles.size());
    volumes.reserve(candles.size());

    for (const auto& c : candles) {
        closes.push_back(c.close);
        volumes.push_back(c.volume);
    }

    for (int i = 0; i < (int)candles.size(); ++i) {
        const Candle& c = candles[i];
        FeatureVector fv;
        fv.timestamp = c.timestamp;

        const double ret_1 = (i > 0)
            ? safe_log_return(candles[i - 1].close, c.close)
            : 0.0;

        const double body = safe_div(c.close - c.open, c.open);
        const double range = safe_div(c.high - c.low, c.open);

        const double upper_wick =
            safe_div(c.high - std::max(c.open, c.close), c.open);

        const double lower_wick =
            safe_div(std::min(c.open, c.close) - c.low, c.open);

        double vol_z = 0.0;
        if (cfg.use_vol_z && i >= cfg.vol_z_window - 1) {
            auto rs = compute_window_stats(
                volumes,
                i - cfg.vol_z_window + 1,
                i
            );
            vol_z = safe_div(c.volume - rs.mean, rs.std, 1e-12);
        }

        double volatility_16 = 0.0;
        if (cfg.use_volatility_16 && i >= cfg.volatility_window) {
            std::vector<double> rets;
            rets.reserve(cfg.volatility_window);
            for (int k = i - cfg.volatility_window + 1; k <= i; ++k) {
                double r = (k > 0)
                    ? safe_log_return(candles[k - 1].close, candles[k].close)
                    : 0.0;
                rets.push_back(r);
            }
            auto rs = compute_window_stats(rets, 0, (int)rets.size() - 1);
            volatility_16 = rs.std;
        }

        double ma_gap_16 = 0.0;
        if (cfg.use_ma_gap_16 && i >= cfg.ma_window - 1) {
            auto rs = compute_window_stats(
                closes,
                i - cfg.ma_window + 1,
                i
            );
            ma_gap_16 = safe_div(c.close - rs.mean, rs.mean);
        }

        if (cfg.use_ret_1)         fv.values.push_back(finite_or_zero(ret_1));
        if (cfg.use_body)          fv.values.push_back(finite_or_zero(body));
        if (cfg.use_range)         fv.values.push_back(finite_or_zero(range));
        if (cfg.use_upper_wick)    fv.values.push_back(finite_or_zero(upper_wick));
        if (cfg.use_lower_wick)    fv.values.push_back(finite_or_zero(lower_wick));
        if (cfg.use_vol_z)         fv.values.push_back(finite_or_zero(vol_z));
        if (cfg.use_volatility_16) fv.values.push_back(finite_or_zero(volatility_16));
        if (cfg.use_ma_gap_16)     fv.values.push_back(finite_or_zero(ma_gap_16));

        out.push_back(std::move(fv));
    }

    return out;
}

std::vector<FeatureValidity> build_feature_validity(
    const std::vector<Candle>& candles,
    const FeatureConfig& cfg) {

    std::vector<FeatureValidity> out(candles.size());

    for (int i = 0; i < (int)candles.size(); ++i) {
        FeatureValidity v;

        v.has_ret_1         = (i >= 1);
        v.has_vol_z         = (!cfg.use_vol_z) || (i >= cfg.vol_z_window - 1);
        v.has_volatility_16 = (!cfg.use_volatility_16) || (i >= cfg.volatility_window);
        v.has_ma_gap_16     = (!cfg.use_ma_gap_16) || (i >= cfg.ma_window - 1);

        out[i] = v;
    }

    return out;
}

} // namespace capigrad
