#ifndef CAPIGRAD_BOOTSTRAP_CONTEXT_HPP
#define CAPIGRAD_BOOTSTRAP_CONTEXT_HPP

#include <string>
#include <vector>

#include "context_builder.hpp"
#include "csv_reader.hpp"

namespace capigrad {

// -----------------------------------------------------------------------------
// BootstrapConfig:
// how many candles to preload from historical raw CSV
// -----------------------------------------------------------------------------
struct BootstrapConfig {
    // Normally should be >= sequence lengths + feature warmup margin
    int m15_bootstrap_count = 160;
    int h1_bootstrap_count  = 96;
};

// -----------------------------------------------------------------------------
// BootstrapResult:
// stats for logging / debugging
// -----------------------------------------------------------------------------
struct BootstrapResult {
    int loaded_m15 = 0;
    int loaded_h1  = 0;
    bool ready_after_bootstrap = false;
};

// -----------------------------------------------------------------------------
// Load last N candles from raw CSV, push into ContextBuilder,
// so offline model can start immediately in live mode.
// -----------------------------------------------------------------------------
BootstrapResult bootstrap_context_from_csv(
    ContextBuilder& builder,
    const std::string& m15_csv_path,
    const std::string& h1_csv_path,
    const BootstrapConfig& cfg = BootstrapConfig());

} // namespace capigrad

#endif // CAPIGRAD_BOOTSTRAP_CONTEXT_HPP
