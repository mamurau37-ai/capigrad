/*
================================================================
  CapiGrad Market Intelligence Engine (CGT-MIE) v4.0
  Zanerhon's BTC Market Analyzer

  MAQSAD: Savdo bot EMAS — Professional bozor analizatori
  - Regime detection (Uptrend/Downtrend/Ranging/Volatile)
  - Trend kuchi va tugash ehtimoli
  - Momentum (qisqa + o'rta muddatli)
  - Reversal riski
  - Breakout sifati (real vs fake)
  - Keyingi 1-2 sham bashorati (regression)
  - Har output uchun confidence

  ARXITEKTURA:
    Input(57) → Dual Branch (H1 + M15)
    → Merge(96) → Dense(64)
    → 4 ta Multi-Head output:
        Regime Head  (4): uptrend/downtrend/ranging/volatile
        Trend Head   (3): strength/exhaustion/breakout
        Momentum Head(4): short/medium/reversal/pressure
        Forecast Head(2): next1_return/next2_return

  BAHOLASH: PnL EMAS — tahlil sifati:
    Regime accuracy, Directional accuracy,
    Reversal detection, Confidence calibration

  ISHLATISH:
    g++ -std=c++17 -O3 -Wno-nonnull -o mie cgt_mie.cpp
    ./mie --train      O'qitish
    ./mie --analyze    Oxirgi shamni tahlil qilish
    ./mie --eval       Tahlil sifatini baholash
================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <deque>
#include <random>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <atomic>
#include <csignal>
// LIVE MODE: threading + sync
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cstring>    // memset
// Network (POSIX sockets — websocket adapter uchun)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// ── Triple-Barrier class labels ───────────────────────────
// K va HORIZON endi statik emas — dynamic regime-aware (BarrierParams)
// Faqat class integer konstantlari qoladi (forward/backward da ishlatiladi)
static constexpr int TB_UP   = 0;
static constexpr int TB_DOWN = 1;
static constexpr int TB_FLAT = 2;
// Dynamic labeling uchun MAX horizon — bound check va alloc uchun
static constexpr int TB_MAX_HORIZON = 6; // hech qaysi regime 6 dan oshmaydi

// Decision threshold for eval/analyze — NOT for training
// Maqsad: argmax(0.34/0.33/0.33) → "UP" deyishi NOTO'G'RI
// Threshold: agar pred_class prob > DECISION_THRESHOLD → actionable
// Aks holda → FLAT (no-trade)
// 0.40: UP yoki DOWN uchun kamida 40% ishonch kerak
static constexpr double DECISION_THRESHOLD = 0.35; // PATCH4: 0.40→0.35, minimal edge above 1/3
// PATCH3: TREND-actionable precision bonus weight for early stopping
static constexpr double ALPHA_TREND = 0.20;
// PATCH1: Focal Loss gamma — FLAT-collapse oldini olish
// gamma=0: CE (vanilla). gamma=2: hard examples amplified, easy discounted.
// Sabab: FLAT=50% → model FLAT collapse qilib past CE olishi mumkin
// Focal gamma=2 → FLAT sample (p=0.8) ni 25x discount, UP/DOWN (p=0.5) ni 2x
static constexpr double FOCAL_GAMMA = 2.0;

// PATCH-A: Regime-weighted classification loss
// TREND = 1.0 (full weight — bu signalimiz)
// RANGE = 0.35 (pasaytirilgan — RANGE da edge kam)
// VOLATILE = 0.20 (minimal — shovqin dominant)
// final_weight = class_weight * regime_weight
static constexpr double W_REGIME_TREND    = 1.00;
static constexpr double W_REGIME_RANGE    = 0.35;
static constexpr double W_REGIME_VOLATILE = 0.20;
// Current sample regime weight — train loop da yangilanadi, backward da o'qiladi
double g_cur_regime_w = 1.0;

std::atomic<bool> g_running{true};
void signal_handler(int){ g_running=false; std::cout<<"\n[STOP]\n"; }

// ── Aktivatsiyalar ───────────────────────────────────────────
double sigmoid_f(double x){ return 1.0/(1+std::exp(-std::max(-50.0,std::min(50.0,x)))); }
double dsigmoid_f(double x){ double s=sigmoid_f(x); return s*(1-s); }
double tanh_f(double x){ return std::tanh(x); }
double mish_f(double x){ double sp=std::log(1+std::exp(std::min(x,50.0))); return x*std::tanh(sp); }
double dmish_f(double x){ double sp=std::log(1+std::exp(std::min(x,50.0))),t=std::tanh(sp),s=sigmoid_f(x); return t+x*(1-t*t)*s; }
double swish_f(double x){ return x*sigmoid_f(x); }
double dswish_f(double x){ double s=sigmoid_f(x); return s*(1+x*(1-s)); }
double capi_f(double x){ return 0.5*mish_f(x)+0.5*swish_f(x); }
double dcapi_f(double x){ return 0.5*dmish_f(x)+0.5*dswish_f(x); }
double relu_f(double x){ return x>0?x:0.0; }
inline double clip(double x,double c=5.0){ return std::max(-c,std::min(c,x)); }

// ── Adam Optimizer ───────────────────────────────────────────
struct AdamState {
    double m=0,v=0;
    // AdamW: weight_decay qo'shildi (L2 regularization)
    // Global norm clipping: tashqaridan normalized gradient keladi
    double step(double g,double lr,int t,double b1=0.9,double b2=0.999,
                double eps=1e-8,double wd=1e-5){
        // Per-parameter clip (kichik)
        g=clip(g,3.0);
        m=b1*m+(1-b1)*g; v=b2*v+(1-b2)*g*g;
        // pow ni incremental hisoblash (tezroq)
        double b1t=std::pow(b1,t), b2t=std::pow(b2,t);
        double mh=m/(1-b1t);
        double vh=v/(1-b2t);
        double update=lr*mh/(std::sqrt(vh)+eps);
        return update; // Weight decay Neuron::update da alohida qo'llanadi (haqiqiy AdamW)
    }
};

// Global gradient norm clipping — butun qatlamni normalizatsiya qilish
inline double global_norm(const std::vector<double>& grads){
    double s=0; for(auto g:grads) s+=g*g; return std::sqrt(s);
}
inline void clip_by_norm(std::vector<double>& grads,double max_norm=1.0){
    double n=global_norm(grads);
    if(n>max_norm){ double s=max_norm/n; for(auto& g:grads) g*=s; }
}

// ── Neuron ────────────────────────────────────────────────────
enum ActType { CAPI_A, TANH_A, RELU_A, SIGMOID_A, LINEAR_A };
struct Neuron {
    ActType act;
    std::vector<double> w,dw;
    double b=0,db=0,sum=0,out=0;
    std::vector<AdamState> aw; AdamState ab;
    Neuron(int n,ActType a=CAPI_A):act(a),w(n),dw(n,0),aw(n){
        std::mt19937 rng(std::random_device{}());
        // Activation-specific initialization:
        // ReLU/CAPI/Swish → He: std=sqrt(2/n)
        // tanh/sigmoid    → Xavier: std=sqrt(1/n)
        // linear          → kichik: std=sqrt(0.1/n)
        double std_val;
        if(act==RELU_A||act==CAPI_A) std_val=std::sqrt(2.0/n);         // He
        else if(act==TANH_A||act==SIGMOID_A) std_val=std::sqrt(1.0/n); // Xavier
        else std_val=std::sqrt(0.1/n);                                  // Linear
        std::normal_distribution<double> d(0,std_val);
        for(auto& x:w) x=d(rng);
    }
    double forward(const std::vector<double>& x){
        sum=b; for(int i=0;i<(int)w.size();i++) sum+=w[i]*x[i];
        switch(act){
            case CAPI_A:out=capi_f(sum);break; case TANH_A:out=tanh_f(sum);break;
            case RELU_A:out=relu_f(sum);break; case SIGMOID_A:out=sigmoid_f(sum);break;
            default:out=sum;
        }
        return out;
    }
    std::vector<double> backward(const std::vector<double>& x,double delta){
        double da;
        switch(act){
            case CAPI_A:da=dcapi_f(sum);break; case TANH_A:da=1-tanh_f(sum)*tanh_f(sum);break;
            case RELU_A:da=sum>0?1.0:0.0;break; case SIGMOID_A:da=dsigmoid_f(sum);break;
            default:da=1.0;
        }
        double ds=clip(delta*da); db+=ds;
        std::vector<double> dx(x.size());
        for(int i=0;i<(int)w.size();i++){dw[i]+=ds*x[i]; dx[i]=ds*w[i];}
        return dx;
    }
    void update(double lr,int t,double wd=1e-5){
        for(int i=0;i<(int)w.size();i++){
            w[i]-=aw[i].step(dw[i],lr,t); // gradient step (Adam)
            w[i]-=lr*wd*w[i];              // AdamW: weight decay alohida, haqiqiy
            dw[i]=0;
        }
        b-=ab.step(db,lr,t); db=0; // bias: weight decay yo'q
    }
};

struct Layer {
    std::vector<Neuron> ns; std::vector<double> last_in;
    Layer(int in,int out,ActType a=CAPI_A){ for(int i=0;i<out;i++) ns.emplace_back(in,a); }
    std::vector<double> forward(const std::vector<double>& x){
        last_in=x; std::vector<double> o; for(auto& n:ns) o.push_back(n.forward(x)); return o;
    }
    std::vector<double> backward(const std::vector<double>& d){
        std::vector<double> dx(last_in.size(),0);
        for(int i=0;i<(int)ns.size();i++){
            auto di=ns[i].backward(last_in,d[i]);
            for(int j=0;j<(int)di.size();j++) dx[j]+=di[j];
        }
        return dx;
    }
    void update(double lr,int t){ for(auto& n:ns) n.update(lr,t); }
};

// ── Temporal Layer — Haqiqiy GRU (learnable update gate) ─────
struct TemporalLayer {
    int in_sz, hid_sz;

    // Reset gate (r): x va h dan
    std::vector<std::vector<double>> Wr;
    std::vector<double> br, dbr;
    std::vector<std::vector<double>> dWr;
    std::vector<std::vector<AdamState>> Wr_adam;
    std::vector<AdamState> br_adam;

    // Update gate (z): learnable mix — GRU ning asosi
    std::vector<std::vector<double>> Wz;
    std::vector<double> bz, dbz;
    std::vector<std::vector<double>> dWz;
    std::vector<std::vector<AdamState>> Wz_adam;
    std::vector<AdamState> bz_adam;

    // Candidate hidden (u): reset gate * h_prev
    std::vector<std::vector<double>> Wu;
    std::vector<double> bu, dbu;
    std::vector<std::vector<double>> dWu;
    std::vector<std::vector<AdamState>> Wu_adam;
    std::vector<AdamState> bu_adam;

    std::vector<double> h;
    // Backprop cache
    std::vector<double> last_x, last_h, last_r, last_z, last_u, last_su;

    TemporalLayer(int in, int hid): in_sz(in), hid_sz(hid),
        Wr(hid, std::vector<double>(in+hid, 0)),
        br(hid, 0.0), dbr(hid, 0),
        dWr(hid, std::vector<double>(in+hid, 0)),
        Wr_adam(hid, std::vector<AdamState>(in+hid)),
        br_adam(hid),
        Wz(hid, std::vector<double>(in+hid, 0)),
        bz(hid, 0.0), dbz(hid, 0),
        dWz(hid, std::vector<double>(in+hid, 0)),
        Wz_adam(hid, std::vector<AdamState>(in+hid)),
        bz_adam(hid),
        Wu(hid, std::vector<double>(in+hid, 0)),
        bu(hid, 0.0), dbu(hid, 0),
        dWu(hid, std::vector<double>(in+hid, 0)),
        Wu_adam(hid, std::vector<AdamState>(in+hid)),
        bu_adam(hid),
        h(hid, 0),
        last_x(in, 0), last_h(hid, 0),
        last_r(hid, 0), last_z(hid, 0),
        last_u(hid, 0), last_su(hid, 0)
    {
        std::mt19937 rng(std::random_device{}());
        double s = std::sqrt(1.0/(in+hid));
        std::normal_distribution<double> d(0, s);
        for(auto& row:Wr) for(auto& v:row) v=d(rng);
        for(auto& row:Wz) for(auto& v:row) v=d(rng);
        // Candidate: He init (capi activation)
        std::normal_distribution<double> dh(0, std::sqrt(2.0/(in+hid)));
        for(auto& row:Wu) for(auto& v:row) v=dh(rng);
        // Update gate bias: 1.0 → boshida ko'proq o'tkazsin (forgetting kam)
        std::fill(bz.begin(), bz.end(), 1.0);
    }

    std::vector<double> forward(const std::vector<double>& x){
        last_x = x; last_h = h;
        // [x, h] concat
        std::vector<double> xh(in_sz+hid_sz);
        for(int i=0;i<in_sz;i++)   xh[i]       = x[i];
        for(int i=0;i<hid_sz;i++)  xh[in_sz+i] = h[i];

        for(int i=0;i<hid_sz;i++){
            // Reset gate: r = sigmoid(Wr*[x,h] + br)
            double sr = br[i];
            for(int j=0;j<in_sz+hid_sz;j++) sr += Wr[i][j]*xh[j];
            last_r[i] = sigmoid_f(sr);

            // Update gate: z = sigmoid(Wz*[x,h] + bz)  ← HAQIQIY GRU
            double sz = bz[i];
            for(int j=0;j<in_sz+hid_sz;j++) sz += Wz[i][j]*xh[j];
            last_z[i] = sigmoid_f(sz);

            // Candidate: u = capi(Wu*[x, r*h] + bu)
            double su = bu[i];
            for(int j=0;j<in_sz;j++)    su += Wu[i][j]*x[j];
            for(int j=0;j<hid_sz;j++)   su += Wu[i][in_sz+j]*(last_r[i]*h[j]);
            last_su[i] = su;
            last_u[i]  = capi_f(su);

            // GRU update: h = z*h + (1-z)*u  ← model o'zi decides
            h[i] = last_z[i]*h[i] + (1.0-last_z[i])*last_u[i];
        }
        return h;
    }

    std::vector<double> backward(const std::vector<double>& dh){
        std::vector<double> dx(in_sz, 0);
        std::vector<double> xh(in_sz+hid_sz);
        for(int i=0;i<in_sz;i++)   xh[i]       = last_x[i];
        for(int i=0;i<hid_sz;i++)  xh[in_sz+i] = last_h[i];

        for(int i=0;i<hid_sz;i++){
            double dhi = dh[i];

            // dh/dz = h_prev - u  (GRU update gate gradient)
            double dz_val = clip(dhi * (last_h[i] - last_u[i]));
            double dz_sig = dz_val * last_z[i] * (1.0 - last_z[i]); // sigmoid deriv
            dbz[i] += dz_sig;
            for(int j=0;j<in_sz+hid_sz;j++){
                dWz[i][j] += dz_sig * xh[j];
                if(j < in_sz) dx[j] += dz_sig * Wz[i][j];
            }

            // dh/du = (1-z)
            double du_val = clip(dhi * (1.0 - last_z[i]));
            double dsu    = du_val * dcapi_f(last_su[i]);
            dbu[i] += dsu;
            for(int j=0;j<in_sz;j++){
                dWu[i][j] += dsu * last_x[j];
                dx[j]     += dsu * Wu[i][j];
            }
            for(int j=0;j<hid_sz;j++)
                dWu[i][in_sz+j] += dsu * last_r[i] * last_h[j];

            // dh/dr (reset gate gradient)
            double dr_sum = 0;
            for(int j=0;j<hid_sz;j++) dr_sum += dsu * Wu[i][in_sz+j] * last_h[j];
            double dr_sig = clip(dr_sum) * last_r[i] * (1.0 - last_r[i]);
            dbr[i] += dr_sig;
            for(int j=0;j<in_sz+hid_sz;j++){
                dWr[i][j] += dr_sig * xh[j];
                if(j < in_sz) dx[j] += dr_sig * Wr[i][j];
            }
        }
        return dx;
    }

    void update(double lr, int t, double wd=1e-5){
        for(int i=0;i<hid_sz;i++){
            // Update gate z
            bz[i] -= bz_adam[i].step(dbz[i],lr,t); dbz[i]=0;
            for(int j=0;j<in_sz+hid_sz;j++){
                Wz[i][j] -= Wz_adam[i][j].step(dWz[i][j],lr,t);
                Wz[i][j] -= lr*wd*Wz[i][j];  // AdamW
                dWz[i][j]=0;
            }
            // Reset gate r
            br[i] -= br_adam[i].step(dbr[i],lr,t); dbr[i]=0;
            for(int j=0;j<in_sz+hid_sz;j++){
                Wr[i][j] -= Wr_adam[i][j].step(dWr[i][j],lr,t);
                Wr[i][j] -= lr*wd*Wr[i][j];  // AdamW
                dWr[i][j]=0;
            }
            // Candidate u
            bu[i] -= bu_adam[i].step(dbu[i],lr,t); dbu[i]=0;
            for(int j=0;j<in_sz+hid_sz;j++){
                Wu[i][j] -= Wu_adam[i][j].step(dWu[i][j],lr,t);
                Wu[i][j] -= lr*wd*Wu[i][j];  // AdamW
                dWu[i][j]=0;
            }
        }
    }

    void reset(){ std::fill(h.begin(),h.end(),0); }
    int pcount(){ return hid_sz*(in_sz+hid_sz)*6; } // 3 gates × 2 (W+b)

    void save(std::ofstream& f){
        for(auto& row:Wu) for(auto& v:row) f.write((char*)&v,8);
        for(auto& row:Wr) for(auto& v:row) f.write((char*)&v,8);
        for(auto& row:Wz) for(auto& v:row) f.write((char*)&v,8);
        for(auto& v:bu) f.write((char*)&v,8);
        for(auto& v:br) f.write((char*)&v,8);
        for(auto& v:bz) f.write((char*)&v,8);
    }
    void load(std::ifstream& f){
        for(auto& row:Wu) for(auto& v:row) f.read((char*)&v,8);
        for(auto& row:Wr) for(auto& v:row) f.read((char*)&v,8);
        for(auto& row:Wz) for(auto& v:row) f.read((char*)&v,8);
        for(auto& v:bu) f.read((char*)&v,8);
        for(auto& v:br) f.read((char*)&v,8);
        for(auto& v:bz) f.read((char*)&v,8);
    }
};

// ════════════════════════════════════════════════════════════
//  MIE MODEL: Dual Branch + 4 Multi-Head
//  H1 branch  → macro trend/regime
//  M15 branch → micro momentum/pressure
//  Merge → Dense(64) → 4 heads
// ════════════════════════════════════════════════════════════
// Global class weights — accessible from MIEModel::backward
// Set once by compute_class_weights() before training loop
// Inverse-frequency: w[c] = N_total/(3*N_c), clipped [0.5, 4.0]
double g_cls_weights[3] = {1.0, 1.0, 1.0};

struct MIEModel {
    // Dual temporal branches
    TemporalLayer h1_branch;   // H1: macro
    TemporalLayer m15_branch;  // M15: micro

    // Shared dense
    Layer merge_dense;   // 128 → 80

    // 5 analysis heads
    Layer regime_head;        // 80 -> 4 (softmax)
    Layer trend_head;         // 80 -> 3 (tanh)
    Layer momentum_head;      // 80 -> 4 (tanh)
    Layer forecast_head;      // 80 -> 2 (tanh) — secondary diagnostic
    Layer forecast_cls_head;  // 80 -> 3 (softmax) — PRIMARY: UP/DOWN/FLAT

    int t=0;
    std::string name;

    // Intermediate activations (backprop uchun)
    std::vector<double> last_merged, last_dense;

    MIEModel(const std::string& n="MIE"):
        h1_branch(70,64), m15_branch(70,64), // feat: 70 (35 base + 35 directional)
        merge_dense(128,80, RELU_A),
        regime_head(80,4, LINEAR_A),
        trend_head(80,3, TANH_A),
        momentum_head(80,4, TANH_A),
        forecast_head(80,2, TANH_A),
        forecast_cls_head(80,3, LINEAR_A),
        name(n){}

    // ── Softmax ─────────────────────────────────────────────
    std::vector<double> softmax(const std::vector<double>& x){
        double mx=*std::max_element(x.begin(),x.end());
        std::vector<double> e(x.size()); double s=0;
        for(int i=0;i<(int)x.size();i++){e[i]=std::exp(x[i]-mx);s+=e[i];}
        for(auto& v:e) v/=s;
        return e;
    }

    // ── Forward pass ────────────────────────────────────────
    // Input: feat_h1(15) + feat_m15(15) = 30 features
    // Returns: MIEResult
    struct Result {
        // Regime [0..1] x 4
        double regime_up, regime_down, regime_range, regime_volatile;
        // Trend
        double trend_strength, trend_exhaustion, breakout_quality;
        // Momentum
        double momentum_short, momentum_medium, reversal_risk, market_pressure;
        // Forecast
        double next1_pct, next2_pct;
        // Confidence
        double confidence;
        // Labels
        std::string regime_label, direction_label;

        // Triple-Barrier Classification output (PRIMARY signal)
        double cls_up, cls_down, cls_flat; // softmax probabilities [0..1]
        std::string cls_label; // "UP" / "DOWN" / "FLAT"
        double cls_confidence; // max(cls_up, cls_down, cls_flat)

        // NARRATIVE HEAD: 5 ta bozor mexanizmi [0..1]
        double narr_continuation;   // trend davom etadi
        double narr_reversal;       // qaytish
        double narr_trap;           // likvidlik tuzoq / fake break
        double narr_breakout;       // haqiqiy expansion
        double narr_mean_reversion; // ortaga qaytish
        std::string dominant_narrative;

        // FEATURE GROUP SCORES [0..1]
        double ctx_score;   // kontekst: qaysi sahna?
        double str_score;   // struktura: qanday tuzilgan?
        double mom_score;   // momentum: ichki kuch?
        double conf_score;  // tasdiq: signal mustahkam?
        double inv_score;   // rad etish: signal noto'g'ri?

        // CONFIRMATION / CONTRADICTION STACK
        std::vector<std::string> confirmations;
        std::vector<std::string> contradictions;
        std::string signal_quality; // STRONG/MODERATE/WEAK/INVALID

        // PATCH3: Regime gate + primary action (analyze va eval uchun)
        bool regime_gate_active; // true = non-TREND, primary bloklanadi
        std::string primary_action; // "TREND-LONG" / "TREND-SHORT" / "WAIT-GATE" / "WAIT-CLARITY" / "NO-TRADE"
    };

    Result forward(const std::vector<double>& feat_h1,
                   const std::vector<double>& feat_m15){
        // Branch forward
        auto h1_out  = h1_branch.forward(feat_h1);
        auto m15_out = m15_branch.forward(feat_m15);

        // Merge (64+64=128)
        last_merged.resize(128);
        for(int i=0;i<64;i++) last_merged[i]=h1_out[i];
        for(int i=0;i<64;i++) last_merged[64+i]=m15_out[i];

        // Shared dense
        last_dense = merge_dense.forward(last_merged);

        // Heads
        auto regime_raw  = regime_head.forward(last_dense);
        auto regime_prob = softmax(regime_raw);
        auto trend_out   = trend_head.forward(last_dense);
        auto mom_out     = momentum_head.forward(last_dense);
        auto fc_out      = forecast_head.forward(last_dense);

        Result r;
        // Regime
        r.regime_up       = regime_prob[0];
        r.regime_down     = regime_prob[1];
        r.regime_range    = regime_prob[2];
        r.regime_volatile = regime_prob[3];

        // Trend
        r.trend_strength   = trend_out[0];                        // tanh: -1..+1
        r.trend_exhaustion = (trend_out[1]+1.0)/2.0;              // → 0..1
        r.breakout_quality = (trend_out[2]+1.0)/2.0;              // → 0..1

        // Momentum
        r.momentum_short  = mom_out[0];                           // tanh: -1..+1
        r.momentum_medium = mom_out[1];                           // tanh: -1..+1
        r.reversal_risk   = (mom_out[2]+1.0)/2.0;                 // → 0..1
        r.market_pressure = mom_out[3];                           // tanh: -1..+1

        // Forecast: scale to ±3%
        r.next1_pct = fc_out[0]*3.0;
        r.next2_pct = fc_out[1]*3.0;

        // Confidence: regime max probability × (1 - reversal_risk*0.3)
        double regime_conf = *std::max_element(regime_prob.begin(),regime_prob.end());
        // Calibrated confidence:
        // 1. Regime entropy: past entropy = aniq signal
        double regime_entropy=0;
        for(auto p:regime_prob) if(p>1e-9) regime_entropy-=p*std::log2(p);
        double max_entropy=std::log2(4.0); // 4 ta regime
        double clarity=1.0-regime_entropy/max_entropy; // [0=noaniq, 1=aniq]
        // 2. Trend va momentum uygunligi
        double alignment=std::abs(r.trend_strength)*0.5+std::abs(r.market_pressure)*0.3;
        alignment=std::min(1.0,alignment);
        // 3. Reversal riski kamaytiradi
        double rev_penalty=r.reversal_risk*0.4;
        // 4. Kombinatsiya — temperature scaling bilan (T=1.5 = kamroq overconfidence)
        double raw_conf=0.3*regime_conf+0.4*clarity+0.3*alignment;
        raw_conf=std::max(0.0,raw_conf-rev_penalty);
        // Temperature scaling: softmax ni tekislash (overconfidence ni kamaytirish)
        double T=1.8; // T>1 = kamroq confidence
        r.confidence=1.0/(1.0+std::exp(-(raw_conf*2-1)/T*3.0)); // sigmoid calibration
        r.confidence=std::max(0.10,std::min(0.90,r.confidence)); // [10%..90%] oralig'i

        // Labels
        int ri=std::distance(regime_prob.begin(),
                std::max_element(regime_prob.begin(),regime_prob.end()));
        const char* rl[]={"TREND_UP","TREND_DOWN","RANGING","VOLATILE"};
        r.regime_label = rl[ri];

        // direction_label: cls_head forward da set qilinadi (pastda)

        // Triple-barrier classification head (PRIMARY)
        auto cls_raw  = forecast_cls_head.forward(last_dense);
        auto cls_prob = softmax(cls_raw);
        r.cls_up   = cls_prob[TB_UP];
        r.cls_down = cls_prob[TB_DOWN];
        r.cls_flat = cls_prob[TB_FLAT];
        // Dominant class
        if(r.cls_up > r.cls_down && r.cls_up > r.cls_flat)
            r.cls_label = "UP";
        else if(r.cls_down > r.cls_up && r.cls_down > r.cls_flat)
            r.cls_label = "DOWN";
        else
            r.cls_label = "FLAT";
        r.cls_confidence = std::max({r.cls_up, r.cls_down, r.cls_flat});

        // direction_label: classification bilan sinxronlash
        r.direction_label = (r.cls_label=="UP") ? "BULLISH" :
                            (r.cls_label=="DOWN") ? "BEARISH" : "NEUTRAL";

        // PATCH1: Regime gate — faqat TREND_UP/TREND_DOWN = actionable
        // RANGING yoki VOLATILE da primary signal bloklanadi
        r.regime_gate_active = (r.regime_label != "TREND_UP" &&
                                r.regime_label != "TREND_DOWN");
        // primary_action: gate + clarity + direction kombinatsiyasi
        if(r.regime_gate_active){
            r.primary_action = "WAIT-GATE"; // non-TREND, bloklanadi
        } else if(r.cls_label == "FLAT"){
            r.primary_action = "NO-TRADE";
        } else if(r.cls_confidence < DECISION_THRESHOLD){
            r.primary_action = "WAIT-CLARITY";
        } else if(r.cls_label == "UP"){
            r.primary_action = "TREND-LONG";
        } else {
            r.primary_action = "TREND-SHORT";
        }

        // Narrative, group scores, confirmations:
        // build_analysis(r) da rule-based hisoblanadi

        return r;
    }

    // ── Backward pass ───────────────────────────────────────
    // targets: {regime(4), trend(3), momentum(4), forecast_reg(2), fc_cls(1-hot:3)}
    // [0-3]=regime, [4-6]=trend, [7-10]=mom, [11-12]=fc_reg, [13]=barrier_class(int)
    void backward(const std::vector<double>& /*feat_h1*/,
                  const std::vector<double>& /*feat_m15*/,
                  const std::vector<double>& targets,
                  const Result& pred){
        // targets[13] = barrier class as int: 0=UP, 1=DOWN, 2=FLAT
        std::vector<double> g_regime(4,0), g_trend(3,0), g_mom(4,0), g_fc(2,0);
        std::vector<double> g_cls(3, 0.0);

        // Regime: softmax + CE loss
        for(int i=0;i<4;i++){
            double p=(&pred.regime_up)[i];
            g_regime[i]=p-targets[i]; // softmax-CE gradient
        }

        // Trend: MSE
        double ts_pred=pred.trend_strength;
        double te_pred=pred.trend_exhaustion;
        double bq_pred=pred.breakout_quality;
        g_trend[0] = (ts_pred - targets[4])*2.0;
        g_trend[1] = ((te_pred - targets[5])*2.0)/2.0; // chain (→0..1 scale)
        g_trend[2] = ((bq_pred - targets[6])*2.0)/2.0;

        // Momentum: MSE
        g_mom[0] = (pred.momentum_short  - targets[7])*2.0;
        g_mom[1] = (pred.momentum_medium - targets[8])*2.0;
        g_mom[2] = ((pred.reversal_risk  - targets[9])*2.0)/2.0;
        g_mom[3] = (pred.market_pressure - targets[10])*2.0;

        // Forecast: MSE (scale back: /3)
        g_fc[0] = (pred.next1_pct/3.0 - targets[11]/3.0)*2.0;
        g_fc[1] = (pred.next2_pct/3.0 - targets[12]/3.0)*2.0;

        // PATCH1: Focal Loss gradient
        // gradient = class_w * regime_w * focal_factor * softmax_grad
        // focal_factor = (1-p_bc)^gamma — easy sample discount
        // Derivation: d/dz_k of focal_ce = w*focal_fac*(p_k - one_hot_k) + w*gamma*(p_bc-1)*p_k
        // Simplification: approximate focal gradient = w * focal_fac * (p_k - one_hot_k)
        // (full formula unstable; this approximation widely used in practice)
        {
            int bc = (int)std::round(targets[13]);
            bc = std::max(0, std::min(2, bc));
            double cls_p[3] = {pred.cls_up, pred.cls_down, pred.cls_flat};
            double p_bc  = std::max(cls_p[bc], 1e-7);
            double focal = std::pow(1.0 - p_bc, FOCAL_GAMMA); // discount easy samples
            double w = g_cls_weights[bc] * g_cur_regime_w * focal;
            for(int k=0;k<3;k++)
                g_cls[k] = w * (cls_p[k] - (k==bc ? 1.0 : 0.0));
        }

        // Loss weights:
        // PRIMARY: forecast classification (barrier edge) — 0.35
        //   Nega 0.35: bu asosiy signal, eng muhim gradient
        // regime CE: 0.25 — kontekst muhim, lekin secondary
        // trend MSE: 0.20 — analog signal, foydali representation
        // momentum MSE: 0.15 — qo'shimcha signal
        // forecast reg: 0.05 — diagnostic, trunk ni o'ldirmaylik
        for(auto& x:g_cls)    x*=0.35/3;
        for(auto& x:g_regime) x*=0.25/4;
        for(auto& x:g_trend)  x*=0.20/3;
        for(auto& x:g_mom)    x*=0.15/4;
        for(auto& x:g_fc)     x*=0.05/2;

        // NaN FIX: gradient NaN tekshirish
        auto nan_check=[](std::vector<double>& g){
            for(auto& x:g) if(std::isnan(x)||std::isinf(x)) x=0.0;
        };
        nan_check(g_regime); nan_check(g_trend);
        nan_check(g_mom); nan_check(g_fc);

        // NaN check cls
        auto nan_check_cls=[](std::vector<double>& g){
            for(auto& x:g) if(std::isnan(x)||std::isinf(x)) x=0.0;
        };
        nan_check_cls(g_cls);

        // Head backward -> dense gradient
        std::vector<double> gd(80,0);
        auto d_regime = regime_head.backward(g_regime);
        auto d_trend  = trend_head.backward(g_trend);
        auto d_mom    = momentum_head.backward(g_mom);
        auto d_fc     = forecast_head.backward(g_fc);
        auto d_cls    = forecast_cls_head.backward(g_cls);
        for(int i=0;i<80;i++)
            gd[i]=d_regime[i]+d_trend[i]+d_mom[i]+d_fc[i]+d_cls[i];
        // Global norm clipping
        clip_by_norm(gd, 1.0);

        // Dense backward → merge gradient
        auto g_merged = merge_dense.backward(gd);
        clip_by_norm(g_merged, 1.0);

        // Branch backward (64+64=128)
        std::vector<double> g_h1(64), g_m15(64);
        for(int i=0;i<64;i++) g_h1[i]=g_merged[i];
        for(int i=0;i<64;i++) g_m15[i]=g_merged[64+i];
        clip_by_norm(g_h1, 0.5); clip_by_norm(g_m15, 0.5);
        h1_branch.backward(g_h1);
        m15_branch.backward(g_m15);
    }

    void update(double lr){
        t++;
        h1_branch.update(lr,t);
        m15_branch.update(lr,t);
        merge_dense.update(lr,t);
        regime_head.update(lr,t);
        trend_head.update(lr,t);
        momentum_head.update(lr,t);
        forecast_head.update(lr,t);
        forecast_cls_head.update(lr,t);
    }

    void reset(){
        h1_branch.reset();
        m15_branch.reset();
    }

    int pcount(){
        return h1_branch.pcount()+m15_branch.pcount()+
               merge_dense.ns.size()*(64+1)+
               regime_head.ns.size()*(64+1)+
               trend_head.ns.size()*(64+1)+
               momentum_head.ns.size()*(64+1)+
               forecast_head.ns.size()*(64+1);
    }

    void save(const std::string& fn){
        std::ofstream f(fn,std::ios::binary);
        if(!f) return;
        h1_branch.save(f); m15_branch.save(f);
        auto save_layer=[&](Layer& l){
            for(auto& n:l.ns){
                for(auto& w:n.w) f.write((char*)&w,8);
                f.write((char*)&n.b,8);
            }
        };
        save_layer(merge_dense);
        save_layer(regime_head); save_layer(trend_head);
        save_layer(momentum_head); save_layer(forecast_head);
        save_layer(forecast_cls_head);
        std::cout<<"[Save] "<<fn<<"\n";
    }

    bool load(const std::string& fn){
        std::ifstream f(fn,std::ios::binary);
        if(!f) return false;
        h1_branch.load(f); m15_branch.load(f);
        auto load_layer=[&](Layer& l){
            for(auto& n:l.ns){
                for(auto& w:n.w) f.read((char*)&w,8);
                f.read((char*)&n.b,8);
            }
        };
        load_layer(merge_dense);
        load_layer(regime_head); load_layer(trend_head);
        load_layer(momentum_head); load_layer(forecast_head);
        // cls_head: eski model bo'lsa skip (graceful degradation)
        if(f.peek()!=EOF) load_layer(forecast_cls_head);
        std::cout<<"[Load] "<<fn<<"\n";
        return true;
    }

    void info(){
        printf("╔══ MIE-v4.1 ════════════════════════════════════════╗\n");
        printf("║  H1+M15: 70->64 | Merge: 128->80                  ║\n");
        printf("║  Heads: Regime+Trend+Mom+Forecast+NARRATIVE(5)     ║\n");
        printf("║  Features: 35 base + 35 directional = 70           ║\n");
        printf("║  GRU: learnable update gate | AdamW: aktiv         ║\n");
        printf("║  Params: ~%d                                  ║\n", pcount());
        printf("╚══════════════════════════════════════════════════════╝\n");
    }
};

