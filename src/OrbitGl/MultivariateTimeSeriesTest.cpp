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

const std::vector<std::string> kSeriesNames{"Series A", "Series B", "Series C"};
static constexpr uint8_t kDefaultValueDecimalDigits = 6;
static constexpr const char* kDefaultValueUnits = "Unit";

constexpr uint64_t kTimestamp1 = 100;
constexpr std::array<double, 3> kValues1{1.1, 1.2, 1.3};
constexpr uint64_t kTimestamp2 = 200;
constexpr std::array<double, 3> kValues2{2.1, 2.2, 2.3};
constexpr uint64_t kTimestamp3 = 300;
constexpr std::array<double, 3> kValues3{3.1, 3.2, 3.3};

void AddTestValuesToSeries(MultivariateTimeSeries& series) {
  series.AddValues(kTimestamp1, kValues1);
  series.AddValues(kTimestamp2, kValues2);
  series.AddValues(kTimestamp3, kValues3);
}

TEST(MultivariateTimeSeries, BasicSetAndGet) {
  MultivariateTimeSeries series{kSeriesNames, kDefaultValueDecimalDigits, kDefaultValueUnits};
  EXPECT_EQ(series.GetSeriesNames(), kSeriesNames);
  EXPECT_EQ(series.GetValueDecimalDigits(), kDefaultValueDecimalDigits);
  EXPECT_EQ(series.GetValueUnit(), kDefaultValueUnits);
  EXPECT_TRUE(series.IsEmpty());

  AddTestValuesToSeries(series);
  EXPECT_EQ(series.GetTimeToSeriesValuesSize(), 3);
  EXPECT_EQ(series.GetMin(), kValues1[0]);
  EXPECT_EQ(series.GetMax(), kValues3[2]);
  EXPECT_EQ(series.StartTimeInNs(), kTimestamp1);
  EXPECT_EQ(series.EndTimeInNs(), kTimestamp3);
}

TEST(MultivariateTimeSeries, GetPreviousOrFirstEntry) {
  MultivariateTimeSeries series{kSeriesNames, kDefaultValueDecimalDigits, kDefaultValueUnits};
  AddTestValuesToSeries(series);

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
  MultivariateTimeSeries series{kSeriesNames, kDefaultValueDecimalDigits, kDefaultValueUnits};
  AddTestValuesToSeries(series);

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
    EXPECT_EQ(entries[0].first, kTimestamp1);
    EXPECT_THAT(entries[0].second, testing::ElementsAre(1.1, 1.2, 1.3));
    EXPECT_EQ(entries[1].first, kTimestamp2);
    EXPECT_THAT(entries[1].second, testing::ElementsAre(2.1, 2.2, 2.3));
    EXPECT_EQ(entries[2].first, kTimestamp3);
    EXPECT_THAT(entries[2].second, testing::ElementsAre(3.1, 3.2, 3.3));
  }
}

}  // namespace orbit_gl