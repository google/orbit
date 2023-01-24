// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_
#define ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/btree_map.h>
#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace orbit_gl {

class MultivariateTimeSeries {
 public:
  // The size of series_names SHOULD be consistent with the series dimension.
  explicit MultivariateTimeSeries(std::vector<std::string> series_names,
                                  uint8_t value_decimal_digits, std::string value_unit);

  [[nodiscard]] const std::vector<std::string>& GetSeriesNames() const { return series_names_; }
  [[nodiscard]] uint8_t GetValueDecimalDigits() const { return value_decimal_digits_; }
  [[nodiscard]] std::string GetValueUnit() const { return value_unit_; }
  [[nodiscard]] double GetMin() const;
  [[nodiscard]] double GetMax() const;
  [[nodiscard]] size_t GetDimension() const { return series_names_.size(); }

  [[nodiscard]] bool IsEmpty() const;
  [[nodiscard]] size_t GetTimeToSeriesValuesSize() const;
  [[nodiscard]] uint64_t StartTimeInNs() const;
  [[nodiscard]] uint64_t EndTimeInNs() const;
  [[nodiscard]] std::vector<double> GetPreviousOrFirstEntry(uint64_t time) const;

  // Returns an empty vector if no overlap between time range [min_time, max_time] and
  // [StartTimeInNs(), EndTimeInNs()].
  // Otherwise returns a range of entries affected by the time range [min_time, max_time] where:
  // * the first entry with the time key right before the time range (min_time, max_time) if exists;
  //   otherwise points to the fist entry.
  // * the last entry with the time key right after the time range (min_time, max_time) if exists;
  //   otherwise points to the last entry.
  [[nodiscard]] std::vector<std::pair<uint64_t, std::vector<double>>> GetEntriesAffectedByTimeRange(
      uint64_t min_time, uint64_t max_time) const;

  void AddValues(uint64_t timestamp_ns, absl::Span<const double> values);

  using TimeSeriesEntryIter = absl::btree_map<uint64_t, std::vector<double>>::const_iterator;

 private:
  [[nodiscard]] TimeSeriesEntryIter GetPreviousOrFirstEntryIterator(uint64_t time) const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  [[nodiscard]] TimeSeriesEntryIter GetNextOrLastEntryIterator(uint64_t time) const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  void UpdateMinAndMax(double value) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  mutable absl::Mutex mutex_;
  absl::btree_map<uint64_t, std::vector<double>> time_to_series_values_ ABSL_GUARDED_BY(mutex_);
  double min_ ABSL_GUARDED_BY(mutex_) = std::numeric_limits<double>::max();
  double max_ ABSL_GUARDED_BY(mutex_) = std::numeric_limits<double>::lowest();

  // Should NOT be modified after construction.
  std::vector<std::string> series_names_;
  uint8_t value_decimal_digits_;
  std::string value_unit_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_
