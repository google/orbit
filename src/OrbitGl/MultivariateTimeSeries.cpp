// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/MultivariateTimeSeries.h"

#include <algorithm>
#include <iterator>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

MultivariateTimeSeries::MultivariateTimeSeries(std::vector<std::string> series_names,
                                               uint8_t value_decimal_digits, std::string value_unit)
    : series_names_{std::move(series_names)},
      value_decimal_digits_{value_decimal_digits},
      value_unit_{std::move(value_unit)} {
  ORBIT_CHECK(!series_names_.empty());
}

double MultivariateTimeSeries::GetMin() const {
  absl::MutexLock lock(&mutex_);
  return min_;
}

double MultivariateTimeSeries::GetMax() const {
  absl::MutexLock lock(&mutex_);
  return max_;
}

bool MultivariateTimeSeries::IsEmpty() const {
  absl::MutexLock lock(&mutex_);
  return time_to_series_values_.empty();
}

size_t MultivariateTimeSeries::GetTimeToSeriesValuesSize() const {
  absl::MutexLock lock(&mutex_);
  return time_to_series_values_.size();
}

uint64_t MultivariateTimeSeries::StartTimeInNs() const {
  absl::MutexLock lock(&mutex_);
  ORBIT_CHECK(!time_to_series_values_.empty());
  return time_to_series_values_.begin()->first;
}

uint64_t MultivariateTimeSeries::EndTimeInNs() const {
  absl::MutexLock lock(&mutex_);
  ORBIT_CHECK(!time_to_series_values_.empty());
  return time_to_series_values_.rbegin()->first;
}

std::vector<double> MultivariateTimeSeries::GetPreviousOrFirstEntry(uint64_t time) const {
  absl::MutexLock lock(&mutex_);
  return GetPreviousOrFirstEntryIterator(time)->second;
}

std::vector<std::pair<uint64_t, std::vector<double>>>
MultivariateTimeSeries::GetEntriesAffectedByTimeRange(uint64_t min_time, uint64_t max_time) const {
  absl::MutexLock lock(&mutex_);
  if (time_to_series_values_.empty() || min_time >= max_time ||
      min_time >= time_to_series_values_.rbegin()->first ||
      max_time <= time_to_series_values_.begin()->first) {
    return {};
  }

  auto current_it = GetPreviousOrFirstEntryIterator(min_time);
  auto last_iterator = GetNextOrLastEntryIterator(max_time);

  std::vector<std::pair<uint64_t, std::vector<double>>> result;
  result.push_back(*current_it);
  do {
    ++current_it;
    result.push_back(*current_it);
  } while (current_it != last_iterator);

  return result;
}

void MultivariateTimeSeries::AddValues(uint64_t timestamp_ns, absl::Span<const double> values) {
  ORBIT_CHECK(values.size() == series_names_.size());

  absl::MutexLock lock(&mutex_);
  time_to_series_values_[timestamp_ns] = std::vector(values.begin(), values.end());
  for (double value : values) UpdateMinAndMax(value);
}

MultivariateTimeSeries::TimeSeriesEntryIter MultivariateTimeSeries::GetPreviousOrFirstEntryIterator(
    uint64_t time) const {
  ORBIT_CHECK(!time_to_series_values_.empty());

  auto iterator_lower = time_to_series_values_.upper_bound(time);
  if (iterator_lower != time_to_series_values_.begin()) --iterator_lower;
  return iterator_lower;
}

MultivariateTimeSeries::TimeSeriesEntryIter MultivariateTimeSeries::GetNextOrLastEntryIterator(
    uint64_t time) const {
  ORBIT_CHECK(!time_to_series_values_.empty());

  auto iterator_higher = time_to_series_values_.lower_bound(time);
  if (iterator_higher == time_to_series_values_.end()) --iterator_higher;
  return iterator_higher;
}

void MultivariateTimeSeries::UpdateMinAndMax(double value) {
  max_ = std::max(max_, value);
  min_ = std::min(min_, value);
}

}  // namespace orbit_gl
