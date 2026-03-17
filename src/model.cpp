#include "model.hpp"
#include "features.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>

// Global o'zgaruvchilar va konstantalar
double g_cls_weights[3] = {1.0, 1.0, 1.0};
double g_cur_regime_w = 1.0;

static constexpr double DECISION_THRESHOLD = 0.35;
static constexpr double FOCAL_GAMMA = 2.0;

MIEModel::MIEModel(const std::string& n):
    h1_branch(70,64), m15_branch(70,64),
    merge_dense(128,80, RELU_A),
    regime_head(80,4, LINEAR_A),
    trend_head(80,3, TANH_A),
    momentum_head(80,4, TANH_A),
    forecast_head(80,2, TANH_A),
    forecast_cls_head(80,3, LINEAR_A),
    name(n){}

std::vector<double> MIEModel::softmax(const std::vector<double>& x){
    double mx=*std::max_element(x.begin(),x.end());
    std::vector<double> e(x.size()); double s=0;
    for(int i=0;i<(int)x.size();i++){e[i]=std::exp(x[i]-mx);s+=e[i];}
    for(auto& v:e) v/=s;
    return e;
}

MIEModel::Result MIEModel::forward(const std::vector<double>& feat_h1,
               const std::vector<double>& feat_m15){
    auto h1_out  = h1_branch.forward(feat_h1);
    auto m15_out = m15_branch.forward(feat_m15);

    last_merged.resize(128);
    for(int i=0;i<64;i++) last_merged[i]=h1_out[i];
    for(int i=0;i<64;i++) last_merged[64+i]=m15_out[i];

    last_dense = merge_dense.forward(last_merged);

    auto regime_raw  = regime_head.forward(last_dense);
    auto regime_prob = softmax(regime_raw);
    auto trend_out   = trend_head.forward(last_dense);
    auto mom_out     = momentum_head.forward(last_dense);
    auto fc_out      = forecast_head.forward(last_dense);

    Result r;
    r.regime_up       = regime_prob[0];
    r.regime_down     = regime_prob[1];
    r.regime_range    = regime_prob[2];
    r.regime_volatile = regime_prob[3];

    r.trend_strength   = trend_out[0];
    r.trend_exhaustion = (trend_out[1]+1.0)/2.0;
    r.breakout_quality = (trend_out[2]+1.0)/2.0;

    r.momentum_short  = mom_out[0];
    r.momentum_medium = mom_out[1];
    r.reversal_risk   = (mom_out[2]+1.0)/2.0;
    r.market_pressure = mom_out[3];

    r.next1_pct = fc_out[0]*3.0;
    r.next2_pct = fc_out[1]*3.0;

    double regime_conf = *std::max_element(regime_prob.begin(),regime_prob.end());
    double regime_entropy=0;
    for(auto p:regime_prob) if(p>1e-9) regime_entropy-=p*std::log2(p);
    double max_entropy=std::log2(4.0);
    double clarity=1.0-regime_entropy/max_entropy;
    double alignment=std::abs(r.trend_strength)*0.5+std::abs(r.market_pressure)*0.3;
    alignment=std::min(1.0,alignment);
    double rev_penalty=r.reversal_risk*0.4;
    double raw_conf=0.3*regime_conf+0.4*clarity+0.3*alignment;
    raw_conf=std::max(0.0,raw_conf-rev_penalty);
    double T=1.8;
    r.confidence=1.0/(1.0+std::exp(-(raw_conf*2-1)/T*3.0));
    r.confidence=std::max(0.10,std::min(0.90,r.confidence));

    int ri=std::distance(regime_prob.begin(), std::max_element(regime_prob.begin(),regime_prob.end()));
    const char* rl[]={"TREND_UP","TREND_DOWN","RANGING","VOLATILE"};
    r.regime_label = rl[ri];

    auto cls_raw  = forecast_cls_head.forward(last_dense);
    auto cls_prob = softmax(cls_raw);
    r.cls_up   = cls_prob[TB_UP];
    r.cls_down = cls_prob[TB_DOWN];
    r.cls_flat = cls_prob[TB_FLAT];
    if(r.cls_up > r.cls_down && r.cls_up > r.cls_flat) r.cls_label = "UP";
    else if(r.cls_down > r.cls_up && r.cls_down > r.cls_flat) r.cls_label = "DOWN";
    else r.cls_label = "FLAT";
    r.cls_confidence = std::max({r.cls_up, r.cls_down, r.cls_flat});

    r.direction_label = (r.cls_label=="UP") ? "BULLISH" : (r.cls_label=="DOWN") ? "BEARISH" : "NEUTRAL";
    r.regime_gate_active = (r.regime_label != "TREND_UP" && r.regime_label != "TREND_DOWN");
    if(r.regime_gate_active){
        r.primary_action = "WAIT-GATE";
    } else if(r.cls_label == "FLAT"){
        r.primary_action = "NO-TRADE";
    } else if(r.cls_confidence < DECISION_THRESHOLD){
        r.primary_action = "WAIT-CLARITY";
    } else if(r.cls_label == "UP"){
        r.primary_action = "TREND-LONG";
    } else {
        r.primary_action = "TREND-SHORT";
    }

    return r;
}

