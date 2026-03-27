#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "offline_model.hpp"
#include "sequence_builder.hpp"

namespace {

double safe_ratio(double a, double b) {
    if (b == 0.0) return 0.0;
    return a / b;
}

void print_confusion_matrix(const std::vector<std::vector<int>>& cm,
                            const std::string& title) {
    std::cout << "\n" << title << "\n";
    std::cout << "Confusion Matrix (rows=true, cols=pred):\n\n";
    std::cout << "          DOWN    FLAT      UP\n";

    static const char* row_names[3] = {"DOWN", "FLAT", "UP"};
    for (int i = 0; i < 3; ++i) {
        std::cout << std::left << std::setw(7) << row_names[i] << " ";
        for (int j = 0; j < 3; ++j) {
            std::cout << std::setw(8) << cm[i][j];
        }
        std::cout << "\n";
    }
}

void print_class_report(const std::vector<std::vector<int>>& cm,
                        const std::string& title) {
    static const char* names[3] = {"DOWN", "FLAT", "UP"};

    std::cout << "\n" << title << "\n";
    std::cout << std::left
              << std::setw(10) << "Class"
              << std::setw(12) << "Precision"
              << std::setw(12) << "Recall"
              << std::setw(12) << "F1"
              << std::setw(12) << "Support"
              << "\n";

    for (int k = 0; k < 3; ++k) {
        double tp = cm[k][k];

        double fp = 0.0;
        for (int i = 0; i < 3; ++i) {
            if (i != k) fp += cm[i][k];
        }

        double fn = 0.0;
        for (int j = 0; j < 3; ++j) {
            if (j != k) fn += cm[k][j];
        }

        double precision = safe_ratio(tp, tp + fp);
        double recall    = safe_ratio(tp, tp + fn);
        double f1        = safe_ratio(2.0 * precision * recall, precision + recall);

        int support = 0;
        for (int j = 0; j < 3; ++j) support += cm[k][j];

        std::cout << std::left
                  << std::setw(10) << names[k]
                  << std::setw(12) << precision
                  << std::setw(12) << recall
                  << std::setw(12) << f1
                  << std::setw(12) << support
                  << "\n";
    }
}

double balanced_accuracy(const std::vector<std::vector<int>>& cm) {
    double sum_recall = 0.0;

    for (int k = 0; k < 3; ++k) {
        double tp = cm[k][k];
        double fn = 0.0;
        for (int j = 0; j < 3; ++j) {
            if (j != k) fn += cm[k][j];
        }
        sum_recall += safe_ratio(tp, tp + fn);
    }

    return sum_recall / 3.0;
}

struct EvalResult {
    std::vector<std::vector<int>> cm = std::vector<std::vector<int>>(3, std::vector<int>(3, 0));
    int correct = 0;
    int total = 0;

    double accuracy() const {
        return safe_ratio((double)correct, (double)total);
    }

    double bal_acc() const {
        return balanced_accuracy(cm);
    }
};

EvalResult eval_constant_baseline(const capigrad::OfflineDataset& ds, int constant_pred) {
    EvalResult r;

    for (const auto& sample : ds.samples) {
        int pred = constant_pred;
        if (sample.label >= 0 && sample.label < 3) {
            r.cm[sample.label][pred] += 1;
            if (pred == sample.label) ++r.correct;
            ++r.total;
        }
    }

    return r;
}

int majority_class(const capigrad::OfflineDataset& ds) {
    std::vector<int> counts(3, 0);
    for (const auto& sample : ds.samples) {
        if (sample.label >= 0 && sample.label < 3) {
            counts[sample.label]++;
        }
    }

    int best = 0;
    for (int i = 1; i < 3; ++i) {
        if (counts[i] > counts[best]) best = i;
    }
    return best;
}

const char* class_name(int k) {
    switch (k) {
        case 0: return "DOWN";
        case 1: return "FLAT";
        case 2: return "UP";
        default: return "UNKNOWN";
    }
}

struct ThresholdResult {
    double threshold = 0.0;

