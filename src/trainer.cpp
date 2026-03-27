#include "trainer.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>

namespace capigrad {

namespace {
static constexpr double EPS = 1e-12;
}

Trainer::Trainer(OfflineDualTFModel& model, const TrainConfig& cfg)
    : model_(model), cfg_(cfg) {}

double Trainer::safe_accuracy(int correct, int total) {
    if (total <= 0) return 0.0;
    return static_cast<double>(correct) / static_cast<double>(total);
}

double Trainer::run_train_epoch(const OfflineDataset& ds,
                                int epoch_idx,
                                int& global_step) {
    if (ds.empty()) {
        throw std::runtime_error("run_train_epoch: empty dataset");
    }

    // Build index array for shuffling if needed
    std::vector<int> idx(ds.size());
    std::iota(idx.begin(), idx.end(), 0);

    // NOTE: For time-series DO NOT shuffle
    // Shuffle only if you have verified your labels are i.i.d.
    // std::shuffle(idx.begin(), idx.end(), std::mt19937(...));

    double loss_sum = 0.0;

    for (int ii = 0; ii < (int)idx.size(); ++ii) {
        const OfflineSample& sample = ds.samples[idx[ii]];

        model_.reset_state();
        double loss = model_.train_step(sample, global_step);

        loss_sum += loss;
        ++global_step;

        if (cfg_.log_every > 0 && (global_step % cfg_.log_every == 0)) {
            double avg = loss_sum / static_cast<double>(ii + 1);
            std::cout << "[train] epoch=" << epoch_idx + 1
                      << " step=" << global_step
                      << " sample=" << (ii + 1) << "/" << ds.size()
                      << " avg_loss=" << avg
                      << "\n";
        }
    }

    return loss_sum / static_cast<double>(ds.size());
}

EvalMetrics Trainer::evaluate(const OfflineDataset& ds) {
    EvalMetrics m;
    m.confusion.assign(3, std::vector<int>(3, 0));

    if (ds.empty()) return m;

    for (int i = 0; i < ds.size(); ++i) {
        const OfflineSample& sample = ds.samples[i];

        model_.reset_state();
        std::vector<double> logits = model_.forward(sample);
        std::vector<double> probs  = model_.softmax(logits);

        int pred = 0;
        for (int k = 1; k < (int)probs.size(); ++k) {
            if (probs[k] > probs[pred]) pred = k;
        }

        double sample_loss = 0.0;
        if (sample.label >= 0 && sample.label < (int)probs.size()) {
            sample_loss = -std::log(std::max(probs[sample.label], EPS));
        }

        m.loss  += sample_loss;
        m.total += 1;
        if (pred == sample.label) ++m.correct;

        if (sample.label >= 0 && sample.label < 3 &&
            pred >= 0 && pred < 3) {
            m.confusion[sample.label][pred] += 1;
        }
    }

    m.loss    /= std::max(1, m.total);
    m.accuracy = safe_accuracy(m.correct, m.total);
    return m;
}

TrainResult Trainer::fit(const OfflineDataset& train_ds,
                         const OfflineDataset& val_ds,
                         const std::string& best_model_path) {
    if (train_ds.empty()) {
        throw std::runtime_error("Trainer::fit: empty train dataset");
    }
    if (val_ds.empty()) {
        throw std::runtime_error("Trainer::fit: empty val dataset");
    }

    TrainResult result;
    int global_step = 1;
    int no_improve  = 0;

    std::cout << "Total params: " << model_.total_params() << "\n";
    std::cout << "Train samples: " << train_ds.size() << "\n";
    std::cout << "Val samples  : " << val_ds.size() << "\n\n";

    for (int epoch = 0; epoch < cfg_.epochs; ++epoch) {
        std::cout << "=== Epoch " << (epoch + 1)
                  << "/" << cfg_.epochs << " ===\n";

        double train_loss = run_train_epoch(train_ds, epoch, global_step);
        std::cout << "[epoch " << (epoch + 1) << "] train_loss="
                  << train_loss << "\n";

        if ((epoch + 1) % cfg_.eval_every_epochs == 0) {
            EvalMetrics val = evaluate(val_ds);

            std::cout << "[epoch " << (epoch + 1) << "] val_loss="
                      << val.loss
                      << " val_acc=" << val.accuracy
                      << " (" << val.correct << "/" << val.total << ")\n";

            // Balanced accuracy
            double bal_acc = 0.0;
            for (int k = 0; k < 3; ++k) {
                int tp = val.confusion[k][k];
                int fn = 0;
                for (int j = 0; j < 3; ++j) {
                    if (j != k) fn += val.confusion[k][j];
                }
                int support = tp + fn;
                if (support > 0) {
                    bal_acc += static_cast<double>(tp) / support;
                }
            }
            bal_acc /= 3.0;
            std::cout << "[epoch " << (epoch + 1) << "] balanced_acc="
                      << bal_acc << "\n";

            if (val.loss < result.best_val_loss) {
                result.best_val_loss = val.loss;
                result.best_epoch    = epoch + 1;
                no_improve = 0;

                model_.save(best_model_path);
                std::cout << "[checkpoint] saved to: "
                          << best_model_path << "\n";

                // Also save optimizer state for full resume
                model_.save_full(best_model_path);
            } else {
                ++no_improve;
                std::cout << "[early-stop] patience "
                          << no_improve << "/"
                          << cfg_.early_stop_patience << "\n";

                if (no_improve >= cfg_.early_stop_patience) {
                    std::cout << "[early-stop] stopping.\n";
                    break;
                }
            }
        }
    }

    return result;
}

} // namespace capigrad
