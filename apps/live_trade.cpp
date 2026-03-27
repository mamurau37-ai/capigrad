#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bootstrap_context.hpp"
#include "candle_aggregator.hpp"
#include "context_builder.hpp"
#include "live_adapter.hpp"
#include "live_engine.hpp"
#include "live_logger.hpp"
#include "normalizer.hpp"
#include "offline_model.hpp"
#include "outcome_tracker.hpp"
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

std::string probs_to_string(const std::vector<double>& probs) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < probs.size(); ++i) {
        oss << probs[i];
        if (i + 1 != probs.size()) oss << ",";
    }
    return oss.str();
}

void print_live_feature_brief(const LiveFeatureVector& fv) {
    if (fv.values.size() < LiveFeatureSchema::DIM) {
        std::cout << " live_features=NOT_READY";
        return;
    }

    std::cout << " spread_bps=" << fv.values[LiveFeatureSchema::SPREAD_BPS]
              << " imb3=" << fv.values[LiveFeatureSchema::IMBALANCE_TOP3]
              << " micro_gap=" << fv.values[LiveFeatureSchema::MICROPRICE_GAP_BPS]
              << " flow_s=" << fv.values[LiveFeatureSchema::SIGNED_FLOW_SHORT];
}

int make_udp_socket(int port) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        throw std::runtime_error("socket() failed");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(fd);
        throw std::runtime_error(std::string("bind() failed: ") + std::strerror(errno));
    }

    return fd;
}

} // anonymous namespace

