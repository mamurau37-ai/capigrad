#include "live.hpp"
#include "features.hpp"
#include "globals.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstring>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// LiveConfig implementations
std::string LiveConfig::ws_host() const { return ""; }
int LiveConfig::ws_port() const { return 443; }
std::string LiveConfig::combined_path() const { return ""; }

struct OfflineContext { MIEModel::Result result; double offline_bias=0; bool valid=false; };

inline OfflineContext run_offline_context(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m) {
    OfflineContext ctx;
    if (d1h.size() < 60) return ctx;
    int i = (int)d1h.size() - 1;
    std::vector<Candle> win1h;
    for (int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
    auto fh = FE::extract_tf_structure(win1h,(int)win1h.size()-1);
    std::vector<Candle> win15m;
    if (!d15m.empty()) {
        int j=(int)d15m.size()-1;
        for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);
    } else win15m = win1h;
    auto fm = FE::extract_tf_structure(win15m,(int)win15m.size()-1, fh);
    model.reset();
    auto raw = model.forward(fh, fm);
    ctx.result = build_analysis(raw);
    double ob = 0.35 * ctx.result.trend_strength + 0.25 * ctx.result.market_pressure + 0.20 * (ctx.result.regime_up - ctx.result.regime_down) + 0.20 * std::max(-1.0, std::min(1.0, ctx.result.next1_pct / 3.0));
    if (ctx.result.regime_range > 0.55 || ctx.result.signal_quality == "INVALID") ob *= 0.30;
    else if (ctx.result.regime_range > 0.35 || ctx.result.signal_quality == "WEAK")  ob *= 0.60;
    ctx.offline_bias = std::max(-1.0, std::min(1.0, ob)); ctx.valid = true;
    return ctx;
}

void LiveOrderBook::update(const std::vector<std::pair<double,double>>& bu, const std::vector<std::pair<double,double>>& au, long long ts_ms) {
    last_update_ms = ts_ms;
    for (auto& [p,q] : bu) { if(q==0) bids.erase(p); else bids[p]=q; }
    for (auto& [p,q] : au) { if(q==0) asks.erase(p); else asks[p]=q; }
    while ((int)bids.size() > max_depth) { auto it=bids.end(); --it; bids.erase(it); }
    while ((int)asks.size() > max_depth) asks.erase(asks.rbegin()->first);
}
OrderBookSnapshot LiveOrderBook::snapshot() const {
    OrderBookSnapshot s; s.ts_ms = last_update_ms;
    if (bids.empty() || asks.empty()) return s;
    s.best_bid = bids.begin()->first; s.best_ask = asks.begin()->first;
    s.bid_qty  = bids.begin()->second; s.ask_qty = asks.begin()->second;
    for (auto& [p,q] : bids) s.bids.push_back({p,q});
    for (auto& [p,q] : asks) s.asks.push_back({p,q}); return s;
}
bool LiveOrderBook::valid() const { return !bids.empty() && !asks.empty(); }