    int trades = 0;
    int wins = 0;
    int losses = 0;
    int flats = 0;

    double coverage = 0.0;
    double hit_rate = 0.0;

    double avg_raw_return = 0.0;
    double avg_net_return = 0.0;
    double avg_abs_return = 0.0;

    double total_raw_return = 0.0;
    double total_net_return = 0.0;
};

ThresholdResult eval_threshold_strategy(capigrad::OfflineDualTFModel& model,
                                        const capigrad::OfflineDataset& ds,
                                        double threshold,
                                        double round_trip_fee,
                                        double slippage_per_trade) {
    ThresholdResult r;
    r.threshold = threshold;

    double raw_sum = 0.0;
    double net_sum = 0.0;
    double abs_sum = 0.0;

    for (const auto& sample : ds.samples) {
        model.reset_state();
        std::vector<double> logits = model.forward(sample);
        std::vector<double> probs  = model.softmax(logits);

        int pred = 0;
        for (int i = 1; i < (int)probs.size(); ++i) {
            if (probs[i] > probs[pred]) pred = i;
        }

        double conf = probs[pred];
        if (conf < threshold) continue;

        // 0 = DOWN -> short
        // 1 = FLAT -> no trade
        // 2 = UP   -> long
        if (pred == 1) {
            r.flats++;
            continue;
        }

        double raw_pnl = 0.0;
        if (pred == 2) {
            raw_pnl = sample.future_return;
        } else if (pred == 0) {
            raw_pnl = -sample.future_return;
        }

        double cost = round_trip_fee + slippage_per_trade;
        double net_pnl = raw_pnl - cost;

        r.trades++;
        raw_sum += raw_pnl;
        net_sum += net_pnl;
        abs_sum += std::abs(raw_pnl);

        if (net_pnl > 0.0) r.wins++;
        else if (net_pnl < 0.0) r.losses++;
    }

    if (r.trades > 0) {
        r.avg_raw_return = raw_sum / r.trades;
        r.avg_net_return = net_sum / r.trades;
        r.avg_abs_return = abs_sum / r.trades;
        r.hit_rate = safe_ratio((double)r.wins, (double)r.trades);
        r.total_raw_return = raw_sum;
        r.total_net_return = net_sum;
    }

    r.coverage = safe_ratio((double)r.trades, (double)ds.size());
    return r;
}

void print_threshold_result(const ThresholdResult& r) {
    std::cout << std::left
              << std::setw(10) << r.threshold
              << std::setw(10) << r.trades
              << std::setw(12) << r.coverage
              << std::setw(12) << r.hit_rate
              << std::setw(16) << r.avg_raw_return
              << std::setw(16) << r.avg_net_return
              << std::setw(14) << r.total_net_return
              << "\n";
}

} // anonymous namespace