// ════════════════════════════════════════════════════════════
//  CANDLE & DATA
// ════════════════════════════════════════════════════════════
// GCC 13 false-positive warning suppression (stl_algobase memmove)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
struct Candle {
    long long timestamp=0;
    double open=0,high=0,low=0,close=0,volume=0;
};
#pragma GCC diagnostic pop

std::vector<Candle> load_csv(const std::string& fn){
    std::ifstream f(fn);
    if(!f) return {};
    std::vector<Candle> d;
    std::string line;
    std::getline(f,line); // header
    while(std::getline(f,line)){
        std::istringstream ss(line);
        std::string tok; Candle c;
        std::getline(ss,tok,','); c.timestamp=std::stoll(tok);
        std::getline(ss,tok,','); c.open=std::stod(tok);
        std::getline(ss,tok,','); c.high=std::stod(tok);
        std::getline(ss,tok,','); c.low=std::stod(tok);
        std::getline(ss,tok,','); c.close=std::stod(tok);
        std::getline(ss,tok,','); c.volume=std::stod(tok);
        d.push_back(c);
    }
    return d;
}

// ════════════════════════════════════════════════════════════
//  FEATURE ENGINEERING
//  H1 features: macro trend/regime
//  M15 features: micro momentum/pressure
// ════════════════════════════════════════════════════════════
namespace FE {

std::vector<double> closes(const std::vector<Candle>& d){
    std::vector<double> c; for(auto& x:d) c.push_back(x.close); return c;
}

double ema(const std::vector<double>& cl,int i,int p){
    if(i<0||(int)cl.size()==0) return cl.empty()?0:cl[0];
    double k=2.0/(p+1),e=cl[std::max(0,i-p)];
    for(int j=std::max(1,i-p+1);j<=i&&j<(int)cl.size();j++) e=cl[j]*k+e*(1-k);
    return e;
}

double atr(const std::vector<Candle>& d,int i,int p=14){
    if(i<1) return d[i].high-d[i].low;
    double s=0; int cnt=0;
    for(int j=std::max(1,i-p+1);j<=i&&j<(int)d.size();j++){
        double tr=std::max({d[j].high-d[j].low,
                            std::abs(d[j].high-d[j-1].close),
                            std::abs(d[j].low-d[j-1].close)});
        s+=tr; cnt++;
    }
    return cnt>0?s/cnt:d[i].high-d[i].low;
}

double rsi(const std::vector<double>& cl,int i,int p=14){
    if(i<p) return 50;
    double u=0,d_=0;
    for(int j=i-p+1;j<=i;j++){
        double ch=cl[j]-cl[j-1];
        if(ch>0) u+=ch; else d_-=ch;
    }
    if(d_<1e-10) return 100;
    return 100-100/(1+u/d_);
}

double vol_ratio(const std::vector<Candle>& d,int i,int p=20){
    if(i<p) return 1.0;
    double s=0; int cnt=0;
    for(int j=i-p;j<i;j++){s+=d[j].volume;cnt++;}
    return cnt>0&&s>0?d[i].volume/(s/cnt):1.0;
}

// 15 features per timeframe
std::vector<double> extract_tf(const std::vector<Candle>& data,int idx){
    if(idx<14||data.empty()) return std::vector<double>(15,0.0);
    auto cl=closes(data);
    double c=data[idx].close;
    double a=std::max(atr(data,idx),c*0.001);
    double e20=ema(cl,idx,20), e50=ema(cl,idx,50);
    double rsi_v=(rsi(cl,idx)-50)/50;
    double macd=clip((ema(cl,idx,12)-ema(cl,idx,26))/a,3);
    double vr=vol_ratio(data,idx);

    // BB
    double mean=0; int bw=20;
    for(int j=std::max(0,idx-bw+1);j<=idx;j++) mean+=cl[j];
    mean/=std::min(bw,idx+1);
    double var=0;
    for(int j=std::max(0,idx-bw+1);j<=idx;j++) var+=(cl[j]-mean)*(cl[j]-mean);
    double std_=std::sqrt(var/std::min(bw,idx+1)+1e-10);
    double bb_pos=(c-(mean-2*std_))/(4*std_+1e-10)-0.5;

    double body=(data[idx].close-data[idx].open)/a;
    double wick_up=(data[idx].high-std::max(data[idx].open,data[idx].close))/a;
    double wick_dn=(std::min(data[idx].open,data[idx].close)-data[idx].low)/a;

    // Multi-horizon returns
    double r1 = idx>=1  ? clip((cl[idx]-cl[idx-1])/a,3)  : 0;
    double r5 = idx>=5  ? clip((cl[idx]-cl[idx-5])/a,3)  : 0;
    double r20= idx>=20 ? clip((cl[idx]-cl[idx-20])/a,3) : 0;

    return {
        clip((c-e20)/a,3),      // EMA20 distance
        clip((c-e50)/a,3),      // EMA50 distance
        clip((e20-e50)/a,3),    // EMA cross
        rsi_v,                  // RSI normalized
        macd,                   // MACD
        bb_pos,                 // BB position
        clip(body,3),           // candle body
        clip(wick_up,3),        // upper wick
        clip(wick_dn,3),        // lower wick
        std::log(std::max(vr,0.01)), // volume ratio log
        r1, r5, r20,            // returns
        clip(std_/c*100,3),     // volatility %
        clip(ema(cl,idx,5)/e20-1,3), // short/mid ema ratio
    };
}

// ════════════════════════════════════════════════════════════
//  20 MARKET STRUCTURE FEATURES
// ════════════════════════════════════════════════════════════

// 1) Swing high/low topish (lookback shamlar ichida)
double swing_high(const std::vector<Candle>& d, int i, int lb=20){
    double mx=0;
    for(int j=std::max(0,i-lb);j<i;j++) mx=std::max(mx,d[j].high);
    return mx;
}
double swing_low(const std::vector<Candle>& d, int i, int lb=20){
    double mn=d[std::max(0,i-lb)].low;
    for(int j=std::max(0,i-lb);j<i;j++) mn=std::min(mn,d[j].low);
    return mn;
}

// 3-4) Liquidity sweep detection
// Sweep up: narx swing high'ni kesdi, lekin yopilishi pastda
bool sweep_up(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb+1) return false;
    double sh=swing_high(d,i,lb);
    // Oxirgi 3 sham ichida sweep
    for(int j=std::max(1,i-2);j<=i;j++){
        if(d[j].high>sh && d[j].close<sh) return true;
    }
    return false;
}
bool sweep_down(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb+1) return false;
    double sl=swing_low(d,i,lb);
    for(int j=std::max(1,i-2);j<=i;j++){
        if(d[j].low<sl && d[j].close>sl) return true;
    }
    return false;
}

// 14) Reclaim after sweep
bool reclaim_after_sweep(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb+3) return false;
    double sh=swing_high(d,i,lb);
    double sl=swing_low(d,i,lb);
    // Sweep high + reclaim (close yuqorida)
    bool sw_up=false, sw_dn=false;
    for(int j=std::max(1,i-4);j<=i;j++){
        if(d[j].high>sh) sw_up=true;
        if(d[j].low<sl)  sw_dn=true;
    }
    if(sw_up && d[i].close>sh) return true; // yuqori sweep + reclaim
    if(sw_dn && d[i].close<sl) return true; // pastki sweep + reclaim
    return false;
}

// 5-6) Break of Structure
bool bos_up(const std::vector<Candle>& d, int i, int lb=30){
    if(i<lb+1) return false;
    double prev_sh=swing_high(d,i-1,lb);
    return d[i].close > prev_sh;
}
bool bos_down(const std::vector<Candle>& d, int i, int lb=30){
    if(i<lb+1) return false;
    double prev_sl=swing_low(d,i-1,lb);
    return d[i].close < prev_sl;
}

// 7-8) Failed breakout
bool failed_bos_up(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb+1) return false;
    double sh=swing_high(d,i,lb);
    // Wick yuqorida (breakout) lekin close pastda
    return d[i].high>sh && d[i].close<sh;
}
bool failed_bos_down(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb+1) return false;
    double sl=swing_low(d,i,lb);
    return d[i].low<sl && d[i].close>sl;
}

// 9) Range boundary proximity [-1=pastda, +1=tepada]
double range_proximity(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb) return 0;
    double sh=swing_high(d,i,lb);
    double sl=swing_low(d,i,lb);
    double rng=sh-sl;
    if(rng<1e-8) return 0;
    return clip((d[i].close-sl)/rng*2-1, 1.0);
}