int main() {
    try {
        const std::string model_path       = "../artifacts/models/offline_best.bin";
        const std::string m15_norm_path    = "../artifacts/normalizers/m15_norm.bin";
        const std::string h1_norm_path     = "../artifacts/normalizers/h1_norm.bin";
        const std::string context_path     = "../data/processed/test.bin";

        const std::string m15_csv_path     = "../data/raw/btcusdt_m15_5y.csv";
        const std::string h1_csv_path      = "../data/raw/btcusdt_h1_5y.csv";

        const std::string live_log_path    = "../artifacts/logs/live_signals.csv";
        const std::string realized_log_path = "../artifacts/logs/live_realized.csv";

        std::cout << "[1/10] Loading one offline sample to infer model dims...\n";
        OfflineDataset context_ds = load_dataset_binary(context_path);
        if (context_ds.empty()) {
            throw std::runtime_error("Context dataset empty.");
        }

        const int m15_input_dim = static_cast<int>(context_ds.samples[0].m15_seq[0].size());
        const int h1_input_dim  = static_cast<int>(context_ds.samples[0].h1_seq[0].size());

        std::cout << "[2/10] Loading normalizers...\n";
        FeatureNormalizer m15_norm;
        FeatureNormalizer h1_norm;
        m15_norm.load(m15_norm_path);
        h1_norm.load(h1_norm_path);

        std::cout << "[3/10] Building offline model template...\n";
        OfflineModelConfig model_cfg;
        model_cfg.m15_embed_dim     = 32;
        model_cfg.h1_embed_dim      = 24;
        model_cfg.m15_hidden_dim    = 48;
        model_cfg.h1_hidden_dim     = 32;
        model_cfg.fusion_hidden_dim = 32;
        model_cfg.num_classes       = 3;
        model_cfg.lr                = 3e-4;
        model_cfg.weight_decay      = 1e-5;
        model_cfg.bptt_window       = 32;

        OfflineDualTFModel offline_template(m15_input_dim, h1_input_dim, model_cfg);
        offline_template.load(model_path);

        std::cout << "[4/10] Building live engine...\n";
        LiveFeatureConfig live_cfg;

        FusionConfig fusion_cfg;
        fusion_cfg.offline_long_threshold  = 0.48;
        fusion_cfg.offline_short_threshold = 0.48;
        fusion_cfg.max_spread_bps = 12.0;
        fusion_cfg.min_micro_long_score  = 0.10;
        fusion_cfg.min_micro_short_score = -0.10;
        fusion_cfg.min_imbalance_top3_long  = 0.03;
        fusion_cfg.max_imbalance_top3_short = -0.03;
        fusion_cfg.min_signed_flow_short_long  = -2000.0;
        fusion_cfg.max_signed_flow_short_short =  2000.0;
        fusion_cfg.min_microprice_gap_bps_long  = -1.0;
        fusion_cfg.max_microprice_gap_bps_short =  1.0;

        LiveEngineConfig engine_cfg;
        engine_cfg.use_offline_replay_context = false;
        engine_cfg.require_live_ready = true;

        LiveEngine engine(offline_template, live_cfg, fusion_cfg, engine_cfg);

        std::cout << "[5/10] Building context infrastructure...\n";
        FeatureConfig feat_cfg;
        SequenceConfig seq_cfg;
        seq_cfg.m15_len = 64;
        seq_cfg.h1_len  = 32;

        ContextBuilder ctx_builder(feat_cfg, seq_cfg, m15_norm, h1_norm);

        CandleAggregator m15_agg(M15_INTERVAL_MS);
        CandleAggregator h1_agg(H1_INTERVAL_MS);

        std::cout << "[5.5/10] Bootstrapping historical context...\n";
        BootstrapConfig boot_cfg;
        boot_cfg.m15_bootstrap_count = 160;
        boot_cfg.h1_bootstrap_count  = 96;

        BootstrapResult boot = bootstrap_context_from_csv(
            ctx_builder,
            m15_csv_path,
            h1_csv_path,
            boot_cfg
        );

        std::cout << "Bootstrap loaded M15=" << boot.loaded_m15
                  << " H1=" << boot.loaded_h1
                  << " ready=" << (boot.ready_after_bootstrap ? 1 : 0)
                  << "\n";

        if (ctx_builder.ready()) {
            OfflineSample sample = ctx_builder.build_sample();
            engine.set_offline_context(sample);
        }

        std::cout << "[6/10] Opening loggers...\n";
        LiveLogger signal_logger(live_log_path);
        LiveLogger realized_logger(realized_log_path);

        std::cout << "[7/10] Creating outcome tracker...\n";
        OutcomeTracker tracker(
            4,       // horizon in M15 bars
            0.0004,  // round-trip fee
            0.0002   // slippage
        );

        std::cout << "[8/10] Starting UDP listener on 127.0.0.1:5555 ...\n";
        int sock = make_udp_socket(5555);

        std::cout << "[9/10] Waiting for feed.py events...\n";
        std::cout << "Start your python feed producer in another terminal.\n";

        char buf[4096];
        std::int64_t recv_count = 0;
        std::int64_t parsed_count = 0;
        std::int64_t trade_count = 0;
        std::int64_t depth_count = 0;
        std::int64_t m15_closed = 0;
        std::int64_t h1_closed = 0;
        std::int64_t decisions = 0;
        std::int64_t logged_rows = 0;
        std::int64_t realized_rows = 0;

        LiveAdapter adapter;

        while (true) {
            ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
            if (n <= 0) continue;

            buf[n] = '\0';
            std::string line(buf);
            ++recv_count;

            LiveEvent ev;
            if (!adapter.parse_line(line, ev)) {
                std::cerr << "[WARN] parse failed line=" << line << "\n";
                continue;
            }
            ++parsed_count;

            if (ev.type == LiveEventType::TRADE) ++trade_count;
            if (ev.type == LiveEventType::ORDERBOOK) ++depth_count;

            engine.on_event(ev);

            if (ev.type == LiveEventType::TRADE) {
                auto m15_closed_candle = m15_agg.on_trade(ev.trade);
                if (m15_closed_candle.has_value()) {
                    ctx_builder.on_m15_candle(*m15_closed_candle);
                    ++m15_closed;

                    std::cout << "[M15 CLOSED]"
                              << " ts=" << m15_closed_candle->timestamp
                              << " o=" << m15_closed_candle->open
                              << " h=" << m15_closed_candle->high
                              << " l=" << m15_closed_candle->low
                              << " c=" << m15_closed_candle->close
                              << " v=" << m15_closed_candle->volume
                              << "\n";

                    std::vector<FinalizedOutcome> fins = tracker.on_m15_close(*m15_closed_candle);
                    for (const auto& fin : fins) {
                        realized_logger.log_row(fin.row);
                        ++realized_rows;
                    }
                    if (!fins.empty()) realized_logger.flush();
                }

                auto h1_closed_candle = h1_agg.on_trade(ev.trade);
                if (h1_closed_candle.has_value()) {
                    ctx_builder.on_h1_candle(*h1_closed_candle);
                    ++h1_closed;

                    std::cout << "[H1 CLOSED]"
                              << " ts=" << h1_closed_candle->timestamp
                              << " o=" << h1_closed_candle->open
                              << " h=" << h1_closed_candle->high
                              << " l=" << h1_closed_candle->low
                              << " c=" << h1_closed_candle->close
                              << " v=" << h1_closed_candle->volume
                              << "\n";
                }

                if (ctx_builder.ready()) {
                    OfflineSample sample = ctx_builder.build_sample();
                    engine.set_offline_context(sample);
                }
            }

            if (engine.has_context() && engine.live_ready()) {
                OfflineSignal off = engine.infer_offline_signal();
                LiveFeatureVector fv = engine.current_live_features();
                FusionDecision d = engine.decide();

                ++decisions;

                std::cout << "[LIVE]"
                          << " ts=" << d.timestamp
                          << " pred=" << off.pred_class
                          << " conf=" << std::fixed << std::setprecision(4) << off.confidence
                          << " probs=" << probs_to_string(off.probs);

                print_live_feature_brief(fv);

                std::cout << " micro_score=" << d.micro_score
                          << " final_score=" << d.final_score
                          << " action=" << action_to_string(d.action)
                          << " reason=" << d.reason
                          << "\n";

                LiveLogRow row = LiveLogger::make_row(off, fv, d);
                signal_logger.log_row(row);
                ++logged_rows;

                if (d.action != TradeAction::NO_TRADE && ev.type == LiveEventType::TRADE) {
                    tracker.on_decision(row, ev.trade.price);
                }

                if (logged_rows % 100 == 0) {
                    signal_logger.flush();
                }
            }

            if (recv_count % 500 == 0) {
                std::cout << "[HEARTBEAT]"
                          << " recv=" << recv_count
                          << " parsed=" << parsed_count
                          << " trade=" << trade_count
                          << " depth=" << depth_count
                          << " m15_closed=" << m15_closed
                          << " h1_closed=" << h1_closed
                          << " ctx_ready=" << (ctx_builder.ready() ? 1 : 0)
                          << " live_ready=" << (engine.live_ready() ? 1 : 0)
                          << " decisions=" << decisions
                          << " logged=" << logged_rows
                          << " realized=" << realized_rows
                          << " pending=" << tracker.pending_count()
                          << "\n";

                signal_logger.flush();
                realized_logger.flush();
            }
        }

        close(sock);
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        return 1;
    }
}
