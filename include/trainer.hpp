#ifndef TRAINER_HPP
#define TRAINER_HPP

#include "data.hpp"
#include "model.hpp"
#include <vector>

void compute_class_weights(const std::vector<Candle>& d1h, int train_end);
void calibrate_barrier_params(const std::vector<Candle>& d1h);

void train_mie(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m, int epochs=500, double lr=0.0001);
void eval_mie(MIEModel& model, const std::vector<Candle>& d1h, const std::vector<Candle>& d15m);

#endif // TRAINER_HPP