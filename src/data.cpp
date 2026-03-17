#include "data.hpp"   // <-- Eng muhimi, Candle ni shu yerdan topib keladi
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

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