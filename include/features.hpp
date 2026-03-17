#ifndef FEATURES_HPP
#define FEATURES_HPP

#include "data.hpp"
#include <vector>
#include <string>

// Triple-Barrier class konstantalari
constexpr int TB_UP   = 0;
constexpr int TB_DOWN = 1;
constexpr int TB_FLAT = 2;
constexpr int TB_MAX_HORIZON = 6;

namespace FE {
    enum class Regime { TREND=0, RANGE=1, VOLATILE=2 };
    const char* regime_str(Regime r);

    struct BarrierParams {
        double up_k;   
        double down_k; 
        int    horizon;
    };

    // Tashqaridan ko'rinadigan global o'zgaruvchilar
    extern BarrierParams g_barrier_params[3];
    extern bool g_barrier_calibrated;

    BarrierParams get_barrier_params(Regime reg);
    BarrierParams get_calibrated_params(Regime reg);

    // Asosiy indikatorlar
    std::vector<double> closes(const std::vector<Candle>& d);
    double ema(const std::vector<double>& cl, int i, int p);
    double atr(const std::vector<Candle>& d, int i, int p=14);
    double rsi(const std::vector<double>& cl, int i, int p=14);
    double vol_ratio(const std::vector<Candle>& d, int i, int p=20);
    
    // Feature Extraction (70 datchik)
    std::vector<double> extract_tf(const std::vector<Candle>& data, int idx);
    std::vector<double> extract_tf_structure(const std::vector<Candle>& data, int idx, const std::vector<double>& other_tf_feat={});
    
    // Triple Barrier va Regime
    Regime detect_regime(const std::vector<Candle>& d, int i);
    int compute_regime(const std::vector<Candle>& data, int i);
    int compute_triple_barrier(const std::vector<Candle>& d, int idx);

    struct BarrierResult {
        int label;           
        Regime reg;
        BarrierParams params;
    };
    BarrierResult compute_triple_barrier_full(const std::vector<Candle>& d, int idx);
}

#endif // FEATURES_HPP