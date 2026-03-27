#ifndef CAPIGRAD_TRAINER_HPP
#define CAPIGRAD_TRAINER_HPP

#include <string>
#include <vector>

#include "offline_model.hpp"
#include "sequence_builder.hpp"

namespace capigrad {

struct TrainConfig {
    double learning_rate   = 3e-4;
    double weight_decay    = 1e-5;
    int epochs             = 10;
    int log_every          = 1000;
    int eval_every_epochs  = 1;
    int early_stop_patience = 5;
};

struct EvalMetrics {
    double loss = 0.0;
    double accuracy = 0.0;

    int total = 0;
    int correct = 0;

    std::vector<std::vector<int>> confusion; // [3][3]
};

struct TrainResult {
    double best_val_loss = 1e100;
    int best_epoch = -1;
};

class Trainer {
public:
    Trainer(OfflineDualTFModel& model, const TrainConfig& cfg);

    TrainResult fit(const OfflineDataset& train_ds,
                    const OfflineDataset& val_ds,
                    const std::string& best_model_path);

    EvalMetrics evaluate(const OfflineDataset& ds);

private:
    OfflineDualTFModel& model_;
    TrainConfig cfg_;

    double run_train_epoch(const OfflineDataset& ds, int epoch_idx, int& global_step);
    static double safe_accuracy(int correct, int total);
};

} // namespace capigrad

#endif // CAPIGRAD_TRAINER_HPP
