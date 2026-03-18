#pragma once
#ifndef MODEL_HPP
#define MODEL_HPP

#include "data.hpp"
#include "nn.hpp"
#include "features.hpp"
#include <vector>
#include <string>
#include <cmath>

// Global class weights
extern double g_cls_weights[3];
extern double g_cur_regime_w;

struct MIEModel {
    TemporalLayer h1_branch;   
    TemporalLayer m15_branch;  

    Layer merge_dense;   

    Layer regime_head;        
    Layer trend_head;         
    Layer momentum_head;      
    Layer forecast_head;      
    Layer forecast_cls_head;  

    int t=0;
    std::string name;

    std::vector<double> last_merged, last_dense;

    MIEModel(const std::string& n="MIE");

    std::vector<double> softmax(const std::vector<double>& x);

    struct Result {
        double regime_up, regime_down, regime_range, regime_volatile;
        double trend_strength, trend_exhaustion, breakout_quality;
        double momentum_short, momentum_medium, reversal_risk, market_pressure;
        double next1_pct, next2_pct;
        double confidence;
        std::string regime_label, direction_label;

        double cls_up, cls_down, cls_flat; 
        std::string cls_label; 
        double cls_confidence; 

        double narr_continuation;   
        double narr_reversal;       
        double narr_trap;           
        double narr_breakout;       
        double narr_mean_reversion; 
        std::string dominant_narrative;

        double ctx_score;   
        double str_score;   
        double mom_score;   
        double conf_score;  
        double inv_score;   

        std::vector<std::string> confirmations;
        std::vector<std::string> contradictions;
        std::string signal_quality; 

        bool regime_gate_active; 
        std::string primary_action; 
    };

    Result forward(const std::vector<double>& feat_h1,
                   const std::vector<double>& feat_m15);

    void backward(const std::vector<double>& feat_h1,
                  const std::vector<double>& feat_m15,
                  const std::vector<double>& targets,
                  const Result& pred);

    void update(double lr);
    void reset();
    int pcount();
    void save(const std::string& fn);
    bool load(const std::string& fn);
    void info();
};

MIEModel::Result build_analysis(MIEModel::Result r);
void print_analysis(const MIEModel::Result& r, double price, long long ts=0);
void print_bar(const std::string& label, double val, double mn=-1, double mx=1, int w=20);

#endif // MODEL_HPP

// Live orderbook va Meta-labeling uchun maxsus yengil tarmoq
struct MetaLiveModel {
    TemporalLayer gru; // Vaqt ketma-ketligini tushunish uchun GRU (8 ta input)
    Layer head;        // Yakuniy qaror chiqaruvchi qatlam (3 ta output: UP, DOWN, FLAT)

    // 8 ta input (live_data features), 16 ta yashirin neyron, 3 ta output
    MetaLiveModel() : gru(8, 16), head(16, 3, SIGMOID_A) {}

    std::vector<double> softmax(const std::vector<double>& x) {
        std::vector<double> res(x.size());
        double mx = x[0], sum = 0;
        for (double v : x) mx = std::max(mx, v);
        for (int i = 0; i < x.size(); i++) {
            res[i] = std::exp(x[i] - mx);
            sum += res[i];
        }
        for (int i = 0; i < x.size(); i++) res[i] /= sum;
        return res;
    }

    std::vector<double> forward_live(const std::vector<double>& features) {
        // 1. Mikrotuzilma xususiyatlarini GRU ga beramiz
        std::vector<double> h = gru.forward(features);
        // 2. Olingan holatni (hidden state) yakuniy qatlamga beramiz
        std::vector<double> logits = head.forward(h);
        // 3. Ehtimolliklarni (0-1) qaytaramiz
        return softmax(logits);
    }

    void backward_live(const std::vector<double>& features, 
                       const std::vector<double>& targets, 
                       const std::vector<double>& pred) {
        // Cross-entropy gradienti: (pred - target)
        std::vector<double> d_out(3);
        for(int i = 0; i < 3; i++) {
            d_out[i] = pred[i] - targets[i];
        }
        
        // Orqaga tarqalish (Backprop)
        std::vector<double> dh = head.backward(d_out);
        gru.backward(dh);
    }

    void update(double lr) {
        gru.update(lr, 1);
        head.update(lr, 1);
    }

    void reset() {
        gru.reset();
    }

    void save(const std::string& fn);
    bool load(const std::string& fn);
};