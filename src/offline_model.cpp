#include "offline_model.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <cstdint>

namespace capigrad {

OfflineDualTFModel::OfflineDualTFModel(int m15_input_dim,
                                       int h1_input_dim,
                                       const OfflineModelConfig& cfg)
    : m15_input_dim_(m15_input_dim),
      h1_input_dim_(h1_input_dim),
      cfg_(cfg),

      // M15 branch
      m15_encoder_(m15_input_dim, cfg.m15_embed_dim, ActType::CAPI),
      m15_gru_(cfg.m15_embed_dim, cfg.m15_hidden_dim),

      // H1 branch
      h1_encoder_(h1_input_dim, cfg.h1_embed_dim, ActType::CAPI),
      h1_gru_(cfg.h1_embed_dim, cfg.h1_hidden_dim),

      // Fusion
      fusion_hidden_(cfg.m15_hidden_dim + cfg.h1_hidden_dim,
                     cfg.fusion_hidden_dim,
                     ActType::CAPI),
      output_head_(cfg.fusion_hidden_dim,
                   cfg.num_classes,
                   ActType::LINEAR) {}

void OfflineDualTFModel::reset_state() {
    m15_gru_.reset();
    h1_gru_.reset();

    last_m15_hidden_.clear();
    last_h1_hidden_.clear();
    last_fusion_in_.clear();
    last_fusion_hidden_.clear();
    last_logits_.clear();
}

std::vector<double> OfflineDualTFModel::concat(const std::vector<double>& a,
                                               const std::vector<double>& b) const {
    std::vector<double> out;
    out.reserve(a.size() + b.size());
    out.insert(out.end(), a.begin(), a.end());
    out.insert(out.end(), b.begin(), b.end());
    return out;
}

std::vector<double> OfflineDualTFModel::run_m15_branch(
    const std::vector<std::vector<double>>& seq) {

    m15_gru_.reset();
    std::vector<double> h;

    for (const auto& x : seq) {
        std::vector<double> e = m15_encoder_.forward(x);
        h = m15_gru_.forward(e);
    }

    return h;
}

std::vector<double> OfflineDualTFModel::run_h1_branch(
    const std::vector<std::vector<double>>& seq) {

    h1_gru_.reset();
    std::vector<double> h;

    for (const auto& x : seq) {
        std::vector<double> e = h1_encoder_.forward(x);
        h = h1_gru_.forward(e);
    }

    return h;
}

std::vector<double> OfflineDualTFModel::forward(const OfflineSample& sample) {
    if (sample.m15_seq.empty() || sample.h1_seq.empty()) {
        throw std::runtime_error("OfflineDualTFModel::forward: empty sequence");
    }

    last_m15_hidden_ = run_m15_branch(sample.m15_seq);
    last_h1_hidden_  = run_h1_branch(sample.h1_seq);

    last_fusion_in_     = concat(last_m15_hidden_, last_h1_hidden_);
    last_fusion_hidden_ = fusion_hidden_.forward(last_fusion_in_);
    last_logits_        = output_head_.forward(last_fusion_hidden_);

    return last_logits_;
}

std::vector<double> OfflineDualTFModel::softmax(const std::vector<double>& logits) const {
    if (logits.empty()) return {};

    double mx = *std::max_element(logits.begin(), logits.end());

    std::vector<double> ex(logits.size());
    double sum = 0.0;
    for (int i = 0; i < (int)logits.size(); ++i) {
        ex[i] = std::exp(logits[i] - mx);
        sum += ex[i];
    }
    if (sum < EPS) sum = EPS;
    for (double& x : ex) x /= sum;
    return ex;
}

double OfflineDualTFModel::cross_entropy(const std::vector<double>& probs,
                                          int target) const {
    if (target < 0 || target >= (int)probs.size()) {
        throw std::runtime_error("cross_entropy: invalid target");
    }
    return -std::log(std::max(probs[target], EPS));
}

int OfflineDualTFModel::predict(const OfflineSample& sample) {
    auto logits = forward(sample);
    auto probs  = softmax(logits);

    int best = 0;
    for (int i = 1; i < (int)probs.size(); ++i) {
        if (probs[i] > probs[best]) best = i;
    }
    return best;
}

int OfflineDualTFModel::total_params() const {
    return m15_encoder_.param_count()
         + m15_gru_.param_count()
         + h1_encoder_.param_count()
         + h1_gru_.param_count()
         + fusion_hidden_.param_count()
         + output_head_.param_count();
}

double OfflineDualTFModel::train_step(const OfflineSample& sample,
                                       int global_step) {
    if (sample.label < 0 || sample.label >= cfg_.num_classes) {
        throw std::runtime_error("train_step: invalid label");
    }

    // ── Forward ──────────────────────────────────────────────────
    // Need fresh encoder/gru runs with caches populated
    m15_gru_.reset();
    h1_gru_.reset();

    // M15 branch forward (caches filled per-step in GRU)
    std::vector<double> m15_enc_out;
    for (const auto& x : sample.m15_seq) {
        m15_enc_out = m15_encoder_.forward(x);
        m15_gru_.forward(m15_enc_out);
    }
    last_m15_hidden_ = m15_gru_.h;

    // H1 branch forward (caches filled per-step in GRU)
    std::vector<double> h1_enc_out;
    for (const auto& x : sample.h1_seq) {
        h1_enc_out = h1_encoder_.forward(x);
        h1_gru_.forward(h1_enc_out);
    }
    last_h1_hidden_ = h1_gru_.h;

    last_fusion_in_     = concat(last_m15_hidden_, last_h1_hidden_);
    last_fusion_hidden_ = fusion_hidden_.forward(last_fusion_in_);
    last_logits_        = output_head_.forward(last_fusion_hidden_);

    auto probs = softmax(last_logits_);
    double loss = cross_entropy(probs, sample.label);

    // ── Backward ─────────────────────────────────────────────────
    // dL/dlogits = probs - onehot
    std::vector<double> dlogits = probs;
    dlogits[sample.label] -= 1.0;

    // Output head backward
    std::vector<double> dfusion_hidden = output_head_.backward(dlogits);

    // Fusion hidden backward
    std::vector<double> dfusion_in = fusion_hidden_.backward(dfusion_hidden);

    // Split gradient for branches
    std::vector<double> dm15(dfusion_in.begin(),
                             dfusion_in.begin() + cfg_.m15_hidden_dim);
    std::vector<double> dh1(dfusion_in.begin() + cfg_.m15_hidden_dim,
                            dfusion_in.end());

    // ── M15 branch full BPTT ─────────────────────────────────────
    GRUBackwardResult m15_bptt = m15_gru_.backward_through_time(
        dm15, cfg_.bptt_window);

    // Propagate GRU input gradient back through encoder
    // The encoder received the last sequence item
    if (!sample.m15_seq.empty()) {
        // Use last encoder output gradient to update encoder
        m15_encoder_.backward(m15_bptt.dx);
    }

    // ── H1 branch full BPTT ──────────────────────────────────────
    GRUBackwardResult h1_bptt = h1_gru_.backward_through_time(
        dh1, cfg_.bptt_window);

    if (!sample.h1_seq.empty()) {
        h1_encoder_.backward(h1_bptt.dx);
    }

    // ── Update all parameters ────────────────────────────────────
    const double lr = cfg_.lr;
    const double wd = cfg_.weight_decay;

    output_head_.update(lr, global_step, wd);
    fusion_hidden_.update(lr, global_step, wd);

    m15_gru_.update(lr, global_step, wd);
    m15_encoder_.update(lr, global_step, wd);

    h1_gru_.update(lr, global_step, wd);
    h1_encoder_.update(lr, global_step, wd);

    // Flush BPTT caches after update
    m15_gru_.flush_cache();
    h1_gru_.flush_cache();

    return loss;
}

// ── Save / Load ──────────────────────────────────────────────────

void OfflineDualTFModel::save(const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("OfflineDualTFModel::save failed: " + path);
    }

