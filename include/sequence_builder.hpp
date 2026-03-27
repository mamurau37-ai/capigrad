#ifndef CAPIGRAD_SEQUENCE_BUILDER_HPP
#define CAPIGRAD_SEQUENCE_BUILDER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "features.hpp"
#include "labeler.hpp"

namespace capigrad {

static constexpr std::int64_t M15_INTERVAL_MS = 15LL * 60LL * 1000LL;
static constexpr std::int64_t H1_INTERVAL_MS  = 60LL * 60LL * 1000LL;

// Final training sample
struct OfflineSample {
    std::int64_t timestamp = 0;

    std::vector<std::vector<double>> m15_seq; // [m15_len][m15_dim]
    std::vector<std::vector<double>> h1_seq;  // [h1_len][h1_dim]

    int label = 1; // FLAT by default
    double future_return = 0.0;
    double threshold     = 0.0;
};

struct OfflineDataset {
    std::vector<OfflineSample> samples;

    int size() const {
        return static_cast<int>(samples.size());
    }

    bool empty() const {
        return samples.empty();
    }
};

struct SequenceConfig {
    int m15_len = 64;
    int h1_len  = 32;
};

struct SampleValidityReport {
    std::int64_t timestamp = 0;

    bool m15_history_ok    = false;
    bool h1_anchor_ok      = false;
    bool h1_history_ok     = false;
    bool future_horizon_ok = false;
    bool m15_features_ok   = false;
    bool h1_features_ok    = false;
    bool label_ok          = false;

    bool all_valid() const {
        return m15_history_ok &&
               h1_anchor_ok &&
               h1_history_ok &&
               future_horizon_ok &&
               m15_features_ok &&
               h1_features_ok &&
               label_ok;
    }
};

bool is_continuous_window(const std::vector<Candle>& candles,
                          int start_idx,
                          int end_idx,
                          std::int64_t expected_delta_ms);

int find_h1_anchor_index(const std::vector<Candle>& h1_candles,
                         std::int64_t m15_timestamp);

SampleValidityReport validate_sample_at(
    int m15_idx,
    const std::vector<Candle>& m15_candles,
    const std::vector<Candle>& h1_candles,
    const std::vector<FeatureValidity>& m15_validity,
    const std::vector<FeatureValidity>& h1_validity,
    const std::vector<LabelTarget>& labels,
    const SequenceConfig& seq_cfg,
    const LabelConfig& label_cfg);

bool build_sample_at(
    int m15_idx,
    const std::vector<Candle>& m15_candles,
    const std::vector<Candle>& h1_candles,
    const std::vector<FeatureVector>& m15_features,
    const std::vector<FeatureVector>& h1_features,
    const std::vector<FeatureValidity>& m15_validity,
    const std::vector<FeatureValidity>& h1_validity,
    const std::vector<LabelTarget>& labels,
    const SequenceConfig& seq_cfg,
    const LabelConfig& label_cfg,
    OfflineSample& out_sample,
    SampleValidityReport* out_report = nullptr);

OfflineDataset build_offline_dataset(
    const std::vector<Candle>& m15_candles,
    const std::vector<Candle>& h1_candles,
    const std::vector<FeatureVector>& m15_features,
    const std::vector<FeatureVector>& h1_features,
    const std::vector<FeatureValidity>& m15_validity,
    const std::vector<FeatureValidity>& h1_validity,
    const std::vector<LabelTarget>& labels,
    const SequenceConfig& seq_cfg,
    const LabelConfig& label_cfg);

struct DatasetSplit {
    OfflineDataset train;
    OfflineDataset val;
    OfflineDataset test;
};

DatasetSplit split_dataset_by_ratio(const OfflineDataset& ds,
                                    double train_ratio,
                                    double val_ratio,
                                    double test_ratio);

// Binary I/O
void save_dataset_binary(const OfflineDataset& ds, const std::string& path);
OfflineDataset load_dataset_binary(const std::string& path);

} // namespace capigrad

#endif // CAPIGRAD_SEQUENCE_BUILDER_HPP
