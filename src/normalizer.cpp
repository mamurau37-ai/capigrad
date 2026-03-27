#include "normalizer.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>

namespace capigrad {

namespace {
static constexpr std::uint32_t NORM_MAGIC   = 0x4E4F524D; // "NORM"
static constexpr std::uint32_t NORM_VERSION = 1;
}

void RunningStat::update(double x) {
    ++count;
    double delta = x - mean;
    mean += delta / count;
    double delta2 = x - mean;
    m2 += delta * delta2;
}

double RunningStat::variance() const {
    if (count <= 0) return 0.0;
    return m2 / count;
}

double RunningStat::stddev(double eps) const {
    return std::sqrt(std::max(variance(), eps * eps));
}

double FeatureNormalizer::normalize_value(double x, int j, double eps) const {
    if (j < 0 || j >= (int)mean.size()) {
        throw std::runtime_error("FeatureNormalizer::normalize_value index out of range");
    }
    return (x - mean[j]) / std::max(std[j], eps);
}

void FeatureNormalizer::normalize_matrix_inplace(std::vector<std::vector<double>>& seq,
                                                 double eps) const {
    for (auto& row : seq) {
        if ((int)row.size() != dim()) {
            throw std::runtime_error("FeatureNormalizer::normalize_matrix_inplace dim mismatch");
        }
        for (int j = 0; j < (int)row.size(); ++j) {
            row[j] = normalize_value(row[j], j, eps);
        }
    }
}

void FeatureNormalizer::save(const std::string& path) const {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("FeatureNormalizer::save failed to open file: " + path);
    }

    std::uint32_t magic = NORM_MAGIC;
    std::uint32_t version = NORM_VERSION;
    std::uint32_t d = static_cast<std::uint32_t>(mean.size());

    f.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    f.write(reinterpret_cast<const char*>(&version), sizeof(version));
    f.write(reinterpret_cast<const char*>(&d), sizeof(d));

    for (std::uint32_t i = 0; i < d; ++i) {
        f.write(reinterpret_cast<const char*>(&mean[i]), sizeof(double));
        f.write(reinterpret_cast<const char*>(&std[i]), sizeof(double));
    }

    if (!f) {
        throw std::runtime_error("FeatureNormalizer::save write error: " + path);
    }
}

void FeatureNormalizer::load(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("FeatureNormalizer::load failed to open file: " + path);
    }

    std::uint32_t magic = 0, version = 0, d = 0;
    f.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    f.read(reinterpret_cast<char*>(&version), sizeof(version));
    f.read(reinterpret_cast<char*>(&d), sizeof(d));

    if (magic != NORM_MAGIC) {
        throw std::runtime_error("FeatureNormalizer::load invalid magic: " + path);
    }
    if (version != NORM_VERSION) {
        throw std::runtime_error("FeatureNormalizer::load invalid version: " + path);
    }

    mean.resize(d);
    std.resize(d);

    for (std::uint32_t i = 0; i < d; ++i) {
        f.read(reinterpret_cast<char*>(&mean[i]), sizeof(double));
        f.read(reinterpret_cast<char*>(&std[i]), sizeof(double));
    }

    if (!f) {
        throw std::runtime_error("FeatureNormalizer::load read error: " + path);
    }
}

static FeatureNormalizer fit_from_sequence_selector(
    const OfflineDataset& ds,
    bool use_m15) {

    FeatureNormalizer norm;
    if (ds.samples.empty()) return norm;

    const auto& first_seq = use_m15 ? ds.samples.front().m15_seq
                                    : ds.samples.front().h1_seq;

    if (first_seq.empty()) return norm;

    const int dim = static_cast<int>(first_seq.front().size());
    std::vector<RunningStat> stats(dim);

    for (const auto& sample : ds.samples) {
        const auto& seq = use_m15 ? sample.m15_seq : sample.h1_seq;

        for (const auto& row : seq) {
            if ((int)row.size() != dim) {
                throw std::runtime_error("fit_from_sequence_selector dim mismatch");
            }
            for (int j = 0; j < dim; ++j) {
                stats[j].update(row[j]);
            }
        }
    }

    norm.mean.resize(dim);
    norm.std.resize(dim);

    for (int j = 0; j < dim; ++j) {
        norm.mean[j] = stats[j].mean;
        norm.std[j]  = stats[j].stddev();
    }

    return norm;
}

FeatureNormalizer fit_m15_normalizer_from_dataset(const OfflineDataset& ds) {
    return fit_from_sequence_selector(ds, true);
}

FeatureNormalizer fit_h1_normalizer_from_dataset(const OfflineDataset& ds) {
    return fit_from_sequence_selector(ds, false);
}

void normalize_dataset_inplace(OfflineDataset& ds,
                               const FeatureNormalizer& m15_norm,
                               const FeatureNormalizer& h1_norm) {
    for (auto& sample : ds.samples) {
        m15_norm.normalize_matrix_inplace(sample.m15_seq);
        h1_norm.normalize_matrix_inplace(sample.h1_seq);
    }
}

} // namespace capigrad