// 10) Range compression score (0=normal, 1=juda siqilgan)
double compression_score(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb*2) return 0;
    // Oxirgi lb ATR o'rtachasi vs oldingi lb ATR o'rtachasi
    double a1=0,a2=0;
    int lb2=lb/2;
    for(int j=i-lb2;j<=i;j++)   a1+=d[j].high-d[j].low;
    for(int j=i-lb;j<i-lb2;j++) a2+=d[j].high-d[j].low;
    a1/=lb2+1; a2/=lb2;
    if(a2<1e-8) return 0;
    return clip(1.0-a1/a2, 1.0); // a1<a2 → siqilish
}

// 11) Expansion score
double expansion_score(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb) return 0;
    double cur=d[i].high-d[i].low;
    double avg=0;
    for(int j=i-lb;j<i;j++) avg+=d[j].high-d[j].low;
    avg/=lb;
    if(avg<1e-8) return 0;
    return clip(cur/avg-1, 3.0); // 0=normal, >1=expansion
}

// 12) Displacement candle score (ATR ga nisbatan body)
double displacement_score(const std::vector<Candle>& d, int i){
    if(i<1) return 0;
    double a=atr(d,i,14);
    if(a<1e-8) return 0;
    double body=std::abs(d[i].close-d[i].open);
    return clip(body/a, 3.0);
}

// 13) Wick trap score (katta wick + kichik body → rejection)
double wick_trap_score(const std::vector<Candle>& d, int i){
    double body=std::abs(d[i].close-d[i].open);
    double wick_up=d[i].high-std::max(d[i].open,d[i].close);
    double wick_dn=std::min(d[i].open,d[i].close)-d[i].low;
    double total=d[i].high-d[i].low;
    if(total<1e-8) return 0;
    double max_wick=std::max(wick_up,wick_dn);
    // Katta wick, kichik body → trap
    return clip((max_wick-body)/total, 1.0);
}

// 15) HTF bias alignment (H1 trend vs M15 micro momentum)
// Returns: +1=aligned bullish, -1=aligned bearish, 0=mixed
double htf_bias_alignment(const std::vector<double>& h1_feat,
                           const std::vector<double>& m15_feat){
    // feat[0]=EMA20 dist, feat[3]=RSI
    if(h1_feat.size()<4||m15_feat.size()<4) return 0;
    double h1_bias = h1_feat[0]>0?1:-1;  // H1 EMA20 yuqorida?
    double m15_mom = m15_feat[3];          // M15 RSI direction
    return clip(h1_bias * (m15_mom>0?1:-1), 1.0);
}

// 16) Pullback quality score
// Sog'lom pullback: kichik, tez, trendsiz
double pullback_quality(const std::vector<Candle>& d, int i, int lb=10){
    if(i<lb+5) return 0;
    auto cl=closes(d);
    double main_trend=(cl[i-lb]-cl[std::max(0,i-lb*2)])/std::max(cl[std::max(0,i-lb*2)],1.0);
    double pullback =(cl[i]-cl[i-lb])/std::max(cl[i-lb],1.0);
    // Teskari yo'nalish va kichik (pullback < 50% of main move)
    if(std::abs(main_trend)<1e-4) return 0;
    bool opposite=(main_trend>0&&pullback<0)||(main_trend<0&&pullback>0);
    if(!opposite) return 0;
    double ratio=std::abs(pullback)/std::abs(main_trend);
    return clip(1.0-ratio*2, 1.0); // kichik pullback → yaxshi sifat
}

// 17) Impulse-to-pullback ratio
double impulse_pullback_ratio(const std::vector<Candle>& d, int i, int lb=20){
    if(i<lb+5) return 1;
    auto cl=closes(d);
    // Oxirgi lb sham ichida max impuls va max pullback
    double max_up=0, max_dn=0;
    for(int j=i-lb+1;j<=i;j++){
        double mv=cl[j]-cl[j-1];
        if(mv>0) max_up+=mv;
        else max_dn-=mv;
    }
    if(max_dn<1e-8) return 3.0;
    return clip(max_up/max_dn, 3.0);
}

// 18) Local liquidity density (qayta tegilgan zonalar)
double liquidity_density(const std::vector<Candle>& d, int i, int lb=30){
    if(i<lb) return 0;
    double sh=swing_high(d,i,lb);
    double sl=swing_low(d,i,lb);
    double zone=0.003; // 0.3% zona
    int touches=0;
    for(int j=std::max(0,i-lb);j<i;j++){
        if(std::abs(d[j].high-sh)/sh < zone) touches++;
        if(std::abs(d[j].low-sl)/sl  < zone) touches++;
    }
    return clip(touches/5.0, 1.0);
}

// 19) Volatility regime transition
// +1=pasayishdan oshishga, -1=oshishdan pasayishga
double vol_regime_transition(const std::vector<Candle>& d, int i, int lb=14){
    if(i<lb*2) return 0;
    double a1=atr(d,i,lb);
    double a2=atr(d,i-lb,lb);
    if(a2<1e-8) return 0;
    double ratio=a1/a2;
    if(ratio>1.3) return 1.0;   // oshish
    if(ratio<0.7) return -1.0;  // pasayish
    return clip(ratio-1.0, 1.0);
}

// 20) Candle sequence pattern state
// 0=grind, 1=impulse, 2=indecision, 3=trap, 4=exhaustion
double candle_sequence(const std::vector<Candle>& d, int i){
    if(i<4) return 0;
    auto a=atr(d,i,14);
    if(a<1e-8) return 0;

    // Oxirgi 3 sham
    double bodies[3], wicks[3];
    for(int k=0;k<3;k++){
        int j=i-2+k;
        bodies[k]=std::abs(d[j].close-d[j].open)/a;
        wicks[k]=(d[j].high-d[j].low-std::abs(d[j].close-d[j].open))/a;
    }
    double avg_body=(bodies[0]+bodies[1]+bodies[2])/3;
    double avg_wick=(wicks[0]+wicks[1]+wicks[2])/3;

    // Pattern detection
    if(avg_body>1.5 && avg_wick<0.5)             return 1.0/4; // impulse
    if(avg_body<0.4 && avg_wick>0.8)             return 2.0/4; // indecision
    if(wicks[2]>1.5 && bodies[2]<0.4)            return 3.0/4; // trap (last candle)
    if(avg_body>1.0 && bodies[2]<bodies[0]*0.5)  return 4.0/4; // exhaustion
    return 0.0; // grind
}


// ════════════════════════════════════════════════════════════
//  DIRECTIONAL FEATURE HELPERS
//  Faqat past/current candles. No leakage.
// ════════════════════════════════════════════════════════════

// avg_abs_body_n: oxirgi n sham uchun |close-open| o'rtachasi
static inline double avg_abs_body(const std::vector<Candle>& d, int i, int n){
    if(i<1) return 1e-8;
    double s=0; int cnt=0;
    for(int j=std::max(0,i-n+1);j<=i;j++){s+=std::abs(d[j].close-d[j].open);cnt++;}
    return cnt>0?s/cnt:1e-8;
}

// avg_volume_n
static inline double avg_vol(const std::vector<Candle>& d, int i, int n){
    if(i<1) return 1e-8;
    double s=0; int cnt=0;
    for(int j=std::max(0,i-n+1);j<=i;j++){s+=d[j].volume;cnt++;}
    return cnt>0?s/cnt:1e-8;
}

// rolling_high / rolling_low: PAST n sham, current sham dahil emas (no lookahead)
static inline double roll_high(const std::vector<Candle>& d, int i, int n){
    double mx=d[std::max(0,i-n)].high;
    for(int j=std::max(0,i-n);j<i;j++) mx=std::max(mx,d[j].high);
    return mx;
}
static inline double roll_low(const std::vector<Candle>& d, int i, int n){
    double mn=d[std::max(0,i-n)].low;
    for(int j=std::max(0,i-n);j<i;j++) mn=std::min(mn,d[j].low);
    return mn;
}

// pressure_persistence: oxirgi n shamda bir xil yo'nalish qanchalik izchil
// Returns [-1,1]: +1=hammasi bullish, -1=hammasi bearish
static inline double pressure_persistence(const std::vector<Candle>& d, int i, int n=5){
    if(i<n) return 0;
    double s=0;
    for(int j=i-n+1;j<=i;j++) s+=(d[j].close>d[j].open)?1.0:-1.0;
    return s/n;
}

// ════════════════════════════════════════════════════════════
//  YANGI extract_tf: 15 indicator + 20 structure + 35 directional = 70 features
// ════════════════════════════════════════════════════════════
std::vector<double> extract_tf_structure(const std::vector<Candle>& data, int idx,
                                          const std::vector<double>& other_tf_feat={}){
    if(idx<20||data.empty()) return std::vector<double>(70,0.0);
    auto cl=closes(data);
    double c=data[idx].close;
    double a=std::max(atr(data,idx),c*0.001);
    double e20=ema(cl,idx,20), e50=ema(cl,idx,50);
    double rsi_v=(rsi(cl,idx)-50)/50;
    double macd_v=clip((ema(cl,idx,12)-ema(cl,idx,26))/a,3);
    double vr=vol_ratio(data,idx);

    // BB
    double mean=0; int bw=20;
    for(int j=std::max(0,idx-bw+1);j<=idx;j++) mean+=cl[j];
    mean/=std::min(bw,idx+1);
    double var=0;
    for(int j=std::max(0,idx-bw+1);j<=idx;j++) var+=(cl[j]-mean)*(cl[j]-mean);
    double std_=std::sqrt(var/std::min(bw,idx+1)+1e-10);
    double bb_pos=(c-(mean-2*std_))/(4*std_+1e-10)-0.5;

    double body=(data[idx].close-data[idx].open)/a;
    double wick_up=(data[idx].high-std::max(data[idx].open,data[idx].close))/a;
    double wick_dn=(std::min(data[idx].open,data[idx].close)-data[idx].low)/a;

    double r1 = idx>=1  ? clip((cl[idx]-cl[idx-1])/a,3)  : 0;
    double r5 = idx>=5  ? clip((cl[idx]-cl[idx-5])/a,3)  : 0;
    double r20= idx>=20 ? clip((cl[idx]-cl[idx-20])/a,3) : 0;

    // === 15 indicator features ===
    std::vector<double> feats = {
        clip((c-e20)/a,3),           // 0: EMA20 distance
        clip((c-e50)/a,3),           // 1: EMA50 distance
        clip((e20-e50)/a,3),         // 2: EMA cross
        rsi_v,                        // 3: RSI
        macd_v,                       // 4: MACD
        bb_pos,                       // 5: BB position
        clip(body,3),                 // 6: candle body
        clip(wick_up,3),              // 7: upper wick
        clip(wick_dn,3),              // 8: lower wick
        std::log(std::max(vr,0.01)), // 9: volume ratio
        r1, r5, r20,                  // 10-12: returns
        clip(std_/c*100,3),           // 13: volatility %
        clip(ema(cl,idx,5)/e20-1,3), // 14: short/mid ema ratio
    };

    // === 20 market structure features ===
    double sh=swing_high(data,idx,20);
    double sl=swing_low(data,idx,20);

    // 1) Swing high distance (ATR ga nisbatan)
    feats.push_back(clip((sh-c)/a, 5.0));
    // 2) Swing low distance
    feats.push_back(clip((c-sl)/a, 5.0));
    // 3) Sweep up flag
    feats.push_back(sweep_up(data,idx) ? 1.0 : 0.0);
    // 4) Sweep down flag
    feats.push_back(sweep_down(data,idx) ? 1.0 : 0.0);
    // 5) BOS up
    feats.push_back(bos_up(data,idx) ? 1.0 : 0.0);
    // 6) BOS down
    feats.push_back(bos_down(data,idx) ? 1.0 : 0.0);
    // 7) Failed BOS up
    feats.push_back(failed_bos_up(data,idx) ? 1.0 : 0.0);
    // 8) Failed BOS down
    feats.push_back(failed_bos_down(data,idx) ? 1.0 : 0.0);
    // 9) Range proximity
    feats.push_back(range_proximity(data,idx));
    // 10) Compression score
    feats.push_back(compression_score(data,idx));
    // 11) Expansion score
    feats.push_back(expansion_score(data,idx));
    // 12) Displacement candle
    feats.push_back(displacement_score(data,idx));
    // 13) Wick trap score
    feats.push_back(wick_trap_score(data,idx));
    // 14) Reclaim after sweep
    feats.push_back(reclaim_after_sweep(data,idx) ? 1.0 : 0.0);
    // 15) HTF bias alignment (agar other_tf_feat berilgan bo'lsa)
    feats.push_back(other_tf_feat.size()>=4 ? htf_bias_alignment(feats,other_tf_feat) : 0.0);
    // 16) Pullback quality
    feats.push_back(pullback_quality(data,idx));
    // 17) Impulse-to-pullback ratio
    feats.push_back(impulse_pullback_ratio(data,idx));
    // 18) Liquidity density
    feats.push_back(liquidity_density(data,idx));
    // 19) Vol regime transition
    feats.push_back(vol_regime_transition(data,idx));
    // 20) Candle sequence pattern
    feats.push_back(candle_sequence(data,idx));

    // ═══ BLOCK 1: CANDLE MICROSTRUCTURE (directional pressure) ═══
    {
        const auto& cd = data[idx];
        double rng = std::max(cd.high - cd.low, 1e-8);
        double eps = 1e-8;
        // close/open position in candle → buy/sell close strength
        double close_pos = (cd.close - cd.low) / rng;          // [0,1]
        double open_pos  = (cd.open  - cd.low) / rng;          // [0,1]

        feats.push_back(clip(close_pos, 1.0)*2 - 1);          // [35] close_pos_in_candle [-1,1]
        feats.push_back(open_pos*2 - 1);                       // [36] open_pos_in_candle [-1,1]
        feats.push_back(std::abs(cd.close-cd.open)/rng);       // [37] body_to_range [0,1]
        // signed body vs ATR — direction + magnitude
        feats.push_back(clip((cd.close-cd.open)/a, 3.0));      // [38] signed_body_vs_atr
        feats.push_back(clip(rng/a, 5.0));                     // [39] range_vs_atr

        // close vs prev candle high/low — did price break or stay?
        double ph = idx>=1 ? data[idx-1].high  : cd.high;
        double pl = idx>=1 ? data[idx-1].low   : cd.low;
        feats.push_back(clip((cd.close - ph)/a, 3.0));         // [40] close_vs_prev_high
        feats.push_back(clip((cd.close - pl)/a, 3.0));         // [41] close_vs_prev_low
    }

    // ═══ BLOCK 2: SHORT-TERM DIRECTIONAL PRESSURE ═══
    {
        // bull/bear count — direction dominance
        int bull3=0, bear3=0, bull5=0, bear5=0;
        for(int j=std::max(0,idx-2);j<=idx;j++){ if(data[j].close>data[j].open) bull3++; else bear3++; }
        for(int j=std::max(0,idx-4);j<=idx;j++){ if(data[j].close>data[j].open) bull5++; else bear5++; }
        int n3=std::min(3,idx+1), n5=std::min(5,idx+1);
        feats.push_back((double)bull3/n3);                      // [42] bull_count_3
        feats.push_back((double)bear3/n3);                      // [43] bear_count_3
        feats.push_back((double)bull5/n5);                      // [44] bull_count_5
        feats.push_back((double)bear5/n5);                      // [45] bear_count_5

        // net_body: signed net force over window
        double nb3=0, nb5=0;
        for(int j=std::max(0,idx-2);j<=idx;j++) nb3+=(data[j].close-data[j].open);
        for(int j=std::max(0,idx-4);j<=idx;j++) nb5+=(data[j].close-data[j].open);
        feats.push_back(clip(nb3/a, 5.0));                      // [46] net_body_3/atr
        feats.push_back(clip(nb5/a, 5.0));                      // [47] net_body_5/atr

        // close change acceleration: momentum change direction
        double ch1 = idx>=1 ? cl[idx]-cl[idx-1]   : 0;
        double ch2 = idx>=2 ? cl[idx-1]-cl[idx-2] : 0;
        feats.push_back(clip((ch1-ch2)/a, 3.0));               // [48] close_change_accel

        // pressure persistence: consistent direction score
        feats.push_back(pressure_persistence(data, idx, 5));   // [49] pressure_persist [-1,1]
    }

    // ═══ BLOCK 3: RANGE LOCATION / LIQUIDITY CONTEXT ═══
    {
        double rh10=roll_high(data,idx,10), rl10=roll_low(data,idx,10);
        double rh20=roll_high(data,idx,20), rl20=roll_low(data,idx,20);
        double rh50=roll_high(data,idx,50), rl50=roll_low(data,idx,50);
        double c=data[idx].close;

        // close position in rolling range → [-1,1]
        auto range_pos=[&](double rh, double rl)->double{
            double rng=std::max(rh-rl,1e-8);
            return clip((c-rl)/rng*2-1, 1.0);
        };
        feats.push_back(range_pos(rh10,rl10));                 // [50] pos_in_10bar
        feats.push_back(range_pos(rh20,rl20));                 // [51] pos_in_20bar
        feats.push_back(range_pos(rh50,rl50));                 // [52] pos_in_50bar

        // dist to boundary — how far to potential breakout/breakdown
        feats.push_back(clip((rh20-c)/a, 5.0));               // [53] dist_to_20bar_high
        feats.push_back(clip((c-rl20)/a, 5.0));               // [54] dist_to_20bar_low

        // breakout close strength: agar current high 20-bar high'ni urdi
        // pozitiv = bullish close above breakout; negatif = bearish
        double prev_rh20 = roll_high(data, std::max(0,idx-1), 20);
        double prev_rl20 = roll_low(data,  std::max(0,idx-1), 20);
        double bk_bull = (data[idx].high > prev_rh20) ?
                         clip((c - prev_rh20)/a, 3.0) : 0.0;
        double bk_bear = (data[idx].low  < prev_rl20) ?
                         clip((prev_rl20 - c)/a, 3.0) : 0.0;
        feats.push_back(bk_bull);                              // [55] breakout_close_str_up
        feats.push_back(bk_bear);                              // [56] breakout_close_str_dn

        // fake break: wick beyond boundary, close back inside
        double fake_up = (data[idx].high > prev_rh20 && c < prev_rh20) ?
                         clip((data[idx].high - prev_rh20)/a, 2.0) : 0.0;
        double fake_dn = (data[idx].low < prev_rl20 && c > prev_rl20) ?
                         clip((prev_rl20 - data[idx].low)/a, 2.0) : 0.0;
        feats.push_back(fake_up);                              // [57] fake_break_up
        feats.push_back(fake_dn);                              // [58] fake_break_dn
    }

    // ═══ BLOCK 4: VOLATILITY TRANSITION (directional context) ═══
    {
        double atr5  = std::max(atr(data,idx, 5), 1e-8);
        double atr20 = std::max(atr(data,idx,20), 1e-8);
        // atr ratio: expansion → direction often continues
        feats.push_back(clip(atr5/atr20, 3.0));               // [59] atr_short_over_long

        // avg range ratios
        double ar5=0, ar10=0, ar20=0;
        for(int j=std::max(0,idx-4);j<=idx;j++)   ar5 +=data[j].high-data[j].low;
        for(int j=std::max(0,idx-9);j<=idx;j++)   ar10+=data[j].high-data[j].low;
        for(int j=std::max(0,idx-19);j<=idx;j++)  ar20+=data[j].high-data[j].low;
        int n5=std::min(5,idx+1),n10=std::min(10,idx+1),n20=std::min(20,idx+1);
        ar5/=n5; ar10/=n10; ar20/=n20;
        double cur_rng=data[idx].high-data[idx].low;
        // expansion_now: big candle relative to recent avg → directional move likely
        feats.push_back(clip(ar20>1e-8?ar5/ar20:1.0, 3.0));   // [60] range_mean_5_over_20
        feats.push_back(clip(ar10>1e-8?cur_rng/ar10:1.0, 5.0)); // [61] expansion_now

        // body expansion: katta body → strong directional push
        double avg_body10 = std::max(avg_abs_body(data,idx,10), 1e-8);
        double cur_body   = std::abs(data[idx].close-data[idx].open);
        feats.push_back(clip(cur_body/avg_body10, 5.0));       // [62] body_expansion_now
    }

    // ═══ BLOCK 5: VOLUME-DERIVED DIRECTIONAL PROXY ═══
    {
        double avgvol20 = std::max(avg_vol(data,idx,20), 1e-8);
        double vol      = std::max(data[idx].volume, 0.0);
        double vr_log   = std::log(1.0 + vol/avgvol20);
        double rng      = std::max(data[idx].high-data[idx].low, 1e-8);

        // vol zscore: normalized volume (direction agnostic, but needed for below)
        {
            double sv=0,sv2=0; int vn=0;
            for(int j=std::max(0,idx-19);j<=idx;j++){sv+=data[j].volume;sv2+=data[j].volume*data[j].volume;vn++;}
            double vm=sv/vn, vstd=std::sqrt(std::max(sv2/vn-vm*vm,0.0)+1e-8);
            feats.push_back(clip((vol-vm)/vstd, 3.0));         // [63] vol_zscore_20
        }

        // signed_vol: direction × effort — KEY directional proxy
        double body_sign = data[idx].close>data[idx].open ? 1.0 : -1.0;
        feats.push_back(clip(body_sign * vr_log, 4.0));        // [64] signed_vol

        // effort_vs_result: high volume + small move = absorption (fakeout warning)
        // high volume + big move = conviction (directional confirm)
        double abs_body = std::max(std::abs(data[idx].close-data[idx].open), 1e-8);
        feats.push_back(clip(vr_log / (abs_body/a), 5.0));     // [65] effort_vs_result

        // close_pos_in_candle (repeat for vol context)
        double close_pos_raw = (data[idx].close - data[idx].low)/rng;
        // up_effort_fail: big volume, bull candle, but close low in range → absorption
        bool is_bull = data[idx].close > data[idx].open;
        feats.push_back((is_bull && vol>avgvol20*1.5 && close_pos_raw<0.35) ? 1.0 : 0.0);  // [66] up_effort_fail
        feats.push_back((!is_bull && vol>avgvol20*1.5 && close_pos_raw>0.65) ? 1.0 : 0.0); // [67] dn_effort_fail

        // vol spike + breakout confirm → directional continuation
        double prev_rh20=roll_high(data,std::max(0,idx-1),20);
        double prev_rl20=roll_low(data, std::max(0,idx-1),20);
        bool vol_spike  = vol > avgvol20*2.0;
        bool bk_up      = data[idx].close > prev_rh20;
        bool bk_dn      = data[idx].close < prev_rl20;
        feats.push_back((vol_spike && bk_up) ? 1.0 : 0.0);    // [68] vol_spike_bk_confirm_up
        feats.push_back((vol_spike && bk_dn) ? 1.0 : 0.0);    // [69] vol_spike_bk_confirm_dn
    }

    return feats; // 70 features
}

