#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "csv_reader.hpp"
#include "features.hpp"
#include "labeler.hpp"
#include "normalizer.hpp"
#include "sequence_builder.hpp"

int main() {
    try {
        using namespace capigrad;

        const std::string m15_path = "../data/raw/btcusdt_m15_5y.csv";
        const std::string h1_path  = "../data/raw/btcusdt_h1_5y.csv";

        const std::string train_out = "../data/processed/train.bin";
        const std::string val_out   = "../data/processed/val.bin";
        const std::string test_out  = "../data/processed/test.bin";

        const std::string m15_norm_out = "../artifacts/normalizers/m15_norm.bin";
        const std::string h1_norm_out  = "../artifacts/normalizers/h1_norm.bin";

       
        std::cout << "[1/8] Loading CSV candles...\n";
        std::vector<Candle> m15_candles = load_candles_from_csv(m15_path);
        std::vector<Candle> h1_candles  = load_candles_from_csv(h1_path);

        print_candle_summary(m15_candles, "M15");
        print_candle_summary(h1_candles, "H1");

        if (m15_candles.empty() || h1_candles.empty()) {
            throw std::runtime_error("One of the candle datasets is empty.");
        }

        FeatureConfig feature_cfg;
        LabelConfig   label_cfg;
        SequenceConfig seq_cfg;

        seq_cfg.m15_len = 64;
        seq_cfg.h1_len  = 32;

        label_cfg.horizon_m15   = 4;
        label_cfg.vol_window    = 32;
        label_cfg.threshold_k   = 0.8;
        label_cfg.min_threshold = 0.0;

        std::cout << "[2/8] Building features...\n";
        std::vector<FeatureVector> m15_features = build_features(m15_candles, feature_cfg);
        std::vector<FeatureVector> h1_features  = build_features(h1_candles, feature_cfg);

        std::vector<FeatureValidity> m15_validity = build_feature_validity(m15_candles, feature_cfg);
        std::vector<FeatureValidity> h1_validity  = build_feature_validity(h1_candles, feature_cfg);

        std::cout << "[3/8] Building labels...\n";
        std::vector<LabelTarget> labels = build_direction_labels(m15_candles, label_cfg);

        std::cout << "[4/8] Building offline dataset...\n";
        OfflineDataset all_ds = build_offline_dataset(
            m15_candles,
            h1_candles,
            m15_features,
            h1_features,
            m15_validity,
            h1_validity,
            labels,
            seq_cfg,
            label_cfg
        );

        std::cout << "All valid samples: " << all_ds.size() << "\n";
        if (all_ds.empty()) {
            throw std::runtime_error("No valid samples were built.");
        }

        std::cout << "[5/8] Splitting dataset...\n";
        DatasetSplit split = split_dataset_by_ratio(all_ds, 0.70, 0.15, 0.15);

        std::cout << "Train samples: " << split.train.size() << "\n";
        std::cout << "Val samples  : " << split.val.size()   << "\n";
        std::cout << "Test samples : " << split.test.size()  << "\n";

        if (split.train.empty()) {
            throw std::runtime_error("Train split is empty.");
        }

        std::cout << "[6/8] Fitting normalizers from TRAIN split...\n";
        FeatureNormalizer m15_norm = fit_m15_normalizer_from_dataset(split.train);
        FeatureNormalizer h1_norm  = fit_h1_normalizer_from_dataset(split.train);

        std::cout << "[7/8] Applying normalization...\n";
        normalize_dataset_inplace(split.train, m15_norm, h1_norm);
        normalize_dataset_inplace(split.val,   m15_norm, h1_norm);
        normalize_dataset_inplace(split.test,  m15_norm, h1_norm);

        std::cout << "[8/8] Saving artifacts...\n";
        save_dataset_binary(split.train, train_out);
        save_dataset_binary(split.val,   val_out);
        save_dataset_binary(split.test,  test_out);

        m15_norm.save(m15_norm_out);
        h1_norm.save(h1_norm_out);

        std::cout << "Done.\n";
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        return 1;
    }
}
