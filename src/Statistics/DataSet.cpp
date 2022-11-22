// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Statistics/DataSet.h"

#include <algorithm>
#include <cstdint>
#include <optional>

namespace orbit_statistics {

[[nodiscard]] std::optional<DataSet> DataSet::Create(absl::Span<const uint64_t> data) {
  if (data.empty()) return std::nullopt;
  const auto [min, max] = std::minmax_element(data.begin(), data.end());

  return DataSet(data, *min, *max);
}

}  // namespace orbit_statistics