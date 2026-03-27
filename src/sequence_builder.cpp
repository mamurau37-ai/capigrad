#include "sequence_builder.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace capigrad {

namespace {
static constexpr std::uint32_t DATA_MAGIC   = 0x44535431; // "DST1"
static constexpr std::uint32_t DATA_VERSION = 1;
}

bool is_continuous_window(const std::vector<Candle>& candles,
                          int start_idx,
                          int end_idx,
                          std::int64_t expected_delta_ms) {
    if (start_idx < 0 || end_idx < start_idx || end_idx >= (int)candles.size()) {
        return false;
    }

    for (int i = start_idx + 1; i <= end_idx; ++i) {
        if (candles[i].timestamp - candles[i - 1].timestamp != expected_delta_ms) {
            return false;
        }
    }

    return true;
}

int find_h1_anchor_index(const std::vector<Candle>& h1_candles,
                         std::int64_t m15_timestamp) {
    int lo = 0;
    int hi = static_cast<int>(h1_candles.size()) - 1;
    int ans = -1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (h1_candles[mid].timestamp <= m15_timestamp) {
            ans = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    return ans;
}

SampleValidityReport validate_sample_at(
    int m15_idx,
    const std::vector<Candle>& m15_candles,
    const std::vector<Candle>& h1_candles,
    const std::vector<FeatureValidity>& m15_validity,
    const std::vector<FeatureValidity>& h1_validity,
    const std::vector<LabelTarget>& labels,
    const SequenceConfig& seq_cfg,
    const LabelConfig& label_cfg) {

    SampleValidityReport r;
    r.timestamp = m15_candles[m15_idx].timestamp;

    const int m15_start = m15_idx - seq_cfg.m15_len + 1;
    r.m15_history_ok = is_continuous_window(
        m15_candles, m15_start, m15_idx, M15_INTERVAL_MS
    );

    r.future_horizon_ok = is_continuous_window(
        m15_candles, m15_idx, m15_idx + label_cfg.horizon_m15, M15_INTERVAL_MS
    );

    int h1_anchor = find_h1_anchor_index(h1_candles, m15_candles[m15_idx].timestamp);
    r.h1_anchor_ok = (h1_anchor >= 0);

    if (r.h1_anchor_ok) {
        int h1_start = h1_anchor - seq_cfg.h1_len + 1;
        r.h1_history_ok = is_continuous_window(
            h1_candles, h1_start, h1_anchor, H1_INTERVAL_MS
        );
    } else {
        r.h1_history_ok = false;
    }

    r.m15_features_ok = true;
    for (int i = m15_start; i <= m15_idx; ++i) {
        if (i < 0 || i >= (int)m15_validity.size() || !m15_validity[i].all_valid()) {
            r.m15_features_ok = false;
            break;
        }
    }

    r.h1_features_ok = r.h1_anchor_ok;
    if (r.h1_anchor_ok) {
        int h1_start = h1_anchor - seq_cfg.h1_len + 1;
        for (int i = h1_start; i <= h1_anchor; ++i) {
            if (i < 0 || i >= (int)h1_validity.size() || !h1_validity[i].all_valid()) {
                r.h1_features_ok = false;
                break;
            }
        }
    }

    r.label_ok = (m15_idx >= 0 && m15_idx < (int)labels.size() && labels[m15_idx].valid);

    return r;
}

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
    SampleValidityReport* out_report) {

    SampleValidityReport rep = validate_sample_at(
        m15_idx,
        m15_candles,
        h1_candles,
        m15_validity,
        h1_validity,
        labels,
        seq_cfg,
        label_cfg
    );

    if (out_report) *out_report = rep;
    if (!rep.all_valid()) return false;

    const int m15_start = m15_idx - seq_cfg.m15_len + 1;
    const int h1_anchor = find_h1_anchor_index(h1_candles, m15_candles[m15_idx].timestamp);
    const int h1_start  = h1_anchor - seq_cfg.h1_len + 1;

    out_sample.timestamp = m15_candles[m15_idx].timestamp;
    out_sample.label = label_to_index(labels[m15_idx].label);
    out_sample.future_return = labels[m15_idx].future_return;
    out_sample.threshold = labels[m15_idx].threshold;

    out_sample.m15_seq.clear();
    out_sample.h1_seq.clear();

    for (int i = m15_start; i <= m15_idx; ++i) {
        out_sample.m15_seq.push_back(m15_features[i].values);
    }

    for (int i = h1_start; i <= h1_anchor; ++i) {
        out_sample.h1_seq.push_back(h1_features[i].values);
    }

    return true;
}

