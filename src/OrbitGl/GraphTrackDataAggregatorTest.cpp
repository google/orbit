// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "GraphTrackDataAggregator.h"

using ::testing::ElementsAre;
using ::testing::FloatEq;

namespace {

TEST(GraphTrackDataAggregator, InitialEntryIsEmpty) {
  GraphTrackDataAggregator<1> aggr{GraphTrackAggregationMode::kAvg};
  EXPECT_EQ(aggr.GetEntry().start_tick, 0);
  EXPECT_EQ(aggr.GetEntry().end_tick, 0);
  EXPECT_EQ(aggr.GetEntry().values[0], 0);
}

TEST(GraphTrackDataAggregator, CanStartEntry) {
  GraphTrackDataAggregator<1> aggr{GraphTrackAggregationMode::kAvg};
  aggr.StartNewEntry(1, 2, {3.f});
  EXPECT_EQ(aggr.GetEntry().start_tick, 1);
  EXPECT_EQ(aggr.GetEntry().end_tick, 2);
  EXPECT_EQ(aggr.GetEntry().values[0], 3.f);
}

TEST(GraphTrackDataAggregator, StartNewEntryOverridesPrevious) {
  GraphTrackDataAggregator<1> aggr{GraphTrackAggregationMode::kAvg};
  aggr.StartNewEntry(1, 2, {3.f});
  aggr.StartNewEntry(3, 4, {5.f});
  EXPECT_EQ(aggr.GetEntry().start_tick, 3);
  EXPECT_EQ(aggr.GetEntry().end_tick, 4);
  EXPECT_EQ(aggr.GetEntry().values[0], 5.f);
}

TEST(GraphTrackDataAggregator, MaxAggrPicksMaxValues) {
  GraphTrackDataAggregator<2> aggr{GraphTrackAggregationMode::kMax};
  aggr.StartNewEntry(1, 2, {10.f, 20.f});
  aggr.AppendData(3, 4, {1.f, 100.f});
  EXPECT_THAT(aggr.GetEntry().values, ElementsAre(10.f, 100.f));
}

TEST(GraphTrackDataAggregator, AvgIsWeighted) {
  GraphTrackDataAggregator<1> aggr{GraphTrackAggregationMode::kAvg};
  aggr.StartNewEntry(1, 1, {1.f});
  aggr.AppendData(2, 3, {4.f});
  EXPECT_THAT(aggr.GetEntry().values[0], FloatEq(3.f));
}

TEST(GraphTrackDataAggregator, TimeBoundsAreMerged) {
  GraphTrackDataAggregator<1> aggr{GraphTrackAggregationMode::kMax};
  aggr.StartNewEntry(1, 2, {});
  aggr.AppendData(10, 20, {});
  EXPECT_EQ(aggr.GetEntry().start_tick, 1);
  EXPECT_EQ(aggr.GetEntry().end_tick, 20);
}

TEST(GraphTrackDataAggregator, TimeBoundsAreInclusive) {
  GraphTrackDataAggregator<1> aggr{GraphTrackAggregationMode::kAvg};
  aggr.StartNewEntry(0, 0, {0.f});
  aggr.AppendData(1, 1, {3.f});
  EXPECT_THAT(aggr.GetEntry().values[0], FloatEq(1.5f));
}

}  // namespace
