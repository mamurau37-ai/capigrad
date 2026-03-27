// ════════════════════════════════════════════════════════════════
//  nn.cpp — CapiGrad Neural Engine v2
//  nn.hpp v2 ga mos to'liq implementatsiya
// ════════════════════════════════════════════════════════════════

#include "nn.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <stdexcept>

// ────────────────────────────────────────────────────────────────
// Activation functions
// ────────────────────────────────────────────────────────────────

double sigmoid_f(double x) {
    x = std::max(-50.0, std::min(50.0, x));
    return 1.0 / (1.0 + std::exp(-x));
}

double dsigmoid_f(double x) {
    double s = sigmoid_f(x);
    return s * (1.0 - s);
}

double tanh_f(double x) {
    return std::tanh(x);
}

double softplus(double x) {
    if (x >  20.0) return x;
    if (x < -20.0) return std::exp(x);
    return std::log1p(std::exp(x));
}

double mish_f(double x) {
    return x * std::tanh(softplus(x));
}

double dmish_f(double x) {
    double sp = softplus(x);
    double t  = std::tanh(sp);
    double s  = sigmoid_f(x);
    return t + x * (1.0 - t * t) * s;
}

double swish_f(double x) {
    return x * sigmoid_f(x);
}

double dswish_f(double x) {
    double s = sigmoid_f(x);
    return s * (1.0 + x * (1.0 - s));
}

double capi_f(double x) {
    return 0.5 * mish_f(x) + 0.5 * swish_f(x);
}

double dcapi_f(double x) {
    return 0.5 * dmish_f(x) + 0.5 * dswish_f(x);
}

double relu_f(double x) {
    return x > 0.0 ? x : 0.0;
}

double clip(double x, double c) {
    return std::max(-c, std::min(c, x));
}

double finite_or_zero(double x) {
    return std::isfinite(x) ? x : 0.0;
}

// ────────────────────────────────────────────────────────────────
// Activation helpers for Neuron
// ────────────────────────────────────────────────────────────────

namespace {

double apply_activation(ActType act, double sum) {
    switch (act) {
        case ActType::CAPI:    return capi_f(sum);
        case ActType::TANH:    return tanh_f(sum);
        case ActType::RELU:    return relu_f(sum);
        case ActType::SIGMOID: return sigmoid_f(sum);
        case ActType::LINEAR:  return sum;
        default:               return sum;
    }
}

// Uses cached out where possible to avoid recomputation
double activation_derivative(ActType act, double sum, double out) {
    switch (act) {
        case ActType::CAPI:    return dcapi_f(sum);          // complex: needs sum
        case ActType::TANH:    return 1.0 - out * out;       // from out
        case ActType::RELU:    return sum > 0.0 ? 1.0 : 0.0; // from sum
        case ActType::SIGMOID: return out * (1.0 - out);     // from out
        case ActType::LINEAR:  return 1.0;
        default:               return 1.0;
    }
}

} // anonymous namespace

// ────────────────────────────────────────────────────────────────
// AdamState
// ────────────────────────────────────────────────────────────────

double AdamState::step(double grad,
                       double lr,
                       int    t,
                       double b1,
                       double b2,
                       double eps) {
    // Gradient clipping at optimizer level
    grad = clip(grad, 3.0);

    // Nan/inf guard
    if (!std::isfinite(grad)) return 0.0;

    m = b1 * m + (1.0 - b1) * grad;
    v = b2 * v + (1.0 - b2) * grad * grad;

    double b1t = std::pow(b1, static_cast<double>(t));
    double b2t = std::pow(b2, static_cast<double>(t));

    double mh = m / (1.0 - b1t);
    double vh = v / (1.0 - b2t);

    double delta = lr * mh / (std::sqrt(vh) + eps);
    return finite_or_zero(delta);
}

void AdamState::reset() {
    m = 0.0;
    v = 0.0;
}

