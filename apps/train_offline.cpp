#include <iostream>
#include <stdexcept>
#include <string>

#include "offline_model.hpp"
#include "sequence_builder.hpp"
#include "trainer.hpp"

int main() {
    try {
        using namespace capigrad;

        const std::string train_path      = "../data/processed/train.bin";
        const std::string val_path        = "../data/processed/val.bin";
        const std::string best_model_path = "../artifacts/models/offline_best.bin";

        std::cout << "[1/5] Loading datasets...\n";
        OfflineDataset train_ds = load_dataset_binary(train_path);
        OfflineDataset val_ds   = load_dataset_binary(val_path);

        if (train_ds.empty() || val_ds.empty()) {
            throw std::runtime_error("Empty train/val dataset.");
        }

        const int m15_dim = static_cast<int>(train_ds.samples[0].m15_seq[0].size());
        const int h1_dim  = static_cast<int>(train_ds.samples[0].h1_seq[0].size());

        std::cout << "[2/5] Building model...\n";
        std::cout << "M15 input dim = " << m15_dim << "\n";
        std::cout << "H1 input dim  = " << h1_dim  << "\n";

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

        OfflineDualTFModel model(m15_dim, h1_dim, model_cfg);
        std::cout << "Total params: " << model.total_params() << "\n";

        std::cout << "[3/5] Creating trainer...\n";
        TrainConfig train_cfg;
        train_cfg.learning_rate       = model_cfg.lr;
        train_cfg.weight_decay        = model_cfg.weight_decay;
        train_cfg.epochs              = 10;
        train_cfg.log_every           = 5000;
        train_cfg.eval_every_epochs   = 1;
        train_cfg.early_stop_patience = 5;

        Trainer trainer(model, train_cfg);

        std::cout << "[4/5] Training...\n";
        TrainResult result = trainer.fit(train_ds, val_ds, best_model_path);

        std::cout << "\n[5/5] Done.\n";
        std::cout << "Best val loss = " << result.best_val_loss
                  << " at epoch " << result.best_epoch << "\n";

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        return 1;
    }
}
