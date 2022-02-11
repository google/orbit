// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/Histogram.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <optional>

#include "HistogramPrivate.h"
#include "OrbitBase/Logging.h"
#include "Statistics/DataSet.h"

namespace orbit_statistics {

constexpr int kNumberOfBinsGridSize = 12;

[[nodiscard]] std::optional<Histogram> BuildHistogram(const std::vector<uint64_t>& data) {
  std::optional<DataSet> data_set = CreateDataSet(&data);
  if (!data_set.has_value()) return std::nullopt;

  size_t number_of_bins = 1;
  double best_risk_score = std::numeric_limits<double>::max();
  Histogram best_histogram;

  for (int i = 0; i < kNumberOfBinsGridSize; ++i) {
    uint64_t bandwidth = NumberOfBinsToBandwidth(data_set.value(), number_of_bins);
    auto histogram = BuildHistogram(data_set.value(), bandwidth);
    double risk_score = HistogramRiskScore(histogram);

    if (risk_score < best_risk_score) {
      best_risk_score = risk_score;
      best_histogram = std::move(histogram);
    }
    number_of_bins *= 2;
  }

  return best_histogram;
}

}  // namespace orbit_statistics