void LiveFeatureEngine::on_book(const OrderBookSnapshot& s) { std::lock_guard<std::mutex> lk(mu); book_buf.push_back(s); long long cut = s.ts_ms - BOOK_WIN_MS; while (!book_buf.empty() && book_buf.front().ts_ms < cut) book_buf.pop_front(); }
void LiveFeatureEngine::on_trade(const TradeTick& t) { std::lock_guard<std::mutex> lk(mu); trade_buf.push_back(t); long long cut = t.ts_ms - WINDOW_MS; while (!trade_buf.empty() && trade_buf.front().ts_ms < cut) trade_buf.pop_front(); _update_bars(t); }
LiveFeatureState LiveFeatureEngine::build() const {
    std::lock_guard<std::mutex> lk(mu); LiveFeatureState fs;
    if ((int)book_buf.size() < MIN_BOOKS) return fs; const auto& bk = book_buf.back();
    double mid = (bk.best_bid + bk.best_ask) / 2.0; if (mid < 1e-8) return fs;
    fs.mid = mid; fs.ts_ms = bk.ts_ms; fs.spread_bps = (bk.best_ask - bk.best_bid) / mid * 10000.0;
    double bq = bk.bid_qty, aq = bk.ask_qty; fs.top_imbalance = (bq - aq) / (bq + aq + 1e-9);
    double sb5=0,sa5=0,sb10=0,sa10=0;
    for (int k=0;k<(int)bk.bids.size();k++){ if(k<5) sb5+=bk.bids[k].qty; if(k<10) sb10+=bk.bids[k].qty; }
    for (int k=0;k<(int)bk.asks.size();k++){ if(k<5) sa5+=bk.asks[k].qty; if(k<10) sa10+=bk.asks[k].qty; }
    fs.depth_imbalance_5  = (sb5 -sa5 )/(sb5 +sa5 +1e-9); fs.depth_imbalance_10 = (sb10-sa10)/(sb10+sa10+1e-9);
    double mp = (bk.best_ask*bq + bk.best_bid*aq)/(bq+aq+1e-9); fs.microprice_bias = (mp - mid)/(mid+1e-9)*300.0;
    double dp_b=0, dp_a=0;
    for (auto& lv : bk.bids){ double d=std::max(mid-lv.price,1e-8); dp_b+=lv.qty/(1+d/mid*1000); }
    for (auto& lv : bk.asks){ double d=std::max(lv.price-mid,1e-8); dp_a+=lv.qty/(1+d/mid*1000); }
    fs.depth_pressure = (dp_b-dp_a)/(dp_b+dp_a+1e-9);
    double tot_b=0,tot_a=0,mx_b=0,mx_a=0;
    for (auto& lv : bk.bids){ tot_b+=lv.qty; mx_b=std::max(mx_b,lv.qty); }
    for (auto& lv : bk.asks){ tot_a+=lv.qty; mx_a=std::max(mx_a,lv.qty); }
    fs.wall_bid_pct = tot_b>1e-9 ? mx_b/tot_b : 0; fs.wall_ask_pct = tot_a>1e-9 ? mx_a/tot_a : 0;
    if ((int)bk.bids.size()>=4){double q0=bk.bids[0].qty,q3=bk.bids[3].qty; fs.slope_bid=std::max(-1.0,std::min(1.0,(q0-q3)/(q0+q3+1e-9)));}
    if ((int)bk.asks.size()>=4){double q0=bk.asks[0].qty,q3=bk.asks[3].qty; fs.slope_ask=std::max(-1.0,std::min(1.0,(q0-q3)/(q0+q3+1e-9)));}
    long long now_ms = bk.ts_ms;
    auto mid_at=[&](long long age)->double{ for(auto& b:book_buf) if(now_ms-b.ts_ms<=age){double m=(b.best_bid+b.best_ask)/2.0;return m>1e-8?m:mid;} return mid; };
    double m5=mid_at(5000), m15=mid_at(15000); fs.mid_return_5s  = (mid-m5 )/(m5 +1e-9)*100.0; fs.mid_return_15s = (mid-m15)/(m15+1e-9)*100.0;
    std::vector<double> mids; for(auto& b:book_buf) if(now_ms-b.ts_ms<=15000) mids.push_back((b.best_bid+b.best_ask)/2.0);
    if(mids.size()>=3){ double s=0,s2=0; for(int k=1;k<(int)mids.size();k++){double r=(mids[k]-mids[k-1])/(mids[k-1]+1e-9)*100.0;s+=r;s2+=r*r;} int n=(int)mids.size()-1; fs.volatility_15s=std::sqrt(std::max(0.0,s2/n-(s/n)*(s/n))); }
    if((int)trade_buf.size()>=MIN_TRADES){
        double bv60=0,sv60=0,bv300=0,sv300=0,cvd60=0,cvd300=0,vn60=0,vd60=0,vn300=0,vd300=0,sv5=0,sv15=0,nd5=0,nd15=0,tot5=0,tot15=0,tot_nd5=0,tot_nd15=0;
        for(auto& t:trade_buf){
            long long age=now_ms-t.ts_ms; double sign=t.is_buyer_maker?-1.0:1.0, nd=t.price*t.qty;
            if(age<=300000){if(sign>0)bv300+=t.qty;else sv300+=t.qty; cvd300+=sign*t.qty; vn300+=nd; vd300+=t.qty;}
            if(age<=60000) {if(sign>0)bv60 +=t.qty;else sv60 +=t.qty; cvd60 +=sign*t.qty; vn60 +=nd; vd60 +=t.qty;}
            if(age<=15000) {sv15+=sign*t.qty; nd15+=sign*nd; tot15+=t.qty; tot_nd15+=nd;}
            if(age<=5000)  {sv5 +=sign*t.qty; nd5 +=sign*nd; tot5 +=t.qty; tot_nd5 +=nd;}
        }
        fs.trade_imbalance_60  = (bv60 -sv60 )/(bv60 +sv60 +1e-9); fs.trade_imbalance_300 = (bv300-sv300)/(bv300+sv300+1e-9);
        double av60=std::max(bv60+sv60,1.0), av300=std::max(bv300+sv300,1.0);
        fs.cvd_60  = std::max(-1.0,std::min(1.0,cvd60 /av60 )); fs.cvd_300 = std::max(-1.0,std::min(1.0,cvd300/av300));
        double vwap60 =vd60 >1e-9?vn60 /vd60 :mid, vwap300=vd300>1e-9?vn300/vd300:mid;
        fs.vwap_dev_60  = (mid-vwap60 )/(vwap60 +1e-9)*100.0; fs.vwap_dev_300 = (mid-vwap300)/(vwap300+1e-9)*100.0;
        fs.signed_vol_5s  = tot5 >1e-9?sv5 /tot5 :0; fs.signed_vol_15s = tot15>1e-9?sv15/tot15:0;
        fs.trade_delta_5s  = tot_nd5 >1e-9?nd5 /tot_nd5 :0; fs.trade_delta_15s = tot_nd15>1e-9?nd15/tot_nd15:0;
    }
    if(!bars.empty()){
        int nb=(int)bars.size(); if(nb>=5){double p0=bars[nb-5].open,p1=bars[nb-1].close; fs.ret_5m=(p1-p0)/(p0+1e-9)*100.0;}
        if(nb>=15){double p0=bars[nb-15].open,p1=bars[nb-1].close; fs.ret_15m=(p1-p0)/(p0+1e-9)*100.0;}
        if(nb>=15){ double atr_s=0; for(int k=nb-14;k<nb;k++){ double pc=bars[k-1].close; double tr=std::max({bars[k].high-bars[k].low, std::abs(bars[k].high-pc),std::abs(bars[k].low-pc)}); atr_s+=tr; } fs.atr_1m = atr_s/14.0; }
    }
    double mp2=std::max(-1.0,std::min(1.0,fs.microprice_bias)), r5n=std::max(-1.0,std::min(1.0,fs.mid_return_5s*20.0));
    fs.book_pressure_score = std::max(-1.0,std::min(1.0, 0.35*fs.top_imbalance+0.25*fs.depth_imbalance_5+0.20*fs.depth_pressure+0.10*mp2+0.10*(fs.slope_bid-fs.slope_ask)));
    fs.flow_pressure_score = std::max(-1.0,std::min(1.0, 0.30*fs.trade_delta_5s+0.25*fs.signed_vol_5s+0.20*fs.cvd_60+0.15*fs.trade_imbalance_60+0.10*r5n));
    fs.valid = true; return fs;
}
std::vector<double> LiveFeatureEngine::meta_features(const LiveFeatureState& lf, const MIEModel::Result& r) const {
    auto t1=[](double x){return std::tanh(x);};
    return { r.cls_up, r.cls_down, r.cls_flat, r.confidence, r.regime_up, r.regime_down, r.regime_range, t1(lf.top_imbalance), t1(lf.spread_bps/5.0), t1(lf.trade_imbalance_60), t1(lf.trade_imbalance_300), t1(lf.cvd_60), t1(lf.cvd_300), lf.wall_bid_pct, lf.wall_ask_pct, t1(lf.depth_pressure), t1(lf.ret_5m/0.3), t1(lf.ret_15m/0.8) };
}
void LiveFeatureEngine::get_synthetic_candles(std::vector<Candle>& out_h1, std::vector<Candle>& out_m15) const {
    std::lock_guard<std::mutex> lk(mu); out_h1.clear(); out_m15.clear(); if (bars.empty()) return;
    auto agg=[&](int n_per)->std::vector<Candle>{
        std::vector<Candle> res; int nb=(int)bars.size(), start=nb%n_per;
        for(int i=start;i+n_per<=nb;i+=n_per){
            Candle c; c.timestamp=bars[i].open_ms/1000; c.open=bars[i].open; c.high=bars[i].high; c.low=bars[i].low; c.close=bars[i+n_per-1].close; c.volume=0;
            for(int j=i;j<i+n_per;j++){ c.high=std::max(c.high,bars[j].high); c.low =std::min(c.low ,bars[j].low); c.volume+=bars[j].volume(); } res.push_back(c);
        } return res;
    }; out_h1=agg(60); out_m15=agg(15);
}
bool LiveFeatureEngine::is_warmed_up() const { std::lock_guard<std::mutex> lk(mu); return (int)book_buf.size()>=MIN_BOOKS && (int)trade_buf.size()>=MIN_TRADES; }
int LiveFeatureEngine::bar_count() const { std::lock_guard<std::mutex> lk(mu); return (int)bars.size(); }
void LiveFeatureEngine::_update_bars(const TradeTick& t) {
    long long bom = (t.ts_ms/60000)*60000;
    if(cur_bar.open_ms==0){cur_bar.open_ms=bom;cur_bar.open=cur_bar.high=cur_bar.low=t.price;}
    if(bom>cur_bar.open_ms){ cur_bar.close=t.price; cur_bar.closed=true; bars.push_back(cur_bar); if((int)bars.size()>MAX_BARS) bars.pop_front(); cur_bar=MicroBar(); cur_bar.open_ms=bom; cur_bar.open=cur_bar.high=cur_bar.low=t.price; }
    cur_bar.high=std::max(cur_bar.high,t.price); cur_bar.low =std::min(cur_bar.low ,t.price); cur_bar.close=t.price; cur_bar.notional+=t.price*t.qty; cur_bar.ntrades++;
    if(t.is_buyer_maker) cur_bar.sell_vol+=t.qty; else cur_bar.buy_vol+=t.qty;
}