    uint32_t magic   = MAGIC;
    uint32_t version = VERSION;
    f.write(reinterpret_cast<const char*>(&magic),   sizeof(magic));
    f.write(reinterpret_cast<const char*>(&version), sizeof(version));

    f.write(reinterpret_cast<const char*>(&m15_input_dim_), sizeof(m15_input_dim_));
    f.write(reinterpret_cast<const char*>(&h1_input_dim_),  sizeof(h1_input_dim_));

    int m15_embed    = cfg_.m15_embed_dim;
    int h1_embed     = cfg_.h1_embed_dim;
    int m15_hidden   = cfg_.m15_hidden_dim;
    int h1_hidden    = cfg_.h1_hidden_dim;
    int fusion_hidden = cfg_.fusion_hidden_dim;
    int n_classes    = cfg_.num_classes;

    f.write(reinterpret_cast<const char*>(&m15_embed),    sizeof(m15_embed));
    f.write(reinterpret_cast<const char*>(&h1_embed),     sizeof(h1_embed));
    f.write(reinterpret_cast<const char*>(&m15_hidden),   sizeof(m15_hidden));
    f.write(reinterpret_cast<const char*>(&h1_hidden),    sizeof(h1_hidden));
    f.write(reinterpret_cast<const char*>(&fusion_hidden), sizeof(fusion_hidden));
    f.write(reinterpret_cast<const char*>(&n_classes),    sizeof(n_classes));