// ── BOZOR REJIMI ─────────────────────────────────────────
// 3 ta sodda, deterministik holat. Faqat past ma'lumot (idx gacha).
// Nima uchun 3 ta? TREND/RANGE/VOLATILE har biri boshqacha barrier politikasi talab qiladi.
// Nima uchun compute_regime() emas, detect_regime()?
//   compute_regime() → 4 holat (up/down/range/volatile), neural head uchun
//   detect_regime()  → 3 holat, barrier policy uchun. Alohida logik.
enum class Regime { TREND=0, RANGE=1, VOLATILE=2 };

inline Regime detect_regime(const std::vector<Candle>& d, int i){
    // Yetarli history bo'lmasa: konservativ RANGE
    if(i < 50) return Regime::RANGE;

    auto cl = closes(d);
    double c   = d[i].close;
    // ATR: 14-period true range o'rtacha. Volatilite o'lchov birligi.
    double a   = std::max(atr(d, i, 14), c * 0.0001);
    // EMA20, EMA50: qisqa va uzun muddatli trend yo'nalishi
    double e20 = ema(cl, i, 20);
    double e50 = ema(cl, i, 50);

    // atr_pct: nisbiy volatilite (ATR / price)
    // BTC H1 uchun: normal ~0.4-0.8%, yuqori >1.2%, extreme >2%
    double atr_pct = a / c;

    // ema_gap: EMA20-EMA50 masofa, ATR bilan normallanadi
    // ATR normallamasiz: absolut dollar farq, bozor narxi o'zgarganda ma'nosiz
    // ATR bilan: "trend kuchi = ATR birliklarida" → bozor-invariant
    double ema_gap = (e20 - e50) / a;

    // mom20: 20-sham narx o'zgarishi, ATR bilan normallanadi
    // Nima uchun 20? Bir kunlik (ish kunlari) drift uchun
    double mom20 = (cl[i] - cl[std::max(0, i-20)]) / a;

    // VOLATILE: atr_pct > 1.2% — har soat narx 1.2%+ harakat qilmoqda
    // Nima uchun 1.2%: bu BTC H1 uchun "elevated volatility" chegarasi
    // Bundan yuqorida: katta barrier kerak, aks holda noise label oladi
    if(atr_pct > 0.012) return Regime::VOLATILE;

    // TREND: EMA gap kuchli VA momentum bir yo'nalishda
    // |ema_gap| > 1.5: EMA20-EMA50 oralig'i 1.5 ATR dan katta = aniq trend
    // |mom20| > 3.0:   20 sham ichida 3 ATR harakat = directional momentum bor
    // Nima uchun ikkalasi birga? EMA gap lag bor, mom20 bilan tasdiqlash kerak
    if(std::abs(ema_gap) > 1.5 && std::abs(mom20) > 3.0)
        return Regime::TREND;

    // Qolgan hamma holat: RANGE (konsolidatsiya, sideways)
    return Regime::RANGE;
}

inline const char* regime_str(Regime r){
    switch(r){
        case Regime::TREND:    return "TREND";
        case Regime::RANGE:    return "RANGE";
        case Regime::VOLATILE: return "VOLATILE";
        default:               return "RANGE";
    }
}

// ── BARRIER PARAMETRLARI ──────────────────────────────────
// Har bir regime uchun alohida politika. Maqsad: class dist ~25/25/50 (±10%)
//
// TREND:    K=2.0, H=5
//   Nima uchun K kichik: trend yo'nalishida harakat "oson". Katta K → FLAT collapse.
//   Nima uchun H uzun: trend 1-2 soatda emas, 4-5 soatda realizatsiya qiladi.
//   Nima uchun K=2.0 va H=5: BTC H1 TREND rejimida 2*ATR ~ 1% barrier,
//     5 soatda urilish ehtimoli ~55-65% → UP+DOWN ~55%, FLAT ~45%
//
// RANGE:    K=3.0, H=3
//   Nima uchun K katta: breakout kuchli bo'lishi kerak, range ichida noise ko'p.
//   Nima uchun H qisqa: range ichida uzoq kutish ma'nosiz, narx qaytib keladi.
//   Nima uchun K=3.0 va H=3: 3*ATR breakout + 3 soat → UP+DOWN ~40-50%, FLAT ~50-60%
//
// VOLATILE: K=4.0, H=4
//   Nima uchun K eng katta: ATR yuqori → kichik K = noise UP/DOWN label.
//   Nima uchun H=4: o'rtacha, volatile bozorda 6+ soat kutish xavfli.
//   Nima uchun K=4.0 va H=4: ATR ~1.5% * 4 = 6% barrier → faqat kuchli harakatlar
struct BarrierParams {
    double up_k;    // upper barrier = entry + up_k * ATR
    double down_k;  // lower barrier = entry - down_k * ATR
    int    horizon; // vertical barrier sham soni
};

// Base params — calibrate_barrier_params() bular ustidan ishlaydi
inline BarrierParams get_barrier_params(Regime reg){
    switch(reg){
        case Regime::TREND:    return {2.0, 2.0, 5};
        case Regime::RANGE:    return {3.0, 3.0, 3};
        case Regime::VOLATILE: return {4.0, 4.0, 4};
        default:               return {3.0, 3.0, 3};
    }
}

// Calibrated params — train boshida bir marta hisoblangan
// Global: calibrate_barrier_params() tomonidan to'ldiriladi
static BarrierParams g_barrier_params[3] = {
    {2.0, 2.0, 5},  // TREND
    {3.0, 3.0, 3},  // RANGE
    {4.0, 4.0, 4},  // VOLATILE
};
static bool g_barrier_calibrated = false;


// get_calibrated_params() — compute_triple_barrier() tomonidan ishlatiladi
// Calibratsiya bo'lmagan bo'lsa: base params
inline BarrierParams get_calibrated_params(Regime reg){
    if(!g_barrier_calibrated) return get_barrier_params(reg);
    return g_barrier_params[(int)reg];
}

// compute_regime: neural head target (eski 4-class, saqlanadi)
// Returns: 0=up, 1=down, 2=range, 3=volatile
int compute_regime(const std::vector<Candle>& data, int i){
    if(i<50) return 2;
    auto cl=closes(data);
    double e20=ema(cl,i,20), e50=ema(cl,i,50);
    double a=atr(data,i,14);
    double c=data[i].close;
    double trend = (e20-e50)/std::max(a,1e-8);
    double r20 = (cl[i]-cl[std::max(0,i-20)])/std::max(a,1e-8);
    if(a/c > 0.025) return 3;
    if(trend > 0.5 && r20 > 1.0) return 0;
    if(trend < -0.5 && r20 < -1.0) return 1;
    return 2;
}

// ── DYNAMIC TRIPLE-BARRIER LABEL ─────────────────────────
// YAGONA SOURCE-OF-TRUTH: train/val/eval/analyze hammasi shu funksiyani chaqiradi.
//
// Nima uchun dynamic?
//   Statik K=3.0, H=4 → barcha rejimlarda bir xil → mismatch:
//   TREND da 3.0*ATR = 1.5% → 4 soatda OSON uriladi → FLAT juda oz
//   VOLATILE da 3.0*ATR juda kichik → noise label
//   Dynamic: har regime uchun moslik → barqaror class dist
//
// Entry semantics (live):
//   entry = d[idx+1].open  (idx sham yopildi → idx+1 open da kiramiz)
//   Nima uchun: d[idx].close da kirish REALISTIK EMAS. Slippage, next-tick semantics.
//   d[idx+1].open — bu eng realistic entry narx.
//
// Future leakage:
//   Label uchun idx+1..idx+1+H sham lazim. Bu ATAYLAB — label shu.
//   Feature extraction (extract_tf_structure) hech qachon idx dan keyin qaramaydi.
//
// Same-candle ambiguity:
//   Bir sham ichida HIGH >= upper VA LOW <= lower → intra-candle order noma'lum
//   Conservative: FLAT (no-trade). Optimistic bias yo'q.
inline int compute_triple_barrier(const std::vector<Candle>& d, int idx){
    // Boundary: idx+1 (entry) + max_horizon (check) uchun
    if(idx + TB_MAX_HORIZON + 2 >= (int)d.size()) return TB_FLAT;

    // 1. Regime detection — faqat idx gacha (past only, no leakage)
    Regime reg     = detect_regime(d, idx);
    BarrierParams p = get_calibrated_params(reg);

    // 2. Entry = idx+1 open (live semantics)
    double entry   = d[idx+1].open;
    double atr_val = std::max(atr(d, idx, 14), entry * 0.001);
    double upper   = entry + p.up_k   * atr_val;
    double lower   = entry - p.down_k * atr_val;

    // 3. Check: idx+2 .. idx+1+horizon (entry sham = idx+1, check = idx+2 dan)
    int max_k = std::min(p.horizon, (int)d.size() - 2 - idx);

    for(int k = 1; k <= max_k; k++){
        double hi = d[idx+1+k].high;
        double lo = d[idx+1+k].low;
        bool hit_up = (hi >= upper);
        bool hit_dn = (lo <= lower);
        if(hit_up && hit_dn) return TB_FLAT; // ambiguous → conservative
        if(hit_up) return TB_UP;
        if(hit_dn) return TB_DOWN;
    }
    return TB_FLAT; // vertical barrier: meaningful move bo'lmadi
}

// Convenience: barrier info bilan birga label qaytaradi (analyze uchun)
struct BarrierResult {
    int label;           // TB_UP / TB_DOWN / TB_FLAT
    Regime reg;
    BarrierParams params;
};
inline BarrierResult compute_triple_barrier_full(const std::vector<Candle>& d, int idx){
    if(idx + TB_MAX_HORIZON + 2 >= (int)d.size())
        return {TB_FLAT, Regime::RANGE, {3.0,3.0,3}};
    Regime reg      = detect_regime(d, idx);
    BarrierParams p = get_calibrated_params(reg);
    double entry    = d[idx+1].open;
    double atr_val  = std::max(atr(d, idx, 14), entry * 0.001);
    double upper    = entry + p.up_k   * atr_val;
    double lower    = entry - p.down_k * atr_val;
    int max_k = std::min(p.horizon, (int)d.size() - 2 - idx);
    for(int k=1; k<=max_k; k++){
        double hi = d[idx+1+k].high;
        double lo = d[idx+1+k].low;
        if(hi>=upper && lo<=lower) return {TB_FLAT, reg, p};
        if(hi>=upper) return {TB_UP,   reg, p};
        if(lo<=lower) return {TB_DOWN,  reg, p};
    }
    return {TB_FLAT, reg, p};
}

} // namespace FE

// ════════════════════════════════════════════════════════════
//  ANALIZ CHIQISHI — professional format
// ════════════════════════════════════════════════════════════
void print_bar(const std::string& label, double val, double mn=-1, double mx=1, int w=20){
    double pct=(val-mn)/(mx-mn);
    pct=std::max(0.0,std::min(1.0,pct));
    int filled=(int)(pct*w);
    std::cout<<"  "<<std::setw(18)<<std::left<<label<<" ";
    for(int i=0;i<w;i++) std::cout<<(i<filled?"█":"░");
    std::cout<<" "<<std::fixed<<std::setprecision(2)<<val<<"\n";
}

// ════════════════════════════════════════════════════════════
//  build_analysis: RULE-BASED post-processing
//  Neural model faqat raw signallar beradi.
//  Bu funksiya ularni ierarxik tizimga soladi.
//  Maqsad: "narx qayerga boradi" emas,
//           "qaysi MEXANIZM ishlayapti"
// ════════════════════════════════════════════════════════════
MIEModel::Result build_analysis(MIEModel::Result r){

    // ─── 1. FEATURE GROUP SCORES ─────────────────────────────
    // A) Kontekst: biz qaysi sahnadamiz?
    double ctx = 0.40*std::max(r.regime_up, r.regime_down)
               + 0.30*(1.0 - r.regime_volatile)
               + 0.30*std::min(1.0, std::abs(r.trend_strength));
    r.ctx_score = std::max(0.0, std::min(1.0, ctx));

    // B) Struktura: bozor qanday tuzilgan?
    double str = 0.40*r.breakout_quality
               + 0.35*(1.0 - r.trend_exhaustion)
               + 0.25*(1.0 - r.reversal_risk);
    r.str_score = std::max(0.0, std::min(1.0, str));

    // C) Momentum: ichki kuch bormi?
    double mom = 0.40*std::abs(r.market_pressure)
               + 0.35*std::abs(r.momentum_short)
               + 0.25*std::abs(r.momentum_medium);
    r.mom_score = std::max(0.0, std::min(1.0, mom));

    // D) Tasdiq: signallar bir-birini mustahkamlaydimi?
    bool trend_bull  = r.trend_strength   >  0.20;
    bool trend_bear  = r.trend_strength   < -0.20;
    bool mom_bull    = r.momentum_short   >  0.10;
    bool mom_bear    = r.momentum_short   < -0.10;
    bool press_bull  = r.market_pressure  >  0.10;
    bool press_bear  = r.market_pressure  < -0.10;
    bool bull_aligned = trend_bull && mom_bull && press_bull;
    bool bear_aligned = trend_bear && mom_bear && press_bear;
    bool aligned = bull_aligned || bear_aligned;
    double conf_s = 0.45*(aligned ? 1.0 : 0.0)
                  + 0.30*(1.0 - r.regime_volatile)
                  + 0.25*(1.0 - r.reversal_risk * 0.5);
    r.conf_score = std::max(0.0, std::min(1.0, conf_s));

    // E) Rad etish: signal noto'g'ri bo'lishi mumkinmi?
    double inv = 0.40*r.reversal_risk
               + 0.30*r.trend_exhaustion
               + 0.20*r.regime_volatile
               + 0.10*(1.0 - r.breakout_quality);
    r.inv_score = std::max(0.0, std::min(1.0, inv));

    // ─── 2. DOMINANT NARRATIVE (rule-based) ──────────────────
    // Ierarxiya: Trap > Breakout > Reversal > Continuation > MeanReversion
    r.narr_continuation   = 0;
    r.narr_reversal       = 0;
    r.narr_trap           = 0;
    r.narr_breakout       = 0;
    r.narr_mean_reversion = 0;

    // TRAP: exhaustion + reversal_risk + failed structure
    double trap_score = 0.40*r.reversal_risk
                      + 0.35*r.trend_exhaustion
                      + 0.25*(1.0 - r.breakout_quality);
    r.narr_trap = std::min(1.0, trap_score);

    // BREAKOUT: kuchli structure + low exhaustion + expansion
    double break_score = 0.45*r.breakout_quality
                       + 0.30*(1.0 - r.trend_exhaustion)
                       + 0.25*r.str_score;
    r.narr_breakout = std::min(1.0, break_score);

    // REVERSAL: high exhaustion + reversal risk + trend zaiflikka
    double rev_score = 0.45*r.trend_exhaustion
                     + 0.35*r.reversal_risk
                     + 0.20*(1.0 - std::abs(r.trend_strength));
    r.narr_reversal = std::min(1.0, rev_score);

    // CONTINUATION: aligned + trend kuchli + low exhaustion
    double cont_score = 0.40*(aligned ? 1.0 : 0.0)
                      + 0.35*std::abs(r.trend_strength)
                      + 0.25*(1.0 - r.trend_exhaustion);
    r.narr_continuation = std::min(1.0, cont_score);

    // MEAN REVERSION: ranging + extreme pressure
    double mr_score = 0.50*r.regime_range
                    + 0.30*std::abs(r.market_pressure)
                    + 0.20*(1.0 - r.str_score);
    r.narr_mean_reversion = std::min(1.0, mr_score);

    // PATCH2: Narrative hard filters — primary signal bilan moslashtir
    // Agar primary = WAIT/FLAT/low clarity → breakout va continuation bloklansin
    bool primary_flat    = (r.cls_label == "FLAT");
    bool low_clarity     = (r.cls_confidence < 0.55);
    bool low_mom         = (r.mom_score < 0.20);
    bool no_confirm      = r.confirmations.empty();

    // Qoida 1: primary zaif bo'lsa → breakout/continuation sezilarli pasaysin
    if(primary_flat || low_clarity || low_mom || no_confirm){
        r.narr_breakout      *= 0.10; // WAIT holatda BREAKOUT chiqmasin
        r.narr_continuation  *= 0.10;
    }
    // Qoida 2: ranging rejimda breakout bo'lishi mumkin emas
    if(r.regime_range > 0.55){
        r.narr_breakout *= 0.15;
    }
    // Qoida 3: trend kuchsiz VA momentum past → continuation bo'lishi mumkin emas
    if(std::abs(r.trend_strength) < 0.20 && r.mom_score < 0.20){
        r.narr_continuation *= 0.10;
    }

    // Dominant narrative: eng kuchli mexanizm (filterdan keyin)
    double narr_vals[5] = {r.narr_continuation, r.narr_reversal,
                           r.narr_trap, r.narr_breakout, r.narr_mean_reversion};
    const char* narr_names[5] = {"TREND_CONTINUATION", "REVERSAL",
                                  "LIQUIDITY_TRAP", "BREAKOUT_EXPANSION", "MEAN_REVERSION"};
    int best_n = (int)(std::max_element(narr_vals, narr_vals+5) - narr_vals);
    r.dominant_narrative = narr_names[best_n];

    // ─── 3. CONFIRMATION STACK ───────────────────────────────
    // Kamida 2 ta TURLI guruhdan tasdiq kerak
    r.confirmations.clear();
    r.contradictions.clear();
    int conf_count = 0;

    // Struktura tasdiq
    if(std::abs(r.trend_strength) > 0.30){
        r.confirmations.push_back(
            std::string("Trend ") + (r.trend_strength>0?"BULL":"BEAR")
            + ": kuch=" + std::to_string((int)(std::abs(r.trend_strength)*100)) + "%");
        conf_count++;
    }
    if(r.breakout_quality > 0.55){
        r.confirmations.push_back("Struktura: breakout sifati " +
            std::to_string((int)(r.breakout_quality*100)) + "%");
        conf_count++;
    }

    // Momentum tasdiq
    if(std::abs(r.market_pressure) > 0.25){
        r.confirmations.push_back(
            std::string("Bozor bosimi: ") + (r.market_pressure>0?"BUY":"SELL")
            + " (" + std::to_string((int)(std::abs(r.market_pressure)*100)) + "%)");
        conf_count++;
    }
    if(std::abs(r.momentum_medium) > 0.20 &&
       (r.momentum_medium>0) == (r.trend_strength>0)){
        r.confirmations.push_back("Momentum o'rta muddatli: trend bilan mos");
        conf_count++;
    }

    // Kontekst tasdiq
    if(aligned){
        r.confirmations.push_back(
            std::string(bull_aligned?"BULL":"BEAR") +
            " alignment: trend+momentum+bosim bir yo'nalishda");
        conf_count++;
    }

    // Volatile past = aniq signal
    if(r.regime_volatile < 0.15 && r.ctx_score > 0.55){
        r.confirmations.push_back("Kontekst aniq: past volatilite, sahna toza");
        conf_count++;
    }

    // ─── 4. CONTRADICTION STACK ──────────────────────────────
    if(r.reversal_risk > 0.50)
        r.contradictions.push_back("Reversal risk YUQORI: " +
            std::to_string((int)(r.reversal_risk*100)) + "%");
    if(r.trend_exhaustion > 0.60)
        r.contradictions.push_back("Trend CHARCHAGAN: " +
            std::to_string((int)(r.trend_exhaustion*100)) + "%");
    if(!aligned && std::abs(r.trend_strength) > 0.25)
        r.contradictions.push_back("ZIDDIYAT: trend/momentum/bosim turli yo'nalishda");
    if(r.narr_trap > 0.45)
        r.contradictions.push_back("TRAP ehtimoli yuqori: " +
            std::to_string((int)(r.narr_trap*100)) + "%");
    if(r.regime_volatile > 0.35)
        r.contradictions.push_back("Kontekst NOANIQ: volatile regime");

    // ─── 5. SIGNAL SIFATI (ierarxik hukm) ───────────────────
    // 2 ta turli guruhdan tasdiq + ziddiyat minimal = STRONG
    int contra = (int)r.contradictions.size();
    if(conf_count >= 3 && contra == 0 && r.inv_score < 0.25)
        r.signal_quality = "STRONG";
    else if(conf_count >= 2 && contra <= 1 && r.inv_score < 0.45)
        r.signal_quality = "MODERATE";
    else if(conf_count >= 1 && contra <= 2)
        r.signal_quality = "WEAK";
    else
        r.signal_quality = "INVALID";

    return r;
}

