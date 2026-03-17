#include "data.hpp"
#include "model.hpp"
#include "live.hpp"
#include "trainer.hpp"
#include "features.hpp"
#include "globals.hpp"

#include <iostream>
#include <csignal>
#include <string>
#include <vector>

std::atomic<bool> g_running{true};
void signal_handler(int){ g_running=false; std::cout<<"\n[STOP]\n"; }

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::string mode = argc > 1 ? argv[1] : "--help";

    // LiveConfig ni o'qish
    LiveConfig lcfg;
    for(int i=2;i<argc;i++){
        std::string a=argv[i];
        auto nxt=[&](const char* d)->std::string{return(i+1<argc)?argv[++i]:d;};
        if     (a=="--symbol")       lcfg.symbol      =nxt("btcusdt");
        else if(a=="--market")       lcfg.market      =nxt("spot");
        else if(a=="--depth")        lcfg.depth       =std::stoi(nxt("20"));
        else if(a=="--book_ms")      lcfg.book_ms     =std::stoi(nxt("100"));
        else if(a=="--meta_on")      lcfg.meta_on     =std::stoi(nxt("1"))!=0;
        else if(a=="--meta_lr")      lcfg.meta_lr     =std::stod(nxt("0.05"));
        else if(a=="--meta_buf")     lcfg.meta_buf    =std::stoi(nxt("20000"));
        else if(a=="--meta_horizon") lcfg.meta_horizon=std::stoi(nxt("5"));
        else if(a=="--meta_k")       lcfg.meta_k      =std::stod(nxt("1.5"));
        else if(a=="--threshold")    lcfg.threshold   =std::stod(nxt("0.35"));
        else if(a=="--min_clarity")  lcfg.min_clarity =std::stod(nxt("0.50"));
        else if(a=="--save_live")    lcfg.save_live   =nxt("live_log.csv");
        else if(a=="--real")         lcfg.use_mock    =false;
        else if(a=="--mock")         lcfg.use_mock    =true;
    }
    for(auto& c:lcfg.symbol) c=tolower(c);

    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║   CapiGrad Market Intelligence Engine v4.1          ║\n");
    printf("║   Zanerhon's BTC Market Analyzer                    ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║   Maqsad: Savdo bot EMAS — Bozor analizatori        ║\n");
    printf("║   Output: Regime + Trend + Momentum + Live Signal   ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    std::vector<Candle> d1h, d15m;
    for(auto& fn:std::vector<std::string>{"btcusdt_1h_5year.csv","btcusdt_1h_365d.csv","btc_1h_1year.csv"}){
        d1h=load_csv(fn); if(!d1h.empty()){printf("[H1] %s (%zu sham)\n",fn.c_str(),d1h.size());break;}
    }
    for(auto& fn:std::vector<std::string>{"btcusdt_15m_5year.csv","btcusdt_15m_365d.csv","btcusdt_15m.csv"}){
        d15m=load_csv(fn); if(!d15m.empty()){printf("[M15] %s (%zu sham)\n",fn.c_str(),d15m.size());break;}
    }
    MIEModel model("MIE-v4");

    if(mode=="--train"){
        if(d1h.empty()){printf("[!] H1 data topilmadi\n");return 1;}
        printf("\n[MODE] O'qitish\n");
        train_mie(model,d1h,d15m,500,0.0001);
    }
    else if(mode=="--eval"){
        printf("\n[MODE] Baholash\n");
        if(!model.load("mie_best.bin")&&!model.load("mie_model.bin")){printf("[!] Model yo'q — avval --train\n");return 1;}
        if(d1h.empty()){printf("[!] H1 data topilmadi\n");return 1;}
        eval_mie(model,d1h,d15m);
    }
    else if(mode=="--live"){
        printf("\n[MODE] LIVE v2\n");
        if(!model.load("mie_best.bin")&&!model.load("mie_model.bin")){printf("[!] Model yo'q — avval --train\n");return 1;}
        run_live_mode_v2(model,d1h,d15m,lcfg);
    }
    else if(mode=="--live-eval"){
        printf("\n[MODE] LIVE-EVAL\n");
        if(!model.load("mie_best.bin")&&!model.load("mie_model.bin")){printf("[!] Model yo'q — avval --train\n");return 1;}
        if(d1h.empty()){printf("[!] H1 data topilmadi\n");return 1;}
        run_live_eval(model,d1h,d15m,lcfg);
    }
    else {
        printf("\nISHLATISH:\n");
        printf("  ./mie --train                         Model o'qitish\n");
        printf("  ./mie --eval                          Tahlil sifatini o'lchash\n");
        printf("  ./mie --live                          Live signal (mock)\n");
        printf("  ./mie --live --real                   Binance WS\n");
        printf("  ./mie --live-eval                     Pseudo-live backtest\n");
    }
    return 0;
}