#ifndef LIVE_HPP
#define LIVE_HPP

#include "data.hpp"
#include "model.hpp"
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <mutex>

struct LiveConfig {
    std::string symbol       = "btcusdt";
    std::string market       = "spot";
    int         depth        = 20;
    int         book_ms      = 100;
    bool        meta_on      = true;
    double      meta_lr      = 0.05;
    int         meta_buf     = 20000;
    int         meta_horizon = 5;     
    double      meta_k       = 1.5;   
    double      threshold    = 0.35;
    double      min_clarity  = 0.50;
    std::string save_live    = "live_log.csv";
    bool        use_mock     = true;

    std::string ws_host() const;
    int ws_port() const;
    std::string combined_path() const;
};

struct BookLevel { double price = 0; double qty = 0; };
struct OrderBookSnapshot {
    long long ts_ms = 0;
    double best_bid = 0, best_ask = 0;
    double bid_qty  = 0, ask_qty  = 0;
    std::vector<BookLevel> bids;
    std::vector<BookLevel> asks;
};
struct TradeTick {
    long long ts_ms     = 0;
    double    price     = 0, qty = 0;
    bool is_buyer_maker = false;
};
struct MicroBar {
    long long open_ms = 0;
    double open=0, high=0, low=1e18, close=0;
    double buy_vol=0, sell_vol=0, notional=0;
    int    ntrades=0;
    bool   closed = false;
    double volume()  const { return buy_vol + sell_vol; }
    double vwap()    const { return notional / (volume() + 1e-9); }
};

struct LiveFeatureState {
    bool      valid            = false;
    long long ts_ms            = 0;
    double    mid              = 0;
    double spread_bps          = 0;
    double top_imbalance       = 0;
    double depth_imbalance_5   = 0;
    double depth_imbalance_10  = 0;
    double microprice_bias     = 0;
    double depth_pressure      = 0;
    double wall_bid_pct        = 0;
    double wall_ask_pct        = 0;
    double slope_bid           = 0;
    double slope_ask           = 0;
    double trade_imbalance_60  = 0;
    double trade_imbalance_300 = 0;
    double cvd_60              = 0;
    double cvd_300             = 0;
    double vwap_dev_60         = 0;
    double vwap_dev_300        = 0;
    double signed_vol_5s       = 0;
    double signed_vol_15s      = 0;
    double trade_delta_5s      = 0;
    double trade_delta_15s     = 0;
    double mid_return_5s       = 0;
    double mid_return_15s      = 0;
    double volatility_15s      = 0;
    double ret_5m              = 0;
    double ret_15m             = 0;
    double atr_1m              = 0;
    double book_pressure_score = 0;
    double flow_pressure_score = 0;
};

struct PendingSample {
    int    id           = 0;
    long long ts_ms     = 0;
    std::vector<double> x;
    double entry_price  = 0;
    int    direction    = 0; 
    double upper        = 0;
    double lower        = 0;
    int    horizon_bars = 5;
    int    bars_elapsed = 0;
    bool   resolved     = false;
    int    label        = -1;
};

class LiveOrderBook {
public:
    int max_depth = 20;
    std::map<double, double, std::greater<double>> bids;
    std::map<double, double>                       asks;
    long long last_update_ms = 0;

    void update(const std::vector<std::pair<double,double>>& bu,
                const std::vector<std::pair<double,double>>& au,
                long long ts_ms);
    OrderBookSnapshot snapshot() const;
    bool valid() const;
};

class LiveFeatureEngine {
public:
    static constexpr int WINDOW_MS   = 300000;
    static constexpr int BOOK_WIN_MS = 30000;
    static constexpr int MIN_BOOKS   = 3;
    static constexpr int MIN_TRADES  = 5;
    static constexpr int MAX_BARS    = 500;

    std::deque<OrderBookSnapshot> book_buf;
    std::deque<TradeTick>         trade_buf;
    std::deque<MicroBar>          bars;
    MicroBar                      cur_bar;
    mutable std::mutex            mu;

    void on_book(const OrderBookSnapshot& s);
    void on_trade(const TradeTick& t);
    LiveFeatureState build() const;
    std::vector<double> meta_features(const LiveFeatureState& lf, const MIEModel::Result& r) const;
    void get_synthetic_candles(std::vector<Candle>& out_h1, std::vector<Candle>& out_m15) const;
    bool is_warmed_up() const;
    int bar_count() const;
private:
    void _update_bars(const TradeTick& t);
};

static constexpr int META_DIM = 18;
class MetaFilter {
public:
    std::vector<double> w;
    double bias     = 0;
    double lr       = 0.05;
    double l2       = 1e-4;
    int  n_updates  = 0, n_correct = 0;
    int  n_signals  = 0, n_filtered= 0;
    double run_loss = 0;
    int  cal_n[10]={}, cal_pos[10]={};

    MetaFilter();
    void set_lr(double l);
    double predict(const std::vector<double>& x) const;
    void update(const std::vector<double>& x, int label);
    double accuracy() const;
    double avg_loss() const;
    bool save(const std::string& p) const;
    bool load(const std::string& p);
};

struct LiveDecision {
    std::string signal      = "WAIT";
    std::string meta_action = "N/A";
    double score_0_100   = 0, meta_prob = 0.5;
    double offline_bias  = 0, live_bias = 0, fused_bias = 0;
    double bull_score    = 0, bear_score= 0, wait_score = 0;
    std::string regime_label;
    double clarity       = 0;
    double spread_bps    = 0, top_imbalance = 0, depth_imbalance = 0;
    double trade_imb_60  = 0, cvd_60 = 0, cvd_300 = 0;
    double delta_5s      = 0, delta_15s = 0;
    double microprice    = 0, wall_bid = 0, wall_ask = 0;
    double vol_15s       = 0;
    std::string reason, invalidation;
};

LiveDecision compute_live_decision_head(
    const MIEModel::Result&    offline,
    double                     offline_bias,
    const LiveFeatureState&    lf,
    const MetaFilter&          meta,
    const std::vector<double>& meta_x,
    const LiveConfig&          cfg);

void print_live_decision_v2(const LiveDecision& ld, const MetaFilter& meta, double price, int n_bars);
void run_live_mode_v2(MIEModel& model, const std::vector<Candle>& d1h_csv, const std::vector<Candle>& d15m_csv, const LiveConfig& cfg);
void run_live_eval(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m, const LiveConfig& cfg);

#endif // LIVE_HPP