void MIEModel::backward(const std::vector<double>& feat_h1,
              const std::vector<double>& feat_m15,
              const std::vector<double>& targets,
              const Result& pred){
    std::vector<double> g_regime(4,0), g_trend(3,0), g_mom(4,0), g_fc(2,0);
    std::vector<double> g_cls(3, 0.0);

    for(int i=0;i<4;i++){ double p=(&pred.regime_up)[i]; g_regime[i]=p-targets[i]; }

    g_trend[0] = (pred.trend_strength - targets[4])*2.0;
    g_trend[1] = ((pred.trend_exhaustion - targets[5])*2.0)/2.0;
    g_trend[2] = ((pred.breakout_quality - targets[6])*2.0)/2.0;

    g_mom[0] = (pred.momentum_short  - targets[7])*2.0;
    g_mom[1] = (pred.momentum_medium - targets[8])*2.0;
    g_mom[2] = ((pred.reversal_risk  - targets[9])*2.0)/2.0;
    g_mom[3] = (pred.market_pressure - targets[10])*2.0;

    g_fc[0] = (pred.next1_pct/3.0 - targets[11]/3.0)*2.0;
    g_fc[1] = (pred.next2_pct/3.0 - targets[12]/3.0)*2.0;

    {
        int bc = (int)std::round(targets[13]);
        bc = std::max(0, std::min(2, bc));
        double cls_p[3] = {pred.cls_up, pred.cls_down, pred.cls_flat};
        double p_bc  = std::max(cls_p[bc], 1e-7);
        double focal = std::pow(1.0 - p_bc, FOCAL_GAMMA);
        double w = g_cls_weights[bc] * g_cur_regime_w * focal;
        for(int k=0;k<3;k++) g_cls[k] = w * (cls_p[k] - (k==bc ? 1.0 : 0.0));
    }

    for(auto& x:g_cls)    x*=0.35/3;
    for(auto& x:g_regime) x*=0.25/4;
    for(auto& x:g_trend)  x*=0.20/3;
    for(auto& x:g_mom)    x*=0.15/4;
    for(auto& x:g_fc)     x*=0.05/2;

    auto nan_check=[](std::vector<double>& g){ for(auto& x:g) if(std::isnan(x)||std::isinf(x)) x=0.0; };
    nan_check(g_regime); nan_check(g_trend); nan_check(g_mom); nan_check(g_fc); nan_check(g_cls);

    std::vector<double> gd(80,0);
    auto d_regime = regime_head.backward(g_regime);
    auto d_trend  = trend_head.backward(g_trend);
    auto d_mom    = momentum_head.backward(g_mom);
    auto d_fc     = forecast_head.backward(g_fc);
    auto d_cls    = forecast_cls_head.backward(g_cls);
    for(int i=0;i<80;i++) gd[i]=d_regime[i]+d_trend[i]+d_mom[i]+d_fc[i]+d_cls[i];
    clip_by_norm(gd, 1.0);

    auto g_merged = merge_dense.backward(gd);
    clip_by_norm(g_merged, 1.0);

    std::vector<double> g_h1(64), g_m15(64);
    for(int i=0;i<64;i++) g_h1[i]=g_merged[i];
    for(int i=0;i<64;i++) g_m15[i]=g_merged[64+i];
    clip_by_norm(g_h1, 0.5); clip_by_norm(g_m15, 0.5);
    h1_branch.backward(g_h1);
    m15_branch.backward(g_m15);
}