void print_analysis(const MIEModel::Result& r, double price, long long /*ts*/=0){
    printf("\n");
    printf("╔══ CAPIGRAD MARKET INTELLIGENCE ENGINE v4.1 ══════════════╗\n");
    printf("║  BTCUSDT H1  |  Price: $%-8.0f                         ║\n", price);
    printf("╠══════════════════════════════════════════════════════════╣\n");

    // ═══ BOSQICH 1: BOZOR HOLATI (Market State) ═══════════════
    printf("║ [1] BOZOR HOLATI                                         ║\n");
    auto pb=[](double v,int w=12) -> std::string {
        int f=(int)(v*w);
        std::string s;
        for(int i=0;i<w;i++) s+=(i<f?"█":"░");
        return s;
    };
    printf("║  %-12s [%s] %.0f%%\n","TREND_UP:", pb(r.regime_up).c_str(),   r.regime_up*100);
    printf("║  %-12s [%s] %.0f%%\n","TREND_DOWN:",pb(r.regime_down).c_str(),r.regime_down*100);
    printf("║  %-12s [%s] %.0f%%\n","RANGING:",  pb(r.regime_range).c_str(),r.regime_range*100);
    printf("║  %-12s [%s] %.0f%%\n","VOLATILE:", pb(r.regime_volatile).c_str(),r.regime_volatile*100);
    printf("║  Kontekst skori:  [%s] %.0f%%\n", pb(r.ctx_score).c_str(), r.ctx_score*100);

    // === PRIMARY SIGNAL: Triple-Barrier + Regime Gate ===
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ ◆ PRIMARY SIGNAL — TRIPLE-BARRIER + REGIME GATE          ║\n");
    {
        auto bar12=[](double v)->std::string{
            int f=(int)(v*12); std::string s;
            for(int i=0;i<12;i++) s+=(i<f?"█":"░"); return s;
        };
        printf("║  UP:   [%s] %5.1f%%\n", bar12(r.cls_up).c_str(),   r.cls_up*100);
        printf("║  DOWN: [%s] %5.1f%%\n", bar12(r.cls_down).c_str(), r.cls_down*100);
        printf("║  FLAT: [%s] %5.1f%%\n", bar12(r.cls_flat).c_str(), r.cls_flat*100);
        printf("║  Clarity:  %.0f%%   Regime: %-14s              ║\n",
               r.cls_confidence*100, r.regime_label.c_str());
        // PATCH1: regime gate status
        if(r.regime_gate_active){
            printf("║  Gate: ACTIVE — TREND regime emas                        ║\n");
            printf("║  TREND regime emas → directional edge ishonchsiz         ║\n");
            printf("║  %s/VOLATILE: primary signal INFORMATIONAL ONLY  ║\n",
                   r.regime_label.c_str());
            printf("║  >> NO TRADE / WAIT — directional trade qilma            ║\n");
        } else {
            printf("║  Gate: OFF (TREND regime — actionable)                   ║\n");
            // primary_action bilan mos chiqish
            if(r.primary_action == "TREND-LONG"){
                printf("║  >> TREND-LONG candidate  (clarity: %.0f%%)               ║\n",
                       r.cls_confidence*100);
                printf("║  [OK] LONG signal — TREND + UP dominant                  ║\n");
            } else if(r.primary_action == "TREND-SHORT"){
                printf("║  >> TREND-SHORT candidate (clarity: %.0f%%)               ║\n",
                       r.cls_confidence*100);
                printf("║  [OK] SHORT signal — TREND + DOWN dominant               ║\n");
            } else if(r.primary_action == "NO-TRADE"){
                printf("║  >> WAIT — FLAT dominant. Directional edge yo'q.         ║\n");
            } else {
                printf("║  >> WAIT — low clarity (%.0f%% < %.0f%%)                    ║\n",
                       r.cls_confidence*100, DECISION_THRESHOLD*100);
            }
        }
    }
    // Barrier semantics (qisqa)
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ ◆ BOZOR REJIMI                                           ║\n");
    printf("║  Neural: %-16s   Gate: %-5s                 ║\n",
           r.regime_label.c_str(), r.regime_gate_active?"ON":"OFF");
    printf("║  Dynamic barrier: TREND K=%.1f H=%d | RANGE K=%.1f H=%d       ║\n",
           FE::g_barrier_params[0].up_k, FE::g_barrier_params[0].horizon,
           FE::g_barrier_params[1].up_k, FE::g_barrier_params[1].horizon);

    // ═══ BOSQICH 2: DOMINANT NARRATIVE ════════════════════════
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ [2] DOMINANT NARRATIVE: %-32s║\n", r.dominant_narrative.c_str());
    printf("║  CONTINUATION:   [%s] %.0f%%\n", pb(r.narr_continuation).c_str(),   r.narr_continuation*100);
    printf("║  REVERSAL:       [%s] %.0f%%\n", pb(r.narr_reversal).c_str(),       r.narr_reversal*100);
    printf("║  LIQUIDITY_TRAP: [%s] %.0f%%\n", pb(r.narr_trap).c_str(),           r.narr_trap*100);
    printf("║  BREAKOUT:       [%s] %.0f%%\n", pb(r.narr_breakout).c_str(),       r.narr_breakout*100);
    printf("║  MEAN_REVERSION: [%s] %.0f%%\n", pb(r.narr_mean_reversion).c_str(),r.narr_mean_reversion*100);

    // ═══ BOSQICH 3: SIGNAL GURUH SKORLARI ══════════════════════
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ [3] SIGNAL GURUH SKORLARI                                ║\n");
    printf("║  Kontekst  [%s] %.0f%%  — qaysi sahna?\n", pb(r.ctx_score).c_str(),  r.ctx_score*100);
    printf("║  Struktura [%s] %.0f%%  — qanday tuzilgan?\n", pb(r.str_score).c_str(), r.str_score*100);
    printf("║  Momentum  [%s] %.0f%%  — ichki kuch?\n", pb(r.mom_score).c_str(),  r.mom_score*100);
    printf("║  Tasdiq    [%s] %.0f%%  — signal mustahkam?\n", pb(r.conf_score).c_str(),r.conf_score*100);
    printf("║  Rad etish [%s] %.0f%%  — signal noto'g'ri?\n", pb(r.inv_score).c_str(), r.inv_score*100);

    // ═══ BOSQICH 4: TASDIQLAR ══════════════════════════════════
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ [4] TASDIQLAR (%d ta):\n", (int)r.confirmations.size());
    if(r.confirmations.empty())
        printf("║  — Tasdiq yo'q\n");
    for(auto& c : r.confirmations)
        printf("║  ✔ %s\n", c.c_str());

    // ═══ BOSQICH 5: RAD ETISHLAR ═══════════════════════════════
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ [5] RAD ETISHLAR (%d ta):\n", (int)r.contradictions.size());
    if(r.contradictions.empty())
        printf("║  — Ziddiyat yo'q\n");
    for(auto& c : r.contradictions)
        printf("║  ✗ %s\n", c.c_str());

    // ═══ BOSQICH 6: TEXNIK DETALLAR ════════════════════════════
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ [6] TEXNIK: Trend %+.2f | Exhaust %.0f%% | BreakQ %.0f%%\n",
           r.trend_strength, r.trend_exhaustion*100, r.breakout_quality*100);
    printf("║  Mom_S %+.2f | Mom_M %+.2f | Pressure %+.2f\n",
           r.momentum_short, r.momentum_medium, r.market_pressure);
    printf("║  Reversal risk: %.0f%% | Forecast 1H: %+.2f%%  2H: %+.2f%%\n",
           r.reversal_risk*100, r.next1_pct, r.next2_pct);

    // ═══ YAKUNIY HUKM ══════════════════════════════════════════
    printf("╠══════════════════════════════════════════════════════════╣\n");
    int cw=(int)(r.confidence*20);
    printf("║  Confidence: ");
    for(int i=0;i<20;i++) printf("%s",i<cw?"█":"░");
    printf(" %.0f%%\n", r.confidence*100);
    printf("║  Signal sifati: %-10s | Yo'nalish: %-10s       ║\n",
           r.signal_quality.c_str(), r.direction_label.c_str());

    // Yakuniy xulosa
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  XULOSA: ");
    if(r.signal_quality=="STRONG"){
        printf("KUCHLI SIGNAL — %s mexanizmi dominant.\n", r.dominant_narrative.c_str());
        if(r.narr_continuation>0.35)
            printf("║  Trend davom etadi. Confirmation: %d ta.              \n", (int)r.confirmations.size());
        else if(r.narr_trap>0.30)
            printf("║  Likvidlik tuzoq! Fake break ehtimoli yuqori.         \n");
        else if(r.narr_breakout>0.30)
            printf("║  Haqiqiy breakout — expansion kutilmoqda.             \n");
        else if(r.narr_reversal>0.30)
            printf("║  Qaytish boshlangan — narrative shift.                \n");
    } else if(r.signal_quality=="MODERATE"){
        printf("O'RTACHA signal — ehtiyot bilan kuzating.           \n");
        printf("║  %d tasdiq, %d ziddiyat bor.                          \n",
               (int)r.confirmations.size(), (int)r.contradictions.size());
    } else if(r.signal_quality=="WEAK"){
        printf("ZAIF signal — kutish tavsiya etiladi.               \n");
        printf("║  Ziddiyatlar ko'p yoki tasdiq yetarli emas.          \n");
    } else {
        printf("INVALID — signallar bir-biriga zid.                 \n");
        printf("║  Hech qanday pozitsiya olmaslik kerak.               \n");
    }
    printf("╚══════════════════════════════════════════════════════════╝\n");
}

// ════════════════════════════════════════════════════════════
//  O'QITISH
// ════════════════════════════════════════════════════════════
// ── BARRIER CALIBRATION ──────────────────────────────────────
// Maqsad: har regime uchun K ni shunday topish:
//   FLAT ≈ FLAT_TARGET (default 0.60) bo'lsin
// Nima uchun 60%: 
//   - 50% = tenglik, lekin FLAT haqiqatan ko'proq bo'ladi
//   - 65% = qabul qilinadi, 86% = model shortcut o'rganadi
//   - 60% = sog'lom: UP~20%, DOWN~20%, FLAT~60%
// Strategiya: binary search K ustida, har regime alohida
// Muammo bo'lsa: K_MIN/K_MAX chegaralarida qoladi
static constexpr double FLAT_TARGET    = 0.50; // PATCH5: 0.60→0.50, UP/DOWN ~25% each
static constexpr double FLAT_TOLERANCE = 0.05; // ±5% qabul qilinadi
static constexpr double K_MIN          = 0.8;  // pastki chegara
static constexpr double K_MAX          = 6.0;  // yuqori chegara

// Inverse-frequency class weights
// w_c = N_total / (3 * N_c), clipped to [0.5, 4.0]
// Nima uchun: FLAT=86% → unweighted CE → gradient 86% vaqt FLAT o'rgatadi
// Weighting: FLAT weight kichik (≈0.5), UP/DOWN weight katta (≈3-4)
// Nima uchun clip: juda katta weight → training beqarorligi
void compute_class_weights(const std::vector<Candle>& d1h, int train_end){
    int safe = TB_MAX_HORIZON + 2;
    int cnt[3]={0,0,0}, total=0;
    for(int i=60; i<train_end-safe; i++){
        int lbl = FE::compute_triple_barrier(d1h, i);
        cnt[lbl]++; total++;
    }
    if(total==0) return;
    printf("[CLASS WEIGHTS] n=%d  UP=%d(%.1f%%) DOWN=%d(%.1f%%) FLAT=%d(%.1f%%)\n",
           total, cnt[0],100.0*cnt[0]/total,
                  cnt[1],100.0*cnt[1]/total,
                  cnt[2],100.0*cnt[2]/total);
    const char* cn[]={"UP  ","DOWN","FLAT"};
    for(int c=0;c<3;c++){
        double w = cnt[c]>0 ? (double)total/(3.0*cnt[c]) : 1.0;
        w = std::max(0.5, std::min(2.0, w)); // PATCH6: [0.5,2.0] (focal loss bilan mos)
        g_cls_weights[c] = w;
        printf("  w[%s] = %.3f\n", cn[c], w);
    }
}

void calibrate_barrier_params(const std::vector<Candle>& d1h){
    if(FE::g_barrier_calibrated) return;

    int n = (int)d1h.size();
    int safe = TB_MAX_HORIZON + 2;
    int train_end = (int)(n * 0.85);

    printf("[CALIBRATE] Barrier K ni FLAT=%.0f%% ga moslashtirish...\n",
           FLAT_TARGET*100);

    for(int rgi=0; rgi<3; rgi++){
        FE::Regime reg = (FE::Regime)rgi;
        double lo = K_MIN, hi = K_MAX;
        double best_k = FE::get_barrier_params(reg).up_k;
        int best_h = FE::get_barrier_params(reg).horizon;

        // Binary search: K ni shunday topish, FLAT ≈ FLAT_TARGET
        for(int iter=0; iter<12; iter++){
            double mid_k = (lo + hi) / 2.0;
            // Temporarily set this regime's K
            FE::g_barrier_params[rgi] = {mid_k, mid_k, best_h};
            FE::g_barrier_calibrated = true; // temporarily enable

            int cnt_flat=0, cnt_total=0;
            for(int i=60; i<train_end-safe; i++){
                if((int)FE::detect_regime(d1h,i) != rgi) continue;
                int lbl = FE::compute_triple_barrier(d1h, i);
                if(lbl == TB_FLAT) cnt_flat++;
                cnt_total++;
            }

            if(cnt_total < 50){
                // Yetarli sample yo'q — base params qoladi
                FE::g_barrier_params[rgi] = FE::get_barrier_params(reg);
                break;
            }

            double flat_rate = (double)cnt_flat / cnt_total;

            if(flat_rate > FLAT_TARGET + FLAT_TOLERANCE){
                hi = mid_k; // K juda katta → pasaytir
            } else if(flat_rate < FLAT_TARGET - FLAT_TOLERANCE){
                lo = mid_k; // K juda kichik → oshir
            } else {
                best_k = mid_k; // maqsadda
                break;
            }
            best_k = mid_k;
        }

        FE::g_barrier_params[rgi] = {best_k, best_k, best_h};
        FE::g_barrier_calibrated = true;

        // Final check
        int cnt_up=0, cnt_dn=0, cnt_fl=0;
        for(int i=60; i<train_end-safe; i++){
            if((int)FE::detect_regime(d1h,i) != rgi) continue;
            int lbl = FE::compute_triple_barrier(d1h, i);
            if(lbl==TB_UP) cnt_up++;
            else if(lbl==TB_DOWN) cnt_dn++;
            else cnt_fl++;
        }
        int tot=cnt_up+cnt_dn+cnt_fl;
        if(tot>0)
            printf("  %-8s K=%.2f H=%d -> UP=%4.1f%% DN=%4.1f%% FLAT=%4.1f%% (n=%d)\n",
                   FE::regime_str(reg), best_k, best_h,
                   100.0*cnt_up/tot, 100.0*cnt_dn/tot, 100.0*cnt_fl/tot, tot);
    }
    printf("[CALIBRATE] Done.\n");
}

