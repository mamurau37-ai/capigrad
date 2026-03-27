#include "csv_reader.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace capigrad {

namespace {

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string item;

    while (std::getline(ss, item, ',')) {
        fields.push_back(item);
    }
    return fields;
}

int find_required_column(const std::unordered_map<std::string, int>& col_idx,
                         const std::string& name) {
    auto it = col_idx.find(name);
    if (it == col_idx.end()) {
        throw std::runtime_error("Missing required CSV column: " + name);
    }
    return it->second;
}

} // anonymous namespace

std::vector<Candle> load_candles_from_csv(const std::string& path) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        throw std::runtime_error("Failed to open CSV file: " + path);
    }

    std::string header_line;
    if (!std::getline(fin, header_line)) {
        throw std::runtime_error("CSV file is empty: " + path);
    }

    std::vector<std::string> header = split_csv_line(header_line);
    std::unordered_map<std::string, int> col_idx;
    for (int i = 0; i < static_cast<int>(header.size()); ++i) {
        col_idx[header[i]] = i;
    }

    const int ts_idx = find_required_column(col_idx, "timestamp");
    const int o_idx  = find_required_column(col_idx, "open");
    const int h_idx  = find_required_column(col_idx, "high");
    const int l_idx  = find_required_column(col_idx, "low");
    const int c_idx  = find_required_column(col_idx, "close");
    const int v_idx  = find_required_column(col_idx, "volume");

    const int max_idx = std::max({ts_idx, o_idx, h_idx, l_idx, c_idx, v_idx});

    std::vector<Candle> candles;
    candles.reserve(200000);

    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::vector<std::string> fields = split_csv_line(line);
        if ((int)fields.size() <= max_idx) {
            continue;
        }

        Candle x;
        try {
            x.timestamp = std::stoll(fields[ts_idx]);
            x.open      = std::stod(fields[o_idx]);
            x.high      = std::stod(fields[h_idx]);
            x.low       = std::stod(fields[l_idx]);
            x.close     = std::stod(fields[c_idx]);
            x.volume    = std::stod(fields[v_idx]);
        } catch (...) {
            continue; // malformed row skip
        }

        if (x.timestamp <= 0) continue;
        if (x.open <= 0.0 || x.high <= 0.0 || x.low <= 0.0 || x.close <= 0.0) continue;
        if (x.volume < 0.0) continue;

        candles.push_back(x);
    }

    return candles;
}

void print_candle_summary(const std::vector<Candle>& candles,
                          const std::string& tag) {
    std::cout << "[CSV][" << tag << "] rows=" << candles.size();
    if (!candles.empty()) {
        std::cout << " first_ts=" << candles.front().timestamp
                  << " last_ts=" << candles.back().timestamp;
    }
    std::cout << "\n";
}

} // namespace capigrad
