// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Histogram.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>

#include "OrbitBase/Logging.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

void HistogramBuilder::SetDataSet(const std::vector<uint64_t>* data) {
  data_ = data;

  const auto min_max_iterators = std::minmax_element(data->begin(), data->end());
  min_ = *min_max_iterators.first;
  max_ = *min_max_iterators.second;
}

void HistogramBuilder::SetBandwidth(uint64_t bandwidth) {
  ORBIT_CHECK(bandwidth > 0);
  bandwidth_ = bandwidth;
}

void HistogramBuilder::SetNumberOfBins(size_t bins_num) {
  const uint64_t width = max_.value() - min_.value() + 1;
  if (width < bins_num) {
    SetBandwidth(width);
  } else {
    uint64_t bandwidth = width / bins_num + ((width % bins_num != 0) ? 1 : 0);
    SetBandwidth(bandwidth);
  }
}

[[nodiscard]] size_t HistogramBuilder::ValueToIndex(uint64_t value) const {
  return (value - min_.value()) / bandwidth_.value();
}

[[nodiscard]] std::unique_ptr<const Histogram> HistogramBuilder::Build() const {
  const size_t bin_num = ValueToIndex(max_.value()) + 1;
  std::vector<size_t> counts(bin_num, 0UL);
  for (uint64_t value : *data_) {
    counts[ValueToIndex(value)]++;
  }
  return std::make_unique<Histogram>(
      Histogram{min_.value(), max_.value(), bandwidth_.value(), data_->size(), std::move(counts)});
}

double HistogramRiskScore(const Histogram& histogram) {
  if (histogram.max == histogram.min) return 0.0;

  const double sum_of_squared_frequencies =
      std::transform_reduce(histogram.counts.begin(), histogram.counts.end(), 0.0, std::plus<>(),
                            [&histogram](const uint64_t count) -> double {
                              double frequency = static_cast<double>(count) /
                                                 static_cast<double>(histogram.data_set_size);
                              return frequency * frequency;
                            });

  const auto bandwidth = static_cast<double>(histogram.bandwidth) /
                         (static_cast<double>(histogram.max) - static_cast<double>(histogram.min));
  const auto data_set_size = static_cast<double>(histogram.data_set_size);
  return (2.0 - (data_set_size + 1) * sum_of_squared_frequencies) / (bandwidth * data_set_size);
}

constexpr int kNumberOfBinsGridSize = 12;

[[nodiscard]] std::unique_ptr<const Histogram> BuildHistogram(const std::vector<uint64_t>& data) {
  ORBIT_CHECK(!data.empty());
  HistogramBuilder histogram_builder;
  histogram_builder.SetDataSet(&data);

  size_t number_of_bins = 1;
  double best_risk_score = std::numeric_limits<double>::max();
  std::unique_ptr<const Histogram> best_histogram;

  for (int i = 0; i < kNumberOfBinsGridSize; ++i) {
    histogram_builder.SetNumberOfBins(number_of_bins);
    auto histogram = histogram_builder.Build();
    double risk_score = HistogramRiskScore(*histogram);

    if (risk_score < best_risk_score) {
      best_risk_score = risk_score;
      best_histogram = std::move(histogram);
    }
    number_of_bins *= 2;
  }

  return best_histogram;
}

}  // namespace orbit_statistics