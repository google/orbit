// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STATISTICS_DATA_SET_H_
#define STATISTICS_DATA_SET_H_

#include <absl/types/span.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace orbit_statistics {

// A data set of `uint64_t` values.
// Also stores and exposes minimum and maximum value.
class DataSet {
 public:
  [[nodiscard]] static std::optional<DataSet> Create(absl::Span<const uint64_t> data);

  [[nodiscard]] absl::Span<const uint64_t> GetData() const { return data_; }
  [[nodiscard]] uint64_t GetMin() const { return min_; }
  [[nodiscard]] uint64_t GetMax() const { return max_; }

 private:
  DataSet(absl::Span<const uint64_t> data, uint64_t min, uint64_t max)
      : data_(data), min_(min), max_(max) {}
  absl::Span<const uint64_t> data_;
  uint64_t min_;
  uint64_t max_;
};
}  // namespace orbit_statistics
#endif  // STATISTICS_DATA_SET_H_
