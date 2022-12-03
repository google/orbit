// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "OrbitGl/MultivariateTimeSeries.h"

namespace orbit_gl {

static constexpr uint8_t kDefaultValueDesimalDigits = 6;
static constexpr const char* kDefaultValueUnits = "";

TEST(MultivariateTimeSeries, BasicSetAndGet) {
  std::array<std::string, 3> series_names = {"Series A", "Series B", "Series C"};
  MultivariateTimeSeries<3> series =
      MultivariateTimeSeries<3>(series_names, kDefaultValueDesimalDigits, kDefaultValueUnits);
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
  MultivariateTimeSeries<3> series = MultivariateTimeSeries<3>(
      {"Series A", "Series B", "Series C"}, kDefaultValueDesimalDigits, kDefaultValueUnits);
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
    auto entry = series.GetPreviousOrFirstEntry(timestamp_before_first_time);
    EXPECT_THAT(entry, testing::ElementsAre(1.1, 1.2, 1.3));
  }

  {
    uint64_t timestamp_within_range = 210;
    auto entry = series.GetPreviousOrFirstEntry(timestamp_within_range);
    EXPECT_THAT(entry, testing::ElementsAre(2.1, 2.2, 2.3));
  }

  {
    uint64_t timestamp_after_last_time = 1000;
    auto entry = series.GetPreviousOrFirstEntry(timestamp_after_last_time);
    EXPECT_THAT(entry, testing::ElementsAre(3.1, 3.2, 3.3));
  }
}

TEST(MultivariateTimeSeries, GetEntriesAffectedByTimeRange) {
  MultivariateTimeSeries<3> series = MultivariateTimeSeries<3>(
      {"Series A", "Series B", "Series C"}, kDefaultValueDesimalDigits, kDefaultValueUnits);
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
    EXPECT_TRUE(range.empty());
  }

  {
    // Test the case that no time series entry with time key located in [min_time, max_time]
    uint64_t min_time = 400;
    uint64_t max_time = 500;
    auto range = series.GetEntriesAffectedByTimeRange(min_time, max_time);
    EXPECT_TRUE(range.empty());
  }

  {
    // Test the case that exists time series entry with time key located in [min_time, max_time]
    uint64_t min_time = 150;
    uint64_t max_time = 400;
    auto entries = series.GetEntriesAffectedByTimeRange(min_time, max_time);
    ASSERT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].first, timestamp_1);
    EXPECT_THAT(entries[0].second, testing::ElementsAre(1.1, 1.2, 1.3));
    EXPECT_EQ(entries[1].first, timestamp_2);
    EXPECT_THAT(entries[1].second, testing::ElementsAre(2.1, 2.2, 2.3));
    EXPECT_EQ(entries[2].first, timestamp_3);
    EXPECT_THAT(entries[2].second, testing::ElementsAre(3.1, 3.2, 3.3));
  }
}

TEST(MultivariateTimeSeries, GetValueDecimalDigits) {
  MultivariateTimeSeries<3> series =
      MultivariateTimeSeries<3>({"Series A", "Series B", "Series C"}, 42, kDefaultValueUnits);
  EXPECT_EQ(series.GetValueDecimalDigits(), 42);
}

TEST(MultivariateTimeSeries, GetValueUnits) {
  MultivariateTimeSeries<3> series = MultivariateTimeSeries<3>(
      {"Series A", "Series B", "Series C"}, kDefaultValueDesimalDigits, "Meeples");
  EXPECT_EQ(series.GetValueUnit(), "Meeples");
}
}  // namespace orbit_gl