void train_mie(MIEModel& model,
               const std::vector<Candle>& d1h,
               const std::vector<Candle>& d15m,
               int epochs=500, double lr=0.0001){

    int n1h=(int)d1h.size();
    bool has15m=d15m.size()>200;
    std::cout<<"[TRAIN] H1:"<<n1h<<" | M15:"<<d15m.size()
             <<" | "<<epochs<<" epoch | early_stop=60\n";
    model.info();

    // Train/Val split: 85%/15%
    int train_end=(int)(n1h*0.85);
    int val_start=train_end;
    std::cout<<"[SPLIT] Train: 0.."<<train_end<<" | Val: "<<val_start<<".."<<n1h<<"\n";
    // Barrier K kalibrlash — train split bilan
    calibrate_barrier_params(d1h);

    // Class weights + regime weights — train boshida bir marta log
    compute_class_weights(d1h, train_end);
    printf("[REGIME WEIGHTS] TREND=%.2f  RANGE=%.2f  VOLATILE=%.2f\n",
           W_REGIME_TREND, W_REGIME_RANGE, W_REGIME_VOLATILE);
    printf("  final_w = class_w * regime_w  (TREND samplelari ustun)\n");

    // Class distribution + regime breakdown — epoch loopdan oldin, bir marta
    // Maqsad: label imbalance va regime dist ni ko'rish
    // Dynamic labeling: har i uchun detect_regime() chaqiriladi
    {
        // cls[split][class], rg[split][regime]
        int cls[2][3]={{0,0,0},{0,0,0}};
        int rg[2][3]= {{0,0,0},{0,0,0}};
        int safe = TB_MAX_HORIZON + 2; // boundary

        for(int i=60; i<train_end-safe; i++){
            cls[0][FE::compute_triple_barrier(d1h,i)]++;
            rg[0][(int)FE::detect_regime(d1h,i)]++;
        }
        for(int i=val_start+60; i<n1h-safe; i++){
            cls[1][FE::compute_triple_barrier(d1h,i)]++;
            rg[1][(int)FE::detect_regime(d1h,i)]++;
        }

        printf("[CLASS DIST] Dynamic regime-aware labeling\n");
        const char* sn[]={"Train","Val  "};
        for(int s=0;s<2;s++){
            int ctot=cls[s][0]+cls[s][1]+cls[s][2];
            if(ctot==0) continue;
            printf("  %s class : UP %5.1f%%(%4d) DOWN %5.1f%%(%4d) FLAT %5.1f%%(%4d) n=%d\n",
                   sn[s],
                   100.0*cls[s][0]/ctot, cls[s][0],
                   100.0*cls[s][1]/ctot, cls[s][1],
                   100.0*cls[s][2]/ctot, cls[s][2], ctot);
            int rtot=rg[s][0]+rg[s][1]+rg[s][2];
            if(rtot>0)
                printf("  %s regime: TREND %4.1f%%(%4d) RANGE %4.1f%%(%4d) VOLATILE %4.1f%%(%4d)\n",
                       sn[s],
                       100.0*rg[s][0]/rtot, rg[s][0],
                       100.0*rg[s][1]/rtot, rg[s][1],
                       100.0*rg[s][2]/rtot, rg[s][2]);
        }
        // Sog'lom chegaralar
        int tr=cls[0][0]+cls[0][1]+cls[0][2];
        if(tr>0){
            double fp=100.0*cls[0][2]/tr;
            if     (fp>75) printf("  [!] FLAT > 75%% — K juda katta, harakat kam\n");
            else if(fp<25) printf("  [!] FLAT < 25%% — K juda kichik, noise label xavfi\n");
            else           printf("  [OK] FLAT %.0f%% — sog'lom diapazon (25-75%%)\n", fp);
        }
    }

    double best_val=1e9; int no_improve=0, patience=60;

    // LR SCHEDULE: warmup + cosine annealing
    // Warmup: boshida kichik LR dan boshlash (gradient explosion oldini olish)
    //   ep 0..warmup_ep: 0.1*lr → lr (linear)
    // Cosine decay: warmup tugagach sekin pasayish
    //   ep warmup..end: lr → lr_min
    // Nima uchun?
    //   - Yangi model og'irliklari tasodifiy → boshida katta LR = beqarorlik
    //   - Warmup beradi modelga "isib olish" imkoniyati
    //   - Cosine yumshoq pasayadi → juda erta convergence yo'q
    int warmup_ep = epochs / 10;  // 10% warmup (500 epochda = 50 ep)
    double lr_max = lr;            // 0.0001 — peak LR
    double lr_min = lr * 0.04;    // 4% ga tushadi oxirida (0.000004)
    double lr_start = lr * 0.05;  // 5% dan boshlanadi (0.000005)

    printf("[LR] Warmup: %d ep (%g->%g) | Cosine: %d ep (%g->%g)\n",
           warmup_ep, lr_start, lr_max,
           epochs-warmup_ep, lr_max, lr_min);

    // M15 timestamp → index map
    auto find15m=[&](long long ts)->int{
        if(!has15m) return 0;
        int lo=0,hi=(int)d15m.size()-1;
        while(lo<hi){int mid=(lo+hi+1)/2;if(d15m[mid].timestamp<=ts)lo=mid;else hi=mid-1;}
        return lo;
    };

    for(int ep=0;ep<epochs&&g_running;ep++){
        double cur_lr;
        if(ep < warmup_ep){
            // Linear warmup: lr_start → lr_max
            double t = (double)ep / warmup_ep;
            cur_lr = lr_start + t*(lr_max - lr_start);
        } else {
            // Cosine annealing: lr_max → lr_min
            double t = (double)(ep - warmup_ep) / (epochs - warmup_ep);
            cur_lr = lr_min + 0.5*(lr_max - lr_min)*(1.0 + std::cos(M_PI*t));
        }
        double train_loss=0; int tn=0;
        model.reset();

        // Shuffle training indices
        std::vector<int> idx;
        for(int i=60;i<train_end-2;i++) idx.push_back(i);
        std::shuffle(idx.begin(),idx.end(),std::mt19937(ep*17+42));

        for(int ii=0;ii<(int)idx.size();ii++){
            int i=idx[ii];

            // Features
            std::vector<Candle> win1h;
            for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
            auto feat_h1=FE::extract_tf_structure(win1h,(int)win1h.size()-1);

            std::vector<Candle> win15m;
            if(has15m){
                int j=find15m(d1h[i].timestamp);
                for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);
            } else { win15m=win1h; }
            // HTF alignment: H1 feat M15 ga uzatiladi (feature 14 = htf_bias)
            auto feat_m15=FE::extract_tf_structure(win15m,(int)win15m.size()-1, feat_h1);

            // === TARGETS ===
            // [0-3]=regime, [4-6]=trend, [7-10]=mom, [11-12]=fc_reg, [13]=fc_cls
            std::vector<double> targets(14,0);

            // Regime target (one-hot)
            int regime=FE::compute_regime(d1h,i);
            targets[regime]=1.0;

            // ATR-based threshold
            double a=std::max(FE::atr(win1h,(int)win1h.size()-1),1e-8);
            double c=d1h[i].close;
            double thresh=std::max(0.002,std::min(0.012, a/c*0.75));

            // Multi-horizon returns
            int last_i=n1h-1;
            double ar1=(d1h[std::min(i+1,last_i)].close-c)/c;
            double ar3=(d1h[std::min(i+3,last_i)].close-c)/c;
            double ar6=(d1h[std::min(i+6,last_i)].close-c)/c;
            double ar=0.5*ar1+0.3*ar3+0.2*ar6;

            // Trend targets [4-6]
            auto cl=FE::closes(d1h);
            double e20=FE::ema(cl,i,20), e50=FE::ema(cl,i,50);
            double trend_str=clip((e20-e50)/a,1.0);
            double exhaustion=std::min(1.0,std::abs(ar)*5); // katta harakat = tugaydi
            double bq_sign = ar>thresh?1.0:ar<-thresh?-1.0:0.0;
            targets[4]=trend_str;
            targets[5]=exhaustion;
            targets[6]=(bq_sign+1.0)/2.0; // 0..1

            // Momentum targets [7-10]
            double mom_s=clip(ar1/thresh,1.0);
            double mom_m=clip(ar/thresh,1.0);
            double rev_risk=std::max(0.0,std::min(1.0,
                std::abs(trend_str)*exhaustion*0.5 +
                (std::abs(mom_s)>0.7?0.3:0.0)));
            double pressure=clip(ar1/thresh,1.0)*FE::vol_ratio(d1h,i,20);
            targets[7]=mom_s;
            targets[8]=mom_m;
            targets[9]=rev_risk;
            targets[10]=clip(pressure,1.0);

            // Forecast targets [11-12]: secondary regression (diagnostic)
            targets[11]=std::max(-3.0,std::min(3.0,ar1*100));
            double ar2=(d1h[std::min(i+2,last_i)].close-c)/c*100;
            targets[12]=std::max(-3.0,std::min(3.0,ar2));

            // Triple-barrier label [13]: PRIMARY target
            targets[13] = (i + TB_MAX_HORIZON + 2 < n1h) ?
                (double)FE::compute_triple_barrier(d1h, i) : (double)TB_FLAT;

            // PATCH-A: regime weight — backward() oldidan yangilanadi
            {
                FE::Regime rg = FE::detect_regime(d1h, i);
                g_cur_regime_w = (rg == FE::Regime::TREND)    ? W_REGIME_TREND :
                                 (rg == FE::Regime::RANGE)    ? W_REGIME_RANGE :
                                                                 W_REGIME_VOLATILE;
            }

            // Forward + backward
            auto pred=model.forward(feat_h1,feat_m15);
            model.backward(feat_h1,feat_m15,targets,pred);

            // Loss hisoblash
            // PATCH1: PRIMARY — focal CE (class_w * regime_w * focal_factor)
            int bc=(int)std::round(targets[13]);
            bc=std::max(0,std::min(2,bc));
            double cls_p[3]={pred.cls_up,pred.cls_down,pred.cls_flat};
            double p_bc = std::max(cls_p[bc], 1e-10);
            double focal = std::pow(1.0 - p_bc, FOCAL_GAMMA);
            double cls_ce = -g_cls_weights[bc] * g_cur_regime_w * focal *
                            std::log(p_bc);

            // SECONDARY: regime CE
            double regime_loss=0;
            for(int k=0;k<4;k++){
                double p=(&pred.regime_up)[k];
                regime_loss-=targets[k]*std::log(std::max(p,1e-10));
            }
            // SECONDARY: trend + forecast reg MSE
            double trend_mse=std::pow(pred.trend_strength-targets[4],2);
            double forecast_mse=std::pow(pred.next1_pct-targets[11],2);

            // Weighted total (weights match backward)
            double total_l = 0.35*cls_ce + 0.25*regime_loss +
                             0.20*trend_mse + 0.05*forecast_mse;
            if(!std::isnan(total_l)&&!std::isinf(total_l)){
                train_loss+=total_l; tn++;
            }

            if(ii%16==0) model.update(cur_lr);
        }
        if(tn>0) train_loss/=tn;

        // Validation — PRIMARY: classification CE (triple-barrier)
        double val_loss=0; int vn=0;
        int dir_correct=0, dir_total=0;
        int cls_correct=0, cls_actionable=0;
        // TREND-gated metrics
        int trend_act=0, trend_act_c=0, trend_n_val=0;
        // PATCH-B: TREND-only CE — sel_score uchun aniqroq
        double trend_val_ce=0; int trend_vn=0;
        // Per-regime UP/DOWN pred tracking (UP prec, DOWN prec per regime)
        int rg_pred_up[3]={}, rg_pred_up_tp[3]={};
        int rg_pred_dn[3]={}, rg_pred_dn_tp[3]={};
        model.reset();
        for(int i=val_start+60;i<n1h-TB_MAX_HORIZON-2;i++){
            std::vector<Candle> win1h;
            for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
            auto fh=FE::extract_tf_structure(win1h,(int)win1h.size()-1);
            std::vector<Candle> win15m;
            if(has15m){int j=find15m(d1h[i].timestamp);for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);}
            else win15m=win1h;
            // H1 context M15 ga uzatish (train bilan mos)
            auto fm=FE::extract_tf_structure(win15m,(int)win15m.size()-1, fh);

            auto pred=model.forward(fh,fm);

            // PRIMARY val: classification CE
            int real_bc=FE::compute_triple_barrier(d1h, i);
            double cls_p[3]={pred.cls_up,pred.cls_down,pred.cls_flat};
            double cls_ce=-std::log(std::max(cls_p[real_bc],1e-10));
            val_loss+=cls_ce; vn++;

            // Decision threshold: faqat max_prob > DECISION_THRESHOLD bo'lsa actionable
            double max_p = std::max({pred.cls_up, pred.cls_down, pred.cls_flat});
            int raw_bc = (pred.cls_up>pred.cls_down && pred.cls_up>pred.cls_flat)?0:
                         (pred.cls_down>pred.cls_flat)?1:2;
            // Threshold filter: low confidence → FLAT (no-trade)
            int pred_bc = (raw_bc != TB_FLAT && max_p >= DECISION_THRESHOLD) ? raw_bc : TB_FLAT;
            if(pred_bc==real_bc) cls_correct++;

            // Overall dir/coverage (eski)
            if(pred_bc!=TB_FLAT && real_bc!=TB_FLAT){
                if(pred_bc==real_bc) dir_correct++;
                dir_total++;
                cls_actionable++;
            }
            // PATCH3: TREND-gated metrics — real regime (deterministik, neural pred emas)
            // Sabab: pred.regime_gate_active neural modeldagi xatoni o'tkazib yuborishi mumkin
            bool real_is_trend = (FE::detect_regime(d1h, i) == FE::Regime::TREND);
            if(real_is_trend){
                trend_n_val++;
                trend_val_ce += -std::log(std::max(cls_p[real_bc], 1e-10));
                trend_vn++;
                if(pred_bc!=TB_FLAT){
                    trend_act++;
                    if(pred_bc==real_bc) trend_act_c++;
                }
            }
        }
        if(vn>0) val_loss/=vn;
        double dir_acc = dir_total>0 ? 100.0*dir_correct/dir_total : -1.0;
        double cls_acc = vn>0 ? 100.0*cls_correct/vn : 0;
        double coverage = vn>0 ? 100.0*cls_actionable/vn : 0;
        double trend_ap   = trend_act>0 ? (double)trend_act_c/trend_act : 0.0;
        double tcov_pct   = trend_n_val>0 ? 100.0*trend_act/trend_n_val : 0.0;
        double trend_ce   = trend_vn>0 ? trend_val_ce/trend_vn : val_loss;

        // PATCH2: sel_score — coverage-weighted AP bonus
        // trend_ap bonus faqat tcov>=15% bo'lganda to'liq kiradi
        // tcov=0% → bonus=0 → FLAT-collapse "yaxshi sel" olmaydi
        // tcov=15% → full bonus → coverage+precision ikkalasi kelganda reward
        double cov_factor = std::min(tcov_pct / 15.0, 1.0); // [0,1], ramps up at 15%
        double sel_score  = trend_ce - ALPHA_TREND * trend_ap * cov_factor;

        bool improved = sel_score < best_val;
        if(improved){best_val=sel_score; no_improve=0; model.save("mie_best.bin");}
        else no_improve++;

        if(ep%5==0||ep==epochs-1||improved){
            const char* phase = ep < warmup_ep ? "W" : "C";
            const char* cov_warn = (coverage < 5.0) ? " [COV!]" : "";
            // dir NA guard + tcov qo'shildi
            const char* tcov_warn = (tcov_pct < 5.0 && trend_n_val > 0) ? " [TCOV!]" : "";
            if(dir_total > 0)
                printf("  ep%4d/%d [%s lr=%.6f] trn:%.4f vce:%.4f sel:%.4f"
                       " cls:%.1f%% dir:%.1f%% cov:%.0f%%%s tap:%.0f%% tcov:%.0f%%%s [%d/%d]%s\n",
                       ep+1, epochs, phase, cur_lr,
                       train_loss, val_loss, sel_score,
                       cls_acc, dir_acc, coverage, cov_warn,
                       trend_ap*100, tcov_pct, tcov_warn,
                       no_improve, patience, improved?" ★":"");
            else
                printf("  ep%4d/%d [%s lr=%.6f] trn:%.4f vce:%.4f sel:%.4f"
                       " cls:%.1f%% dir:N/A cov:%.0f%%%s tap:%.0f%% tcov:%.0f%%%s [%d/%d]%s\n",
                       ep+1, epochs, phase, cur_lr,
                       train_loss, val_loss, sel_score,
                       cls_acc, coverage, cov_warn,
                       trend_ap*100, tcov_pct, tcov_warn,
                       no_improve, patience, improved?" ★":"");
        }

        if(no_improve>=patience){
            printf("[EARLY STOP] ep=%d  Best sel=%.4f (trend_ce - %.2f*tap)\n",
                   ep+1, best_val, ALPHA_TREND);
            break;
        }
    }
    model.save("mie_model.bin");
    printf("[TRAIN] Done. Best sel=%.4f (trend_ce - %.2f*tap)\n",
           best_val, ALPHA_TREND);
}

// ════════════════════════════════════════════════════════════
//  EVALUATION — tahlil sifatini o'lchash
// ════════════════════════════════════════════════════════════
void eval_mie(MIEModel& model, const std::vector<Candle>& d1h,
             const std::vector<Candle>& d15m){
    int n=(int)d1h.size();
    int test_start=(int)(n*0.80);
    bool has15m=d15m.size()>200;
    std::cout<<"\n[EVAL] Test set: "<<test_start<<".."<<n<<" ("<<n-test_start<<" sham)";
    std::cout<<(has15m?" [H1+M15 REAL]":" [H1 only]")<<"\n";

    auto find15m=[&](long long ts)->int{
        int lo=0,hi=(int)d15m.size()-1;
        while(lo<hi){int mid=(lo+hi+1)/2;if(d15m[mid].timestamp<=ts)lo=mid;else hi=mid-1;}
        return lo;
    };

    int dir_correct=0,dir_total=0;
    int regime_correct=0,regime_total=0;
    double forecast_mae=0; int fn=0;
    std::vector<double> conf_bins(10,0),conf_correct(10,0);

    // Trend head metriklar
    double trend_mae=0, mom_mae=0, rev_correct=0;
    int trend_n=0, mom_n=0, rev_n=0;
    int trend_dir_correct=0;

    // Triple-barrier classification metriklar
    int confusion[3][3]={};
    int total_cls=0;
    int actionable=0, actionable_correct=0;
    // Per-regime tracking
    int rg_cls[3][3]={}, rg_act[3]={}, rg_act_c[3]={};
    // PATCH-C: per-regime UP/DOWN prediction tracking
    int rg_pred_up[3]={}, rg_pred_up_tp[3]={};
    int rg_pred_dn[3]={}, rg_pred_dn_tp[3]={};

    model.reset();
    for(int i=test_start+60;i<n-TB_MAX_HORIZON-2;i++){
        std::vector<Candle> win1h;
        for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
        auto fh=FE::extract_tf_structure(win1h,(int)win1h.size()-1);

        // FIX4: Real M15 ishlatish (train bilan mos)
        std::vector<Candle> win15m;
        if(has15m){
            int j=find15m(d1h[i].timestamp);
            for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);
        } else win15m=win1h;
        // FIX5: H1 feat ni M15 ga uzatish (train bilan mos)
        auto fm=FE::extract_tf_structure(win15m,(int)win15m.size()-1, fh);

        auto pred=model.forward(fh,fm);

        double ar1=(d1h[std::min(i+1,n-1)].close-d1h[i].close)/d1h[i].close*100;

        // FIX1: Noise filter — kichik harakatlar dir_acc dan chiqariladi
        // |ar1| < 0.15% → noise, baholanmaydi
        // |ar1| >= 0.15% → aniq signal, baholanadi
        double noise_threshold = 0.15; // %
        bool pu=pred.next1_pct>0, ru=ar1>0;
        if(std::abs(ar1) >= noise_threshold){
            if(pu==ru) dir_correct++;
            dir_total++;
        }

        // Forecast MAE — secondary diagnostic
        forecast_mae+=std::abs(pred.next1_pct-ar1);
        fn++;

        // Triple-barrier classification metrics (dynamic)
        int real_bc = FE::compute_triple_barrier(d1h, i);
        // Threshold: argmax(UP=0.34,DN=0.33,FL=0.33) → UP deyish noto'g'ri
        // DECISION_THRESHOLD qo'llaniladi — training da emas, faqat eval da
        {
            double mp = std::max({pred.cls_up, pred.cls_down, pred.cls_flat});
            // raw argmax
        }
        int pred_bc; {
            double mp = std::max({pred.cls_up, pred.cls_down, pred.cls_flat});
            int raw = (pred.cls_up>pred.cls_down && pred.cls_up>pred.cls_flat)?0:
                      (pred.cls_down>pred.cls_flat)?1:2;
            pred_bc = (raw != TB_FLAT && mp >= DECISION_THRESHOLD) ? raw : TB_FLAT;
        }
        // PATCH1: regime gate — non-TREND da pred_bc force FLAT (eval gated)
        int gated_bc = pred.regime_gate_active ? TB_FLAT : pred_bc;

        confusion[real_bc][pred_bc]++;   // full confusion (ungated)
        total_cls++;
        // Per-regime tracking (past only, no leakage)
        int cur_rg = (int)FE::detect_regime(d1h, i);
        rg_cls[cur_rg][real_bc]++;
        if(pred_bc!=TB_FLAT){
            actionable++;
            if(pred_bc==real_bc) actionable_correct++;
            rg_act[cur_rg]++;
            if(pred_bc==real_bc) rg_act_c[cur_rg]++;
            // PATCH-C: UP/DOWN per-regime precision tracking
            if(pred_bc == TB_UP){
                rg_pred_up[cur_rg]++;
                if(real_bc == TB_UP) rg_pred_up_tp[cur_rg]++;
            } else if(pred_bc == TB_DOWN){
                rg_pred_dn[cur_rg]++;
                if(real_bc == TB_DOWN) rg_pred_dn_tp[cur_rg]++;
            }
        }

        // Regime accuracy
        int real_regime=FE::compute_regime(d1h,i);
        double rp[4]={pred.regime_up,pred.regime_down,pred.regime_range,pred.regime_volatile};
        int pred_regime=(int)(std::max_element(rp,rp+4)-rp);
        if(pred_regime==real_regime) regime_correct++;
        regime_total++;

        // Confidence calibration (noise filter bilan)
        if(std::abs(ar1) >= noise_threshold){
            int bin=(int)(pred.confidence*10);
            bin=std::min(9,std::max(0,bin));
            conf_bins[bin]++;
            if(pu==ru) conf_correct[bin]++;
        }

        // FIX2a: Trend direction — faqat aniq harakatlar (|ar1|>0.3%)
        if(std::abs(ar1) > 0.30){
            bool trend_correct_dir=(pred.trend_strength>0)==(ar1>0);
            if(trend_correct_dir) trend_dir_correct++;
            trend_mae+=std::abs(pred.trend_strength);
            trend_n++;
        }

        // Momentum MAE
        double real_mom=ar1*10;
        mom_mae+=std::abs(pred.momentum_short - std::max(-1.0,std::min(1.0,real_mom)));
        mom_n++;

        // FIX2b: Reversal metric — TO'G'RI hisoblash
        // Real reversal: kuchli trend (5 sham) + keskin qaytish
        // Oldingi: prev3>1.5% AND ar1<-0.5% → juda kam → 96% "yo'q" desa to'g'ri
        // Yangi: prev5 trend + current reversal
        if(i>=5){
            double prev5_ret=(d1h[i].close-d1h[i-5].close)/d1h[i-5].close*100;
            double curr_ret=ar1;
            // Haqiqiy reversal: 5 shamlik kuchli trend + qarama-qarshi sham
            bool real_reversal=(prev5_ret> 2.0 && curr_ret<-0.5) ||  // up→down
                               (prev5_ret<-2.0 && curr_ret> 0.5);    // down→up
            bool pred_reversal=pred.reversal_risk > 0.45;
            // Faqat reversal bo'lgan holatlarda baholash (imbalance yo'qotish)
            if(real_reversal){
                if(pred_reversal) rev_correct++;
                rev_n++;
            }
            // Miss rate ham ko'rsatamiz (false negative)
            if(!real_reversal && pred_reversal){
                // False alarm — alohida sanash uchun
                rev_n++; // denominator oshadi, lekin rev_correct oshmaydi
            }
        }
    }

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  EVAL NATIJALARI  (Dynamic regime-aware labeling)            ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    // ── Per-class stats ──────────────────────────────────────────
    const char* cls_nm[3]={"UP  ","DOWN","FLAT"};
    double prec_arr[3], rec_arr[3], f1_arr[3];
    for(int c=0;c<3;c++){
        int tp=confusion[c][c];
        int pred_tot=0; for(int r=0;r<3;r++) pred_tot+=confusion[r][c];
        int real_tot=0; for(int p=0;p<3;p++) real_tot+=confusion[c][p];
        prec_arr[c]=pred_tot>0?(double)tp/pred_tot:0;
        rec_arr[c] =real_tot>0?(double)tp/real_tot:0;
        double pr=prec_arr[c], rc=rec_arr[c];
        f1_arr[c]=(pr+rc>0)?2*pr*rc/(pr+rc):0;
    }

    // Confusion matrix
    printf("║  [PRIMARY] TRIPLE-BARRIER CLASSIFICATION                     ║\n");
    printf("║  Confusion matrix (rows=real, cols=pred):                     ║\n");
    printf("║           pred_UP  pred_DWN  pred_FLAT  | real_total          ║\n");
    for(int r=0;r<3;r++){
        int rt=0; for(int p=0;p<3;p++) rt+=confusion[r][p];
        printf("║  real_%-4s  %5d    %5d     %5d    | %5d               ║\n",
               cls_nm[r], confusion[r][0], confusion[r][1], confusion[r][2], rt);
    }
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    // Per-class metrics
    printf("║  Class  Precision  Recall   F1      (real_n  pred_n)          ║\n");
    for(int c=0;c<3;c++){
        int pred_tot=0; for(int r=0;r<3;r++) pred_tot+=confusion[r][c];
        int real_tot=0; for(int p=0;p<3;p++) real_tot+=confusion[c][p];
        printf("║  %-4s   %6.1f%%   %6.1f%%  %.3f   (%-5d  %-5d)         ║\n",
               cls_nm[c],
               prec_arr[c]*100, rec_arr[c]*100, f1_arr[c],
               real_tot, pred_tot);
    }

    // Aggregate
    double cls_acc=total_cls>0?
        100.0*(confusion[0][0]+confusion[1][1]+confusion[2][2])/total_cls:0;
    double macro_f1=(f1_arr[0]+f1_arr[1]+f1_arr[2])/3.0;
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Overall accuracy:       %5.1f%%   (n=%d)                    ║\n",
           cls_acc, total_cls);
    printf("║  Macro F1:               %.3f                                 ║\n",
           macro_f1);

    // Actionable metrics — ASOSIY METRIKA
    // actionable = model UP yoki DOWN dedi
    // actionable_correct = UP ↔ UP yoki DOWN ↔ DOWN (exact match)
    // actionable_dir_correct = yo'nalish to'g'ri (UP pred & real_UP, DOWN pred & real_DOWN)
    int act_up_tp   = confusion[TB_UP][TB_UP];
    int act_dn_tp   = confusion[TB_DOWN][TB_DOWN];
    int pred_up_tot = 0; for(int r=0;r<3;r++) pred_up_tot+=confusion[r][TB_UP];
    int pred_dn_tot = 0; for(int r=0;r<3;r++) pred_dn_tot+=confusion[r][TB_DOWN];
    int real_up_tot = 0; for(int p=0;p<3;p++) real_up_tot+=confusion[TB_UP][p];
    int real_dn_tot = 0; for(int p=0;p<3;p++) real_dn_tot+=confusion[TB_DOWN][p];
    // Actionable recall: real UP yoki DOWN bo'lganda model UP/DOWN dedi?
    int act_real_tot = real_up_tot + real_dn_tot;
    int act_recall_num = 0;
    // pred was UP or DOWN when real was UP or DOWN
    for(int p=0;p<2;p++) for(int r=0;r<2;r++) act_recall_num+=confusion[r][p];

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [ACTIONABLE METRICS]  (threshold=%.0f%%)                        ║\n",
           DECISION_THRESHOLD*100);

    // Majority baseline: always predict FLAT
    // Nima uchun: coverage=0% model bilan "hamma FLAT" model ni taqqoslash uchun
    int real_flat_tot = 0; for(int p=0;p<3;p++) real_flat_tot+=confusion[TB_FLAT][p];
    double majority_acc = total_cls>0 ? 100.0*real_flat_tot/total_cls : 0;
    printf("║  [Baseline] Always-FLAT acc: %5.1f%%  (reference)              ║\n",
           majority_acc);

    double cov_pct = total_cls>0?100.0*actionable/total_cls:0;
    printf("║  Coverage:             %5.1f%%   (%d/%d UP yoki DOWN dedi)     ║\n",
           cov_pct, actionable, total_cls);

    // Coverage warning
    if(cov_pct < 5.0)
        printf("║  [!] DIR IS NOT STATISTICALLY MEANINGFUL (cov<5%%)           ║\n");
    else if(cov_pct < 15.0)
        printf("║  [~] Low coverage (%.1f%%) — ehtiyot bilan                     ║\n", cov_pct);
    else
        printf("║  [OK] Coverage %.1f%% — yetarli sample                         ║\n", cov_pct);

    if(actionable>0){
        printf("║  Actionable precision: %5.1f%%   (to'g'ri yo'nalish)          ║\n",
               100.0*actionable_correct/actionable);
        printf("║    UP   precision:     %5.1f%%   (pred=%d)                     ║\n",
               pred_up_tot>0?100.0*act_up_tp/pred_up_tot:0, pred_up_tot);
        printf("║    DOWN precision:     %5.1f%%   (pred=%d)                     ║\n",
               pred_dn_tot>0?100.0*act_dn_tp/pred_dn_tot:0, pred_dn_tot);
    } else {
        printf("║  [!] Actionable=0 — model hech qachon UP/DOWN demadi          ║\n");
    }
    if(act_real_tot>0)
        printf("║  Actionable recall:    %5.1f%%   (real UP/DOWN uchun)          ║\n",
               100.0*act_recall_num/act_real_tot);

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [PRIMARY KPI — TREND GATED]                                 ║\n");
    {
        int tn     = rg_cls[0][0]+rg_cls[0][1]+rg_cls[0][2];
        int t_act  = rg_act[0];
        int t_actc = rg_act_c[0];
        int t_real = rg_cls[0][0]+rg_cls[0][1]; // real UP+DOWN in TREND
        double t_cov  = tn>0    ? 100.0*t_act/tn    : 0;
        double t_prec = t_act>0 ? 100.0*t_actc/t_act : 0;
        double t_rec  = t_real>0? 100.0*t_actc/t_real: 0;
        // UP/DOWN precision in TREND
        double t_up_prec = rg_pred_up[0]>0 ? 100.0*rg_pred_up_tp[0]/rg_pred_up[0] : 0;
        double t_dn_prec = rg_pred_dn[0]>0 ? 100.0*rg_pred_dn_tp[0]/rg_pred_dn[0] : 0;
        if(tn>0){
            printf("║  TREND n=%4d  cov=%5.1f%%  act_prec=%5.1f%%  recall=%5.1f%% ║\n",
                   tn, t_cov, t_prec, t_rec);
            printf("║    UP  precision: %5.1f%%  (pred_n=%d)                       ║\n",
                   t_up_prec, rg_pred_up[0]);
            printf("║    DN  precision: %5.1f%%  (pred_n=%d)                       ║\n",
                   t_dn_prec, rg_pred_dn[0]);
            if(t_act < 10)
                printf("║  [!] n=%d actionable — NOT STATISTICALLY MEANINGFUL         ║\n", t_act);
        } else {
            printf("║  TREND samples = 0 in test set                               ║\n");
        }
    }
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [REGIME BREAKDOWN — FULL]                                   ║\n");
    {
        const char* rn[3]={"TREND   ","RANGE   ","VOLATILE"};
        for(int r=0;r<3;r++){
            int rtot=rg_cls[r][0]+rg_cls[r][1]+rg_cls[r][2];
            if(rtot==0) continue;
            double cov     = 100.0*rg_act[r]/rtot;
            double prec    = rg_act[r]>0 ? 100.0*rg_act_c[r]/rg_act[r] : 0;
            double flat_pct= 100.0*rg_cls[r][2]/rtot;
            double up_prec = rg_pred_up[r]>0 ? 100.0*rg_pred_up_tp[r]/rg_pred_up[r] : 0;
            double dn_prec = rg_pred_dn[r]>0 ? 100.0*rg_pred_dn_tp[r]/rg_pred_dn[r] : 0;
            printf("║  %s n=%4d FLAT=%4.1f%% cov=%4.1f%% prec=%4.1f%%"
                   " UP=%4.1f%% DN=%4.1f%%  ║\n",
                   rn[r], rtot, flat_pct, cov, prec, up_prec, dn_prec);
        }
    }
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [SECONDARY]                                                  ║\n");
    printf("║  Dir acc (UP↔DOWN only):  %.1f%%  (n=%d)                    ║\n",
           dir_total>0?100.0*dir_correct/dir_total:0, dir_total);
    printf("║  Regime accuracy:         %.1f%%                             ║\n",
           100.0*regime_correct/regime_total);
    printf("║  Forecast MAE:            %.3f%%  (regression)               ║\n",
           forecast_mae/fn);
    printf("╠═══════════════════════════════════════════════════════╣\n");
    printf("║  CONFIDENCE CALIBRATION:\n");
    printf("║  Conf bin  | Count | Accuracy\n");
    for(int b=0;b<10;b++){
        if(conf_bins[b]>0)
            printf("║  %d0-%d0%%  | %4.0f  | %.1f%%\n",
                   b,b+1,conf_bins[b],100.0*conf_correct[b]/conf_bins[b]);
    }
    printf("╠═══════════════════════════════════════════════════════╣\n");
    printf("║  HEAD METRIKLAR:\n");
    if(trend_n>0)
        printf("║  Trend dir acc:    %.1f%%  (|ar|>0.3%%, n=%d)\n",
               100.0*trend_dir_correct/trend_n, trend_n);
    if(mom_n>0)
        printf("║  Momentum MAE:     %.3f  (n=%d)\n",
               mom_mae/mom_n, mom_n);
    if(rev_n>0)
        printf("║  Reversal prec:    %.1f%%  (real+false n=%d)\n",
               100.0*rev_correct/rev_n, rev_n);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    // PRIMARY KPI summary
    {
        int tn    = rg_cls[0][0]+rg_cls[0][1]+rg_cls[0][2];
        int t_act = rg_act[0];
        int t_c   = rg_act_c[0];
        double t_cov  = tn>0    ? 100.0*t_act/tn   : 0;
        double t_prec = t_act>0 ? 100.0*t_c/t_act  : 0;
        printf("║  PRIMARY KPI: TREND act_prec=%5.1f%%  cov=%5.1f%%  n=%d        ║\n",
               t_prec, t_cov, t_act);
        if(t_act < 10)
            printf("║  [!] Low TREND actionable count — metrics unreliable          ║\n");
        else if(t_prec > 55.0)
            printf("║  [OK] Edge detected in TREND regime                           ║\n");
        else if(t_prec > 45.0)
            printf("║  [~] Marginal edge — more training needed                     ║\n");
        else
            printf("║  [X] No edge in TREND — model needs improvement               ║\n");
    }
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

