#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "live_engine.hpp"
#include "sequence_builder.hpp"

using namespace capigrad;

namespace {

const char* action_to_string(TradeAction a) {
    switch (a) {
        case TradeAction::LONG:     return "LONG";
        case TradeAction::SHORT:    return "SHORT";
        case TradeAction::NO_TRADE: return "NO_TRADE";
        default:                    return "UNKNOWN";
    }
}

void print_probs(const std::vector<double>& probs) {
    std::cout << "[probs] ";
    for (size_t i = 0; i < probs.size(); ++i) {
        std::cout << std::fixed << std::setprecision(4)
                  << probs[i] << (i + 1 == probs.size() ? "" : ", ");
    }
    std::cout << "\n";
}

OrderBookSnapshot build_demo_book(std::int64_t ts,
                                  double mid,
                                  double spread_bps,
                                  double bid_size1,
                                  double ask_size1,
                                  double bid_mult = 1.0,
                                  double ask_mult = 1.0) {
    OrderBookSnapshot ob;
    ob.timestamp = ts;

    double spread = mid * spread_bps / 10000.0;
    double bb = mid - spread * 0.5;
    double ba = mid + spread * 0.5;

    for (int i = 0; i < 10; ++i) {
        double step = mid * 0.0001 * (i + 1);
        ob.bids.push_back(BookLevel{bb - step, bid_size1 * bid_mult / (1.0 + i * 0.2)});
        ob.asks.push_back(BookLevel{ba + step, ask_size1 * ask_mult / (1.0 + i * 0.2)});
    }

    return ob;
}

TradeTick build_demo_trade(std::int64_t ts, double price, double size, bool is_buy) {
    TradeTick tr;
    tr.timestamp = ts;
    tr.price = price;
    tr.size = size;
    tr.is_buy = is_buy;
    return tr;
}

void print_live_features(const LiveFeatureVector& fv) {
    if (fv.values.size() < LiveFeatureSchema::DIM) {
        std::cout << "[live] feature vector incomplete\n";
        return;
    }

    std::cout << "[live] spread_bps=" << fv.values[LiveFeatureSchema::SPREAD_BPS]
              << " imb_top3=" << fv.values[LiveFeatureSchema::IMBALANCE_TOP3]
              << " imb_top10=" << fv.values[LiveFeatureSchema::IMBALANCE_TOP10]
              << " micro_gap_bps=" << fv.values[LiveFeatureSchema::MICROPRICE_GAP_BPS]
              << " signed_flow_short=" << fv.values[LiveFeatureSchema::SIGNED_FLOW_SHORT]
              << " signed_flow_medium=" << fv.values[LiveFeatureSchema::SIGNED_FLOW_MEDIUM]
              << " pressure_short=" << fv.values[LiveFeatureSchema::BOOK_PRESSURE_SHORT]
              << "\n";
}

} // anonymous namespace

