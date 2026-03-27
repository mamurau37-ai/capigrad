#include "bootstrap_context.hpp"

#include <algorithm>
#include <stdexcept>

namespace capigrad {

BootstrapResult bootstrap_context_from_csv(
    ContextBuilder& builder,
    const std::string& m15_csv_path,
    const std::string& h1_csv_path,
    const BootstrapConfig& cfg) {

    std::vector<Candle> m15_all = load_candles_from_csv(m15_csv_path);
    std::vector<Candle> h1_all  = load_candles_from_csv(h1_csv_path);

    if (m15_all.empty()) {
        throw std::runtime_error("bootstrap_context_from_csv: empty M15 CSV");
    }
    if (h1_all.empty()) {
        throw std::runtime_error("bootstrap_context_from_csv: empty H1 CSV");
    }

    const int m15_take = std::min((int)m15_all.size(), cfg.m15_bootstrap_count);
    const int h1_take  = std::min((int)h1_all.size(),  cfg.h1_bootstrap_count);

    const int m15_start = (int)m15_all.size() - m15_take;
    const int h1_start  = (int)h1_all.size()  - h1_take;

    builder.reset();

    BootstrapResult out;

    for (int i = m15_start; i < (int)m15_all.size(); ++i) {
        builder.on_m15_candle(m15_all[i]);
        out.loaded_m15++;
    }

    for (int i = h1_start; i < (int)h1_all.size(); ++i) {
        builder.on_h1_candle(h1_all[i]);
        out.loaded_h1++;
    }

    out.ready_after_bootstrap = builder.ready();
    return out;
}

} // namespace capigrad