MetaFilter::MetaFilter() : w(META_DIM, 0.0) {}
void MetaFilter::set_lr(double l){ lr=l; }
double MetaFilter::predict(const std::vector<double>& x) const { if((int)x.size()!=META_DIM) return 0.5; double logit=bias; for(int k=0;k<META_DIM;k++) logit+=w[k]*x[k]; logit=std::max(-15.0,std::min(15.0,logit)); return 1.0/(1.0+std::exp(-logit)); }
void MetaFilter::update(const std::vector<double>& x, int label){
    if((int)x.size()!=META_DIM) return; double p=predict(x), err=p-(double)label; bias-=lr*err;
    for(int k=0;k<META_DIM;k++) w[k]-=lr*(err*x[k]+l2*w[k]); n_updates++; run_loss+=-(label*std::log(p+1e-9)+(1-label)*std::log(1-p+1e-9));
    if((p>=0.5)==(label==1)) n_correct++; int bin=std::min(9,(int)(p*10)); cal_n[bin]++; if(label==1) cal_pos[bin]++;
}
double MetaFilter::accuracy() const { return n_updates>0?(double)n_correct/n_updates:0; }
double MetaFilter::avg_loss() const  { return n_updates>0?run_loss/n_updates:0; }
bool MetaFilter::save(const std::string& p) const { FILE* f=fopen(p.c_str(),"wb"); if(!f) return false; fwrite(&bias,sizeof(double),1,f); fwrite(w.data(),sizeof(double),META_DIM,f); fwrite(&n_updates,sizeof(int),1,f); fwrite(&n_correct,sizeof(int),1,f); fwrite(cal_n,sizeof(int),10,f); fwrite(cal_pos,sizeof(int),10,f); fclose(f); return true; }
bool MetaFilter::load(const std::string& p) { FILE* f=fopen(p.c_str(),"rb"); if(!f) return false; fread(&bias,sizeof(double),1,f); fread(w.data(),sizeof(double),META_DIM,f); fread(&n_updates,sizeof(int),1,f); fread(&n_correct,sizeof(int),1,f); fread(cal_n,sizeof(int),10,f); fread(cal_pos,sizeof(int),10,f); fclose(f); return true; }