int main() {
    try {
        const std::string model_path = "../artifacts/models/offline_best.bin";
        const std::string test_path  = "../data/processed/test.bin";

        std::cout << "[1/6] Loading replay dataset...\n";
        OfflineDataset test_ds = load_dataset_binary(test_path);

        if (test_ds.empty()) {
            throw std::runtime_error("Replay dataset is empty.");
        }

        const int m15_input_dim = static_cast<int>(test_ds.samples[0].m15_seq[0].size());
        const int h1_input_dim  = static_cast<int>(test_ds.samples[0].h1_seq[0].size());

        std::cout << "[2/6] Building offline model template...\n";
        OfflineDualTFModel offline_template(
            m15_input_dim,
            h1_input_dim,
            32, // m15 embed
            24, // h1 embed
            48, // m15 hidden
            32, // h1 hidden
            32  // fusion hidden
        );
        offline_template.load(model_path);

        std::cout << "[3/6] Creating live engine...\n";
        LiveFeatureConfig live_cfg;
        FusionConfig fusion_cfg;
        fusion_cfg.offline_long_threshold  = 0.55;
        fusion_cfg.offline_short_threshold = 0.55;
        fusion_cfg.max_spread_bps = 10.0;
        fusion_cfg.min_micro_long_score  = 0.02;
        fusion_cfg.min_micro_short_score = -0.02;

        LiveEngineConfig engine_cfg;
        engine_cfg.use_offline_replay_context = true;
        engine_cfg.require_live_ready = true;

        LiveEngine engine(offline_template, live_cfg, fusion_cfg, engine_cfg);

        std::cout << "[4/6] Starting replay/demo loop...\n";

        int processed = 0;
        int long_count = 0;
        int short_count = 0;
        int no_trade_count = 0;

        for (const auto& sample : test_ds.samples) {
            engine.reset();
            engine.set_offline_context(sample);

            // Offline prediction for demo market shaping
            OfflineSignal off = engine.infer_offline_signal();

            // Synthetic live market generation (replace later with real feed)
            double base_mid = 50000.0 + std::fmod((double)sample.timestamp / 1000.0, 10000.0);

            double spread_bps = 5.0;
            double bid_mult = 1.0;
            double ask_mult = 1.0;

            if (off.pred_class == 2) {         // offline says UP
                bid_mult = 1.3;
                ask_mult = 0.8;
            } else if (off.pred_class == 0) {  // offline says DOWN
                bid_mult = 0.8;
                ask_mult = 1.3;
            }

            LiveEvent ev_book;
            ev_book.type = LiveEventType::ORDERBOOK;
            ev_book.orderbook = build_demo_book(
                sample.timestamp,
                base_mid,
                spread_bps,
                10.0,
                10.0,
                bid_mult,
                ask_mult
            );
            engine.on_event(ev_book);

            // A few trades
            LiveEvent ev_trade1, ev_trade2, ev_trade3;
            ev_trade1.type = LiveEventType::TRADE;
            ev_trade2.type = LiveEventType::TRADE;
            ev_trade3.type = LiveEventType::TRADE;

            if (off.pred_class == 2) {
                ev_trade1.trade = build_demo_trade(sample.timestamp + 1, base_mid, 0.8, true);
                ev_trade2.trade = build_demo_trade(sample.timestamp + 2, base_mid, 1.1, true);
                ev_trade3.trade = build_demo_trade(sample.timestamp + 3, base_mid, 0.5, false);
            } else if (off.pred_class == 0) {
                ev_trade1.trade = build_demo_trade(sample.timestamp + 1, base_mid, 0.8, false);
                ev_trade2.trade = build_demo_trade(sample.timestamp + 2, base_mid, 1.1, false);
                ev_trade3.trade = build_demo_trade(sample.timestamp + 3, base_mid, 0.5, true);
            } else {
                ev_trade1.trade = build_demo_trade(sample.timestamp + 1, base_mid, 0.5, true);
                ev_trade2.trade = build_demo_trade(sample.timestamp + 2, base_mid, 0.5, false);
                ev_trade3.trade = build_demo_trade(sample.timestamp + 3, base_mid, 0.3, true);
            }

            engine.on_event(ev_trade1);
            engine.on_event(ev_trade2);
            engine.on_event(ev_trade3);

            LiveDecisionLog log = engine.step();
            LiveFeatureVector fv = engine.current_live_features();

            if (processed < 10) {
                std::cout << "\n--- Replay Sample " << (processed + 1) << " ---\n";
                std::cout << "timestamp=" << sample.timestamp
                          << " true_label=" << sample.label
                          << " offline_pred=" << log.offline_pred
                          << " offline_conf=" << log.offline_confidence
                          << "\n";
                print_probs(off.probs);
                print_live_features(fv);
                std::cout << "[decision] action=" << action_to_string(log.action)
                          << " reason=" << log.reason
                          << " micro_score=" << log.micro_score
                          << " final_score=" << log.final_score
                          << "\n";
            }

            if (log.action == TradeAction::LONG) long_count++;
            else if (log.action == TradeAction::SHORT) short_count++;
            else no_trade_count++;

            processed++;
            if (processed >= 1000) break; // demo subset
        }

        std::cout << "\n[5/6] Replay summary\n";
        std::cout << "processed   = " << processed << "\n";
        std::cout << "long_count  = " << long_count << "\n";
        std::cout << "short_count = " << short_count << "\n";
        std::cout << "no_trade    = " << no_trade_count << "\n";

        std::cout << "\n[6/6] Done.\n";
        std::cout << "Next step: replace synthetic event generation with real market feed.\n";

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        return 1;
    }
}