OfflineDataset build_offline_dataset(
    const std::vector<Candle>& m15_candles,
    const std::vector<Candle>& h1_candles,
    const std::vector<FeatureVector>& m15_features,
    const std::vector<FeatureVector>& h1_features,
    const std::vector<FeatureValidity>& m15_validity,
    const std::vector<FeatureValidity>& h1_validity,
    const std::vector<LabelTarget>& labels,
    const SequenceConfig& seq_cfg,
    const LabelConfig& label_cfg) {

    OfflineDataset ds;

    for (int i = 0; i < (int)m15_candles.size(); ++i) {
        OfflineSample sample;
        if (build_sample_at(
                i,
                m15_candles,
                h1_candles,
                m15_features,
                h1_features,
                m15_validity,
                h1_validity,
                labels,
                seq_cfg,
                label_cfg,
                sample,
                nullptr)) {
            ds.samples.push_back(std::move(sample));
        }
    }

    return ds;
}

DatasetSplit split_dataset_by_ratio(const OfflineDataset& ds,
                                    double train_ratio,
                                    double val_ratio,
                                    double test_ratio) {
    if (ds.samples.empty()) {
        return {};
    }

    const double s = train_ratio + val_ratio + test_ratio;
    if (std::abs(s - 1.0) > 1e-9) {
        throw std::runtime_error("split_dataset_by_ratio ratios must sum to 1.0");
    }

    DatasetSplit out;
    const int n = ds.size();
    const int n_train = static_cast<int>(n * train_ratio);
    const int n_val   = static_cast<int>(n * val_ratio);
    const int n_test  = n - n_train - n_val;

    out.train.samples.insert(out.train.samples.end(),
                             ds.samples.begin(),
                             ds.samples.begin() + n_train);

    out.val.samples.insert(out.val.samples.end(),
                           ds.samples.begin() + n_train,
                           ds.samples.begin() + n_train + n_val);

    out.test.samples.insert(out.test.samples.end(),
                            ds.samples.begin() + n_train + n_val,
                            ds.samples.end());

    if (out.test.size() != n_test) {
        throw std::runtime_error("split_dataset_by_ratio internal split error");
    }

    return out;
}

