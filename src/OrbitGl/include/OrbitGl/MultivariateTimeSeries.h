// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_
#define ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_

#include <absl/container/btree_map.h>
#include <absl/synchronization/mutex.h>

#include <array>
#include <map>
#include <optional>
#include <string>

#include "OrbitBase/Logging.h"

template <size_t Dimension>
class MultivariateTimeSeries {
  static_assert(Dimension >= 1, "Dimension must be at least 1");

 public:
  explicit MultivariateTimeSeries(std::array<std::string, Dimension> series_names,
                                  uint8_t value_decimal_digits, std::string value_unit)
      : series_names_(std::move(series_names)),
        value_decimal_digits_{value_decimal_digits},
        value_unit_{std::move(value_unit)} {}

  [[nodiscard]] const std::array<std::string, Dimension>& GetSeriesNames() const {
    return series_names_;
  }

  [[nodiscard]] size_t GetTimeToSeriesValuesSize() const {
    absl::MutexLock lock(&mutex_);
    return time_to_series_values_.size();
  }

  [[nodiscard]] double GetMin() const {
    absl::MutexLock lock(&mutex_);
    return min_;
  }

  [[nodiscard]] double GetMax() const {
    absl::MutexLock lock(&mutex_);
    return max_;
  }
  [[nodiscard]] uint8_t GetValueDecimalDigits() const { return value_decimal_digits_; }
  [[nodiscard]] std::string GetValueUnit() const { return value_unit_; }

  void AddValues(uint64_t timestamp_ns, const std::array<double, Dimension>& values) {
    absl::MutexLock lock(&mutex_);
    time_to_series_values_[timestamp_ns] = values;
    for (double value : values) {
      UpdateMinAndMax(value);
    }
  }

  [[nodiscard]] bool IsEmpty() const {
    absl::MutexLock lock(&mutex_);
    return time_to_series_values_.empty();
  }

  [[nodiscard]] uint64_t StartTimeInNs() const {
    absl::MutexLock lock(&mutex_);
    ORBIT_CHECK(!time_to_series_values_.empty());
    return time_to_series_values_.begin()->first;
  }
  [[nodiscard]] uint64_t EndTimeInNs() const {
    absl::MutexLock lock(&mutex_);
    ORBIT_CHECK(!time_to_series_values_.empty());
    return time_to_series_values_.rbegin()->first;
  }

  const std::array<double, Dimension>& GetPreviousOrFirstEntry(uint64_t time) const {
    absl::MutexLock lock(&mutex_);
    return GetPreviousOrFirstEntryIterator(time)->second;
  }

  using TimeSeriesEntryIter =
      typename absl::btree_map<uint64_t, std::array<double, Dimension>>::const_iterator;

  // If there is no overlap between time range [min_time, max_time] and [StartTimeInNs(),
  // EndTimeInNs()], return empty array. Otherwise return a range of entries affected by the time
  // range [min_time, max_time] where:
  // * the first entry with the time key right before the time range
  // (min_time, max_time) if exists; otherwise points to the fist entry.
  // * the last entry with the time key right after the time range
  // (min_time, max_time) if exists; otherwise points to the last entry.
  [[nodiscard]] std::vector<std::pair<uint64_t, std::array<double, Dimension>>>
  GetEntriesAffectedByTimeRange(uint64_t min_time, uint64_t max_time) const {
    absl::MutexLock lock(&mutex_);
    if (time_to_series_values_.empty() || min_time >= max_time ||
        min_time >= time_to_series_values_.rbegin()->first ||
        max_time <= time_to_series_values_.begin()->first) {
      return {};
    }

    auto current_it = GetPreviousOrFirstEntryIterator(min_time);
    auto last_iterator = GetNextOrLastEntryIterator(max_time);

    std::vector<std::pair<uint64_t, std::array<double, Dimension>>> result;
    result.push_back(*current_it);
    do {
      ++current_it;
      result.push_back(*current_it);
    } while (current_it != last_iterator);

    return result;
  }

 private:
  [[nodiscard]] TimeSeriesEntryIter GetPreviousOrFirstEntryIterator(uint64_t time) const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    ORBIT_CHECK(!time_to_series_values_.empty());

    auto iterator_lower = time_to_series_values_.upper_bound(time);
    if (iterator_lower != time_to_series_values_.begin()) --iterator_lower;
    return iterator_lower;
  }

  [[nodiscard]] TimeSeriesEntryIter GetNextOrLastEntryIterator(uint64_t time) const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    ORBIT_CHECK(!time_to_series_values_.empty());

    auto iterator_higher = time_to_series_values_.lower_bound(time);
    if (iterator_higher == time_to_series_values_.end()) --iterator_higher;
    return iterator_higher;
  }

  void UpdateMinAndMax(double value) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    max_ = std::max(max_, value);
    min_ = std::min(min_, value);
  }

  mutable absl::Mutex mutex_;
  absl::btree_map<uint64_t, std::array<double, Dimension>> time_to_series_values_
      ABSL_GUARDED_BY(mutex_);
  double min_ ABSL_GUARDED_BY(mutex_) = std::numeric_limits<double>::max();
  double max_ ABSL_GUARDED_BY(mutex_) = std::numeric_limits<double>::lowest();

  const std::array<std::string, Dimension> series_names_;
  const uint8_t value_decimal_digits_;
  const std::string value_unit_;
};

#endif  // ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_
