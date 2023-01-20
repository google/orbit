// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "OrbitGl/GraphTrackDataAggregator.h"

using ::testing::ElementsAre;

namespace orbit_gl {

TEST(GraphTrackDataAggregator, InitialEntryIsEmpty) {
  GraphTrackDataAggregator aggr;
  EXPECT_EQ(aggr.GetAccumulatedEntry(), nullptr);
}

TEST(GraphTrackDataAggregator, CanStartEntry) {
  GraphTrackDataAggregator aggr;
  aggr.SetEntry(1, 2, {3.f});
  ASSERT_NE(aggr.GetAccumulatedEntry(), nullptr);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->start_tick, 1);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->end_tick, 2);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->min_vals[0], 3.f);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->max_vals[0], 3.f);
}

TEST(GraphTrackDataAggregator, MergingDataIntoEmptyStartsNewEntry) {
  GraphTrackDataAggregator aggr;
  aggr.MergeDataIntoEntry(1, 2, {3.f});
  ASSERT_NE(aggr.GetAccumulatedEntry(), nullptr);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->start_tick, 1);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->end_tick, 2);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->min_vals[0], 3.f);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->max_vals[0], 3.f);
}

TEST(GraphTrackDataAggregator, StartNewEntryOverridesPrevious) {
  GraphTrackDataAggregator aggr;
  aggr.SetEntry(1, 2, {3.f});
  aggr.SetEntry(3, 4, {5.f});
  EXPECT_EQ(aggr.GetAccumulatedEntry()->start_tick, 3);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->end_tick, 4);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->min_vals[0], 5.f);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->max_vals[0], 5.f);
}

TEST(GraphTrackDataAggregator, MaxAggrPicksMaxValues) {
  GraphTrackDataAggregator aggr;
  aggr.SetEntry(1, 2, {10.f, 20.f});
  aggr.MergeDataIntoEntry(3, 4, {1.f, 100.f});
  EXPECT_THAT(aggr.GetAccumulatedEntry()->max_vals, ElementsAre(10.f, 100.f));
}

TEST(GraphTrackDataAggregator, MinAggrPicksMinValues) {
  GraphTrackDataAggregator aggr;
  aggr.SetEntry(1, 2, {10.f, 20.f});
  aggr.MergeDataIntoEntry(3, 4, {1.f, 100.f});
  EXPECT_THAT(aggr.GetAccumulatedEntry()->min_vals, ElementsAre(1.f, 20.f));
}

TEST(GraphTrackDataAggregator, TimeBoundsAreMerged) {
  GraphTrackDataAggregator aggr;
  aggr.SetEntry(1, 2, {});
  aggr.MergeDataIntoEntry(10, 20, {});
  EXPECT_EQ(aggr.GetAccumulatedEntry()->start_tick, 1);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->end_tick, 20);
}

TEST(GraphTrackDataAggregator, DataCanBeAddedOutOfOrder) {
  GraphTrackDataAggregator aggr;
  aggr.SetEntry(10, 20, {});
  aggr.MergeDataIntoEntry(1, 10, {});
  aggr.MergeDataIntoEntry(30, 40, {});
  EXPECT_EQ(aggr.GetAccumulatedEntry()->start_tick, 1);
  EXPECT_EQ(aggr.GetAccumulatedEntry()->end_tick, 40);
}

}  // namespace orbit_gl
