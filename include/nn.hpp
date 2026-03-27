#ifndef CAPIGRAD_NN_HPP
#define CAPIGRAD_NN_HPP

// ════════════════════════════════════════════════════════════════
//  nn.hpp — CapiGrad Neural Engine v2
//
//  Dizayn prinsiplari:
//  1. AdamW style: weight decay gradientdan ajratilgan
//  2. Unified update policy: Layer va TemporalLayer bir xil
//  3. Optimizer state save/load: training resume uchun
//  4. Truncated BPTT ready: per-step cache, dh_prev
//  5. Gradient hygiene: global clip, nan guard, monitor
//  6. Const correctness: forward const, backward mutable
//  7. Future-proof API: keyin norm, dropout qo'shish oson
// ════════════════════════════════════════════════════════════════

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// ────────────────────────────────────────────────────────────────
// Activation types
// ────────────────────────────────────────────────────────────────
enum class ActType : int {
    CAPI    = 0,
    TANH    = 1,
    RELU    = 2,
    SIGMOID = 3,
    LINEAR  = 4
};

// ────────────────────────────────────────────────────────────────
// Activation functions
// ────────────────────────────────────────────────────────────────
double sigmoid_f(double x);
double dsigmoid_f(double x);
double tanh_f(double x);
double softplus(double x);
double mish_f(double x);
double dmish_f(double x);
double swish_f(double x);
double dswish_f(double x);
double capi_f(double x);
double dcapi_f(double x);
double relu_f(double x);

// Numerically safe clip
double clip(double x, double c = 5.0);

// Finite guard: returns x if finite, else 0.0
double finite_or_zero(double x);

// ────────────────────────────────────────────────────────────────
// AdamW optimizer state
//
// Policy:
//   step() returns gradient update ONLY (no weight decay inside)
//   Weight decay must be applied by caller:
//       w -= step(grad, lr, t);
//       w -= lr * wd * w;       <- decoupled decay
//   Bias never gets weight decay (standard practice)
//
// Why decoupled?
//   - Standard Adam absorbs decay into gradient
//   - AdamW (Loshchilov & Hutter 2019) separates them
//   - This gives better regularization for adaptive optimizers
// ────────────────────────────────────────────────────────────────
struct AdamState {
    double m = 0.0; // first moment (mean)
    double v = 0.0; // second moment (variance)

    // Returns gradient update delta
    // Caller applies: w -= delta; w -= lr*wd*w;
    double step(double grad,
                double lr,
                int t,
                double b1  = 0.9,
                double b2  = 0.999,
                double eps = 1e-8);

    void reset();

    // Optimizer state persistence
    void save(std::ofstream& f) const;
    void load(std::ifstream& f);
};

// ────────────────────────────────────────────────────────────────
// Gradient utilities
// ────────────────────────────────────────────────────────────────
double global_norm(const std::vector<double>& grads);

// Scale all grads so ||grads||_2 <= max_norm
void clip_by_norm(std::vector<double>& grads, double max_norm = 1.0);

// Nan/inf check
bool has_nan_or_inf(const std::vector<double>& v);

// ────────────────────────────────────────────────────────────────
// GradMonitor
//
// Tracks per-update statistics for training stability.
// Use every N steps to detect:
//   - Exploding gradients: grad_norm >> expected
//   - Vanishing gradients: grad_norm ~= 0
//   - Bad LR: update_norm / param_norm > 0.1
// ────────────────────────────────────────────────────────────────
struct GradMonitor {
    double grad_norm   = 0.0;
    double update_norm = 0.0;
    double param_norm  = 0.0;
    int    n           = 0;

    void reset();

    void record(double grad, double update, double param);

    // update_norm / param_norm
    // > 0.1  => LR may be too high
    // < 1e-6 => gradients may be vanishing
    double update_ratio() const;

    void print(const char* tag) const;
};

// ────────────────────────────────────────────────────────────────
// Neuron
//
// Single fully-connected unit with bias.
//
// Update policy:
//   weights: Adam step + decoupled weight decay
//   bias:    Adam step ONLY (no weight decay)
//
// Forward caches sum and out for backward.
// Backward uses cached out for activation derivatives
// (avoids recomputation, improves numerical stability)
// ────────────────────────────────────────────────────────────────
struct Neuron {
    ActType act = ActType::CAPI;

    std::vector<double> w;    // weights
    std::vector<double> dw;   // weight gradients (accumulated)

    double b   = 0.0;         // bias
    double db  = 0.0;         // bias gradient

    double sum = 0.0;         // pre-activation (cached for backward)
    double out = 0.0;         // post-activation (cached for backward)

    std::vector<AdamState> aw; // per-weight optimizer state
    AdamState              ab; // bias optimizer state

    Neuron(int input_dim, ActType a = ActType::CAPI);

    // Forward: caches sum and out
    double forward(const std::vector<double>& x);

    // Backward: uses cached sum/out for activation derivative
    // Returns dx (gradient wrt input)
    // Accumulates dw and db (call update() to apply)
    std::vector<double> backward(const std::vector<double>& x,
                                 double delta);

    // Apply accumulated gradients via AdamW
    // t: global step counter (for bias correction)
    // wd: weight decay coefficient (NOT applied to bias)
    void update(double lr, int t, double wd = 1e-5);

    // Zero accumulated gradients
    void zero_grad();

    // Save/load weights + bias (no optimizer state)
    void save_weights(std::ofstream& f) const;
    void load_weights(std::ifstream& f);

