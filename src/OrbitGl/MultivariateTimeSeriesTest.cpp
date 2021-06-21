// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "MultivariateTimeSeries.h"

namespace orbit_gl {

TEST(MultivariateTimeSeries, BasicSetAndGet) {
  std::array<std::string, 3> series_names = {"Series A", "Series B", "Series C"};
  MultivariateTimeSeries<3> series = MultivariateTimeSeries<3>(series_names);
  EXPECT_EQ(series.GetSeriesNames(), series_names);
  EXPECT_TRUE(series.IsEmpty());

  uint64_t timestamp_1 = 100;
  std::array<double, 3> values_1 = {1.1, 1.2, 1.3};
  uint64_t timestamp_2 = 200;
  std::array<double, 3> values_2 = {2.1, 2.2, 2.3};
  uint64_t timestamp_3 = 300;
  std::array<double, 3> values_3 = {3.1, 3.2, 3.3};
  series.AddValues(timestamp_1, values_1);
  series.AddValues(timestamp_2, values_2);
  series.AddValues(timestamp_3, values_3);
  EXPECT_FALSE(series.IsEmpty());
  EXPECT_EQ(series.GetMax(), values_3[2]);
  EXPECT_EQ(series.GetMin(), values_1[0]);
  EXPECT_EQ(series.StartTimeInNs(), timestamp_1);
  EXPECT_EQ(series.EndTimeInNs(), timestamp_3);
}

TEST(MultivariateTimeSeries, GetPreviousOrFirstEntry) {
  MultivariateTimeSeries<3> series =
      MultivariateTimeSeries<3>({"Series A", "Series B", "Series C"});
  uint64_t timestamp_1 = 100;
  std::array<double, 3> values_1 = {1.1, 1.2, 1.3};
  uint64_t timestamp_2 = 200;
  std::array<double, 3> values_2 = {2.1, 2.2, 2.3};
  uint64_t timestamp_3 = 300;
  std::array<double, 3> values_3 = {3.1, 3.2, 3.3};
  series.AddValues(timestamp_1, values_1);
  series.AddValues(timestamp_2, values_2);
  series.AddValues(timestamp_3, values_3);

  {
    uint64_t timestamp_before_first_time = 50;
    auto it = series.GetPreviousOrFirstEntry(timestamp_before_first_time);
    EXPECT_EQ(it->first, timestamp_1);
  }

  {
    uint64_t timestamp_within_range = 210;
    auto it = series.GetPreviousOrFirstEntry(timestamp_within_range);
    EXPECT_EQ(it->first, timestamp_2);
  }

  {
    uint64_t timestamp_after_last_time = 1000;
    auto it = series.GetPreviousOrFirstEntry(timestamp_after_last_time);
    EXPECT_EQ(it->first, timestamp_3);
  }
}

TEST(MultivariateTimeSeries, GetNextOrLastEntry) {
  MultivariateTimeSeries<3> series =
      MultivariateTimeSeries<3>({"Series A", "Series B", "Series C"});
  uint64_t timestamp_1 = 100;
  std::array<double, 3> values_1 = {1.1, 1.2, 1.3};
  uint64_t timestamp_2 = 200;
  std::array<double, 3> values_2 = {2.1, 2.2, 2.3};
  uint64_t timestamp_3 = 300;
  std::array<double, 3> values_3 = {3.1, 3.2, 3.3};
  series.AddValues(timestamp_1, values_1);
  series.AddValues(timestamp_2, values_2);
  series.AddValues(timestamp_3, values_3);

  {
    uint64_t timestamp_before_first_time = 50;
    auto it = series.GetNextOrLastEntry(timestamp_before_first_time);
    EXPECT_EQ(it->first, timestamp_1);
  }

  {
    uint64_t timestamp_within_range = 210;
    auto it = series.GetNextOrLastEntry(timestamp_within_range);
    EXPECT_EQ(it->first, timestamp_3);
  }

  {
    uint64_t timestamp_after_last_time = 1000;
    auto it = series.GetNextOrLastEntry(timestamp_after_last_time);
    EXPECT_EQ(it->first, timestamp_3);
  }
}

TEST(MultivariateTimeSeries, GetEntriesAffectedByTimeRange) {
  MultivariateTimeSeries<3> series =
      MultivariateTimeSeries<3>({"Series A", "Series B", "Series C"});
  uint64_t timestamp_1 = 100;
  std::array<double, 3> values_1 = {1.1, 1.2, 1.3};
  uint64_t timestamp_2 = 200;
  std::array<double, 3> values_2 = {2.1, 2.2, 2.3};
  uint64_t timestamp_3 = 300;
  std::array<double, 3> values_3 = {3.1, 3.2, 3.3};
  series.AddValues(timestamp_1, values_1);
  series.AddValues(timestamp_2, values_2);
  series.AddValues(timestamp_3, values_3);

  {
    // Test the case that min_time >= max time
    uint64_t min_time = 300;
    uint64_t max_time = 100;
    auto range = series.GetEntriesAffectedByTimeRange(min_time, max_time);
    EXPECT_FALSE(range.has_value());
  }

  {
    // Test the case that no time series entry with time key located in [min_time, max_time]
    uint64_t min_time = 400;
    uint64_t max_time = 500;
    auto range = series.GetEntriesAffectedByTimeRange(min_time, max_time);
    EXPECT_FALSE(range.has_value());
  }

  {
    // Test the case that exists time series entry with time key located in [min_time, max_time]
    uint64_t min_time = 150;
    uint64_t max_time = 400;
    auto range = series.GetEntriesAffectedByTimeRange(min_time, max_time);
    EXPECT_EQ(range.value().begin->first, timestamp_1);
    EXPECT_EQ(range.value().end->first, timestamp_3);
  }
}

}  // namespace orbit_gl