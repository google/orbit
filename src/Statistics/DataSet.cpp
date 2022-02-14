// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/DataSet.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "OrbitBase/Logging.h"
#include "Statistics/Histogram.h"

namespace orbit_statistics {

[[nodiscard]] std::optional<DataSet> CreateDataSet(const std::vector<uint64_t>* data) {
  ORBIT_CHECK(data != nullptr);
  if (data->empty()) return std::nullopt;
  const auto [min, max] = std::minmax_element(data->begin(), data->end());

  return DataSet(data, *min, *max);
}

}  // namespace orbit_statistics