LiveDecision compute_live_decision_head( const MIEModel::Result& offline, double offline_bias, const LiveFeatureState& lf, const MetaFilter& meta, const std::vector<double>& meta_x, const LiveConfig& cfg) {
    LiveDecision ld; ld.regime_label = offline.regime_label; ld.clarity = offline.cls_confidence; ld.offline_bias = offline_bias; ld.spread_bps = lf.spread_bps; ld.top_imbalance = lf.top_imbalance; ld.depth_imbalance = lf.depth_imbalance_5; ld.trade_imb_60 = lf.trade_imbalance_60; ld.cvd_60 = lf.cvd_60; ld.cvd_300 = lf.cvd_300; ld.delta_5s = lf.trade_delta_5s; ld.delta_15s = lf.trade_delta_15s; ld.microprice = lf.microprice_bias; ld.wall_bid = lf.wall_bid_pct; ld.wall_ask = lf.wall_ask_pct; ld.vol_15s = lf.volatility_15s;
    if (!lf.valid) { ld.signal="WAIT"; ld.reason="live feed warming up"; ld.invalidation="no data"; return ld; }
    double mp2=std::max(-1.0,std::min(1.0,lf.microprice_bias)), r5n=std::max(-1.0,std::min(1.0,lf.mid_return_5s*20.0)), live_bias = 0.28*lf.top_imbalance + 0.22*lf.depth_imbalance_5 + 0.18*lf.flow_pressure_score + 0.12*lf.cvd_60 + 0.10*mp2 + 0.06*lf.trade_imbalance_300 + 0.04*r5n;
    live_bias = std::max(-1.0,std::min(1.0,live_bias)); ld.live_bias = live_bias;
    bool same_sign = (offline_bias>=0)==(live_bias>=0), is_trend = !offline.regime_gate_active, is_vol = offline.regime_volatile > 0.45, is_range = !is_trend && !is_vol;
    double fused;
    if (is_trend) fused = same_sign ? 0.60*offline_bias+0.40*live_bias : 0.70*offline_bias+0.30*live_bias; else if (is_range) { double ob2 = std::max(-0.4,std::min(0.4,offline_bias)); fused = same_sign ? 0.40*ob2+0.60*live_bias : 0.50*ob2+0.50*live_bias; fused *= 0.60; } else { fused = same_sign ? 0.50*offline_bias+0.50*live_bias : 0.55*offline_bias+0.45*live_bias; fused *= 0.50; }
    if (!same_sign) fused *= 0.70;
    std::vector<std::string> gates;
    if (lf.spread_bps > 10.0) { fused *= 0.15; gates.push_back("spread wide"); } else if (lf.spread_bps > 6.0) fused *= 0.65;
    if (lf.volatility_15s > 0.12) { fused *= 0.50; gates.push_back("high vol"); } if (ld.clarity < cfg.min_clarity) { fused *= 0.70; gates.push_back("low clarity"); }
    fused = std::max(-1.0,std::min(1.0,fused)); ld.fused_bias = fused; ld.bull_score = std::max(0.0, fused)*100.0; ld.bear_score = std::max(0.0,-fused)*100.0; ld.wait_score = (1.0-std::abs(fused))*60.0;
    ld.meta_prob = meta.predict(meta_x); ld.meta_action = (!cfg.meta_on || meta.n_updates < 20) ? "UNREADY" : (ld.meta_prob >= 0.52) ? "ALLOW" : "FILTER";
    double af = std::abs(fused);
    if (!same_sign && af < 0.08) ld.signal = "CONFLICT"; else if (af >= 0.32 && ld.meta_action != "FILTER") ld.signal = fused > 0 ? "BUY" : "SELL"; else if (af >= 0.16) ld.signal = fused > 0 ? "BUY_WATCH" : "SELL_WATCH"; else ld.signal = gates.empty() ? "WAIT" : "NO_TRADE";
    if (!gates.empty() && ld.signal=="BUY") ld.signal="BUY_WATCH"; if (!gates.empty() && ld.signal=="SELL") ld.signal="SELL_WATCH";
    ld.score_0_100 = std::min(100.0, af*75.0 + (same_sign?20.0:0.0)*std::min(af*3.0,1.0) + ld.meta_prob*5.0);
    ld.reason=""; ld.invalidation=gates.empty()?"none":gates[0]; for(int k=1;k<(int)gates.size();k++) ld.invalidation+=", "+gates[k]; return ld;
}

