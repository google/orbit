// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <numeric>

#include "Statistics/DataSet.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

[[nodiscard]] double HistogramRiskScore(const Histogram& histogram) {
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

[[nodiscard]] size_t ValueToIndex(uint64_t value, const DataSet& data_set, uint64_t bandwidth) {
  return (value - data_set.GetMin()) / bandwidth;
}

[[nodiscard]] uint64_t NumberOfBinsToBandwidth(const DataSet& data_set, size_t bins_num) {
  const uint64_t width = data_set.GetMax() - data_set.GetMin() + 1;
  if (width < bins_num) {
    return width;
  }
  return width / bins_num + ((width % bins_num != 0) ? 1 : 0);
}

[[nodiscard]] Histogram BuildHistogram(const DataSet& data_set, uint64_t bandwidth) {
  const size_t bin_num = ValueToIndex(data_set.GetMax(), data_set, bandwidth) + 1;
  std::vector<size_t> counts(bin_num, 0UL);
  for (uint64_t value : *data_set.GetData()) {
    counts[ValueToIndex(value, data_set, bandwidth)]++;
  }
  return {data_set.GetMin(), data_set.GetMax(), bandwidth, data_set.GetData()->size(),
          std::move(counts)};
}

}  // namespace orbit_statistics