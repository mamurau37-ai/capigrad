#ifndef CAPIGRAD_CONTEXT_BUILDER_HPP
#define CAPIGRAD_CONTEXT_BUILDER_HPP

#include <deque>
#include <vector>

#include "features.hpp"
#include "labeler.hpp"
#include "normalizer.hpp"
#include "sequence_builder.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// ContextBuilder:
// keeps rolling M15/H1 candle history and builds normalized OfflineSample
// for live offline-model inference.
//
// Notes:
// - label is unknown in live mode, so sample.label will be set to FLAT(=1) by default
// - future_return / threshold are placeholders in live mode
// - only historical context is used
// -----------------------------------------------------------------------------
class ContextBuilder {
public:
    ContextBuilder(const FeatureConfig& feature_cfg,
                   const SequenceConfig& seq_cfg,
                   const FeatureNormalizer& m15_norm,
                   const FeatureNormalizer& h1_norm);

    // reset all rolling candle history
    void reset();

    // push freshly closed candle
    void on_m15_candle(const Candle& c);
    void on_h1_candle(const Candle& c);

    // ready only if enough rolling history exists and all feature windows valid
    bool ready() const;

    // build live inference sample
    OfflineSample build_sample() const;

    // accessors
    int m15_count() const;
    int h1_count() const;

private:
    FeatureConfig feature_cfg_;
    SequenceConfig seq_cfg_;
    FeatureNormalizer m15_norm_;
    FeatureNormalizer h1_norm_;

    std::deque<Candle> m15_candles_;
    std::deque<Candle> h1_candles_;

    // internal helpers
    static std::vector<Candle> deque_to_vector(const std::deque<Candle>& dq);

    bool check_continuity(const std::vector<Candle>& candles,
                          std::int64_t expected_delta_ms) const;
};

} // namespace capigrad

#endif // CAPIGRAD_CONTEXT_BUILDER_HPP
