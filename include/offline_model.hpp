#ifndef CAPIGRAD_OFFLINE_MODEL_HPP
#define CAPIGRAD_OFFLINE_MODEL_HPP

// ════════════════════════════════════════════════════════════════
//  offline_model.hpp — CapiGrad Dual-Timeframe Offline Model
//  nn.hpp v2 ga mos. Yangi o'zgarishlar:
//  - ActType::CAPI / ActType::LINEAR (enum class)
//  - Layer::update(lr, t, wd) unified
//  - save_weights / load_weights alohida
//  - save_optimizer / load_optimizer alohida
//  - backward_through_time bilan haqiqiy branch training
// ════════════════════════════════════════════════════════════════

#include <string>
#include <vector>

#include "nn.hpp"
#include "sequence_builder.hpp"

namespace capigrad {

// ────────────────────────────────────────────────────────────────
// Model hyperparameters
// ────────────────────────────────────────────────────────────────
struct OfflineModelConfig {
    int m15_embed_dim    = 32;
    int h1_embed_dim     = 24;
    int m15_hidden_dim   = 48;
    int h1_hidden_dim    = 32;
    int fusion_hidden_dim = 32;
    int num_classes      = 3;

    double lr           = 3e-4;
    double weight_decay = 1e-5;
    int bptt_window     = 32;
};

// ────────────────────────────────────────────────────────────────
// OfflineDualTFModel
//
// Architecture:
//   M15 encoder (dense) → M15 GRU
//   H1 encoder (dense)  → H1 GRU
//   concat(M15 hidden, H1 hidden) → fusion dense → output head
//
// Training:
//   Full branch training via backward_through_time
//   M15/H1 GRU gradient flows through encoder
//   Fusion + output head also trained
// ────────────────────────────────────────────────────────────────
class OfflineDualTFModel {
public:
    OfflineDualTFModel(int m15_input_dim,
                       int h1_input_dim,
                       const OfflineModelConfig& cfg = OfflineModelConfig());

    // Reset all recurrent states and BPTT caches
    void reset_state();

    // Forward: returns raw logits [3]
    std::vector<double> forward(const OfflineSample& sample);

    // Predict class index: 0=DOWN, 1=FLAT, 2=UP
    int predict(const OfflineSample& sample);

    // Full train step with BPTT through branches
    // Returns cross-entropy loss
    double train_step(const OfflineSample& sample,
                      int global_step);

    // Softmax
    std::vector<double> softmax(const std::vector<double>& logits) const;

    // Model info
    int num_classes() const { return cfg_.num_classes; }
    int total_params() const;

    // Save weights only (for inference)
    void save(const std::string& path);
    void load(const std::string& path);

    // Save optimizer state (for training resume)
    void save_full(const std::string& path);
    void load_full(const std::string& path);

    // Config access
    const OfflineModelConfig& config() const { return cfg_; }

private:
    int m15_input_dim_;
    int h1_input_dim_;
    OfflineModelConfig cfg_;

    // M15 branch
    Layer         m15_encoder_;
    TemporalLayer m15_gru_;

    // H1 branch
    Layer         h1_encoder_;
    TemporalLayer h1_gru_;

    // Fusion
    Layer fusion_hidden_;
    Layer output_head_;

    // Forward cache (for backward)
    std::vector<double> last_m15_hidden_;
    std::vector<double> last_h1_hidden_;
    std::vector<double> last_fusion_in_;
    std::vector<double> last_fusion_hidden_;
    std::vector<double> last_logits_;

    // Forward pass helpers
    std::vector<double> run_m15_branch(const std::vector<std::vector<double>>& seq);
    std::vector<double> run_h1_branch(const std::vector<std::vector<double>>& seq);

    std::vector<double> concat(const std::vector<double>& a,
                               const std::vector<double>& b) const;

    double cross_entropy(const std::vector<double>& probs,
                         int target) const;

    static constexpr double EPS = 1e-12;
    static constexpr uint32_t MAGIC   = 0x43504D44; // "CPMD"
    static constexpr uint32_t VERSION = 2;
};

} // namespace capigrad

#endif // CAPIGRAD_OFFLINE_MODEL_HPP
