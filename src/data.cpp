#include "data.hpp"   // <-- Eng muhimi, Candle ni shu yerdan topib keladi
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

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
std::vector<LiveTick> load_live_log(const std::string& fn){
    std::ifstream f(fn);
    if(!f) {
        std::cerr << "[ERROR] " << fn << " faylini ochib bo'lmadi!" << std::endl;
        return {};
    }
    
    std::vector<LiveTick> d;
    std::string line;
    
    // Header qatorini o'qib tashlab yuboramiz
    std::getline(f, line); 
    
    while(std::getline(f, line)){
        if(line.empty()) continue; // Bo'sh qatorlarni o'tkazib yuborish
        
        std::istringstream ss(line);
        std::string tok; 
        LiveTick tick;
        
        try {
            std::getline(ss, tok, ','); tick.ts = std::stoll(tok);
            std::getline(ss, tok, ','); tick.price = std::stod(tok);
            std::getline(ss, tok, ','); tick.spread_bps = std::stod(tok);
            std::getline(ss, tok, ','); tick.imb = std::stod(tok);
            std::getline(ss, tok, ','); tick.ti60 = std::stod(tok);
            std::getline(ss, tok, ','); tick.cvd60 = std::stod(tok);
            std::getline(ss, tok, ','); tick.cls_up = std::stod(tok);
            std::getline(ss, tok, ','); tick.cls_dn = std::stod(tok);
            std::getline(ss, tok, ','); tick.cls_fl = std::stod(tok);
            std::getline(ss, tok, ','); tick.clarity = std::stod(tok);
            std::getline(ss, tok, ','); tick.regime = tok; // String
            std::getline(ss, tok, ','); tick.gate = tok;   // String
            std::getline(ss, tok, ','); tick.action = tok; // String
            
            // Agar meta_prob ustuni mavjud bo'lsa
            if(std::getline(ss, tok, ',')) {
                tick.meta_prob = std::stod(tok);
            } else {
                tick.meta_prob = 0.5; // Default qiymat
            }
            
            d.push_back(tick);
        } catch (const std::exception& e) {
            // Agar qaysidir qatorda xatolik bo'lsa, logga yozib o'tkazib yuboramiz
            // std::cerr << "Parse error at ts: " << tick.ts << " - " << e.what() << std::endl;
        }
    }
    
    std::cout << "[DATA] " << fn << " faylidan " << d.size() << " ta qator o'qildi.\n";
    return d;
}