// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_HISTOGRAM_H_
#define STATISTICS_HISTOGRAM_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace orbit_statistics {

/**
 * An histogram of a dataset of `uint64_t` values with bins of equal width (with a possible
 * exception for the last one). The bins are:
 * [min, min + bandwidth), [min + bandwidth, min + 2*bandwidth), ...
 * ... [min + (counts*size() - 1) * bandwidth, max]
 *
 * `counts[i]` stores the number of elements in i-th bin.
 */
struct Histogram {
  const uint64_t min;
  const uint64_t max;
  const uint64_t bandwidth;
  const size_t data_set_size;
  const std::vector<size_t> counts;
};

std::unique_ptr<Histogram> BuildHistogram(const std::vector<uint64_t>& data);
}  // namespace orbit_statistics

#endif  // STATISTICS_HISTOGRAM_H_