#ifndef CAPIGRAD_NORMALIZER_HPP
#define CAPIGRAD_NORMALIZER_HPP

#include <string>
#include <vector>
#include "sequence_builder.hpp"

namespace capigrad {

// Online running stat per feature
struct RunningStat {
    int count = 0;
    double mean = 0.0;
    double m2   = 0.0;

    void update(double x);
    double variance() const;
    double stddev(double eps = 1e-12) const;
};

// Fitted feature normalizer
struct FeatureNormalizer {
    std::vector<double> mean;
    std::vector<double> std;

    int dim() const {
        return static_cast<int>(mean.size());
    }

    bool empty() const {
        return mean.empty();
    }

    double normalize_value(double x, int j, double eps = 1e-12) const;

    void normalize_matrix_inplace(std::vector<std::vector<double>>& seq,
                                  double eps = 1e-12) const;

    void save(const std::string& path) const;
    void load(const std::string& path);
};

// Separate fitters for each branch
FeatureNormalizer fit_m15_normalizer_from_dataset(const OfflineDataset& ds);
FeatureNormalizer fit_h1_normalizer_from_dataset(const OfflineDataset& ds);

// Apply both normalizers to dataset in-place
void normalize_dataset_inplace(OfflineDataset& ds,
                               const FeatureNormalizer& m15_norm,
                               const FeatureNormalizer& h1_norm);

} // namespace capigrad

#endif // CAPIGRAD_NORMALIZER_HPP