void print_live_decision_v2(const LiveDecision& ld, const MetaFilter& meta, double price, int n_bars) {
    const char* G="\033[32m",*R="\033[31m",*Y="\033[33m",*X="\033[0m", *sc = (ld.signal.find("BUY")!=std::string::npos)?G:(ld.signal.find("SELL")!=std::string::npos)?R:(ld.signal=="CONFLICT")?Y:X, *mc=(ld.meta_action=="ALLOW")?G:(ld.meta_action=="FILTER")?R:Y;
    printf("\033[2J\033[H╔══ CAPIGRAD LIVE (HFT Bridge) ══════════════════════════╗\n");
    printf("║  Price: $%-10.2f  Bars(1m): %-5d  %s%-16s%s║\n", price, n_bars, sc, ld.signal.c_str(), X);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  REGIME: %-12s  clarity=%.2f  gate=%-3s               ║\n", ld.regime_label.c_str(), ld.clarity, ld.regime_label.find("TREND")!=std::string::npos?"OFF":"ON ");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  offline_bias : %+.4f                                   ║\n║  live_bias    : %+.4f                                   ║\n║  fused_bias   : %+.4f                                   ║\n║  score=%-3.0f/100                                            ║\n", ld.offline_bias, ld.live_bias, ld.fused_bias, ld.score_0_100);
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  META: %smeta_prob=%.3f  %s%-8s%s  trained=%-4d           ║\n╚══════════════════════════════════════════════════════════╝\n", mc, ld.meta_prob, mc, ld.meta_action.c_str(), X, meta.n_updates);
    fflush(stdout);
}

