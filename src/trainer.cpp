#include "trainer.hpp"
#include "features.hpp"
#include "globals.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <random>
#include "data.hpp"
#include <iostream>
#include <vector>
#include <cmath>
static constexpr double DECISION_THRESHOLD = 0.35;
static constexpr double ALPHA_TREND = 0.20;
static constexpr double FLAT_TARGET    = 0.50; 
static constexpr double FLAT_TOLERANCE = 0.05; 
static constexpr double K_MIN          = 0.8;  
static constexpr double K_MAX          = 6.0;  
static constexpr double W_REGIME_TREND    = 1.00;
static constexpr double W_REGIME_RANGE    = 0.35;
static constexpr double W_REGIME_VOLATILE = 0.20;

void compute_class_weights(const std::vector<Candle>& d1h, int train_end){
    int safe = TB_MAX_HORIZON + 2;
    int cnt[3]={0,0,0}, total=0;
    for(int i=60; i<train_end-safe; i++){
        int lbl = FE::compute_triple_barrier(d1h, i);
        cnt[lbl]++; total++;
    }
    if(total==0) return;
    printf("[CLASS WEIGHTS] n=%d  UP=%.1f%% DOWN=%.1f%% FLAT=%.1f%%\n",
           total, 100.0*cnt[0]/total, 100.0*cnt[1]/total, 100.0*cnt[2]/total);
    for(int c=0;c<3;c++){
        double w = cnt[c]>0 ? (double)total/(3.0*cnt[c]) : 1.0;
        w = std::max(0.5, std::min(2.0, w));
        g_cls_weights[c] = w;
    }
}

void calibrate_barrier_params(const std::vector<Candle>& d1h){
    if(FE::g_barrier_calibrated) return;
    int n = (int)d1h.size(), safe = TB_MAX_HORIZON + 2, train_end = (int)(n * 0.85);
    printf("[CALIBRATE] Barrier K ni FLAT=%.0f%% ga moslashtirish...\n", FLAT_TARGET*100);
    for(int rgi=0; rgi<3; rgi++){
        FE::Regime reg = (FE::Regime)rgi;
        double lo = K_MIN, hi = K_MAX, best_k = FE::get_barrier_params(reg).up_k;
        int best_h = FE::get_barrier_params(reg).horizon;
        for(int iter=0; iter<12; iter++){
            double mid_k = (lo + hi) / 2.0;
            FE::g_barrier_params[rgi] = {mid_k, mid_k, best_h};
            FE::g_barrier_calibrated = true;
            int cnt_flat=0, cnt_total=0;
            for(int i=60; i<train_end-safe; i++){
                if((int)FE::detect_regime(d1h,i) != rgi) continue;
                if(FE::compute_triple_barrier(d1h, i) == TB_FLAT) cnt_flat++;
                cnt_total++;
            }
            if(cnt_total < 50){ FE::g_barrier_params[rgi] = FE::get_barrier_params(reg); break; }
            double flat_rate = (double)cnt_flat / cnt_total;
            if(flat_rate > FLAT_TARGET + FLAT_TOLERANCE) hi = mid_k;
            else if(flat_rate < FLAT_TARGET - FLAT_TOLERANCE) lo = mid_k;
            else { best_k = mid_k; break; }
            best_k = mid_k;
        }
        FE::g_barrier_params[rgi] = {best_k, best_k, best_h};
        FE::g_barrier_calibrated = true;
    }
}