    m15_encoder_.save_weights(f);
    m15_gru_.save_weights(f);
    h1_encoder_.save_weights(f);
    h1_gru_.save_weights(f);
    fusion_hidden_.save_weights(f);
    output_head_.save_weights(f);

    if (!f) throw std::runtime_error("OfflineDualTFModel::save write error");
}

void OfflineDualTFModel::load(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("OfflineDualTFModel::load failed: " + path);
    }

    uint32_t magic = 0, version = 0;
    f.read(reinterpret_cast<char*>(&magic),   sizeof(magic));
    f.read(reinterpret_cast<char*>(&version), sizeof(version));

    if (magic != MAGIC) {
        throw std::runtime_error("OfflineDualTFModel::load invalid magic");
    }

    int m15_input = 0, h1_input = 0;
    int m15_embed = 0, h1_embed = 0;
    int m15_hidden = 0, h1_hidden = 0;
    int fusion_hidden = 0, n_classes = 0;

    f.read(reinterpret_cast<char*>(&m15_input),     sizeof(m15_input));
    f.read(reinterpret_cast<char*>(&h1_input),      sizeof(h1_input));
    f.read(reinterpret_cast<char*>(&m15_embed),     sizeof(m15_embed));
    f.read(reinterpret_cast<char*>(&h1_embed),      sizeof(h1_embed));
    f.read(reinterpret_cast<char*>(&m15_hidden),    sizeof(m15_hidden));
    f.read(reinterpret_cast<char*>(&h1_hidden),     sizeof(h1_hidden));
    f.read(reinterpret_cast<char*>(&fusion_hidden), sizeof(fusion_hidden));
    f.read(reinterpret_cast<char*>(&n_classes),     sizeof(n_classes));

    if (m15_input   != m15_input_dim_        ||
        h1_input    != h1_input_dim_         ||
        m15_embed   != cfg_.m15_embed_dim    ||
        h1_embed    != cfg_.h1_embed_dim     ||
        m15_hidden  != cfg_.m15_hidden_dim   ||
        h1_hidden   != cfg_.h1_hidden_dim    ||
        fusion_hidden != cfg_.fusion_hidden_dim ||
        n_classes   != cfg_.num_classes) {
        throw std::runtime_error("OfflineDualTFModel::load architecture mismatch");
    }

    m15_encoder_.load_weights(f);
    m15_gru_.load_weights(f);
    h1_encoder_.load_weights(f);
    h1_gru_.load_weights(f);
    fusion_hidden_.load_weights(f);
    output_head_.load_weights(f);

    if (!f) throw std::runtime_error("OfflineDualTFModel::load read error");
}

void OfflineDualTFModel::save_full(const std::string& path) {
    save(path);

    std::string opt_path = path + ".optim";
    std::ofstream f(opt_path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("save_full: cannot open optimizer file");
    }

    m15_encoder_.save_optimizer(f);
    m15_gru_.save_optimizer(f);
    h1_encoder_.save_optimizer(f);
    h1_gru_.save_optimizer(f);
    fusion_hidden_.save_optimizer(f);
    output_head_.save_optimizer(f);
}

void OfflineDualTFModel::load_full(const std::string& path) {
    load(path);

    std::string opt_path = path + ".optim";
    std::ifstream f(opt_path, std::ios::binary);
    if (!f.is_open()) return; // optimizer state optional

    m15_encoder_.load_optimizer(f);
    m15_gru_.load_optimizer(f);
    h1_encoder_.load_optimizer(f);
    h1_gru_.load_optimizer(f);
    fusion_hidden_.load_optimizer(f);
    output_head_.load_optimizer(f);
}

} // namespace capigrad