void save_dataset_binary(const OfflineDataset& ds, const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("save_dataset_binary failed to open file: " + path);
    }

    std::uint32_t magic = DATA_MAGIC;
    std::uint32_t version = DATA_VERSION;
    std::uint32_t n = static_cast<std::uint32_t>(ds.samples.size());

    std::uint32_t m15_len = 0, m15_dim = 0, h1_len = 0, h1_dim = 0;
    if (!ds.samples.empty()) {
        m15_len = static_cast<std::uint32_t>(ds.samples[0].m15_seq.size());
        h1_len  = static_cast<std::uint32_t>(ds.samples[0].h1_seq.size());
        m15_dim = m15_len > 0 ? static_cast<std::uint32_t>(ds.samples[0].m15_seq[0].size()) : 0;
        h1_dim  = h1_len  > 0 ? static_cast<std::uint32_t>(ds.samples[0].h1_seq[0].size()) : 0;
    }

    f.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    f.write(reinterpret_cast<const char*>(&version), sizeof(version));
    f.write(reinterpret_cast<const char*>(&n), sizeof(n));
    f.write(reinterpret_cast<const char*>(&m15_len), sizeof(m15_len));
    f.write(reinterpret_cast<const char*>(&m15_dim), sizeof(m15_dim));
    f.write(reinterpret_cast<const char*>(&h1_len), sizeof(h1_len));
    f.write(reinterpret_cast<const char*>(&h1_dim), sizeof(h1_dim));

    for (const auto& s : ds.samples) {
        f.write(reinterpret_cast<const char*>(&s.timestamp), sizeof(s.timestamp));

        for (const auto& row : s.m15_seq) {
            for (double x : row) {
                float xf = static_cast<float>(x);
                f.write(reinterpret_cast<const char*>(&xf), sizeof(float));
            }
        }

        for (const auto& row : s.h1_seq) {
            for (double x : row) {
                float xf = static_cast<float>(x);
                f.write(reinterpret_cast<const char*>(&xf), sizeof(float));
            }
        }

        float future_return_f = static_cast<float>(s.future_return);
        f.write(reinterpret_cast<const char*>(&future_return_f), sizeof(float));

        std::int8_t label_i8 = static_cast<std::int8_t>(s.label);
        f.write(reinterpret_cast<const char*>(&label_i8), sizeof(label_i8));
    }

    if (!f) {
        throw std::runtime_error("save_dataset_binary write error: " + path);
    }
}

OfflineDataset load_dataset_binary(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("load_dataset_binary failed to open file: " + path);
    }

    std::uint32_t magic = 0, version = 0, n = 0;
    std::uint32_t m15_len = 0, m15_dim = 0, h1_len = 0, h1_dim = 0;

    f.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    f.read(reinterpret_cast<char*>(&version), sizeof(version));
    f.read(reinterpret_cast<char*>(&n), sizeof(n));
    f.read(reinterpret_cast<char*>(&m15_len), sizeof(m15_len));
    f.read(reinterpret_cast<char*>(&m15_dim), sizeof(m15_dim));
    f.read(reinterpret_cast<char*>(&h1_len), sizeof(h1_len));
    f.read(reinterpret_cast<char*>(&h1_dim), sizeof(h1_dim));

    if (magic != DATA_MAGIC) {
        throw std::runtime_error("load_dataset_binary invalid magic: " + path);
    }
    if (version != DATA_VERSION) {
        throw std::runtime_error("load_dataset_binary invalid version: " + path);
    }

    OfflineDataset ds;
    ds.samples.reserve(n);

    for (std::uint32_t i = 0; i < n; ++i) {
        OfflineSample s;
        f.read(reinterpret_cast<char*>(&s.timestamp), sizeof(s.timestamp));

        s.m15_seq.assign(m15_len, std::vector<double>(m15_dim, 0.0));
        s.h1_seq.assign(h1_len, std::vector<double>(h1_dim, 0.0));

        for (std::uint32_t r = 0; r < m15_len; ++r) {
            for (std::uint32_t c = 0; c < m15_dim; ++c) {
                float xf = 0.0f;
                f.read(reinterpret_cast<char*>(&xf), sizeof(float));
                s.m15_seq[r][c] = xf;
            }
        }

        for (std::uint32_t r = 0; r < h1_len; ++r) {
            for (std::uint32_t c = 0; c < h1_dim; ++c) {
                float xf = 0.0f;
                f.read(reinterpret_cast<char*>(&xf), sizeof(float));
                s.h1_seq[r][c] = xf;
            }
        }

        float future_return_f = 0.0f;
        f.read(reinterpret_cast<char*>(&future_return_f), sizeof(float));
        s.future_return = future_return_f;

        std::int8_t label_i8 = 0;
        f.read(reinterpret_cast<char*>(&label_i8), sizeof(label_i8));
        s.label = static_cast<int>(label_i8);

        ds.samples.push_back(std::move(s));
    }

    if (!f) {
        throw std::runtime_error("load_dataset_binary read error: " + path);
    }

    return ds;
}

} // namespace capigrad