void MIEModel::update(double lr){
    t++;
    h1_branch.update(lr,t); m15_branch.update(lr,t); merge_dense.update(lr,t);
    regime_head.update(lr,t); trend_head.update(lr,t); momentum_head.update(lr,t);
    forecast_head.update(lr,t); forecast_cls_head.update(lr,t);
}
void MIEModel::reset(){ h1_branch.reset(); m15_branch.reset(); }
int MIEModel::pcount(){
    return h1_branch.pcount()+m15_branch.pcount()+merge_dense.ns.size()*(64+1)+
           regime_head.ns.size()*(64+1)+trend_head.ns.size()*(64+1)+
           momentum_head.ns.size()*(64+1)+forecast_head.ns.size()*(64+1);
}
void MIEModel::save(const std::string& fn){
    std::ofstream f(fn,std::ios::binary); if(!f) return;
    h1_branch.save(f); m15_branch.save(f);
    auto save_layer=[&](Layer& l){for(auto& n:l.ns){for(auto& w:n.w) f.write((char*)&w,8);f.write((char*)&n.b,8);}};
    save_layer(merge_dense); save_layer(regime_head); save_layer(trend_head);
    save_layer(momentum_head); save_layer(forecast_head); save_layer(forecast_cls_head);
    std::cout<<"[Save] "<<fn<<"\n";
}
bool MIEModel::load(const std::string& fn){
    std::ifstream f(fn,std::ios::binary); if(!f) return false;
    h1_branch.load(f); m15_branch.load(f);
    auto load_layer=[&](Layer& l){for(auto& n:l.ns){for(auto& w:n.w) f.read((char*)&w,8);f.read((char*)&n.b,8);}};
    load_layer(merge_dense); load_layer(regime_head); load_layer(trend_head);
    load_layer(momentum_head); load_layer(forecast_head);
    if(f.peek()!=EOF) load_layer(forecast_cls_head);
    std::cout<<"[Load] "<<fn<<"\n"; return true;
}
void MIEModel::info(){
    printf("╔══ MIE-v4.1 ════════════════════════════════════════╗\n");
    printf("║  H1+M15: 70->64 | Merge: 128->80                  ║\n");
    printf("║  Heads: Regime+Trend+Mom+Forecast+NARRATIVE(5)     ║\n");
    printf("║  Features: 35 base + 35 directional = 70           ║\n");
    printf("║  Params: ~%d                                  ║\n", pcount());
    printf("╚══════════════════════════════════════════════════════╝\n");
}

void print_bar(const std::string& label, double val, double mn, double mx, int w){
    double pct=(val-mn)/(mx-mn); pct=std::max(0.0,std::min(1.0,pct));
    int filled=(int)(pct*w); std::cout<<"  "<<std::setw(18)<<std::left<<label<<" ";
    for(int i=0;i<w;i++) std::cout<<(i<filled?"█":"░");
    std::cout<<" "<<std::fixed<<std::setprecision(2)<<val<<"\n";
}