// ════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════

// ════════════════════════════════════════════════════════════════
//  LIVE MODE — Real-time signal pipeline
//  Offline model = macro context/structure
//  Live feed    = immediate pressure/confirmation
//  Fusion       = combined signal
// ════════════════════════════════════════════════════════════════

// ── 1. DATA STRUCTURES ───────────────────────────────────────────

// Order book level (price + quantity)
struct BookLevel {
    double price = 0;
    double qty   = 0;
};

// Full order book snapshot at one moment
struct OrderBookSnapshot {
    long long ts_ms  = 0;
    double best_bid  = 0;
    double best_ask  = 0;
    double bid_qty   = 0;  // top-of-book bid quantity
    double ask_qty   = 0;  // top-of-book ask quantity
    std::vector<BookLevel> bids; // top N bids (descending)
    std::vector<BookLevel> asks; // top N asks (ascending)
};

// Single trade tick from exchange
// buyer_maker=true  → seller was aggressor (market sell) → bearish
// buyer_maker=false → buyer was aggressor  (market buy)  → bullish
struct TradeTick {
    long long ts_ms    = 0;
    double price       = 0;
    double qty         = 0;
    bool is_buyer_maker = false; // true=market sell, false=market buy
};

// Computed live features from rolling buffers
struct LiveFeatureState {
    bool     valid           = false;  // enough data?
    long long ts_ms          = 0;

    // Book-derived
    double spread_bps        = 0; // (ask-bid)/mid*10000
    double top_imbalance     = 0; // (bid_qty-ask_qty)/(bid_qty+ask_qty), [-1,+1]
    double depth_imbalance_5 = 0; // top-5 levels
    double microprice_bias   = 0; // (microprice-mid)/mid, normalized

    // Trade-derived
    double signed_volume_5s  = 0; // net buy-sell qty, last 5s, ATR-normalized
    double signed_volume_15s = 0;
    double trade_delta_5s    = 0; // net notional (price*qty), last 5s
    double trade_delta_15s   = 0;

    // Mid price dynamics
    double mid_return_5s     = 0; // % return over 5s
    double mid_return_15s    = 0;
    double volatility_15s    = 0; // realized vol, 15s window

    // Composite scores [-1,+1]
    double book_pressure_score = 0; // imbalance + microprice
    double flow_pressure_score = 0; // trade delta + signed vol + return
};

// ── 2. LIVE FEATURE ENGINE ───────────────────────────────────────

class LiveFeatureEngine {
public:
    static constexpr int WINDOW_MS   = 30000; // 30 second rolling window
    static constexpr int MIN_BOOKS   = 3;     // warm-up: need at least N snapshots
    static constexpr int MIN_TRADES  = 5;     // warm-up: need at least N trades

    std::deque<OrderBookSnapshot> book_buf;
    std::deque<TradeTick>         trade_buf;
    mutable std::mutex            mu;

    void on_book(const OrderBookSnapshot& s) {
        std::lock_guard<std::mutex> lk(mu);
        book_buf.push_back(s);
        // Prune old snapshots
        long long cutoff = s.ts_ms - WINDOW_MS;
        while (!book_buf.empty() && book_buf.front().ts_ms < cutoff)
            book_buf.pop_front();
    }

    void on_trade(const TradeTick& t) {
        std::lock_guard<std::mutex> lk(mu);
        trade_buf.push_back(t);
        long long cutoff = t.ts_ms - WINDOW_MS;
        while (!trade_buf.empty() && trade_buf.front().ts_ms < cutoff)
            trade_buf.pop_front();
    }

    // Build current LiveFeatureState from buffers
    LiveFeatureState build() const {
        std::lock_guard<std::mutex> lk(mu);
        LiveFeatureState fs;
        if ((int)book_buf.size() < MIN_BOOKS) return fs; // not warmed up

        const auto& bk = book_buf.back();
        fs.ts_ms = bk.ts_ms;

        double mid  = (bk.best_bid + bk.best_ask) / 2.0;
        if (mid < 1e-8) return fs;

        // ── A. Spread ──
        fs.spread_bps = (bk.best_ask - bk.best_bid) / mid * 10000.0;

        // ── B. Top-of-book imbalance ──
        double bq = bk.bid_qty, aq = bk.ask_qty;
        fs.top_imbalance = (bq - aq) / (bq + aq + 1e-9); // [-1,+1]

        // ── C. Depth imbalance (top 5 levels) ──
        double sum_bid5 = 0, sum_ask5 = 0;
        for (int k = 0; k < (int)bk.bids.size() && k < 5; k++) sum_bid5 += bk.bids[k].qty;
        for (int k = 0; k < (int)bk.asks.size() && k < 5; k++) sum_ask5 += bk.asks[k].qty;
        fs.depth_imbalance_5 = (sum_bid5 - sum_ask5) / (sum_bid5 + sum_ask5 + 1e-9);

        // ── D. Microprice bias ──
        // Microprice = volume-weighted mid. Positive bias = bid side heavier.
        double microprice = (bk.best_ask * bq + bk.best_bid * aq) / (bq + aq + 1e-9);
        fs.microprice_bias = (microprice - mid) / (mid + 1e-9) * 200.0; // amplify to [-1,+1] range

        // ── E. Mid return from rolling book buffer ──
        long long now_ms  = bk.ts_ms;
        auto mid_at = [&](long long cutoff_ms) -> double {
            // find oldest snapshot within window
            for (int k = 0; k < (int)book_buf.size(); k++) {
                if (now_ms - book_buf[k].ts_ms <= cutoff_ms) {
                    double m = (book_buf[k].best_bid + book_buf[k].best_ask) / 2.0;
                    return m > 1e-8 ? m : mid;
                }
            }
            return mid;
        };
        double mid_5s  = mid_at(5000);
        double mid_15s = mid_at(15000);
        fs.mid_return_5s  = (mid - mid_5s)  / (mid_5s  + 1e-9) * 100.0; // % return
        fs.mid_return_15s = (mid - mid_15s) / (mid_15s + 1e-9) * 100.0;

        // ── F. Volatility (15s realized) ──
        // Use mid prices from book buffer over 15s
        {
            std::vector<double> mids;
            for (auto& b : book_buf) {
                if (now_ms - b.ts_ms <= 15000)
                    mids.push_back((b.best_bid + b.best_ask) / 2.0);
            }
            if (mids.size() >= 3) {
                double sum = 0, sum2 = 0;
                for (int k = 1; k < (int)mids.size(); k++) {
                    double r = (mids[k] - mids[k-1]) / (mids[k-1] + 1e-9) * 100.0;
                    sum  += r;
                    sum2 += r * r;
                }
                int n = (int)mids.size() - 1;
                double mean = sum / n;
                double var  = sum2 / n - mean * mean;
                fs.volatility_15s = std::sqrt(std::max(var, 0.0));
            }
        }

        // ── G. Trade-derived: signed volume & delta ──
        if ((int)trade_buf.size() >= MIN_TRADES) {
            double sv5=0, sv15=0, nd5=0, nd15=0;
            for (auto& t : trade_buf) {
                long long age = now_ms - t.ts_ms;
                // buyer_maker=false → market buy → +qty; buyer_maker=true → market sell → -qty
                double sign = t.is_buyer_maker ? -1.0 : +1.0;
                double notional = t.price * t.qty;
                if (age <= 5000) {
                    sv5  += sign * t.qty;
                    nd5  += sign * notional;
                }
                if (age <= 15000) {
                    sv15 += sign * t.qty;
                    nd15 += sign * notional;
                }
            }
            // Normalize by total volume traded
            double tot_vol5 = 0, tot_vol15 = 0;
            for (auto& t : trade_buf) {
                long long age = now_ms - t.ts_ms;
                if (age <= 5000)  tot_vol5  += t.qty;
                if (age <= 15000) tot_vol15 += t.qty;
            }
            fs.signed_volume_5s  = tot_vol5  > 1e-9 ? sv5  / tot_vol5  : 0; // [-1,+1]
            fs.signed_volume_15s = tot_vol15 > 1e-9 ? sv15 / tot_vol15 : 0;
            // Trade delta: normalized notional imbalance
            double tot_nd5 = 0, tot_nd15 = 0;
            for (auto& t : trade_buf) {
                long long age = now_ms - t.ts_ms;
                if (age <= 5000)  tot_nd5  += t.price * t.qty;
                if (age <= 15000) tot_nd15 += t.price * t.qty;
            }
            fs.trade_delta_5s  = tot_nd5  > 1e-9 ? nd5  / tot_nd5  : 0; // [-1,+1]
            fs.trade_delta_15s = tot_nd15 > 1e-9 ? nd15 / tot_nd15 : 0;
        }

        // ── H. Composite scores ──
        // book_pressure: order book structure → buy/sell imbalance
        fs.book_pressure_score = std::max(-1.0, std::min(1.0,
            0.45 * fs.top_imbalance +
            0.35 * fs.depth_imbalance_5 +
            0.20 * std::max(-1.0, std::min(1.0, fs.microprice_bias))
        ));

        // flow_pressure: aggressive trade flow → who is hitting the market
        double ret5_norm = std::max(-1.0, std::min(1.0, fs.mid_return_5s * 20.0)); // scale 0.05% → 1.0
        fs.flow_pressure_score = std::max(-1.0, std::min(1.0,
            0.35 * fs.trade_delta_5s +
            0.30 * fs.signed_volume_5s +
            0.20 * fs.trade_delta_15s +
            0.15 * ret5_norm
        ));

        fs.valid = true;
        return fs;
    }

    bool is_warmed_up() const {
        std::lock_guard<std::mutex> lk(mu);
        return (int)book_buf.size() >= MIN_BOOKS &&
               (int)trade_buf.size() >= MIN_TRADES;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(mu);
        book_buf.clear();
        trade_buf.clear();
    }
};

// ── 3. OFFLINE CONTEXT EXTRACTOR ─────────────────────────────────

// Extracts offline model inference from latest closed candles.
// Returns MIEModel::Result with build_analysis() applied.
// Also computes a scalar offline_bias in [-1,+1].
struct OfflineContext {
    MIEModel::Result result;
    double           offline_bias = 0; // [-1,+1] scalar
    bool             valid        = false;
};

OfflineContext run_offline_context(MIEModel& model,
                                   const std::vector<Candle>& d1h,
                                   const std::vector<Candle>& d15m)
{
    OfflineContext ctx;
    if (d1h.size() < 60) return ctx;

    int i = (int)d1h.size() - 1;
    std::vector<Candle> win1h;
    for (int k = std::max(0, i-99); k <= i; k++) win1h.push_back(d1h[k]);
    auto fh = FE::extract_tf_structure(win1h, (int)win1h.size()-1);

    std::vector<Candle> win15m;
    if (!d15m.empty()) {
        int j = (int)d15m.size() - 1;
        for (int k = std::max(0, j-99); k <= j; k++) win15m.push_back(d15m[k]);
    } else {
        win15m = win1h;
    }
    auto fm = FE::extract_tf_structure(win15m, (int)win15m.size()-1, fh);

    model.reset();
    auto raw = model.forward(fh, fm);
    ctx.result = build_analysis(raw);

    // Offline bias scalar
    // trend_strength: EMA-based directional strength
    // market_pressure: signed momentum pressure
    // regime_up/down: neural regime probabilities
    // next1_pct: regression forecast, clipped to ±3
    double ob =
        0.35 * ctx.result.trend_strength +
        0.25 * ctx.result.market_pressure +
        0.20 * (ctx.result.regime_up - ctx.result.regime_down) +
        0.20 * std::max(-1.0, std::min(1.0, ctx.result.next1_pct / 3.0));

    ob = std::max(-1.0, std::min(1.0, ob));

    // Weaken if: range regime dominant or INVALID signal quality
    if (ctx.result.regime_range > 0.55 || ctx.result.signal_quality == "INVALID")
        ob *= 0.30; // strong suppression: range context
    else if (ctx.result.regime_range > 0.35 || ctx.result.signal_quality == "WEAK")
        ob *= 0.60; // moderate suppression

    ctx.offline_bias = ob;
    ctx.valid = true;
    return ctx;
}

// ── 4. LIVE DECISION (output struct) ─────────────────────────────

struct LiveDecision {
    std::string signal      = "WAIT"; // BUY / SELL / WAIT
    double score_0_100      = 0;
    std::string reason;
    std::string invalidation;
    double offline_bias     = 0;
    double live_bias        = 0;
    double fused_bias       = 0;
    // Extra diagnostics
    double spread_bps       = 0;
    double top_imbalance    = 0;
    double trade_delta_5s   = 0;
    double volatility_15s   = 0;
};

// ── 5. FUSION ENGINE ─────────────────────────────────────────────

// Gate thresholds
static constexpr double SPREAD_GATE_BPS    = 8.0;  // > 8bps → spread too wide
static constexpr double VOLATILITY_GATE    = 0.08; // > 0.08% realized vol/tick → too noisy
static constexpr double SIGNAL_THRESHOLD   = 0.18; // fused_bias > this → BUY/SELL
static constexpr double HIGH_CONF_THRESHOLD= 0.32; // stronger signal threshold

LiveDecision fuse_signal(double offline_bias,
                          const LiveFeatureState& lf,
                          const MIEModel::Result& offline_full)
{
    LiveDecision ld;
    ld.offline_bias    = offline_bias;
    ld.spread_bps      = lf.spread_bps;
    ld.top_imbalance   = lf.top_imbalance;
    ld.trade_delta_5s  = lf.trade_delta_5s;
    ld.volatility_15s  = lf.volatility_15s;

    if (!lf.valid) {
        ld.signal      = "WAIT";
        ld.reason      = "live feed not ready";
        ld.invalidation= "warming up";
        return ld;
    }

    // ── A. Live bias ──
    double ret5_norm = std::max(-1.0, std::min(1.0, lf.mid_return_5s * 20.0));
    double live_bias =
        0.30 * lf.top_imbalance +
        0.25 * lf.depth_imbalance_5 +
        0.20 * lf.flow_pressure_score +
        0.15 * std::max(-1.0, std::min(1.0, lf.microprice_bias)) +
        0.10 * ret5_norm;
    live_bias = std::max(-1.0, std::min(1.0, live_bias));
    ld.live_bias = live_bias;

    // ── B. Gating checks ──
    std::vector<std::string> invalidations;

    if (lf.spread_bps > SPREAD_GATE_BPS)
        invalidations.push_back("spread wide (" + std::to_string((int)lf.spread_bps) + "bps)");

    if (lf.volatility_15s > VOLATILITY_GATE)
        invalidations.push_back("high volatility");

    if (offline_full.regime_gate_active)
        invalidations.push_back("non-trend regime");

    // ── C. Agreement logic ──
    bool same_sign = (offline_bias >= 0) == (live_bias >= 0);
    double fused;
    if (same_sign) {
        // Both agree → stronger signal
        fused = 0.55 * offline_bias + 0.45 * live_bias;
    } else {
        // Disagreement → offline wins but dampened
        fused = 0.60 * offline_bias + 0.40 * live_bias;
        // Additional penalty for contradiction
        fused *= 0.65;
        invalidations.push_back("flow disagrees with context");
    }

    // Spread/vol gate: attenuate fused
    if (lf.spread_bps > SPREAD_GATE_BPS)
        fused *= 0.20; // near zero — wait
    else if (lf.spread_bps > SPREAD_GATE_BPS * 0.5)
        fused *= 0.70; // moderate attenuation

    if (lf.volatility_15s > VOLATILITY_GATE)
        fused *= 0.50;

    // Non-trend regime: suppress directional
    if (offline_full.regime_gate_active)
        fused *= 0.40;

    fused = std::max(-1.0, std::min(1.0, fused));
    ld.fused_bias = fused;

    // ── D. Signal decision ──
    double abs_fused = std::abs(fused);
    if (!invalidations.empty() && abs_fused < HIGH_CONF_THRESHOLD) {
        ld.signal = "WAIT";
    } else if (fused > SIGNAL_THRESHOLD) {
        ld.signal = abs_fused > HIGH_CONF_THRESHOLD ? "BUY" : "BUY WATCH";
    } else if (fused < -SIGNAL_THRESHOLD) {
        ld.signal = abs_fused > HIGH_CONF_THRESHOLD ? "SELL" : "SELL WATCH";
    } else {
        ld.signal = "WAIT";
    }

    // ── E. Score 0-100 ──
    // Based on |fused| + agreement bonus
    double raw_score = abs_fused * 80.0;
    if (same_sign) raw_score += 20.0 * std::min(abs_fused * 3.0, 1.0);
    ld.score_0_100 = std::max(0.0, std::min(100.0, raw_score));

    // ── F. Reason string ──
    std::string reason;
    if      (offline_bias >  0.15) reason += "offline bullish";
    else if (offline_bias < -0.15) reason += "offline bearish";
    else                           reason += "offline neutral";

    if      (lf.top_imbalance >  0.15) reason += " + bid pressure";
    else if (lf.top_imbalance < -0.15) reason += " + ask pressure";

    if      (lf.trade_delta_5s >  0.15) reason += " + buy flow";
    else if (lf.trade_delta_5s < -0.15) reason += " + sell flow";

    if      (lf.mid_return_5s >  0.02) reason += " + price rising";
    else if (lf.mid_return_5s < -0.02) reason += " + price falling";

    ld.reason = reason.empty() ? "mixed signals" : reason;

    // ── G. Invalidation ──
    ld.invalidation = invalidations.empty() ? "none" : invalidations[0];
    for (int k = 1; k < (int)invalidations.size(); k++)
        ld.invalidation += ", " + invalidations[k];

    return ld;
}

