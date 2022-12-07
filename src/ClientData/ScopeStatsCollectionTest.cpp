// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "ClientData/MockScopeIdProvider.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStats.h"
#include "ClientData/ScopeStatsCollection.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

using ::testing::ElementsAre;
using ::testing::IsNull;
using ::testing::Return;

static const ScopeStats kDefaultScopeStats;
static const uint64_t kFunctionId1 = 1;
static const uint64_t kFunctionId2 = 2;
static const ScopeId kScopeId1 = ScopeId(kFunctionId1);
static const ScopeId kScopeId2 = ScopeId(kFunctionId2);

constexpr size_t kNumTimers = 3;
constexpr std::array<uint64_t, kNumTimers> kStarts = {1000, 2050, 6789};
constexpr std::array<uint64_t, kNumTimers> kEnds = {1500, 2059, 9789};
constexpr std::array<uint64_t, kNumTimers> kOrderedDiffs = {9, 500, 3000};
const std::array<TimerInfo, kNumTimers> kTimersScopeId1 = [] {
  std::array<TimerInfo, kNumTimers> timers;
  for (size_t i = 0; i < kNumTimers; i++) {
    timers[i].set_start(kStarts[i]);
    timers[i].set_end(kEnds[i]);
    timers[i].set_function_id(kFunctionId1);
  }
  return timers;
}();
const ScopeStats kScope1Stats = [] {
  ScopeStats stats;
  for (TimerInfo timer : kTimersScopeId1) {
    stats.UpdateStats(timer.end() - timer.start());
  }
  return stats;
}();
const TimerInfo kTimerScopeId2 = [] {
  TimerInfo timer;
  timer.set_start(100);
  timer.set_end(320);
  timer.set_function_id(kFunctionId2);
  return timer;
}();

static void ExpectStatsAreEqual(ScopeStats actual, ScopeStats expect) {
  EXPECT_EQ(actual.count(), expect.count());
  EXPECT_EQ(actual.max_ns(), expect.max_ns());
  EXPECT_EQ(actual.min_ns(), expect.min_ns());
  EXPECT_EQ(actual.total_time_ns(), expect.total_time_ns());
  EXPECT_EQ(actual.variance_ns(), expect.variance_ns());
}

TEST(ScopeStatsCollectionTest, CreateEmpty) {
  ScopeStatsCollection collection = ScopeStatsCollection();
  EXPECT_TRUE(collection.GetAllProvidedScopeIds().empty());
  ScopeStats stats = collection.GetScopeStatsOrDefault(kScopeId1);
  ExpectStatsAreEqual(stats, kDefaultScopeStats);
  EXPECT_THAT(collection.GetSortedTimerDurationsForScopeId(kScopeId1), IsNull());
}

TEST(ScopeStatsCollectionTest, AddTimersWithUpdateStats) {
  ScopeStatsCollection collection = ScopeStatsCollection();
  for (TimerInfo timer : kTimersScopeId1) {
    collection.UpdateScopeStats(kScopeId1, timer);
  }
  EXPECT_EQ(collection.GetAllProvidedScopeIds().size(), 1);

  collection.UpdateScopeStats(kScopeId2, kTimerScopeId2);
  EXPECT_EQ(collection.GetAllProvidedScopeIds().size(), 2);

  ExpectStatsAreEqual(collection.GetScopeStatsOrDefault(kScopeId1), kScope1Stats);

  const auto* timer_durations = collection.GetSortedTimerDurationsForScopeId(kScopeId1);
  EXPECT_THAT(timer_durations, IsNull());
  collection.OnCaptureComplete();
  timer_durations = collection.GetSortedTimerDurationsForScopeId(kScopeId1);
  EXPECT_THAT(*timer_durations, ElementsAre(kOrderedDiffs[0], kOrderedDiffs[1], kOrderedDiffs[2]));
}

TEST(ScopeStatsCollectionTest, CreateWithTimers) {
  MockScopeIdProvider mock_scope_id_provider;
  std::vector<const TimerInfo*> timers;
  timers.push_back(&kTimerScopeId2);
  for (size_t i = 0; i < kNumTimers; ++i) {
    const TimerInfo* timer = &kTimersScopeId1.at(i);
    timers.push_back(timer);
  }
  EXPECT_CALL(mock_scope_id_provider, ProvideId)
      .Times(4)
      .WillOnce(Return(kScopeId2))
      .WillRepeatedly(Return(kScopeId1));
  ScopeStatsCollection collection = ScopeStatsCollection(mock_scope_id_provider, timers);

  EXPECT_EQ(collection.GetAllProvidedScopeIds().size(), 2);
  ExpectStatsAreEqual(collection.GetScopeStatsOrDefault(kScopeId1), kScope1Stats);
  const auto* timer_durations = collection.GetSortedTimerDurationsForScopeId(kScopeId1);
  EXPECT_THAT(*timer_durations, ElementsAre(kOrderedDiffs[0], kOrderedDiffs[1], kOrderedDiffs[2]));
}

}  // namespace orbit_client_data