// 🚀 YANGI HFT UDP "Bridge" Qabul Qiluvchi 🚀
using BookCallback  = std::function<void(const OrderBookSnapshot&)>;
using TradeCallback = std::function<void(const TradeTick&)>;

class ILiveFeed {
public:
    virtual ~ILiveFeed() = default;
    virtual void set_book_callback(BookCallback cb)   = 0;
    virtual void set_trade_callback(TradeCallback cb) = 0;
    virtual void start() = 0;
    virtual void stop()  = 0;
};

class MockFeed : public ILiveFeed { ... }; // O'chirib qo'yish mumkin, joy tejash uchun yashirdik

class UDPFeed : public ILiveFeed {
public:
    LiveConfig cfg; bool running = false; std::thread ws_thread;
    BookCallback book_cb; TradeCallback trade_cb; LiveOrderBook lob;

    explicit UDPFeed(LiveConfig c) : cfg(std::move(c)) { lob.max_depth = 10; }
    void set_book_callback(BookCallback cb) override { book_cb = cb; }
    void set_trade_callback(TradeCallback cb) override { trade_cb = cb; }
    void start() override { running = true; ws_thread = std::thread([this]{ run(); }); }
    void stop() override { running = false; if(ws_thread.joinable()) ws_thread.join(); }
    ~UDPFeed() { stop(); }

private:
    void run() {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); return; }
        
        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        struct sockaddr_in servaddr{};
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(5555);

        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("bind"); close(sockfd); return;
        }

        struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        char buffer[4096];
        printf("\n[C++] 🚀 CapiGrad Dvigateli UDP:5555 da ishga tushdi. Python feed.py ni kuting...\n");

        while (running && g_running) {
            int n = recvfrom(sockfd, (char *)buffer, 4096, 0, nullptr, nullptr);
            if (n > 0) {
                buffer[n] = '\0';
                parse_msg(buffer);
            }
        }
        close(sockfd);
    }

    void parse_msg(char* msg) {
        if (msg[0] == 'T') {
            if (!trade_cb) return;
            long long ts; double p, q; int m;
            if (sscanf(msg, "T,%lld,%lf,%lf,%d", &ts, &p, &q, &m) == 4) {
                TradeTick t; t.ts_ms = ts; t.price = p; t.qty = q; t.is_buyer_maker = (m == 1);
                trade_cb(t);
            }
        } else if (msg[0] == 'D') {
            if (!book_cb) return;
            std::vector<std::pair<double, double>> bids, asks;
            char* ptr = msg + 2; char* next;
            long long ts = strtoll(ptr, &next, 10); ptr = next + 1;
            
            for (int i=0; i<10; i++) {
                double p = strtod(ptr, &next); ptr = next + 1;
                double q = strtod(ptr, &next); ptr = next + 1;
                if (p > 0) bids.push_back({p, q});
            }
            for (int i=0; i<10; i++) {
                double p = strtod(ptr, &next); ptr = next + 1;
                double q = strtod(ptr, &next); if (*next == ',') ptr = next + 1;
                if (p > 0) asks.push_back({p, q});
            }
            lob.update(bids, asks, ts);
            if (lob.valid()) book_cb(lob.snapshot());
        }
    }
};