MIEModel::Result build_analysis(MIEModel::Result r){
    double ctx = 0.40*std::max(r.regime_up, r.regime_down) + 0.30*(1.0 - r.regime_volatile) + 0.30*std::min(1.0, std::abs(r.trend_strength));
    r.ctx_score = std::max(0.0, std::min(1.0, ctx));
    double str = 0.40*r.breakout_quality + 0.35*(1.0 - r.trend_exhaustion) + 0.25*(1.0 - r.reversal_risk);
    r.str_score = std::max(0.0, std::min(1.0, str));
    double mom = 0.40*std::abs(r.market_pressure) + 0.35*std::abs(r.momentum_short) + 0.25*std::abs(r.momentum_medium);
    r.mom_score = std::max(0.0, std::min(1.0, mom));
    
    bool trend_bull=r.trend_strength>0.20, trend_bear=r.trend_strength<-0.20;
    bool mom_bull=r.momentum_short>0.10, mom_bear=r.momentum_short<-0.10;
    bool press_bull=r.market_pressure>0.10, press_bear=r.market_pressure<-0.10;
    bool bull_aligned=trend_bull&&mom_bull&&press_bull, bear_aligned=trend_bear&&mom_bear&&press_bear;
    bool aligned=bull_aligned||bear_aligned;
    double conf_s = 0.45*(aligned?1.0:0.0) + 0.30*(1.0-r.regime_volatile) + 0.25*(1.0-r.reversal_risk*0.5);
    r.conf_score = std::max(0.0, std::min(1.0, conf_s));
    double inv = 0.40*r.reversal_risk + 0.30*r.trend_exhaustion + 0.20*r.regime_volatile + 0.10*(1.0-r.breakout_quality);
    r.inv_score = std::max(0.0, std::min(1.0, inv));

    r.narr_continuation=0; r.narr_reversal=0; r.narr_trap=0; r.narr_breakout=0; r.narr_mean_reversion=0;
    r.narr_trap = std::min(1.0, 0.40*r.reversal_risk + 0.35*r.trend_exhaustion + 0.25*(1.0-r.breakout_quality));
    r.narr_breakout = std::min(1.0, 0.45*r.breakout_quality + 0.30*(1.0-r.trend_exhaustion) + 0.25*r.str_score);
    r.narr_reversal = std::min(1.0, 0.45*r.trend_exhaustion + 0.35*r.reversal_risk + 0.20*(1.0-std::abs(r.trend_strength)));
    r.narr_continuation = std::min(1.0, 0.40*(aligned?1.0:0.0) + 0.35*std::abs(r.trend_strength) + 0.25*(1.0-r.trend_exhaustion));
    r.narr_mean_reversion = std::min(1.0, 0.50*r.regime_range + 0.30*std::abs(r.market_pressure) + 0.20*(1.0-r.str_score));

    bool primary_flat=(r.cls_label=="FLAT"), low_clarity=(r.cls_confidence<0.55), low_mom=(r.mom_score<0.20);
    if(primary_flat||low_clarity||low_mom){r.narr_breakout*=0.10; r.narr_continuation*=0.10;}
    if(r.regime_range>0.55) r.narr_breakout*=0.15;
    if(std::abs(r.trend_strength)<0.20&&r.mom_score<0.20) r.narr_continuation*=0.10;

    double narr_vals[5]={r.narr_continuation,r.narr_reversal,r.narr_trap,r.narr_breakout,r.narr_mean_reversion};
    const char* narr_names[5]={"TREND_CONTINUATION","REVERSAL","LIQUIDITY_TRAP","BREAKOUT_EXPANSION","MEAN_REVERSION"};
    r.dominant_narrative = narr_names[(int)(std::max_element(narr_vals,narr_vals+5)-narr_vals)];

    r.confirmations.clear(); r.contradictions.clear(); int conf_count=0;
    if(std::abs(r.trend_strength)>0.30){r.confirmations.push_back(std::string("Trend ")+(r.trend_strength>0?"BULL":"BEAR"));conf_count++;}
    if(r.breakout_quality>0.55){r.confirmations.push_back("Struktura breakout sifati yuqori");conf_count++;}
    if(std::abs(r.market_pressure)>0.25){r.confirmations.push_back(std::string("Bozor bosimi ")+(r.market_pressure>0?"BUY":"SELL"));conf_count++;}
    if(aligned){r.confirmations.push_back(std::string(bull_aligned?"BULL":"BEAR")+" alignment");conf_count++;}
    
    if(r.reversal_risk>0.50) r.contradictions.push_back("Reversal risk YUQORI");
    if(r.trend_exhaustion>0.60) r.contradictions.push_back("Trend CHARCHAGAN");
    if(!aligned&&std::abs(r.trend_strength)>0.25) r.contradictions.push_back("ZIDDIYAT: yo'nalish turlicha");

    int contra=(int)r.contradictions.size();
    if(conf_count>=3&&contra==0&&r.inv_score<0.25) r.signal_quality="STRONG";
    else if(conf_count>=2&&contra<=1&&r.inv_score<0.45) r.signal_quality="MODERATE";
    else if(conf_count>=1&&contra<=2) r.signal_quality="WEAK";
    else r.signal_quality="INVALID";
    return r;
}

void print_analysis(const MIEModel::Result& r, double price, long long /*ts*/){
    printf("\n╔══ CAPIGRAD MARKET INTELLIGENCE ENGINE v4.1 ══════════════╗\n");
    printf("║  Price: $%-8.0f | Signal: %-10s                 ║\n", price, r.signal_quality.c_str());
    printf("╠══════════════════════════════════════════════════════════╣\n");
    auto pb=[](double v,int w=12)->std::string{int f=(int)(v*w);std::string s;for(int i=0;i<w;i++)s+=(i<f?"█":"░");return s;};
    printf("║  UP:   [%s] %5.1f%% | Gate: %-5s                   ║\n", pb(r.cls_up).c_str(), r.cls_up*100, r.regime_gate_active?"ACTIVE":"OFF");
    printf("║  DOWN: [%s] %5.1f%% | Action: %-15s      ║\n", pb(r.cls_down).c_str(), r.cls_down*100, r.primary_action.c_str());
    printf("║  FLAT: [%s] %5.1f%% | Regime: %-15s      ║\n", pb(r.cls_flat).c_str(), r.cls_flat*100, r.regime_label.c_str());
    printf("╚══════════════════════════════════════════════════════════╝\n");
}