void train_mie(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m, int epochs, double lr){
    int n1h=(int)d1h.size(); bool has15m=d15m.size()>200;
    int train_end=(int)(n1h*0.85); int val_start=train_end;
    calibrate_barrier_params(d1h); compute_class_weights(d1h, train_end);

    double best_val=1e9; int no_improve=0, patience=60;
    int warmup_ep = epochs / 10;
    double lr_max = lr, lr_min = lr * 0.04, lr_start = lr * 0.05;

    auto find15m=[&](long long ts)->int{
        if(!has15m) return 0;
        int lo=0,hi=(int)d15m.size()-1;
        while(lo<hi){int mid=(lo+hi+1)/2;if(d15m[mid].timestamp<=ts)lo=mid;else hi=mid-1;}
        return lo;
    };

    for(int ep=0;ep<epochs&&g_running;ep++){
        double cur_lr = (ep < warmup_ep) ? (lr_start + ((double)ep/warmup_ep)*(lr_max-lr_start)) 
                                         : (lr_min + 0.5*(lr_max-lr_min)*(1.0+std::cos(M_PI*(ep-warmup_ep)/(epochs-warmup_ep))));
        double train_loss=0; int tn=0;
        model.reset();
        std::vector<int> idx; for(int i=60;i<train_end-2;i++) idx.push_back(i);
        std::shuffle(idx.begin(),idx.end(),std::mt19937(ep*17+42));

        for(int ii=0;ii<(int)idx.size();ii++){
            int i=idx[ii];
            std::vector<Candle> win1h; for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
            auto feat_h1=FE::extract_tf_structure(win1h,(int)win1h.size()-1);
            std::vector<Candle> win15m;
            if(has15m){int j=find15m(d1h[i].timestamp);for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);}
            else win15m=win1h;
            auto feat_m15=FE::extract_tf_structure(win15m,(int)win15m.size()-1, feat_h1);

            std::vector<double> targets(14,0);
            targets[FE::compute_regime(d1h,i)]=1.0;
            double a=std::max(FE::atr(win1h,(int)win1h.size()-1),1e-8), c=d1h[i].close;
            double thresh=std::max(0.002,std::min(0.012, a/c*0.75));
            int last_i=n1h-1;
            double ar1=(d1h[std::min(i+1,last_i)].close-c)/c, ar3=(d1h[std::min(i+3,last_i)].close-c)/c, ar6=(d1h[std::min(i+6,last_i)].close-c)/c;
            double ar=0.5*ar1+0.3*ar3+0.2*ar6;
            auto cl=FE::closes(d1h);
            double trend_str=clip((FE::ema(cl,i,20)-FE::ema(cl,i,50))/a,1.0);
            targets[4]=trend_str; targets[5]=std::min(1.0,std::abs(ar)*5); targets[6]=((ar>thresh?1.0:ar<-thresh?-1.0:0.0)+1.0)/2.0;
            targets[7]=clip(ar1/thresh,1.0); targets[8]=clip(ar/thresh,1.0);
            targets[9]=std::max(0.0,std::min(1.0,std::abs(trend_str)*targets[5]*0.5+(std::abs(targets[7])>0.7?0.3:0.0)));
            targets[10]=clip(clip(ar1/thresh,1.0)*FE::vol_ratio(d1h,i,20),1.0);
            targets[11]=std::max(-3.0,std::min(3.0,ar1*100)); targets[12]=std::max(-3.0,std::min(3.0,(d1h[std::min(i+2,last_i)].close-c)/c*100));
            targets[13] = (i + TB_MAX_HORIZON + 2 < n1h) ? (double)FE::compute_triple_barrier(d1h, i) : (double)TB_FLAT;

            FE::Regime rg = FE::detect_regime(d1h, i);
            g_cur_regime_w = (rg == FE::Regime::TREND) ? W_REGIME_TREND : (rg == FE::Regime::RANGE) ? W_REGIME_RANGE : W_REGIME_VOLATILE;

            auto pred=model.forward(feat_h1,feat_m15);
            model.backward(feat_h1,feat_m15,targets,pred);

            int bc=(int)std::round(targets[13]); bc=std::max(0,std::min(2,bc));
            double p_bc = std::max((&pred.cls_up)[bc], 1e-10);
            double cls_ce = -g_cls_weights[bc] * g_cur_regime_w * std::pow(1.0 - p_bc, 2.0) * std::log(p_bc);
            double tl = 0.35*cls_ce;
            if(!std::isnan(tl)&&!std::isinf(tl)){train_loss+=tl; tn++;}
            if(ii%16==0) model.update(cur_lr);
        }
        if(tn>0) train_loss/=tn;

        double val_loss=0; int vn=0, dir_correct=0, dir_total=0, cls_correct=0, trend_act=0, trend_act_c=0, trend_n_val=0;
        model.reset();
        for(int i=val_start+60;i<n1h-TB_MAX_HORIZON-2;i++){
            std::vector<Candle> win1h; for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
            auto fh=FE::extract_tf_structure(win1h,(int)win1h.size()-1);
            std::vector<Candle> win15m;
            if(has15m){int j=find15m(d1h[i].timestamp);for(int k=std::max(0,j-99);k<=j;k++) win15m.push_back(d15m[k]);}
            else win15m=win1h;
            auto fm=FE::extract_tf_structure(win15m,(int)win15m.size()-1, fh);

            auto pred=model.forward(fh,fm);
            int real_bc=FE::compute_triple_barrier(d1h, i);
            val_loss+=-std::log(std::max((&pred.cls_up)[real_bc],1e-10)); vn++;

            double max_p = std::max({pred.cls_up, pred.cls_down, pred.cls_flat});
            int raw_bc = (pred.cls_up>pred.cls_down && pred.cls_up>pred.cls_flat)?0:(pred.cls_down>pred.cls_flat)?1:2;
            int pred_bc = (raw_bc != TB_FLAT && max_p >= DECISION_THRESHOLD) ? raw_bc : TB_FLAT;
            if(pred_bc==real_bc) cls_correct++;

            if(pred_bc!=TB_FLAT && real_bc!=TB_FLAT){if(pred_bc==real_bc) dir_correct++; dir_total++;}
            if(FE::detect_regime(d1h, i) == FE::Regime::TREND){
                trend_n_val++;
                if(pred_bc!=TB_FLAT){trend_act++; if(pred_bc==real_bc) trend_act_c++;}
            }
        }
        if(vn>0) val_loss/=vn;
        double dir_acc = dir_total>0 ? 100.0*dir_correct/dir_total : -1.0;
        double cls_acc = vn>0 ? 100.0*cls_correct/vn : 0;
        double trend_ap = trend_act>0 ? (double)trend_act_c/trend_act : 0.0;
        double sel_score = val_loss - ALPHA_TREND * trend_ap;

        bool improved = sel_score < best_val;
        if(improved){best_val=sel_score; no_improve=0; model.save("mie_best.bin");} else no_improve++;

        if(ep%5==0||improved){
            printf("  ep%4d/%d lr=%.6f trn:%.4f vce:%.4f sel:%.4f cls:%.1f%% dir:%.1f%% tap:%.0f%% [%d/%d]%s\n",
                   ep+1, epochs, cur_lr, train_loss, val_loss, sel_score, cls_acc, dir_acc, trend_ap*100, no_improve, patience, improved?" ★":"");
        }
        if(no_improve>=patience) break;
    }
    model.save("mie_model.bin");
    printf("[TRAIN] Done.\n");
}