class LiveCSVLogger {
public:
    FILE* f=nullptr; int signal_id=0;
    void open(const std::string& p){ f=fopen(p.c_str(),"a"); if(!f) return; fseek(f,0,SEEK_END); if(ftell(f)==0) fprintf(f,"ts,price,spread_bps,imb,ti60,cvd60,cls_up,cls_dn,cls_fl,clarity,regime,gate,action,meta_prob,meta_action,sid,row_type\n"); }
    void close(){if(f){fflush(f);fclose(f);f=nullptr;}}
    void log_tick(long long ts_ms,const LiveFeatureState& lf, const LiveDecision& ld,const MIEModel::Result& r){
        if(!f) return; int sid=0; if(ld.signal.find("BUY")!=std::string::npos||ld.signal.find("SELL")!=std::string::npos) sid=++signal_id;
        fprintf(f,"%lld,%.2f,%.2f,%+.4f,%+.4f,%+.4f,%.4f,%.4f,%.4f,%.3f,%s,%s,%s,%.4f,%s,%d,TICK\n", ts_ms,lf.mid,lf.spread_bps,lf.top_imbalance,lf.trade_imbalance_60,lf.cvd_60,r.cls_up,r.cls_down,r.cls_flat,ld.clarity, ld.regime_label.c_str(),r.regime_gate_active?"ON":"OFF",ld.signal.c_str(),ld.meta_prob,ld.meta_action.c_str(),sid); fflush(f);
    }
    void log_resolve(long long ts_ms,int sid,int label,double pnl_pct){ if(!f) return; fprintf(f,"%lld,0,0,0,0,0,0,0,0,0,,,RESOLVE,0,,0,RESOLVE_label=%d_pnl=%.4f\n", ts_ms,label,pnl_pct); fflush(f); }
};

