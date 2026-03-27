#include "live_logger.hpp"

#include <stdexcept>

namespace capigrad {

LiveLogger::LiveLogger(const std::string& csv_path)
    : file_(csv_path, std::ios::app) {
    if (!file_.is_open()) {
        throw std::runtime_error("LiveLogger failed to open file: " + csv_path);
    }
}

void LiveLogger::write_header_if_needed() {
    if (header_written_) return;

    file_
        << "timestamp,"
        << "offline_pred,"
        << "p_down,"
        << "p_flat,"
        << "p_up,"
        << "offline_conf,"
        << "spread_bps,"
        << "imbalance_top3,"
        << "imbalance_top10,"
        << "micro_gap_bps,"
        << "signed_flow_short,"
        << "signed_flow_medium,"
        << "micro_score,"
        << "final_score,"
        << "action,"
        << "reason,"
        << "has_realized,"
        << "realized_return,"
        << "realized_net_return"
        << "\n";

    header_written_ = true;
}

LiveLogRow LiveLogger::make_row(const OfflineSignal& off,
                                const LiveFeatureVector& fv,
                                const FusionDecision& d) {
    LiveLogRow row;
    row.timestamp = d.timestamp != 0 ? d.timestamp : off.timestamp;

    row.offline_pred = off.pred_class;
    row.offline_conf = off.confidence;

    if (off.probs.size() >= 3) {
        row.p_down = off.probs[0];
        row.p_flat = off.probs[1];
        row.p_up   = off.probs[2];
    }

    if (fv.values.size() >= LiveFeatureSchema::DIM) {
        row.spread_bps         = fv.values[LiveFeatureSchema::SPREAD_BPS];
        row.imbalance_top3     = fv.values[LiveFeatureSchema::IMBALANCE_TOP3];
        row.imbalance_top10    = fv.values[LiveFeatureSchema::IMBALANCE_TOP10];
        row.micro_gap_bps      = fv.values[LiveFeatureSchema::MICROPRICE_GAP_BPS];
        row.signed_flow_short  = fv.values[LiveFeatureSchema::SIGNED_FLOW_SHORT];
        row.signed_flow_medium = fv.values[LiveFeatureSchema::SIGNED_FLOW_MEDIUM];
    }

    row.micro_score = d.micro_score;
    row.final_score = d.final_score;

    if (d.action == TradeAction::LONG) row.action = 1;
    else if (d.action == TradeAction::SHORT) row.action = -1;
    else row.action = 0;

    row.reason = d.reason ? d.reason : "UNKNOWN";
    return row;
}

void LiveLogger::log_row(const LiveLogRow& row) {
    write_header_if_needed();

    file_
        << row.timestamp << ","
        << row.offline_pred << ","
        << row.p_down << ","
        << row.p_flat << ","
        << row.p_up << ","
        << row.offline_conf << ","
        << row.spread_bps << ","
        << row.imbalance_top3 << ","
        << row.imbalance_top10 << ","
        << row.micro_gap_bps << ","
        << row.signed_flow_short << ","
        << row.signed_flow_medium << ","
        << row.micro_score << ","
        << row.final_score << ","
        << row.action << ","
        << row.reason << ","
        << row.has_realized << ","
        << row.realized_return << ","
        << row.realized_net_return
        << "\n";

    if (!file_) {
        throw std::runtime_error("LiveLogger write failed");
    }
}

void LiveLogger::flush() {
    file_.flush();
}

} // namespace capigrad