void eval_mie(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m){
    printf("\n[EVAL] Running...\n");
    int n=(int)d1h.size(), test_start=(int)(n*0.80);
    int correct=0, total=0;
    model.reset();
    for(int i=test_start+60;i<n-10;i++){
        std::vector<Candle> win1h; for(int k=std::max(0,i-99);k<=i;k++) win1h.push_back(d1h[k]);
        auto fh=FE::extract_tf_structure(win1h,(int)win1h.size()-1);
        auto pred=model.forward(fh,fh);
        int real_bc = FE::compute_triple_barrier(d1h, i);
        int raw_bc = (pred.cls_up>pred.cls_down && pred.cls_up>pred.cls_flat)?0:(pred.cls_down>pred.cls_flat)?1:2;
        if(raw_bc == real_bc) correct++;
        total++;
    }
    printf("[EVAL] Accuracy: %.2f%%\n", 100.0*correct/std::max(1,total));
}

// Live ma'lumotlar bilan ishlash uchun yangi o'qitish tsikli
void train_mie_live(MetaLiveModel& model, const std::vector<LiveTick>& live_data, int epochs, double lr) {
    int n = (int)live_data.size();
    int window_size = 60; // N=60 qatorlik oyna (taxminan 1 soatlik mikrotuzilma)
    int lookahead = 15;   // Kelajakdagi narxni bilish uchun 15 qadam oldinga qaraymiz

    if (n < window_size + lookahead) {
        std::cout << "[ERROR] Data juda kam!" << std::endl;
        return;
    }

    int train_end = (int)(n * 0.85); // 85% train, 15% validation

    std::cout << "\n[LIVE TRAIN] O'qitish boshlandi. Jami qatorlar: " << n << std::endl;

    for (int ep = 0; ep < epochs && g_running; ep++) {
        double train_loss = 0; 
        int tn = 0;
        
        // DIQQAT: std::shuffle ISHLATILMAYDI! Ma'lumotlar faqat xronologik tartibda beriladi.
        for (int i = window_size; i < train_end - lookahead; i++) {
            
            // 1. Har bir yangi oyna uchun GRU xotirasini (h) tozalaymiz
            model.reset(); 
            
            std::vector<double> final_pred;
            std::vector<double> last_features;

            // 2. Vaqt oynasi bo'ylab yurib chiqamiz (i-60 dan i gacha)
            for (int t = i - window_size; t <= i; t++) {
                // Tarmoqqa beriladigan mikrotuzilma xususiyatlari (Features)
                std::vector<double> features = {
                    live_data[t].spread_bps,
                    live_data[t].imb,
                    live_data[t].ti60,
                    live_data[t].cvd60,
                    live_data[t].cls_up,
                    live_data[t].cls_dn,
                    live_data[t].cls_fl,
                    live_data[t].clarity
                };
                
                // Eslatma: Agar MIEModel faqat makro data (h1, m15) qabul qilsa, 
                // uni shu 'features' ni qabul qiladigan qilib moslashtirish kerak.
                final_pred = model.forward_live(features); 
                last_features = features;
            }

            // 3. Target (Nishon) yaratish: Kelajakdagi narx o'zgarishi
            double current_price = live_data[i].price;
            double future_price = live_data[i + lookahead].price;
            double price_diff_pct = (future_price - current_price) / current_price;
            
            std::vector<double> targets(3, 0.0); // 0: UP, 1: DOWN, 2: FLAT
            // 0.05% (5 bps) lik chegarani o'rnatamiz, bu bozor volatilligiga qarab o'zgarishi mumkin
            if (price_diff_pct > 0.0005) {
                targets[0] = 1.0; // UP
            } else if (price_diff_pct < -0.0005) {
                targets[1] = 1.0; // DOWN
            } else {
                targets[2] = 1.0; // FLAT
            }

            // 4. Xatoni hisoblash (Cross-Entropy)
            int target_idx = targets[0] == 1.0 ? 0 : (targets[1] == 1.0 ? 1 : 2);
            double p_bc = std::max(final_pred[target_idx], 1e-10);
            double loss = -std::log(p_bc);
            
            if (!std::isnan(loss) && !std::isinf(loss)) {
                train_loss += loss; 
                tn++;
            }

            // 5. Orqaga tarqalish (Backward) va Vaznlarni yangilash (Update)
            // Backward faqat oynaning oxirgi qadamidagi xato bo'yicha qilinadi
            model.backward_live(last_features, targets, final_pred);
            
            // Batch Size = 16 (har 16 qadamda weights yangilanadi)
            if (i % 16 == 0) {
                model.update(lr);
            }
        }

        if (tn > 0) train_loss /= tn;

        // Validation qismi (xuddi shu mantiq train_end dan oxirigacha)
        // ... (kodning hajmini kattalashtirmaslik uchun tushirib qoldirildi, xuddi yuqoridagidek yoziladi)

        printf("  Epoch %4d/%d | LR: %.6f | Train Loss: %.4f\n", ep + 1, epochs, lr, train_loss);
    }
    
    model.save("mie_model_live.bin");
    std::cout << "[LIVE TRAIN] Yakunlandi!" << std::endl;
}