// ── 6. DISPLAY ────────────────────────────────────────────────────

void print_live_decision(const LiveDecision& ld) {
    printf("\033[2J\033[H"); // clear terminal
    printf("╔══ CAPIGRAD LIVE SIGNAL ══════════════════════════════╗\n");
    printf("║  %-52s║\n", ld.signal.c_str());
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  offline_bias  : %+.3f                               ║\n", ld.offline_bias);
    printf("║  live_bias     : %+.3f                               ║\n", ld.live_bias);
    printf("║  fused_bias    : %+.3f                               ║\n", ld.fused_bias);
    printf("║  score         : %.0f/100                              ║\n", ld.score_0_100);
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  spread_bps    : %.1f                                 ║\n", ld.spread_bps);
    printf("║  top_imbalance : %+.3f                               ║\n", ld.top_imbalance);
    printf("║  delta_5s      : %+.3f                               ║\n", ld.trade_delta_5s);
    printf("║  vol_15s       : %.4f%%                              ║\n", ld.volatility_15s);
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  reason     : %-38s║\n", ld.reason.c_str());
    printf("║  invalidate : %-38s║\n", ld.invalidation.c_str());
    printf("╚══════════════════════════════════════════════════════╝\n");
    fflush(stdout);
}

// ════════════════════════════════════════════════════════════════
//  WEBSOCKET / FEED ADAPTER LAYER
//  Ikki qatlamli dizayn:
//  A) ILiveFeed — abstract interface (har qanday source uchun)
//  B) MockFeed  — CSV/synthetic data (test va offline sim uchun)
//  C) BinanceFeed — real WS adapter (USE_WEBSOCKET defined bo'lsa)
// ════════════════════════════════════════════════════════════════

// Callback types
using BookCallback  = std::function<void(const OrderBookSnapshot&)>;
using TradeCallback = std::function<void(const TradeTick&)>;

// Abstract feed interface
class ILiveFeed {
public:
    virtual ~ILiveFeed() = default;
    virtual void set_book_callback(BookCallback cb)  = 0;
    virtual void set_trade_callback(TradeCallback cb) = 0;
    virtual void start() = 0;  // begin streaming (blocking or in background thread)
    virtual void stop()  = 0;
};

// ── MOCK FEED ────────────────────────────────────────────────────
// Simulates live feed from last candles in CSV data.
// Injects synthetic ticks based on OHLCV to test the full pipeline.
class MockFeed : public ILiveFeed {
public:
    std::vector<Candle> candles; // source candles
    double tick_interval_ms = 500; // how often to generate a tick
    bool running = false;
    std::thread feed_thread;
    BookCallback  book_cb;
    TradeCallback trade_cb;

    void set_book_callback(BookCallback cb)  override { book_cb  = cb; }
    void set_trade_callback(TradeCallback cb) override { trade_cb = cb; }

    void start() override {
        if (candles.empty()) {
            printf("[MockFeed] No candles — feed empty\n");
            return;
        }
        running = true;
        feed_thread = std::thread([this]() { run(); });
    }

    void stop() override {
        running = false;
        if (feed_thread.joinable()) feed_thread.join();
    }

    ~MockFeed() { stop(); }

private:
    void run() {
        // Simulate ticks from last candle repeatedly (live sim)
        int idx = (int)candles.size() - 1;
        int tick = 0;
        while (running && g_running) {
            const Candle& c = candles[std::max(0, idx)];

            // Synthetic mid based on OHLC position
            double t_frac = (tick % 20) / 20.0; // position within candle cycle
            double mid = c.low + (c.high - c.low) * (0.5 + 0.4 * std::sin(t_frac * 2 * M_PI));
            double spread = mid * 0.0003; // 3bps synthetic spread
            double best_bid = mid - spread / 2.0;
            double best_ask = mid + spread / 2.0;

            // Synthetic order sizes: based on candle direction
            double body = c.close - c.open;
            double bias = (body > 0) ? 0.2 : -0.2; // bullish/bearish candle bias
            double bid_qty = 1.0 + bias + 0.3 * std::sin(tick * 0.7);
            double ask_qty = 1.0 - bias + 0.3 * std::cos(tick * 0.7);
            bid_qty = std::max(0.1, bid_qty);
            ask_qty = std::max(0.1, ask_qty);

            long long now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            if (book_cb) {
                OrderBookSnapshot snap;
                snap.ts_ms    = now_ms;
                snap.best_bid = best_bid;
                snap.best_ask = best_ask;
                snap.bid_qty  = bid_qty;
                snap.ask_qty  = ask_qty;
                // Synthetic depth: 5 levels
                for (int k = 0; k < 5; k++) {
                    snap.bids.push_back({best_bid - k * spread, bid_qty * (1.0 - k*0.15)});
                    snap.asks.push_back({best_ask + k * spread, ask_qty * (1.0 - k*0.15)});
                }
                book_cb(snap);
            }

            if (trade_cb) {
                // Simulate a trade tick
                TradeTick t;
                t.ts_ms = now_ms;
                t.price = mid + (std::rand() % 3 - 1) * spread * 0.5;
                t.qty   = 0.01 + (std::rand() % 100) * 0.001;
                // buyer_maker: biased by candle direction
                t.is_buyer_maker = (body < 0) ? (std::rand() % 3 != 0) : (std::rand() % 3 == 0);
                trade_cb(t);
            }

            tick++;
            std::this_thread::sleep_for(
                std::chrono::milliseconds((int)tick_interval_ms));
        }
    }
};

// ── BINANCE WEBSOCKET FEED (optional) ────────────────────────────
// Real Binance depth + aggTrades WebSocket client.
// Uses raw POSIX sockets + manual HTTP upgrade + JSON parsing.
// Enable with: -DUSE_WEBSOCKET at compile time.
#ifdef USE_WEBSOCKET

// Minimal HTTP WebSocket handshake + frame reader
// Production code would use a real WS library (libwebsockets, Boost.Beast, etc.)
class BinanceFeed : public ILiveFeed {
public:
    std::string symbol;    // e.g. "btcusdt"
    bool running = false;
    std::thread depth_thread, trade_thread;
    BookCallback  book_cb;
    TradeCallback trade_cb;

    explicit BinanceFeed(std::string sym) : symbol(std::move(sym)) {}

    void set_book_callback(BookCallback cb)  override { book_cb  = cb; }
    void set_trade_callback(TradeCallback cb) override { trade_cb = cb; }

    void start() override {
        running = true;
        depth_thread = std::thread([this]{ stream_depth(); });
        trade_thread = std::thread([this]{ stream_trades(); });
    }

    void stop() override {
        running = false;
        if (depth_thread.joinable()) depth_thread.join();
        if (trade_thread.joinable()) trade_thread.join();
    }

    ~BinanceFeed() { stop(); }

private:
    // Connect raw TCP to stream.binance.com:9443, upgrade to WS
    int connect_ws(const char* host, int port, const char* path) {
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        char port_str[16]; snprintf(port_str, sizeof(port_str), "%d", port);
        if (getaddrinfo(host, port_str, &hints, &res) != 0) return -1;
        int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0) { freeaddrinfo(res); return -1; }
        if (connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
            close(fd); freeaddrinfo(res); return -1;
        }
        freeaddrinfo(res);
        // HTTP Upgrade handshake
        char req[512];
        snprintf(req, sizeof(req),
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n\r\n", path, host);
        send(fd, req, strlen(req), 0);
        // Read response (skip headers)
        char buf[1024]; int n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(fd); return -1; }
        buf[n] = 0;
        if (!strstr(buf, "101")) { close(fd); return -1; }
        return fd;
    }

    // Read one WebSocket text frame (unmasked, server→client)
    std::string read_ws_frame(int fd) {
        uint8_t hdr[2]; if (recv(fd, hdr, 2, MSG_WAITALL) != 2) return {};
        bool fin = (hdr[0] & 0x80) != 0;
        int opcode = hdr[0] & 0x0F;
        if (opcode == 8) return {}; // close
        uint64_t payload_len = hdr[1] & 0x7F;
        if (payload_len == 126) {
            uint8_t ext[2]; recv(fd, ext, 2, MSG_WAITALL);
            payload_len = ((uint64_t)ext[0]<<8)|ext[1];
        } else if (payload_len == 127) {
            uint8_t ext[8]; recv(fd, ext, 8, MSG_WAITALL);
            payload_len = 0;
            for (int k=0;k<8;k++) payload_len=(payload_len<<8)|ext[k];
        }
        if (payload_len > 1<<20) return {}; // sanity
        std::string data(payload_len, '\0');
        uint64_t got = 0;
        while (got < payload_len) {
            int r = recv(fd, &data[got], payload_len-got, 0);
            if (r <= 0) break;
            got += r;
        }
        (void)fin;
        return data;
    }

    // Minimal JSON field extractor — no external deps
    static double jdbl(const std::string& j, const char* key) {
        std::string k = std::string("\"") + key + "\":";
        auto pos = j.find(k);
        if (pos == std::string::npos) return 0;
        pos += k.size();
        while (pos < j.size() && (j[pos]==' '||j[pos]=='"')) pos++;
        return std::stod(j.substr(pos));
    }
    static bool jbool(const std::string& j, const char* key) {
        std::string k = std::string("\"") + key + "\":";
        auto pos = j.find(k);
        if (pos == std::string::npos) return false;
        pos += k.size();
        while (pos < j.size() && j[pos]==' ') pos++;
        return j.substr(pos, 4) == "true";
    }

    void stream_depth() {
        // Binance book ticker: wss://stream.binance.com/ws/<symbol>@bookTicker
        std::string path = "/ws/" + symbol + "@bookTicker";
        int fd = connect_ws("stream.binance.com", 9443, path.c_str());
        if (fd < 0) {
            printf("[BinanceFeed] depth connect failed — check network\n");
            return;
        }
        printf("[BinanceFeed] depth stream connected: %s\n", symbol.c_str());
        while (running && g_running) {
            auto frame = read_ws_frame(fd);
            if (frame.empty()) break;
            // Parse bookTicker: {"b":"bid","B":"bid_qty","a":"ask","A":"ask_qty"}
            try {
                OrderBookSnapshot snap;
                snap.ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                snap.best_bid = jdbl(frame, "b");
                snap.best_ask = jdbl(frame, "a");
                snap.bid_qty  = jdbl(frame, "B");
                snap.ask_qty  = jdbl(frame, "A");
                if (snap.best_bid > 0 && snap.best_ask > 0 && book_cb)
                    book_cb(snap);
            } catch (...) {}
        }
        close(fd);
    }

    void stream_trades() {
        // Binance aggTrades: wss://stream.binance.com/ws/<symbol>@aggTrade
        std::string path = "/ws/" + symbol + "@aggTrade";
        int fd = connect_ws("stream.binance.com", 9443, path.c_str());
        if (fd < 0) {
            printf("[BinanceFeed] trades connect failed\n");
            return;
        }
        printf("[BinanceFeed] trade stream connected: %s\n", symbol.c_str());
        while (running && g_running) {
            auto frame = read_ws_frame(fd);
            if (frame.empty()) break;
            // Parse aggTrade: {"p":"price","q":"qty","m":buyer_maker}
            try {
                TradeTick t;
                t.ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                t.price          = jdbl(frame, "p");
                t.qty            = jdbl(frame, "q");
                t.is_buyer_maker = jbool(frame, "m");
                if (t.price > 0 && trade_cb)
                    trade_cb(t);
            } catch (...) {}
        }
        close(fd);
    }
};

#endif // USE_WEBSOCKET

// ── 7. --live MODE RUNNER ─────────────────────────────────────────

void run_live_mode(MIEModel& model,
                   const std::vector<Candle>& d1h,
                   const std::vector<Candle>& d15m,
                   bool use_mock = true)
{
    printf("\n╔══ CAPIGRAD LIVE MODE ════════════════════════════════╗\n");
    printf("║  H1+M15 offline context + real-time order flow      ║\n");
    printf("║  Signal: BUY / SELL / WAIT  (informational only)    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    if (d1h.size() < 60) {
        printf("[!] H1 data insufficient for offline context\n");
        return;
    }

    // --- Offline context (runs once, or periodically on new H1 close) ---
    printf("[LIVE] Computing offline context from %zu H1 candles...\n", d1h.size());
    OfflineContext octx = run_offline_context(model, d1h, d15m);
    if (!octx.valid) {
        printf("[!] Offline context failed\n");
        return;
    }
    printf("[LIVE] Offline bias: %.3f  regime: %s  quality: %s\n",
           octx.offline_bias,
           octx.result.regime_label.c_str(),
           octx.result.signal_quality.c_str());

    // --- Live feature engine ---
    LiveFeatureEngine lfe;

    // --- Feed selection ---
    std::unique_ptr<ILiveFeed> feed;

#ifdef USE_WEBSOCKET
    if (!use_mock) {
        auto* bf = new BinanceFeed("btcusdt");
        feed.reset(bf);
        printf("[LIVE] Using Binance WebSocket feed\n");
    } else {
#endif
        auto* mf = new MockFeed();
        mf->candles = d1h; // use last H1 candles as synthetic source
        mf->tick_interval_ms = 400; // 400ms between ticks
        feed.reset(mf);
        printf("[LIVE] Using MOCK feed (last H1 candles as synthetic source)\n");
        printf("[LIVE] To use real Binance data: compile with -DUSE_WEBSOCKET and run ./mie --live --real\n");
#ifdef USE_WEBSOCKET
    }
#endif

    // Connect feed → live engine
    feed->set_book_callback([&](const OrderBookSnapshot& s) {
        lfe.on_book(s);
    });
    feed->set_trade_callback([&](const TradeTick& t) {
        lfe.on_trade(t);
    });

    feed->start();

    // Offline context refresh interval (ms)
    const int OFFLINE_REFRESH_MS = 60000; // refresh every 60s
    auto last_offline_refresh = std::chrono::steady_clock::now();

    printf("[LIVE] Warming up live buffers...\n");
    printf("[LIVE] Press Ctrl+C to stop\n\n");

    int signal_interval_ms = 1000; // print signal every 1s

    while (g_running) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(signal_interval_ms));

        // Refresh offline context periodically
        auto now_clk = std::chrono::steady_clock::now();
        int elapsed_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(
            now_clk - last_offline_refresh).count();
        if (elapsed_ms >= OFFLINE_REFRESH_MS) {
            octx = run_offline_context(model, d1h, d15m);
            last_offline_refresh = now_clk;
        }

        if (!lfe.is_warmed_up()) {
            printf("\r[LIVE] warming up live buffers...  "
                   "books=%zu trades=%zu          ",
                   lfe.book_buf.size(), lfe.trade_buf.size());
            fflush(stdout);
            continue;
        }

        LiveFeatureState lfs = lfe.build();
        LiveDecision ld = fuse_signal(octx.offline_bias, lfs, octx.result);
        print_live_decision(ld);
    }

    feed->stop();
    printf("\n[LIVE] Stopped.\n");
}

int main(int argc,char** argv){
    std::signal(SIGINT,signal_handler);
    std::string mode=argc>1?argv[1]:"--help";

    std::cout<<"╔══════════════════════════════════════════════════════╗\n";
    std::cout<<"║   CapiGrad Market Intelligence Engine v4.0          ║\n";
    std::cout<<"║   Zanerhon's BTC Market Analyzer                    ║\n";
    std::cout<<"╠══════════════════════════════════════════════════════╣\n";
    std::cout<<"║   Maqsad: Savdo bot EMAS — Bozor analizatori        ║\n";
    std::cout<<"║   Output: Regime + Trend + Momentum + Forecast      ║\n";
    std::cout<<"╚══════════════════════════════════════════════════════╝\n";

    // Data yuklash
    std::vector<Candle> d1h, d15m;
    for(auto& fn:std::vector<std::string>{
        "btcusdt_1h_5year.csv","btcusdt_1h_365d.csv","btc_1h_1year.csv"}){
        d1h=load_csv(fn);
        if(!d1h.empty()){std::cout<<"[H1] "<<fn<<" ("<<d1h.size()<<" sham)\n";break;}
    }
    for(auto& fn:std::vector<std::string>{
        "btcusdt_15m_5year.csv","btcusdt_15m_365d.csv","btcusdt_15m.csv"}){
        d15m=load_csv(fn);
        if(!d15m.empty()){std::cout<<"[M15] "<<fn<<" ("<<d15m.size()<<" sham)\n";break;}
    }

    MIEModel model("MIE-v4");

    if(mode=="--train"){
        if(d1h.empty()){std::cout<<"[!] H1 data topilmadi\n";return 1;}
        std::cout<<"\n[MODE] O'qitish\n";
        train_mie(model,d1h,d15m,500,0.0001);
    }
    else if(mode=="--analyze"){
        std::cout<<"\n[MODE] Tahlil\n";
        if(!model.load("mie_best.bin")&&!model.load("mie_model.bin")){
            std::cout<<"[!] Model yo'q — avval --train\n"; return 1;
        }
        if(d1h.empty()){std::cout<<"[!] H1 data topilmadi\n";return 1;}

        // Oxirgi shamni tahlil qilish
        int i=(int)d1h.size()-1;
        std::vector<Candle> win1h;
        for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
        auto fh=FE::extract_tf_structure(win1h,(int)win1h.size()-1);

        std::vector<Candle> win15m;
        if(!d15m.empty()){
            int j=(int)d15m.size()-1;
            for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);
        } else win15m=win1h;
        // FIX4: H1 feat ni M15 ga uzatish (train bilan mos)
        auto fm=FE::extract_tf_structure(win15m,(int)win15m.size()-1, fh);

        model.reset();
        auto raw_result=model.forward(fh,fm);
        auto result=build_analysis(raw_result); // rule-based narrative
        print_analysis(result,d1h.back().close,d1h.back().timestamp);

        // So'nggi 5 shamni tahlil qilish
        auto find15m_a=[&](long long ts)->int{
            int lo=0,hi=(int)d15m.size()-1;
            while(lo<hi){int mid=(lo+hi+1)/2;if(d15m[mid].timestamp<=ts)lo=mid;else hi=mid-1;}
            return lo;
        };
        std::cout<<"\n[So'nggi 5 sham tahlili] (H1+M15 real)\n";
        printf("%-6s %-8s %-10s %-10s %-10s %-8s %-8s\n",
               "Offset","Price","Regime","Trend","Pressure","N1h_fc","Conf%");
        for(int off=5;off>=0;off--){
            int idx=i-off;
            if(idx<60) continue;
            std::vector<Candle> w1h;
            for(int k=std::max(0,idx-99);k<=idx;k++) w1h.push_back(d1h[k]);
            auto f=FE::extract_tf_structure(w1h,(int)w1h.size()-1);
            // FIX4+5: real M15 + H1 context uzatish
            std::vector<Candle> w15m;
            if(!d15m.empty()){
                int j=find15m_a(d1h[idx].timestamp);
                for(int k=std::max(0,j-99);k<=j;k++) w15m.push_back(d15m[k]);
            } else w15m=w1h;
            auto fm2=FE::extract_tf_structure(w15m,(int)w15m.size()-1, f);
            model.reset();
            auto r=build_analysis(model.forward(f,fm2));
            printf("-%d     $%-8.0f %-10s %+.2f       %+.2f       %+.2f%%   %.0f%%\n",
                   off,d1h[idx].close,
                   r.regime_label.c_str(),
                   r.trend_strength, r.market_pressure,
                   r.next1_pct, r.confidence*100);
        }
    }
    else if(mode=="--eval"){
        std::cout<<"\n[MODE] Baholash\n";
        if(!model.load("mie_best.bin")&&!model.load("mie_model.bin")){
            std::cout<<"[!] Model yo'q — avval --train\n"; return 1;
        }
        if(d1h.empty()){std::cout<<"[!] H1 data topilmadi\n";return 1;}
        eval_mie(model,d1h,d15m);
    }
    else if(mode=="--live"){
        std::cout<<"\n[MODE] LIVE\n";
        if(!model.load("mie_best.bin")&&!model.load("mie_model.bin")){
            std::cout<<"[!] Model yo'q — avval --train\n"; return 1;
        }
        if(d1h.empty()){std::cout<<"[!] H1 data topilmadi\n"; return 1;}
        // --live --real: Binance WS ishlatish (USE_WEBSOCKET kerak)
        bool use_mock = true;
        if(argc > 2 && std::string(argv[2]) == "--real"){
#ifdef USE_WEBSOCKET
            use_mock = false;
#else
            std::cout<<"[!] USE_WEBSOCKET enabled emas. Mock mode ishlatiladi.\n";
            std::cout<<"    Real feed uchun: g++ -DUSE_WEBSOCKET ... -o mie cgt_mie.cpp\n";
#endif
        }
        run_live_mode(model, d1h, d15m, use_mock);
    }
    else {
        std::cout<<"\nISHLATISH:\n";
        std::cout<<"  g++ -std=c++17 -O3 -o mie cgt_mie.cpp\n";
        std::cout<<"  ./mie --train           Model o'qitish\n";
        std::cout<<"  ./mie --analyze         Oxirgi shamni tahlil\n";
        std::cout<<"  ./mie --eval            Tahlil sifatini o'lchash\n";
        std::cout<<"  ./mie --live            Live signal (mock feed)\n";
        std::cout<<"  ./mie --live --real     Live signal (Binance WS, USE_WEBSOCKET kerak)\n";
    }
    return 0;
}