int main() {
    try {
        using namespace capigrad;

        const std::string test_path  = "../data/processed/test.bin";
        const std::string model_path = "../artifacts/models/offline_best.bin";

        // Example assumptions:
        // 0.04% round-trip fee + 0.02% slippage
        const double round_trip_fee   = 0.0004;
        const double slippage_per_trade = 0.0002;

        std::cout << "[1/5] Loading test dataset...\n";
        OfflineDataset test_ds = load_dataset_binary(test_path);

        std::cout << "Test samples: " << test_ds.size() << "\n";
        if (test_ds.empty()) {
            throw std::runtime_error("Test dataset is empty.");
        }

        const int m15_input_dim = static_cast<int>(test_ds.samples[0].m15_seq[0].size());
        const int h1_input_dim  = static_cast<int>(test_ds.samples[0].h1_seq[0].size());

        std::cout << "[2/5] Building model...\n";
        OfflineModelConfig cfg;
        OfflineDualTFModel model(m15_input_dim, h1_input_dim, cfg);


        std::cout << "[3/5] Loading trained weights...\n";
        model.load(model_path);

        std::cout << "[4/5] Evaluating trained model...\n";

        EvalResult model_result;

        for (const auto& sample : test_ds.samples) {
            model.reset_state();
            int pred = model.predict(sample);

            if (sample.label >= 0 && sample.label < 3 && pred >= 0 && pred < 3) {
                model_result.cm[sample.label][pred] += 1;
            }

            if (pred == sample.label) {
                ++model_result.correct;
            }
            ++model_result.total;
        }

        std::cout << "\n=== Trained Model Result ===\n";
        std::cout << "Accuracy          : " << model_result.accuracy()
                  << " (" << model_result.correct << "/" << model_result.total << ")\n";
        std::cout << "Balanced Accuracy : " << model_result.bal_acc() << "\n";

        print_confusion_matrix(model_result.cm, "Model");
        print_class_report(model_result.cm, "Model Per-class metrics");

        std::cout << "\n[5/5] Evaluating simple baselines...\n";

        int maj = majority_class(test_ds);

        EvalResult flat_result = eval_constant_baseline(test_ds, 1);
        EvalResult down_result = eval_constant_baseline(test_ds, 0);
        EvalResult up_result   = eval_constant_baseline(test_ds, 2);
        EvalResult maj_result  = eval_constant_baseline(test_ds, maj);

        std::cout << "\n=== Baseline Comparison ===\n";
        std::cout << std::left
                  << std::setw(18) << "Baseline"
                  << std::setw(14) << "Accuracy"
                  << std::setw(18) << "BalancedAcc"
                  << "\n";

        std::cout << std::left
                  << std::setw(18) << "Model"
                  << std::setw(14) << model_result.accuracy()
                  << std::setw(18) << model_result.bal_acc()
                  << "\n";

        std::cout << std::left
                  << std::setw(18) << "Always DOWN"
                  << std::setw(14) << down_result.accuracy()
                  << std::setw(18) << down_result.bal_acc()
                  << "\n";

        std::cout << std::left
                  << std::setw(18) << "Always FLAT"
                  << std::setw(14) << flat_result.accuracy()
                  << std::setw(18) << flat_result.bal_acc()
                  << "\n";

        std::cout << std::left
                  << std::setw(18) << "Always UP"
                  << std::setw(14) << up_result.accuracy()
                  << std::setw(18) << up_result.bal_acc()
                  << "\n";

        std::string maj_name = class_name(maj);
        std::cout << std::left
                  << std::setw(18) << ("Majority (" + maj_name + ")")
                  << std::setw(14) << maj_result.accuracy()
                  << std::setw(18) << maj_result.bal_acc()
                  << "\n";

        std::cout << "\n=== Fee-aware Confidence Threshold Pseudo-Backtest ===\n";
        std::cout << "Assumptions:\n";
        std::cout << "  round_trip_fee   = " << round_trip_fee << "\n";
        std::cout << "  slippage_trade   = " << slippage_per_trade << "\n";
        std::cout << "  total_cost_trade = " << (round_trip_fee + slippage_per_trade) << "\n\n";

        std::cout << std::left
                  << std::setw(10) << "Thresh"
                  << std::setw(10) << "Trades"
                  << std::setw(12) << "Coverage"
                  << std::setw(12) << "HitRate"
                  << std::setw(16) << "AvgRawRet"
                  << std::setw(16) << "AvgNetRet"
                  << std::setw(14) << "TotalNetRet"
                  << "\n";

        std::vector<double> thresholds = {0.34, 0.40, 0.45, 0.50, 0.55, 0.60, 0.65};
        for (double thr : thresholds) {
            ThresholdResult tr = eval_threshold_strategy(
                model,
                test_ds,
                thr,
                round_trip_fee,
                slippage_per_trade
            );
            print_threshold_result(tr);
        }

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        return 1;
    }
}
