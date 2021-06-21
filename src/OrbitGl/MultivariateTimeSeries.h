// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_
#define ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_

#include <array>
#include <map>
#include <optional>
#include <string>

#include "OrbitBase/Logging.h"

template <size_t Dimension>
class MultivariateTimeSeries {
  static_assert(Dimension >= 1, "Dimension must be at least 1");

 public:
  explicit MultivariateTimeSeries(std::array<std::string, Dimension> series_names)
      : series_names_(std::move(series_names)) {}

  [[nodiscard]] const std::array<std::string, Dimension>& GetSeriesNames() const {
    return series_names_;
  }
  [[nodiscard]] const std::map<uint64_t, std::array<double, Dimension>>& GetTimeToSeriesValues()
      const {
    return time_to_series_values_;
  }
  [[nodiscard]] double GetMin() const { return min_; }
  [[nodiscard]] double GetMax() const { return max_; }
  [[nodiscard]] std::optional<uint8_t> GetValueDecimalDigits() const {
    return value_decimal_digits_;
  }
  [[nodiscard]] std::string GetValueUnit() const { return value_unit_; }

  void AddValues(uint64_t timestamp_ns, const std::array<double, Dimension>& values) {
    time_to_series_values_[timestamp_ns] = values;
    for (double value : values) {
      UpdateMinAndMax(value);
    }
  }
  void SetValueUnit(std::string value_unit) { value_unit_ = std::move(value_unit); }
  void SetNumberOfDecimalDigits(uint8_t value_decimal_digits) {
    value_decimal_digits_ = value_decimal_digits;
  }

  [[nodiscard]] bool IsEmpty() const { return time_to_series_values_.empty(); }

  [[nodiscard]] uint64_t StartTimeInNs() const {
    CHECK(!IsEmpty());
    return time_to_series_values_.begin()->first;
  }
  [[nodiscard]] uint64_t EndTimeInNs() const {
    CHECK(!IsEmpty());
    return time_to_series_values_.rbegin()->first;
  }

  using TimeSeriesEntryIter =
      typename std::map<uint64_t, std::array<double, Dimension>>::const_iterator;
  [[nodiscard]] TimeSeriesEntryIter GetPreviousOrFirstEntry(uint64_t time) const {
    CHECK(!IsEmpty());

    auto iterator_lower = time_to_series_values_.upper_bound(time);
    if (iterator_lower != time_to_series_values_.begin()) --iterator_lower;
    return iterator_lower;
  }
  [[nodiscard]] TimeSeriesEntryIter GetNextOrLastEntry(uint64_t time) const {
    CHECK(!IsEmpty());

    auto iterator_higher = time_to_series_values_.lower_bound(time);
    if (iterator_higher == time_to_series_values_.end()) --iterator_higher;
    return iterator_higher;
  }

  struct Range {
    TimeSeriesEntryIter begin;
    TimeSeriesEntryIter end;
  };
  // If there is no overlap between time range [min_time, max_time] and [StartTimeInNs(),
  // EndTimeInNs()], return std::nullopt. Otherwise return a range of entries affected by the time
  // range [min_time, max_time] where:
  // * `Range::begin` points to the entry with the time key right before the time range (min_time,
  // max_time) if exists; otherwise points to the fist entry.
  // * `Range::end` points to the entry with the time key right after the time range (min_time,
  // max_time) if exists; otherwise points to the last entry.
  [[nodiscard]] std::optional<Range> GetEntriesAffectedByTimeRange(uint64_t min_time,
                                                                   uint64_t max_time) const {
    if (IsEmpty() || min_time >= max_time || min_time >= time_to_series_values_.rbegin()->first ||
        max_time <= time_to_series_values_.begin()->first) {
      return std::nullopt;
    }

    auto first_iterator = GetPreviousOrFirstEntry(min_time);
    auto last_iterator = GetNextOrLastEntry(max_time);
    return Range{first_iterator, last_iterator};
  }

 private:
  void UpdateMinAndMax(double value) {
    max_ = std::max(max_, value);
    min_ = std::min(min_, value);
  }

  std::array<std::string, Dimension> series_names_;
  std::map<uint64_t, std::array<double, Dimension>> time_to_series_values_;
  double min_ = std::numeric_limits<double>::max();
  double max_ = std::numeric_limits<double>::lowest();
  std::optional<uint8_t> value_decimal_digits_ = std::nullopt;
  std::string value_unit_;
};

#endif  // ORBIT_GL_MULTIVARIATE_TIME_SERIES_H_