    // Save/load optimizer state (for training resume)
    void save_optimizer(std::ofstream& f) const;
    void load_optimizer(std::ifstream& f);
};

// ────────────────────────────────────────────────────────────────
// Layer
//
// Standard fully-connected layer.
// update() always takes wd to match TemporalLayer policy.
// ────────────────────────────────────────────────────────────────
struct Layer {
    std::vector<Neuron> neurons;
    std::vector<double> last_in; // cached input for backward

    Layer(int input_dim, int output_dim, ActType a = ActType::CAPI);

    std::vector<double> forward(const std::vector<double>& x);

    std::vector<double> backward(const std::vector<double>& delta);

    // wd applied to weights, NOT to biases
    void update(double lr, int t, double wd = 1e-5);

    void zero_grad();

    int input_dim()  const;
    int output_dim() const;
    int param_count() const;

    void save_weights(std::ofstream& f) const;
    void load_weights(std::ifstream& f);

    void save_optimizer(std::ofstream& f) const;
    void load_optimizer(std::ifstream& f);
};

// ────────────────────────────────────────────────────────────────
// GRU Step Cache
//
// Stores all intermediate values needed for backward pass.
// Required for truncated BPTT.
// ────────────────────────────────────────────────────────────────
struct GRUCache {
    std::vector<double> x;       // input at this step
    std::vector<double> h_prev;  // hidden state BEFORE step
    std::vector<double> r;       // reset gate output
    std::vector<double> z;       // update gate output
    std::vector<double> u;       // candidate output
    std::vector<double> su;      // candidate pre-activation
    std::vector<double> h;       // hidden state AFTER step
};

// Return type for truncated BPTT backward
struct GRUBackwardResult {
    std::vector<double> dx;      // gradient wrt input (last step)
    std::vector<double> dh_prev; // gradient wrt h_prev (for chaining)
};

// ────────────────────────────────────────────────────────────────
// TemporalLayer (GRU with Truncated BPTT)
//
// Architecture: 3-gate GRU
//   r  = sigmoid(Wr * [x, h] + br)          reset gate
//   z  = sigmoid(Wz * [x, h] + bz)          update gate  (init bz=1)
//   u  = capi(Wu * [x, r*h] + bu)            candidate
//   h' = z * h + (1-z) * u                  GRU output
//
// Training:
//   forward() accumulates per-step cache (max BPTT_WINDOW steps)
//   backward_through_time() does full reverse over cache
//   backward() is legacy single-step for compatibility
//
// Update policy:
//   Weights: Adam + decoupled weight decay
//   Biases:  Adam only (no decay)
//
// Initialization:
//   Gates: Xavier  sqrt(1/(in+hid))
//   Candidate: He  sqrt(2/(in+hid))
//   bz initialized to 1.0 (helps preserve h_prev early in training)
// ────────────────────────────────────────────────────────────────
static constexpr int BPTT_WINDOW = 32; // truncation window length

struct TemporalLayer {
    int in_sz;
    int hid_sz;

    // Reset gate (r)
    std::vector<std::vector<double>>    Wr;
    std::vector<double>                 br, dbr;
    std::vector<std::vector<double>>    dWr;
    std::vector<std::vector<AdamState>> Wr_adam;
    std::vector<AdamState>              br_adam;

    // Update gate (z)
    std::vector<std::vector<double>>    Wz;
    std::vector<double>                 bz, dbz;
    std::vector<std::vector<double>>    dWz;
    std::vector<std::vector<AdamState>> Wz_adam;
    std::vector<AdamState>              bz_adam;

    // Candidate (u)
    std::vector<std::vector<double>>    Wu;
    std::vector<double>                 bu, dbu;
    std::vector<std::vector<double>>    dWu;
    std::vector<std::vector<AdamState>> Wu_adam;
    std::vector<AdamState>              bu_adam;

    // Current hidden state
    std::vector<double> h;

    // BPTT cache
    std::vector<GRUCache> step_cache;

    // Legacy single-step cache
    std::vector<double> last_x, last_h;
    std::vector<double> last_r, last_z, last_u, last_su;

    explicit TemporalLayer(int in, int hid);

    // Forward one step, pushes to step_cache
    std::vector<double> forward(const std::vector<double>& x);

    // Truncated BPTT backward (recommended)
    // dh_loss: loss gradient wrt h at last step
    // T: how many steps to unroll (default: BPTT_WINDOW)
    GRUBackwardResult backward_through_time(
        const std::vector<double>& dh_loss,
        int T = BPTT_WINDOW);

    // Legacy single-step backward (for compatibility)
    // Does NOT propagate dh_prev
    std::vector<double> backward(const std::vector<double>& dh);

    // AdamW update: weights decay, biases do NOT
    void update(double lr, int t, double wd = 1e-5);

    // Zero accumulated gradients
    void zero_grad();

    // Reset hidden state and cache
    void reset();

    // Flush BPTT cache only (keep hidden state)
    void flush_cache();

    // Total parameter count
    int param_count() const;

    // Weights + biases save/load
    void save_weights(std::ofstream& f) const;
    void load_weights(std::ifstream& f);

    // Optimizer state save/load
    void save_optimizer(std::ofstream& f) const;
    void load_optimizer(std::ifstream& f);

private:
    // Single-step backward used inside BPTT loop
    GRUBackwardResult _backward_step(
        const GRUCache&            cache,
        const std::vector<double>& dh,
        bool                       accumulate_grad = true);
};

#endif // CAPIGRAD_NN_HPP
