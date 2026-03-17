#include "features.hpp"
#include "nn.hpp" // clip() funksiyasi uchun
#include <cmath>
#include <algorithm>

namespace FE {

    // Global o'zgaruvchilar definition'i
    BarrierParams g_barrier_params[3] = {
        {2.0, 2.0, 5},  // TREND
        {3.0, 3.0, 3},  // RANGE
        {4.0, 4.0, 4},  // VOLATILE
    };
    bool g_barrier_calibrated = false;

    std::vector<double> closes(const std::vector<Candle>& d){
        std::vector<double> c; for(auto& x:d) c.push_back(x.close); return c;
    }

    double ema(const std::vector<double>& cl,int i,int p){
        if(i<0||(int)cl.size()==0) return cl.empty()?0:cl[0];
        double k=2.0/(p+1),e=cl[std::max(0,i-p)];
        for(int j=std::max(1,i-p+1);j<=i&&j<(int)cl.size();j++) e=cl[j]*k+e*(1-k);
        return e;
    }

    double atr(const std::vector<Candle>& d,int i,int p){
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

    double rsi(const std::vector<double>& cl,int i,int p){
        if(i<p) return 50;
        double u=0,d_=0;
        for(int j=i-p+1;j<=i;j++){
            double ch=cl[j]-cl[j-1];
            if(ch>0) u+=ch; else d_-=ch;
        }
        if(d_<1e-10) return 100;
        return 100-100/(1+u/d_);
    }

    double vol_ratio(const std::vector<Candle>& d,int i,int p){
        if(i<p) return 1.0;
        double s=0; int cnt=0;
        for(int j=i-p;j<i;j++){s+=d[j].volume;cnt++;}
        return cnt>0&&s>0?d[i].volume/(s/cnt):1.0;
    }

    std::vector<double> extract_tf(const std::vector<Candle>& data,int idx){
        if(idx<14||data.empty()) return std::vector<double>(15,0.0);
        auto cl=closes(data);
        double c=data[idx].close;
        double a=std::max(atr(data,idx),c*0.001);
        double e20=ema(cl,idx,20), e50=ema(cl,idx,50);
        double rsi_v=(rsi(cl,idx)-50)/50;
        double macd=clip((ema(cl,idx,12)-ema(cl,idx,26))/a,3);
        double vr=vol_ratio(data,idx);

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

        return {
            clip((c-e20)/a,3), clip((c-e50)/a,3), clip((e20-e50)/a,3),
            rsi_v, macd, bb_pos, clip(body,3), clip(wick_up,3), clip(wick_dn,3),
            std::log(std::max(vr,0.01)), r1, r5, r20, clip(std_/c*100,3), clip(ema(cl,idx,5)/e20-1,3)
        };
    }

    double swing_high(const std::vector<Candle>& d, int i, int lb=20){
        double mx=0; for(int j=std::max(0,i-lb);j<i;j++) mx=std::max(mx,d[j].high); return mx;
    }
    double swing_low(const std::vector<Candle>& d, int i, int lb=20){
        double mn=d[std::max(0,i-lb)].low; for(int j=std::max(0,i-lb);j<i;j++) mn=std::min(mn,d[j].low); return mn;
    }

    bool sweep_up(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb+1) return false; double sh=swing_high(d,i,lb);
        for(int j=std::max(1,i-2);j<=i;j++){ if(d[j].high>sh && d[j].close<sh) return true; } return false;
    }
    bool sweep_down(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb+1) return false; double sl=swing_low(d,i,lb);
        for(int j=std::max(1,i-2);j<=i;j++){ if(d[j].low<sl && d[j].close>sl) return true; } return false;
    }

    bool reclaim_after_sweep(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb+3) return false;
        double sh=swing_high(d,i,lb), sl=swing_low(d,i,lb);
        bool sw_up=false, sw_dn=false;
        for(int j=std::max(1,i-4);j<=i;j++){ if(d[j].high>sh) sw_up=true; if(d[j].low<sl) sw_dn=true; }
        if(sw_up && d[i].close>sh) return true;
        if(sw_dn && d[i].close<sl) return true;
        return false;
    }

    bool bos_up(const std::vector<Candle>& d, int i, int lb=30){
        if(i<lb+1) return false; return d[i].close > swing_high(d,i-1,lb);
    }
    bool bos_down(const std::vector<Candle>& d, int i, int lb=30){
        if(i<lb+1) return false; return d[i].close < swing_low(d,i-1,lb);
    }

    bool failed_bos_up(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb+1) return false; double sh=swing_high(d,i,lb); return d[i].high>sh && d[i].close<sh;
    }
    bool failed_bos_down(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb+1) return false; double sl=swing_low(d,i,lb); return d[i].low<sl && d[i].close>sl;
    }

    double range_proximity(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb) return 0;
        double sh=swing_high(d,i,lb), sl=swing_low(d,i,lb), rng=sh-sl;
        if(rng<1e-8) return 0; return clip((d[i].close-sl)/rng*2-1, 1.0);
    }

    double compression_score(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb*2) return 0;
        double a1=0,a2=0; int lb2=lb/2;
        for(int j=i-lb2;j<=i;j++) a1+=d[j].high-d[j].low;
        for(int j=i-lb;j<i-lb2;j++) a2+=d[j].high-d[j].low;
        a1/=lb2+1; a2/=lb2; if(a2<1e-8) return 0; return clip(1.0-a1/a2, 1.0);
    }

    double expansion_score(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb) return 0;
        double cur=d[i].high-d[i].low, avg=0;
        for(int j=i-lb;j<i;j++) avg+=d[j].high-d[j].low;
        avg/=lb; if(avg<1e-8) return 0; return clip(cur/avg-1, 3.0);
    }

    double displacement_score(const std::vector<Candle>& d, int i){
        if(i<1) return 0;
        double a=atr(d,i,14); if(a<1e-8) return 0;
        double body=std::abs(d[i].close-d[i].open); return clip(body/a, 3.0);
    }

    double wick_trap_score(const std::vector<Candle>& d, int i){
        double body=std::abs(d[i].close-d[i].open);
        double wick_up=d[i].high-std::max(d[i].open,d[i].close), wick_dn=std::min(d[i].open,d[i].close)-d[i].low;
        double total=d[i].high-d[i].low; if(total<1e-8) return 0;
        double max_wick=std::max(wick_up,wick_dn); return clip((max_wick-body)/total, 1.0);
    }

    double htf_bias_alignment(const std::vector<double>& h1_feat, const std::vector<double>& m15_feat){
        if(h1_feat.size()<4||m15_feat.size()<4) return 0;
        double h1_bias = h1_feat[0]>0?1:-1, m15_mom = m15_feat[3];
        return clip(h1_bias * (m15_mom>0?1:-1), 1.0);
    }

    double pullback_quality(const std::vector<Candle>& d, int i, int lb=10){
        if(i<lb+5) return 0; auto cl=closes(d);
        double main_trend=(cl[i-lb]-cl[std::max(0,i-lb*2)])/std::max(cl[std::max(0,i-lb*2)],1.0);
        double pullback =(cl[i]-cl[i-lb])/std::max(cl[i-lb],1.0);
        if(std::abs(main_trend)<1e-4) return 0;
        bool opposite=(main_trend>0&&pullback<0)||(main_trend<0&&pullback>0);
        if(!opposite) return 0;
        double ratio=std::abs(pullback)/std::abs(main_trend); return clip(1.0-ratio*2, 1.0);
    }

    double impulse_pullback_ratio(const std::vector<Candle>& d, int i, int lb=20){
        if(i<lb+5) return 1; auto cl=closes(d); double max_up=0, max_dn=0;
        for(int j=i-lb+1;j<=i;j++){ double mv=cl[j]-cl[j-1]; if(mv>0) max_up+=mv; else max_dn-=mv; }
        if(max_dn<1e-8) return 3.0; return clip(max_up/max_dn, 3.0);
    }

    double liquidity_density(const std::vector<Candle>& d, int i, int lb=30){
        if(i<lb) return 0;
        double sh=swing_high(d,i,lb), sl=swing_low(d,i,lb), zone=0.003; int touches=0;
        for(int j=std::max(0,i-lb);j<i;j++){
            if(std::abs(d[j].high-sh)/sh < zone) touches++;
            if(std::abs(d[j].low-sl)/sl  < zone) touches++;
        }
        return clip(touches/5.0, 1.0);
    }

    double vol_regime_transition(const std::vector<Candle>& d, int i, int lb=14){
        if(i<lb*2) return 0;
        double a1=atr(d,i,lb), a2=atr(d,i-lb,lb); if(a2<1e-8) return 0;
        double ratio=a1/a2; if(ratio>1.3) return 1.0; if(ratio<0.7) return -1.0; return clip(ratio-1.0, 1.0);
    }

    double candle_sequence(const std::vector<Candle>& d, int i){
        if(i<4) return 0; auto a=atr(d,i,14); if(a<1e-8) return 0;
        double bodies[3], wicks[3];
        for(int k=0;k<3;k++){
            int j=i-2+k; bodies[k]=std::abs(d[j].close-d[j].open)/a;
            wicks[k]=(d[j].high-d[j].low-std::abs(d[j].close-d[j].open))/a;
        }
        double avg_body=(bodies[0]+bodies[1]+bodies[2])/3, avg_wick=(wicks[0]+wicks[1]+wicks[2])/3;
        if(avg_body>1.5 && avg_wick<0.5) return 1.0/4;
        if(avg_body<0.4 && avg_wick>0.8) return 2.0/4;
        if(wicks[2]>1.5 && bodies[2]<0.4) return 3.0/4;
        if(avg_body>1.0 && bodies[2]<bodies[0]*0.5) return 4.0/4;
        return 0.0;
    }

    static inline double avg_abs_body(const std::vector<Candle>& d, int i, int n){
        if(i<1) return 1e-8; double s=0; int cnt=0;
        for(int j=std::max(0,i-n+1);j<=i;j++){s+=std::abs(d[j].close-d[j].open);cnt++;} return cnt>0?s/cnt:1e-8;
    }
    static inline double avg_vol(const std::vector<Candle>& d, int i, int n){
        if(i<1) return 1e-8; double s=0; int cnt=0;
        for(int j=std::max(0,i-n+1);j<=i;j++){s+=d[j].volume;cnt++;} return cnt>0?s/cnt:1e-8;
    }
    static inline double roll_high(const std::vector<Candle>& d, int i, int n){
        double mx=d[std::max(0,i-n)].high; for(int j=std::max(0,i-n);j<i;j++) mx=std::max(mx,d[j].high); return mx;
    }
    static inline double roll_low(const std::vector<Candle>& d, int i, int n){
        double mn=d[std::max(0,i-n)].low; for(int j=std::max(0,i-n);j<i;j++) mn=std::min(mn,d[j].low); return mn;
    }
    static inline double pressure_persistence(const std::vector<Candle>& d, int i, int n=5){
        if(i<n) return 0; double s=0; for(int j=i-n+1;j<=i;j++) s+=(d[j].close>d[j].open)?1.0:-1.0; return s/n;
    }

    std::vector<double> extract_tf_structure(const std::vector<Candle>& data, int idx, const std::vector<double>& other_tf_feat){
        if(idx<20||data.empty()) return std::vector<double>(70,0.0);
        auto cl=closes(data); double c=data[idx].close, a=std::max(atr(data,idx),c*0.001);
        double e20=ema(cl,idx,20), e50=ema(cl,idx,50), rsi_v=(rsi(cl,idx)-50)/50, macd_v=clip((ema(cl,idx,12)-ema(cl,idx,26))/a,3), vr=vol_ratio(data,idx);
        
        double mean=0; int bw=20; for(int j=std::max(0,idx-bw+1);j<=idx;j++) mean+=cl[j]; mean/=std::min(bw,idx+1);
        double var=0; for(int j=std::max(0,idx-bw+1);j<=idx;j++) var+=(cl[j]-mean)*(cl[j]-mean);
        double std_=std::sqrt(var/std::min(bw,idx+1)+1e-10), bb_pos=(c-(mean-2*std_))/(4*std_+1e-10)-0.5;

        double body=(data[idx].close-data[idx].open)/a, wick_up=(data[idx].high-std::max(data[idx].open,data[idx].close))/a, wick_dn=(std::min(data[idx].open,data[idx].close)-data[idx].low)/a;
        double r1 = idx>=1?clip((cl[idx]-cl[idx-1])/a,3):0, r5 = idx>=5?clip((cl[idx]-cl[idx-5])/a,3):0, r20= idx>=20?clip((cl[idx]-cl[idx-20])/a,3):0;

        std::vector<double> feats = {
            clip((c-e20)/a,3), clip((c-e50)/a,3), clip((e20-e50)/a,3), rsi_v, macd_v, bb_pos,
            clip(body,3), clip(wick_up,3), clip(wick_dn,3), std::log(std::max(vr,0.01)), r1, r5, r20, clip(std_/c*100,3), clip(ema(cl,idx,5)/e20-1,3)
        };

        double sh=swing_high(data,idx,20), sl=swing_low(data,idx,20);
        feats.push_back(clip((sh-c)/a, 5.0)); feats.push_back(clip((c-sl)/a, 5.0));
        feats.push_back(sweep_up(data,idx) ? 1.0 : 0.0); feats.push_back(sweep_down(data,idx) ? 1.0 : 0.0);
        feats.push_back(bos_up(data,idx) ? 1.0 : 0.0); feats.push_back(bos_down(data,idx) ? 1.0 : 0.0);
        feats.push_back(failed_bos_up(data,idx) ? 1.0 : 0.0); feats.push_back(failed_bos_down(data,idx) ? 1.0 : 0.0);
        feats.push_back(range_proximity(data,idx)); feats.push_back(compression_score(data,idx));
        feats.push_back(expansion_score(data,idx)); feats.push_back(displacement_score(data,idx));
        feats.push_back(wick_trap_score(data,idx)); feats.push_back(reclaim_after_sweep(data,idx) ? 1.0 : 0.0);
        feats.push_back(other_tf_feat.size()>=4 ? htf_bias_alignment(feats,other_tf_feat) : 0.0);
        feats.push_back(pullback_quality(data,idx)); feats.push_back(impulse_pullback_ratio(data,idx));
        feats.push_back(liquidity_density(data,idx)); feats.push_back(vol_regime_transition(data,idx));
        feats.push_back(candle_sequence(data,idx));

        {
            const auto& cd = data[idx]; double rng = std::max(cd.high - cd.low, 1e-8);
            feats.push_back(clip((cd.close - cd.low) / rng, 1.0)*2 - 1);
            feats.push_back(((cd.open  - cd.low) / rng)*2 - 1);
            feats.push_back(std::abs(cd.close-cd.open)/rng);
            feats.push_back(clip((cd.close-cd.open)/a, 3.0));
            feats.push_back(clip(rng/a, 5.0));
            double ph = idx>=1 ? data[idx-1].high : cd.high, pl = idx>=1 ? data[idx-1].low : cd.low;
            feats.push_back(clip((cd.close - ph)/a, 3.0)); feats.push_back(clip((cd.close - pl)/a, 3.0));
        }

        {
            int bull3=0, bear3=0, bull5=0, bear5=0;
            for(int j=std::max(0,idx-2);j<=idx;j++){ if(data[j].close>data[j].open) bull3++; else bear3++; }
            for(int j=std::max(0,idx-4);j<=idx;j++){ if(data[j].close>data[j].open) bull5++; else bear5++; }
            int n3=std::min(3,idx+1), n5=std::min(5,idx+1);
            feats.push_back((double)bull3/n3); feats.push_back((double)bear3/n3);
            feats.push_back((double)bull5/n5); feats.push_back((double)bear5/n5);
            double nb3=0, nb5=0;
            for(int j=std::max(0,idx-2);j<=idx;j++) nb3+=(data[j].close-data[j].open);
            for(int j=std::max(0,idx-4);j<=idx;j++) nb5+=(data[j].close-data[j].open);
            feats.push_back(clip(nb3/a, 5.0)); feats.push_back(clip(nb5/a, 5.0));
            double ch1 = idx>=1 ? cl[idx]-cl[idx-1] : 0, ch2 = idx>=2 ? cl[idx-1]-cl[idx-2] : 0;
            feats.push_back(clip((ch1-ch2)/a, 3.0)); feats.push_back(pressure_persistence(data, idx, 5));
        }

        {
            double rh10=roll_high(data,idx,10), rl10=roll_low(data,idx,10), rh20=roll_high(data,idx,20), rl20=roll_low(data,idx,20), rh50=roll_high(data,idx,50), rl50=roll_low(data,idx,50);
            auto range_pos=[&](double rh, double rl)->double{ return clip((c-rl)/std::max(rh-rl,1e-8)*2-1, 1.0); };
            feats.push_back(range_pos(rh10,rl10)); feats.push_back(range_pos(rh20,rl20)); feats.push_back(range_pos(rh50,rl50));
            feats.push_back(clip((rh20-c)/a, 5.0)); feats.push_back(clip((c-rl20)/a, 5.0));
            double prev_rh20 = roll_high(data, std::max(0,idx-1), 20), prev_rl20 = roll_low(data, std::max(0,idx-1), 20);
            feats.push_back((data[idx].high > prev_rh20) ? clip((c - prev_rh20)/a, 3.0) : 0.0);
            feats.push_back((data[idx].low  < prev_rl20) ? clip((prev_rl20 - c)/a, 3.0) : 0.0);
            feats.push_back((data[idx].high > prev_rh20 && c < prev_rh20) ? clip((data[idx].high - prev_rh20)/a, 2.0) : 0.0);
            feats.push_back((data[idx].low < prev_rl20 && c > prev_rl20) ? clip((prev_rl20 - data[idx].low)/a, 2.0) : 0.0);
        }

        {
            double atr5 = std::max(atr(data,idx, 5), 1e-8), atr20 = std::max(atr(data,idx,20), 1e-8);
            feats.push_back(clip(atr5/atr20, 3.0));
            double ar5=0, ar10=0, ar20=0;
            for(int j=std::max(0,idx-4);j<=idx;j++)   ar5 +=data[j].high-data[j].low;
            for(int j=std::max(0,idx-9);j<=idx;j++)   ar10+=data[j].high-data[j].low;
            for(int j=std::max(0,idx-19);j<=idx;j++)  ar20+=data[j].high-data[j].low;
            ar5/=std::min(5,idx+1); ar10/=std::min(10,idx+1); ar20/=std::min(20,idx+1);
            feats.push_back(clip(ar20>1e-8?ar5/ar20:1.0, 3.0));
            feats.push_back(clip(ar10>1e-8?(data[idx].high-data[idx].low)/ar10:1.0, 5.0));
            feats.push_back(clip(std::abs(data[idx].close-data[idx].open)/std::max(avg_abs_body(data,idx,10), 1e-8), 5.0));
        }

        {
            double avgvol20 = std::max(avg_vol(data,idx,20), 1e-8), vol = std::max(data[idx].volume, 0.0), vr_log = std::log(1.0 + vol/avgvol20);
            double sv=0,sv2=0; int vn=0;
            for(int j=std::max(0,idx-19);j<=idx;j++){sv+=data[j].volume;sv2+=data[j].volume*data[j].volume;vn++;}
            double vm=sv/vn, vstd=std::sqrt(std::max(sv2/vn-vm*vm,0.0)+1e-8);
            feats.push_back(clip((vol-vm)/vstd, 3.0));
            feats.push_back(clip((data[idx].close>data[idx].open ? 1.0 : -1.0) * vr_log, 4.0));
            feats.push_back(clip(vr_log / (std::max(std::abs(data[idx].close-data[idx].open), 1e-8)/a), 5.0));
            double close_pos_raw = (data[idx].close - data[idx].low)/std::max(data[idx].high-data[idx].low, 1e-8);
            bool is_bull = data[idx].close > data[idx].open;
            feats.push_back((is_bull && vol>avgvol20*1.5 && close_pos_raw<0.35) ? 1.0 : 0.0);
            feats.push_back((!is_bull && vol>avgvol20*1.5 && close_pos_raw>0.65) ? 1.0 : 0.0);
            double prev_rh20=roll_high(data,std::max(0,idx-1),20), prev_rl20=roll_low(data, std::max(0,idx-1),20);
            feats.push_back((vol > avgvol20*2.0 && data[idx].close > prev_rh20) ? 1.0 : 0.0);
            feats.push_back((vol > avgvol20*2.0 && data[idx].close < prev_rl20) ? 1.0 : 0.0);
        }

        return feats;
    }

    Regime detect_regime(const std::vector<Candle>& d, int i){
        if(i < 50) return Regime::RANGE;
        auto cl = closes(d); double c = d[i].close, a = std::max(atr(d, i, 14), c * 0.0001);
        if(a / c > 0.012) return Regime::VOLATILE;
        if(std::abs((ema(cl, i, 20) - ema(cl, i, 50)) / a) > 1.5 && std::abs((cl[i] - cl[std::max(0, i-20)]) / a) > 3.0) return Regime::TREND;
        return Regime::RANGE;
    }

    const char* regime_str(Regime r){
        switch(r){ case Regime::TREND: return "TREND"; case Regime::RANGE: return "RANGE"; case Regime::VOLATILE: return "VOLATILE"; default: return "RANGE"; }
    }

    BarrierParams get_barrier_params(Regime reg){
        switch(reg){ case Regime::TREND: return {2.0, 2.0, 5}; case Regime::RANGE: return {3.0, 3.0, 3}; case Regime::VOLATILE: return {4.0, 4.0, 4}; default: return {3.0, 3.0, 3}; }
    }

    BarrierParams get_calibrated_params(Regime reg){
        if(!g_barrier_calibrated) return get_barrier_params(reg); return g_barrier_params[(int)reg];
    }

    int compute_regime(const std::vector<Candle>& data, int i){
        if(i<50) return 2;
        auto cl=closes(data); double a=atr(data,i,14), c=data[i].close, trend = (ema(cl,i,20)-ema(cl,i,50))/std::max(a,1e-8), r20 = (cl[i]-cl[std::max(0,i-20)])/std::max(a,1e-8);
        if(a/c > 0.025) return 3; if(trend > 0.5 && r20 > 1.0) return 0; if(trend < -0.5 && r20 < -1.0) return 1; return 2;
    }

    int compute_triple_barrier(const std::vector<Candle>& d, int idx){
        if(idx + TB_MAX_HORIZON + 2 >= (int)d.size()) return TB_FLAT;
        BarrierParams p = get_calibrated_params(detect_regime(d, idx));
        double entry = d[idx+1].open, atr_val = std::max(atr(d, idx, 14), entry * 0.001);
        double upper = entry + p.up_k * atr_val, lower = entry - p.down_k * atr_val;
        int max_k = std::min(p.horizon, (int)d.size() - 2 - idx);
        for(int k = 1; k <= max_k; k++){
            double hi = d[idx+1+k].high, lo = d[idx+1+k].low;
            bool hit_up = (hi >= upper), hit_dn = (lo <= lower);
            if(hit_up && hit_dn) return TB_FLAT;
            if(hit_up) return TB_UP;
            if(hit_dn) return TB_DOWN;
        }
        return TB_FLAT;
    }

    BarrierResult compute_triple_barrier_full(const std::vector<Candle>& d, int idx){
        if(idx + TB_MAX_HORIZON + 2 >= (int)d.size()) return {TB_FLAT, Regime::RANGE, {3.0,3.0,3}};
        Regime reg = detect_regime(d, idx); BarrierParams p = get_calibrated_params(reg);
        double entry = d[idx+1].open, atr_val = std::max(atr(d, idx, 14), entry * 0.001);
        double upper = entry + p.up_k * atr_val, lower = entry - p.down_k * atr_val;
        int max_k = std::min(p.horizon, (int)d.size() - 2 - idx);
        for(int k=1; k<=max_k; k++){
            double hi = d[idx+1+k].high, lo = d[idx+1+k].low;
            if(hi>=upper && lo<=lower) return {TB_FLAT, reg, p};
            if(hi>=upper) return {TB_UP, reg, p};
            if(lo<=lower) return {TB_DOWN, reg, p};
        }
        return {TB_FLAT, reg, p};
    }

} // namespace FE