void AdamState::save(std::ofstream& f) const {
    f.write(reinterpret_cast<const char*>(&m), sizeof(m));
    f.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

void AdamState::load(std::ifstream& f) {
    f.read(reinterpret_cast<char*>(&m), sizeof(m));
    f.read(reinterpret_cast<char*>(&v), sizeof(v));
}

// ────────────────────────────────────────────────────────────────
// Gradient utilities
// ────────────────────────────────────────────────────────────────

double global_norm(const std::vector<double>& grads) {
    double s = 0.0;
    for (double g : grads) {
        if (std::isfinite(g)) s += g * g;
    }
    return std::sqrt(s);
}

void clip_by_norm(std::vector<double>& grads, double max_norm) {
    double n = global_norm(grads);
    if (n > max_norm && n > 0.0) {
        double scale = max_norm / n;
        for (double& g : grads) g *= scale;
    }
}

bool has_nan_or_inf(const std::vector<double>& v) {
    for (double x : v) {
        if (!std::isfinite(x)) return true;
    }
    return false;
}

// ────────────────────────────────────────────────────────────────
// GradMonitor
// ────────────────────────────────────────────────────────────────

void GradMonitor::reset() {
    grad_norm   = 0.0;
    update_norm = 0.0;
    param_norm  = 0.0;
    n           = 0;
}

void GradMonitor::record(double grad, double update, double param) {
    if (std::isfinite(grad))   grad_norm   += grad   * grad;
    if (std::isfinite(update)) update_norm += update * update;
    if (std::isfinite(param))  param_norm  += param  * param;
    ++n;
}

double GradMonitor::update_ratio() const {
    if (param_norm < 1e-12) return 0.0;
    return std::sqrt(update_norm) / std::sqrt(param_norm);
}

void GradMonitor::print(const char* tag) const {
    if (n <= 0) return;
    std::printf(
        "[GradMon][%s] grad=%.5f update=%.5f param=%.5f ratio=%.5f (n=%d)\n",
        tag,
        std::sqrt(grad_norm),
        std::sqrt(update_norm),
        std::sqrt(param_norm),
        update_ratio(),
        n);
}

// ────────────────────────────────────────────────────────────────
// Neuron
// ────────────────────────────────────────────────────────────────

Neuron::Neuron(int input_dim, ActType a)
    : act(a),
      w(input_dim),
      dw(input_dim, 0.0),
      b(0.0), db(0.0),
      sum(0.0), out(0.0),
      aw(input_dim) {

    if (input_dim <= 0) {
        throw std::runtime_error("Neuron: input_dim must be > 0");
    }

    std::mt19937 rng(std::random_device{}());

    double std_val;
    switch (act) {
        case ActType::RELU:
        case ActType::CAPI:
            // He initialization: good for ReLU-like activations
            std_val = std::sqrt(2.0 / static_cast<double>(input_dim));
            break;
        case ActType::TANH:
        case ActType::SIGMOID:
            // Xavier initialization: good for symmetric activations
            std_val = std::sqrt(1.0 / static_cast<double>(input_dim));
            break;
        case ActType::LINEAR:
            // Conservative for output layers
            std_val = std::sqrt(0.1 / static_cast<double>(input_dim));
            break;
        default:
            std_val = std::sqrt(1.0 / static_cast<double>(input_dim));
    }

    std::normal_distribution<double> dist(0.0, std_val);
    for (double& wi : w) wi = dist(rng);
}

double Neuron::forward(const std::vector<double>& x) {
    if (static_cast<int>(x.size()) != static_cast<int>(w.size())) {
        throw std::runtime_error("Neuron::forward: input dim mismatch");
    }

    sum = b;
    for (int i = 0; i < static_cast<int>(w.size()); ++i) {
        sum += w[i] * x[i];
    }

    out = apply_activation(act, sum);
    return out;
}

std::vector<double> Neuron::backward(const std::vector<double>& x,
                                      double delta) {
    if (static_cast<int>(x.size()) != static_cast<int>(w.size())) {
        throw std::runtime_error("Neuron::backward: input dim mismatch");
    }

    // Activation derivative using cached sum/out
    double da = activation_derivative(act, sum, out);

    // Local gradient
    double ds = finite_or_zero(clip(delta * da));

    // Bias gradient accumulation
    db += ds;

    // Weight and input gradients
    std::vector<double> dx(x.size(), 0.0);
    for (int i = 0; i < static_cast<int>(w.size()); ++i) {
        dw[i] += ds * x[i];
        dx[i]  = ds * w[i];
    }

    return dx;
}

void Neuron::update(double lr, int t, double wd) {
    for (int i = 0; i < static_cast<int>(w.size()); ++i) {
        double delta = aw[i].step(dw[i], lr, t);
        w[i] -= delta;
        // Decoupled weight decay (AdamW style)
        w[i] -= lr * wd * w[i];
        dw[i] = 0.0;
    }

    // Bias: Adam only, NO weight decay (standard practice)
    b  -= ab.step(db, lr, t);
    db  = 0.0;
}

void Neuron::zero_grad() {
    std::fill(dw.begin(), dw.end(), 0.0);
    db = 0.0;
}

void Neuron::save_weights(std::ofstream& f) const {
    f.write(reinterpret_cast<const char*>(&b), sizeof(b));
    for (const double wi : w) {
        f.write(reinterpret_cast<const char*>(&wi), sizeof(wi));
    }
}

void Neuron::load_weights(std::ifstream& f) {
    f.read(reinterpret_cast<char*>(&b), sizeof(b));
    for (double& wi : w) {
        f.read(reinterpret_cast<char*>(&wi), sizeof(wi));
    }
}

void Neuron::save_optimizer(std::ofstream& f) const {
    ab.save(f);
    for (const auto& ai : aw) ai.save(f);
}

void Neuron::load_optimizer(std::ifstream& f) {
    ab.load(f);
    for (auto& ai : aw) ai.load(f);
}

// ────────────────────────────────────────────────────────────────
// Layer
// ────────────────────────────────────────────────────────────────

Layer::Layer(int input_dim, int output_dim, ActType a) {
    if (input_dim <= 0 || output_dim <= 0) {
        throw std::runtime_error("Layer: dims must be > 0");
    }

    neurons.reserve(output_dim);
    for (int i = 0; i < output_dim; ++i) {
        neurons.emplace_back(input_dim, a);
    }
}

std::vector<double> Layer::forward(const std::vector<double>& x) {
    last_in = x;

    std::vector<double> out;
    out.reserve(neurons.size());

    for (auto& n : neurons) {
        out.push_back(n.forward(x));
    }

    return out;
}

std::vector<double> Layer::backward(const std::vector<double>& delta) {
    if (static_cast<int>(delta.size()) != static_cast<int>(neurons.size())) {
        throw std::runtime_error("Layer::backward: delta dim mismatch");
    }

    std::vector<double> dx(last_in.size(), 0.0);

    for (int i = 0; i < static_cast<int>(neurons.size()); ++i) {
        std::vector<double> di = neurons[i].backward(last_in, delta[i]);
        for (int j = 0; j < static_cast<int>(di.size()); ++j) {
            dx[j] += di[j];
        }
    }

    return dx;
}

void Layer::update(double lr, int t, double wd) {
    for (auto& n : neurons) {
        n.update(lr, t, wd);
    }
}

void Layer::zero_grad() {
    for (auto& n : neurons) n.zero_grad();
}

int Layer::input_dim() const {
    if (neurons.empty()) return 0;
    return static_cast<int>(neurons.front().w.size());
}

int Layer::output_dim() const {
    return static_cast<int>(neurons.size());
}

int Layer::param_count() const {
    if (neurons.empty()) return 0;
    return static_cast<int>(neurons.size()) *
           (static_cast<int>(neurons.front().w.size()) + 1);
}

void Layer::save_weights(std::ofstream& f) const {
    for (const auto& n : neurons) n.save_weights(f);
}

void Layer::load_weights(std::ifstream& f) {
    for (auto& n : neurons) n.load_weights(f);
}

void Layer::save_optimizer(std::ofstream& f) const {
    for (const auto& n : neurons) n.save_optimizer(f);
}

void Layer::load_optimizer(std::ifstream& f) {
    for (auto& n : neurons) n.load_optimizer(f);
}

// ────────────────────────────────────────────────────────────────
// TemporalLayer (GRU)
// ────────────────────────────────────────────────────────────────

TemporalLayer::TemporalLayer(int in, int hid)
    : in_sz(in), hid_sz(hid),

      Wr(hid, std::vector<double>(in + hid, 0.0)),
      br(hid, 0.0), dbr(hid, 0.0),
      dWr(hid, std::vector<double>(in + hid, 0.0)),
      Wr_adam(hid, std::vector<AdamState>(in + hid)),
      br_adam(hid),

      Wz(hid, std::vector<double>(in + hid, 0.0)),
      bz(hid, 1.0), dbz(hid, 0.0),
      dWz(hid, std::vector<double>(in + hid, 0.0)),
      Wz_adam(hid, std::vector<AdamState>(in + hid)),
      bz_adam(hid),

      Wu(hid, std::vector<double>(in + hid, 0.0)),
      bu(hid, 0.0), dbu(hid, 0.0),
      dWu(hid, std::vector<double>(in + hid, 0.0)),
      Wu_adam(hid, std::vector<AdamState>(in + hid)),
      bu_adam(hid),

      h(hid, 0.0),
      last_x(in, 0.0),  last_h(hid, 0.0),
      last_r(hid, 0.0), last_z(hid, 0.0),
      last_u(hid, 0.0), last_su(hid, 0.0) {

    if (in <= 0 || hid <= 0) {
        throw std::runtime_error("TemporalLayer: dims must be > 0");
    }

    std::mt19937 rng(std::random_device{}());

    // Xavier for gates
    double s_gate = std::sqrt(1.0 / static_cast<double>(in + hid));
    std::normal_distribution<double> d_gate(0.0, s_gate);
    for (auto& row : Wr) for (double& v : row) v = d_gate(rng);
    for (auto& row : Wz) for (double& v : row) v = d_gate(rng);

    // He for candidate (capi activation)
    double s_cand = std::sqrt(2.0 / static_cast<double>(in + hid));
    std::normal_distribution<double> d_cand(0.0, s_cand);
    for (auto& row : Wu) for (double& v : row) v = d_cand(rng);

    // bz=1.0 already set: encourages passing h_prev through early in training
}

// ── Forward ──────────────────────────────────────────────────────

std::vector<double> TemporalLayer::forward(const std::vector<double>& x) {
    if (static_cast<int>(x.size()) != in_sz) {
        throw std::runtime_error("TemporalLayer::forward: input dim mismatch");
    }

    GRUCache cache;
    cache.x      = x;
    cache.h_prev = h;
    cache.r .resize(hid_sz);
    cache.z .resize(hid_sz);
    cache.u .resize(hid_sz);
    cache.su.resize(hid_sz);

    std::vector<double> xh(in_sz + hid_sz);
    for (int i = 0; i < in_sz;  ++i) xh[i]         = x[i];
    for (int i = 0; i < hid_sz; ++i) xh[in_sz + i] = h[i];

    for (int i = 0; i < hid_sz; ++i) {
        // Reset gate
        double sr = br[i];
        for (int j = 0; j < in_sz + hid_sz; ++j) sr += Wr[i][j] * xh[j];
        cache.r[i] = sigmoid_f(sr);

        // Update gate
        double sz = bz[i];
        for (int j = 0; j < in_sz + hid_sz; ++j) sz += Wz[i][j] * xh[j];
        cache.z[i] = sigmoid_f(sz);

        // Candidate
        double su = bu[i];
        for (int j = 0; j < in_sz;  ++j) su += Wu[i][j]         * x[j];
        for (int j = 0; j < hid_sz; ++j) su += Wu[i][in_sz + j] * (cache.r[i] * h[j]);

        cache.su[i] = su;
        cache.u[i]  = capi_f(su);

        // GRU update
        h[i] = cache.z[i] * h[i] + (1.0 - cache.z[i]) * cache.u[i];
    }

    cache.h = h;

    // Legacy single-step cache
    last_x  = x;
    last_h  = cache.h_prev;
    last_r  = cache.r;
    last_z  = cache.z;
    last_u  = cache.u;
    last_su = cache.su;

    // BPTT cache management
    step_cache.push_back(std::move(cache));
    if (static_cast<int>(step_cache.size()) > BPTT_WINDOW) {
        step_cache.erase(step_cache.begin());
    }

    return h;
}

// ── Private: single-step backward ────────────────────────────────

GRUBackwardResult TemporalLayer::_backward_step(
    const GRUCache&            cache,
    const std::vector<double>& dh,
    bool                       accumulate_grad) {

    GRUBackwardResult res;
    res.dx.assign(in_sz, 0.0);
    res.dh_prev.assign(hid_sz, 0.0);

    std::vector<double> xh(in_sz + hid_sz);
    for (int i = 0; i < in_sz;  ++i) xh[i]         = cache.x[i];
    for (int i = 0; i < hid_sz; ++i) xh[in_sz + i] = cache.h_prev[i];

    for (int i = 0; i < hid_sz; ++i) {
        double dhi = dh[i];

        // ── h = z*h_prev + (1-z)*u ──────────────────────────────
        // Gradient through h_prev via update gate
        res.dh_prev[i] += dhi * cache.z[i];

        // ── Update gate (z) ─────────────────────────────────────
        double dz_val = finite_or_zero(clip(dhi * (cache.h_prev[i] - cache.u[i])));
        double dz_sig = dz_val * cache.z[i] * (1.0 - cache.z[i]);

        if (accumulate_grad) dbz[i] += dz_sig;
        for (int j = 0; j < in_sz + hid_sz; ++j) {
            if (accumulate_grad) dWz[i][j] += dz_sig * xh[j];
            if (j < in_sz) res.dx[j]            += dz_sig * Wz[i][j];
            else           res.dh_prev[j - in_sz] += dz_sig * Wz[i][j];
        }

        // ── Candidate (u) ───────────────────────────────────────
        double du_val = finite_or_zero(clip(dhi * (1.0 - cache.z[i])));
        double dsu    = du_val * dcapi_f(cache.su[i]);

        if (accumulate_grad) dbu[i] += dsu;
        for (int j = 0; j < in_sz; ++j) {
            if (accumulate_grad) dWu[i][j] += dsu * cache.x[j];
            res.dx[j] += dsu * Wu[i][j];
        }
        for (int j = 0; j < hid_sz; ++j) {
            if (accumulate_grad) dWu[i][in_sz + j] += dsu * cache.r[i] * cache.h_prev[j];
            // dh_prev through candidate (r * h_prev)
            res.dh_prev[j] += dsu * Wu[i][in_sz + j] * cache.r[i];
        }

        // ── Reset gate (r) ──────────────────────────────────────
        double dr_sum = 0.0;
        for (int j = 0; j < hid_sz; ++j) {
            dr_sum += dsu * Wu[i][in_sz + j] * cache.h_prev[j];
        }
        double dr_sig = finite_or_zero(clip(dr_sum)) *
                        cache.r[i] * (1.0 - cache.r[i]);

        if (accumulate_grad) dbr[i] += dr_sig;
        for (int j = 0; j < in_sz + hid_sz; ++j) {
            if (accumulate_grad) dWr[i][j] += dr_sig * xh[j];
            if (j < in_sz) res.dx[j]            += dr_sig * Wr[i][j];
            else           res.dh_prev[j - in_sz] += dr_sig * Wr[i][j];
        }
    }

    return res;
}

// ── Truncated BPTT backward (primary) ────────────────────────────

GRUBackwardResult TemporalLayer::backward_through_time(
    const std::vector<double>& dh_loss,
    int T) {

    if (step_cache.empty()) {
        GRUBackwardResult empty;
        empty.dx.assign(in_sz, 0.0);
        empty.dh_prev.assign(hid_sz, 0.0);
        return empty;
    }

    const int n = static_cast<int>(step_cache.size());
    const int t_start = std::max(0, n - T);

    std::vector<double> dh = dh_loss;
    GRUBackwardResult last_res;
    last_res.dx.assign(in_sz, 0.0);
    last_res.dh_prev.assign(hid_sz, 0.0);

    for (int t = n - 1; t >= t_start; --t) {
        GRUBackwardResult res = _backward_step(step_cache[t], dh, true);

        if (t == n - 1) {
            last_res.dx = res.dx;
        }

        // Propagate dh_prev to previous step
        dh = res.dh_prev;
    }

    last_res.dh_prev = dh;
    return last_res;
}

// ── Legacy single-step backward ──────────────────────────────────

std::vector<double> TemporalLayer::backward(const std::vector<double>& dh_out) {
    if (static_cast<int>(dh_out.size()) != hid_sz) {
        throw std::runtime_error("TemporalLayer::backward: dh dim mismatch");
    }

    GRUCache tmp;
    tmp.x      = last_x;
    tmp.h_prev = last_h;
    tmp.r      = last_r;
    tmp.z      = last_z;
    tmp.u      = last_u;
    tmp.su     = last_su;
    tmp.h      = h;

    GRUBackwardResult res = _backward_step(tmp, dh_out, true);
    return res.dx;
}

// ── Update ───────────────────────────────────────────────────────

void TemporalLayer::update(double lr, int t, double wd) {
    for (int i = 0; i < hid_sz; ++i) {
        // Update gate z — bias no decay
        bz[i] -= bz_adam[i].step(dbz[i], lr, t);
        dbz[i] = 0.0;
        for (int j = 0; j < in_sz + hid_sz; ++j) {
            double delta = Wz_adam[i][j].step(dWz[i][j], lr, t);
            Wz[i][j] -= delta;
            Wz[i][j] -= lr * wd * Wz[i][j]; // decoupled decay
            dWz[i][j] = 0.0;
        }

        // Reset gate r — bias no decay
        br[i] -= br_adam[i].step(dbr[i], lr, t);
        dbr[i] = 0.0;
        for (int j = 0; j < in_sz + hid_sz; ++j) {
            double delta = Wr_adam[i][j].step(dWr[i][j], lr, t);
            Wr[i][j] -= delta;
            Wr[i][j] -= lr * wd * Wr[i][j]; // decoupled decay
            dWr[i][j] = 0.0;
        }

        // Candidate u — bias no decay
        bu[i] -= bu_adam[i].step(dbu[i], lr, t);
        dbu[i] = 0.0;
        for (int j = 0; j < in_sz + hid_sz; ++j) {
            double delta = Wu_adam[i][j].step(dWu[i][j], lr, t);
            Wu[i][j] -= delta;
            Wu[i][j] -= lr * wd * Wu[i][j]; // decoupled decay
            dWu[i][j] = 0.0;
        }
    }
}

void TemporalLayer::zero_grad() {
    for (int i = 0; i < hid_sz; ++i) {
        dbz[i] = 0.0; dbr[i] = 0.0; dbu[i] = 0.0;
        for (int j = 0; j < in_sz + hid_sz; ++j) {
            dWz[i][j] = 0.0;
            dWr[i][j] = 0.0;
            dWu[i][j] = 0.0;
        }
    }
}

void TemporalLayer::reset() {
    std::fill(h.begin(), h.end(), 0.0);
    step_cache.clear();
}

void TemporalLayer::flush_cache() {
    step_cache.clear();
}

int TemporalLayer::param_count() const {
    return 3 * hid_sz * (in_sz + hid_sz) + 3 * hid_sz;
}

// ── Save / Load ──────────────────────────────────────────────────

void TemporalLayer::save_weights(std::ofstream& f) const {
    auto wm = [&](const std::vector<std::vector<double>>& M) {
        for (const auto& row : M)
            for (double v : row)
                f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    };
    auto wv = [&](const std::vector<double>& v) {
        for (double x : v)
            f.write(reinterpret_cast<const char*>(&x), sizeof(x));
    };

    wm(Wu); wm(Wr); wm(Wz);
    wv(bu); wv(br); wv(bz);
}

void TemporalLayer::load_weights(std::ifstream& f) {
    auto rm = [&](std::vector<std::vector<double>>& M) {
        for (auto& row : M)
            for (double& v : row)
                f.read(reinterpret_cast<char*>(&v), sizeof(v));
    };
    auto rv = [&](std::vector<double>& v) {
        for (double& x : v)
            f.read(reinterpret_cast<char*>(&x), sizeof(x));
    };

    rm(Wu); rm(Wr); rm(Wz);
    rv(bu); rv(br); rv(bz);
}

void TemporalLayer::save_optimizer(std::ofstream& f) const {
    auto sam = [&](const std::vector<std::vector<AdamState>>& AM) {
        for (const auto& row : AM)
            for (const auto& a : row)
                a.save(f);
    };
    auto sav = [&](const std::vector<AdamState>& AV) {
        for (const auto& a : AV) a.save(f);
    };

    sam(Wu_adam); sav(bu_adam);
    sam(Wr_adam); sav(br_adam);
    sam(Wz_adam); sav(bz_adam);
}

void TemporalLayer::load_optimizer(std::ifstream& f) {
    auto lam = [&](std::vector<std::vector<AdamState>>& AM) {
        for (auto& row : AM)
            for (auto& a : row)
                a.load(f);
    };
    auto lav = [&](std::vector<AdamState>& AV) {
        for (auto& a : AV) a.load(f);
    };

    lam(Wu_adam); lav(bu_adam);
    lam(Wr_adam); lav(br_adam);
    lam(Wz_adam); lav(bz_adam);
}
