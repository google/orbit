// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/DataSet.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <optional>

#include "OrbitBase/Logging.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

[[nodiscard]] std::optional<DataSet> CreateDataSet(const std::vector<uint64_t>* data) {
  if (data->empty()) return std::nullopt;
  const auto min_max_iterators = std::minmax_element(data->begin(), data->end());
  uint64_t min = *min_max_iterators.first;
  uint64_t max = *min_max_iterators.second;

  DataSet result(data, min, max);
  return result;
}

}  // namespace orbit_statistics