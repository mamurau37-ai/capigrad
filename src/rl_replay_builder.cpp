#include "rl_replay_builder.hpp"

#include <fstream>
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

int find_col(const std::unordered_map<std::string, int>& cols,
             const std::string& name) {
    auto it = cols.find(name);
    if (it == cols.end()) {
        throw std::runtime_error("Missing CSV column: " + name);
    }
    return it->second;
}

} // anonymous namespace

std::vector<LiveSignalCsvRow> load_live_signal_csv(const std::string& path) {
    std::ifstream fin(path);
    if (!fin.is_open()) {
        throw std::runtime_error("load_live_signal_csv failed to open file: " + path);
    }

    std::string header_line;
    if (!std::getline(fin, header_line)) {
        throw std::runtime_error("load_live_signal_csv empty csv: " + path);
    }

    std::vector<std::string> header = split_csv_line(header_line);
    std::unordered_map<std::string, int> cols;
    for (int i = 0; i < (int)header.size(); ++i) {
        cols[header[i]] = i;
    }

    const int c_timestamp          = find_col(cols, "timestamp");
    const int c_offline_pred       = find_col(cols, "offline_pred");
    const int c_p_down             = find_col(cols, "p_down");
    const int c_p_flat             = find_col(cols, "p_flat");
    const int c_p_up               = find_col(cols, "p_up");
    const int c_offline_conf       = find_col(cols, "offline_conf");
    const int c_spread_bps         = find_col(cols, "spread_bps");
    const int c_imbalance_top3     = find_col(cols, "imbalance_top3");
    const int c_imbalance_top10    = find_col(cols, "imbalance_top10");
    const int c_micro_gap_bps      = find_col(cols, "micro_gap_bps");
    const int c_signed_flow_short  = find_col(cols, "signed_flow_short");
    const int c_signed_flow_medium = find_col(cols, "signed_flow_medium");
    const int c_micro_score        = find_col(cols, "micro_score");
    const int c_final_score        = find_col(cols, "final_score");
    const int c_action             = find_col(cols, "action");
    const int c_reason             = find_col(cols, "reason");
    const int c_has_realized       = find_col(cols, "has_realized");
    const int c_realized_return    = find_col(cols, "realized_return");
    const int c_realized_net_ret   = find_col(cols, "realized_net_return");

    int max_idx = 0;
    for (auto& kv : cols) {
        if (kv.second > max_idx) max_idx = kv.second;
    }

    std::vector<LiveSignalCsvRow> out;
    std::string line;

    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::vector<std::string> f = split_csv_line(line);
        if ((int)f.size() <= max_idx) continue;

        LiveSignalCsvRow r;
        try {
            r.timestamp           = std::stoll(f[c_timestamp]);
            r.offline_pred        = std::stoi(f[c_offline_pred]);
            r.p_down              = std::stod(f[c_p_down]);
            r.p_flat              = std::stod(f[c_p_flat]);
            r.p_up                = std::stod(f[c_p_up]);
            r.offline_conf        = std::stod(f[c_offline_conf]);

            r.spread_bps          = std::stod(f[c_spread_bps]);
            r.imbalance_top3      = std::stod(f[c_imbalance_top3]);
            r.imbalance_top10     = std::stod(f[c_imbalance_top10]);
            r.micro_gap_bps       = std::stod(f[c_micro_gap_bps]);
            r.signed_flow_short   = std::stod(f[c_signed_flow_short]);
            r.signed_flow_medium  = std::stod(f[c_signed_flow_medium]);
            r.micro_score         = std::stod(f[c_micro_score]);
            r.final_score         = std::stod(f[c_final_score]);

            r.action              = std::stoi(f[c_action]);
            r.reason              = f[c_reason];
            r.has_realized        = std::stoi(f[c_has_realized]);
            r.realized_return     = std::stod(f[c_realized_return]);
            r.realized_net_return = std::stod(f[c_realized_net_ret]);
        } catch (...) {
            continue;
        }

        out.push_back(std::move(r));
    }

    return out;
}

std::vector<RLReplayRow> build_rl_replay_rows(
    const std::vector<LiveSignalCsvRow>& rows,
    const RLReplayBuilderConfig& cfg) {

    std::vector<RLReplayRow> out;
    out.reserve(rows.size());

    for (const auto& r : rows) {
        if (cfg.skip_unrealized_rows && r.has_realized == 0) {
            continue;
        }

        RLReplayRow x;
        x.timestamp = r.timestamp;

        x.p_down = r.p_down;
        x.p_flat = r.p_flat;
        x.p_up   = r.p_up;
        x.offline_conf = r.offline_conf;

        x.spread_bps = r.spread_bps;
        x.imbalance_top3 = r.imbalance_top3;
        x.imbalance_top10 = r.imbalance_top10;
        x.micro_gap_bps = r.micro_gap_bps;
        x.signed_flow_short = r.signed_flow_short;
        x.signed_flow_medium = r.signed_flow_medium;
        x.micro_score = r.micro_score;

        // Use offline_pred as a proxy label if you need one,
        // but actual RL reward comes from future_return below.
        x.true_label = r.offline_pred;

        x.future_return = cfg.use_realized_net_return
                        ? r.realized_net_return
                        : r.realized_return;

        out.push_back(std::move(x));
    }

    return out;
}

std::vector<RLReplayRow> load_rl_replay_rows(
    const std::string& csv_path,
    const RLReplayBuilderConfig& cfg) {
    std::vector<LiveSignalCsvRow> rows = load_live_signal_csv(csv_path);
    return build_rl_replay_rows(rows, cfg);
}

} // namespace capigrad
