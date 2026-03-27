
#include "bootstrap_context.hpp"
#include "context_builder.hpp"

#include <stdexcept>

namespace capigrad {

ContextBuilder::ContextBuilder(const FeatureConfig& feature_cfg,
                               const SequenceConfig& seq_cfg,
                               const FeatureNormalizer& m15_norm,
                               const FeatureNormalizer& h1_norm)
    : feature_cfg_(feature_cfg),
      seq_cfg_(seq_cfg),
      m15_norm_(m15_norm),
      h1_norm_(h1_norm) {}

void ContextBuilder::reset() {
    m15_candles_.clear();
    h1_candles_.clear();
}

void ContextBuilder::on_m15_candle(const Candle& c) {
    m15_candles_.push_back(c);

    const int keep = std::max(seq_cfg_.m15_len + 64, seq_cfg_.m15_len + 16);
    while ((int)m15_candles_.size() > keep) {
        m15_candles_.pop_front();
    }
}

void ContextBuilder::on_h1_candle(const Candle& c) {
    h1_candles_.push_back(c);

    const int keep = std::max(seq_cfg_.h1_len + 64, seq_cfg_.h1_len + 16);
    while ((int)h1_candles_.size() > keep) {
        h1_candles_.pop_front();
    }
}

int ContextBuilder::m15_count() const {
    return static_cast<int>(m15_candles_.size());
}

int ContextBuilder::h1_count() const {
    return static_cast<int>(h1_candles_.size());
}

std::vector<Candle> ContextBuilder::deque_to_vector(const std::deque<Candle>& dq) {
    return std::vector<Candle>(dq.begin(), dq.end());
}

bool ContextBuilder::check_continuity(const std::vector<Candle>& candles,
                                      std::int64_t expected_delta_ms) const {
    if (candles.empty()) return false;

    for (int i = 1; i < (int)candles.size(); ++i) {
        if (candles[i].timestamp - candles[i - 1].timestamp != expected_delta_ms) {
            return false;
        }
    }
    return true;
}

bool ContextBuilder::ready() const {
    if ((int)m15_candles_.size() < seq_cfg_.m15_len) return false;
    if ((int)h1_candles_.size()  < seq_cfg_.h1_len)  return false;

    std::vector<Candle> m15 = deque_to_vector(m15_candles_);
    std::vector<Candle> h1  = deque_to_vector(h1_candles_);

    std::vector<Candle> m15_tail(m15.end() - seq_cfg_.m15_len, m15.end());
    std::vector<Candle> h1_tail(h1.end()  - seq_cfg_.h1_len,  h1.end());

    if (!check_continuity(m15_tail, M15_INTERVAL_MS)) return false;
    if (!check_continuity(h1_tail,  H1_INTERVAL_MS))  return false;

    std::vector<FeatureValidity> m15_validity = build_feature_validity(m15, feature_cfg_);
    std::vector<FeatureValidity> h1_validity  = build_feature_validity(h1, feature_cfg_);

    for (int i = (int)m15_validity.size() - seq_cfg_.m15_len; i < (int)m15_validity.size(); ++i) {
        if (i < 0 || !m15_validity[i].all_valid()) return false;
    }

    for (int i = (int)h1_validity.size() - seq_cfg_.h1_len; i < (int)h1_validity.size(); ++i) {
        if (i < 0 || !h1_validity[i].all_valid()) return false;
    }

    return true;
}

OfflineSample ContextBuilder::build_sample() const {
    if (!ready()) {
        throw std::runtime_error("ContextBuilder::build_sample called before ready()");
    }

    std::vector<Candle> m15 = deque_to_vector(m15_candles_);
    std::vector<Candle> h1  = deque_to_vector(h1_candles_);

    std::vector<FeatureVector> m15_feats = build_features(m15, feature_cfg_);
    std::vector<FeatureVector> h1_feats  = build_features(h1, feature_cfg_);

    OfflineSample sample;
    sample.timestamp = m15.back().timestamp;

    // live mode placeholders
    sample.label = 1; // FLAT
    sample.future_return = 0.0;
    sample.threshold = 0.0;

    sample.m15_seq.clear();
    sample.h1_seq.clear();

    for (int i = (int)m15_feats.size() - seq_cfg_.m15_len; i < (int)m15_feats.size(); ++i) {
        sample.m15_seq.push_back(m15_feats[i].values);
    }

    for (int i = (int)h1_feats.size() - seq_cfg_.h1_len; i < (int)h1_feats.size(); ++i) {
        sample.h1_seq.push_back(h1_feats[i].values);
    }

    m15_norm_.normalize_matrix_inplace(sample.m15_seq);
    h1_norm_.normalize_matrix_inplace(sample.h1_seq);

    return sample;
}

} // namespace capigrad
