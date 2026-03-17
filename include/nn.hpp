#ifndef NN_HPP
#define NN_HPP

#include <vector>
#include <fstream>

// ── Aktivatsiyalar ───────────────────────────────────────────
double sigmoid_f(double x);
double dsigmoid_f(double x);
double tanh_f(double x);
double mish_f(double x);
double dmish_f(double x);
double swish_f(double x);
double dswish_f(double x);
double capi_f(double x);
double dcapi_f(double x);
double relu_f(double x);
double clip(double x, double c=5.0);

// ── Adam Optimizer ───────────────────────────────────────────
struct AdamState {
    double m=0, v=0;
    double step(double g, double lr, int t, double b1=0.9, double b2=0.999,
                double eps=1e-8, double wd=1e-5);
};

double global_norm(const std::vector<double>& grads);
void clip_by_norm(std::vector<double>& grads, double max_norm=1.0);

// ── Neuron va Qatlamlar ────────────────────────────────────────
enum ActType { CAPI_A, TANH_A, RELU_A, SIGMOID_A, LINEAR_A };

struct Neuron {
    ActType act;
    std::vector<double> w, dw;
    double b=0, db=0, sum=0, out=0;
    std::vector<AdamState> aw; 
    AdamState ab;

    Neuron(int n, ActType a=CAPI_A);
    double forward(const std::vector<double>& x);
    std::vector<double> backward(const std::vector<double>& x, double delta);
    void update(double lr, int t, double wd=1e-5);
};

struct Layer {
    std::vector<Neuron> ns; 
    std::vector<double> last_in;

    Layer(int in, int out, ActType a=CAPI_A);
    std::vector<double> forward(const std::vector<double>& x);
    std::vector<double> backward(const std::vector<double>& d);
    void update(double lr, int t);
};

// ── Temporal Layer (GRU) ───────────────────────────────────────
struct TemporalLayer {
    int in_sz, hid_sz;
    
    std::vector<std::vector<double>> Wr; std::vector<double> br, dbr;
    std::vector<std::vector<double>> dWr; std::vector<std::vector<AdamState>> Wr_adam; std::vector<AdamState> br_adam;
    
    std::vector<std::vector<double>> Wz; std::vector<double> bz, dbz;
    std::vector<std::vector<double>> dWz; std::vector<std::vector<AdamState>> Wz_adam; std::vector<AdamState> bz_adam;
    
    std::vector<std::vector<double>> Wu; std::vector<double> bu, dbu;
    std::vector<std::vector<double>> dWu; std::vector<std::vector<AdamState>> Wu_adam; std::vector<AdamState> bu_adam;
    
    std::vector<double> h;
    std::vector<double> last_x, last_h, last_r, last_z, last_u, last_su;

    TemporalLayer(int in, int hid);
    std::vector<double> forward(const std::vector<double>& x);
    std::vector<double> backward(const std::vector<double>& dh);
    void update(double lr, int t, double wd=1e-5);
    
    void reset();
    int pcount();
    void save(std::ofstream& f);
    void load(std::ifstream& f);
};

#endif // NN_HPP