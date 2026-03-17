#ifndef MODEL_HPP
#define MODEL_HPP

#include "data.hpp"
#include "nn.hpp"
#include "features.hpp"
#include <vector>
#include <string>

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