void run_live_mode_v2(MIEModel& model, const std::vector<Candle>& d1h_csv, const std::vector<Candle>& d15m_csv, const LiveConfig& cfg) {
    MetaFilter meta; meta.set_lr(cfg.meta_lr); if(meta.load("meta_state.bin")) printf("[META] Loaded\n");
    LiveCSVLogger csv; if(!cfg.save_live.empty()) csv.open(cfg.save_live);
    LiveFeatureEngine lfe; 
    
    // 🚀 Endi biz doim UDPFeed ishlatamiz
    std::unique_ptr<ILiveFeed> feed;
    feed.reset(new UDPFeed(cfg));

    feed->set_book_callback([&](const OrderBookSnapshot& s){lfe.on_book(s);});
    feed->set_trade_callback([&](const TradeTick& t){lfe.on_trade(t);});
    feed->start();

    std::vector<PendingSample> pending; int next_sample_id=1; std::mutex pending_mu;
    OfflineContext octx; if(d1h_csv.size()>=60) octx=run_offline_context(model,d1h_csv,d15m_csv);

    auto last_meta_save=std::chrono::steady_clock::now(); auto last_offline_ref=std::chrono::steady_clock::now();
    double last_printed_price = 0.0; std::string last_printed_signal = "";

    while(g_running){
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Tekshirish tezligini oshirdik
        long long now_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        auto now_clk=std::chrono::steady_clock::now();

        if(!lfe.is_warmed_up()) continue;
        LiveFeatureState lfs=lfe.build(); if(!lfs.valid) continue;

        int n_bars=lfe.bar_count(); int elapsed_off=(int)std::chrono::duration_cast<std::chrono::seconds>(now_clk-last_offline_ref).count();
        if(elapsed_off>=60||!octx.valid){
            if(n_bars>=120){ std::vector<Candle> sh1,sm15; lfe.get_synthetic_candles(sh1,sm15);
                if(sh1.size()>=10){ std::vector<Candle> mh1=d1h_csv,mm15=d15m_csv; for(auto& c:sh1) mh1.push_back(c); for(auto& c:sm15) mm15.push_back(c); octx=run_offline_context(model,mh1,mm15); } 
                else if(d1h_csv.size()>=60) octx=run_offline_context(model,d1h_csv,d15m_csv);
            } else if(d1h_csv.size()>=60) octx=run_offline_context(model,d1h_csv,d15m_csv);
            last_offline_ref=now_clk;
        }
        if(!octx.valid) continue;

        std::vector<double> meta_x=lfe.meta_features(lfs,octx.result);
        {
            std::lock_guard<std::mutex> lk(pending_mu);
            for(auto& ps:pending){
                if(ps.resolved) continue; ps.bars_elapsed++; double cur=lfs.mid;
                bool hu=(ps.direction==1&&cur>=ps.upper)||(ps.direction==-1&&cur<=ps.lower), hl=(ps.direction==1&&cur<=ps.lower)||(ps.direction==-1&&cur>=ps.upper);
                if(hu){ps.label=1;ps.resolved=true;} else if(hl||ps.bars_elapsed>=ps.horizon_bars*60){ps.label=0;ps.resolved=true;}
                if(ps.resolved&&cfg.meta_on){ meta.update(ps.x,ps.label); csv.log_resolve(now_ms,ps.id,ps.label,(cur-ps.entry_price)/ps.entry_price*100.0); }
            }
            if((int)pending.size()>cfg.meta_buf) pending.erase(pending.begin(),pending.begin()+(pending.size()-cfg.meta_buf/2));
        }

        LiveDecision ld=compute_live_decision_head(octx.result,octx.offline_bias,lfs,meta,meta_x,cfg);
        meta.n_signals++; if(ld.meta_action=="FILTER") meta.n_filtered++;

        bool is_dir=(ld.signal.find("BUY")!=std::string::npos||ld.signal.find("SELL")!=std::string::npos);
        if(is_dir&&lfs.atr_1m>0&&cfg.meta_on){
            std::lock_guard<std::mutex> lk(pending_mu); PendingSample ps;
            ps.id=next_sample_id++; ps.ts_ms=now_ms; ps.x=meta_x; ps.entry_price=lfs.mid;
            ps.direction=(ld.signal.find("BUY")!=std::string::npos)?1:-1;
            ps.upper=lfs.mid+ps.direction*cfg.meta_k*lfs.atr_1m; ps.lower=lfs.mid-ps.direction*cfg.meta_k*lfs.atr_1m;
            ps.horizon_bars=cfg.meta_horizon; pending.push_back(ps);
        }
        
        csv.log_tick(now_ms,lfs,ld,octx.result); 
        
        // Ekranga faqat o'zgarish bo'lsa chiqarish mantiqi:
        if (std::abs(lfs.mid - last_printed_price) > 0.5 || ld.signal != last_printed_signal) {
             print_live_decision_v2(ld,meta,lfs.mid,n_bars);
             last_printed_price = lfs.mid; last_printed_signal = ld.signal;
        }

        if((int)std::chrono::duration_cast<std::chrono::seconds>(now_clk-last_meta_save).count()>=60&&cfg.meta_on){meta.save("meta_state.bin");last_meta_save=now_clk;}
    }
    feed->stop(); if(cfg.meta_on) meta.save("meta_state.bin"); csv.close(); printf("\n[LIVE] Stopped.\n");
}

void run_live_eval(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m, const LiveConfig& cfg) {}