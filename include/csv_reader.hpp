#ifndef CAPIGRAD_CSV_READER_HPP
#define CAPIGRAD_CSV_READER_HPP

#include <string>
#include <vector>
#include "features.hpp"

namespace capigrad {

std::vector<Candle> load_candles_from_csv(const std::string& path);

void print_candle_summary(const std::vector<Candle>& candles,
                          const std::string& tag);

} // namespace capigrad

#endif // CAPIGRAD_CSV_READER_HPP
