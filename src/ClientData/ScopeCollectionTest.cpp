// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iterator>

#include "ClientData/ScopeCollection.h"
#include "ClientData/ScopeIdProvider.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

static const ScopeStats kDefaultScopeStats;
static const uint64_t kFunctionId = 1;
static const ScopeId kScopeId = ScopeId(kFunctionId);

constexpr size_t kNumTimers = 3;
constexpr std::array<uint64_t, kNumTimers> kStarts = {1000, 2050, 6789};
constexpr std::array<uint64_t, kNumTimers> kEnds = {1500, 2059, 9789};
const std::array<TimerInfo, kNumTimers> kTimersScopeId1 = []() {
  std::array<TimerInfo, kNumTimers> timers;
  for (size_t i = 0; i < kNumTimers; i++) {
    timers[i].set_start(kStarts[i]);
    timers[i].set_end(kEnds[i]);
    timers[i].set_function_id(kFunctionId);
  }
  return timers;
}();
const ScopeStats kScope1Stats = []() {
  ScopeStats stats;
  for (TimerInfo timer : kTimersScopeId1) {
    stats.UpdateStats(timer.end() - timer.start());
  }
  return stats;
}();
const TimerInfo kTimerScopeId2 = []() {
  TimerInfo timer;
  timer.set_start(100);
  timer.set_end(320);
  timer.set_function_id(2);
  return timer;
}();

static void AssertStatsAreEqual(ScopeStats actual, ScopeStats expect) {
  ASSERT_EQ(actual.count(), expect.count());
  ASSERT_EQ(actual.max_ns(), expect.max_ns());
  ASSERT_EQ(actual.min_ns(), expect.min_ns());
  ASSERT_EQ(actual.total_time_ns(), expect.total_time_ns());
  ASSERT_EQ(actual.variance_ns(), expect.variance_ns());
}

TEST(ScopeCollectionTest, CreateEmpty) {
  ScopeCollection collection = ScopeCollection();
  ASSERT_TRUE(collection.GetAllProvidedScopeIds().empty());
  ScopeStats stats = collection.GetScopeStatsOrDefault(kScopeId);
  AssertStatsAreEqual(stats, kDefaultScopeStats);
  ASSERT_EQ(collection.GetSortedTimerDurationsForScopeId(kScopeId), nullptr);
}

TEST(ScopeCollectionTest, AddTimersWithUpdateStats) {
  ScopeCollection collection = ScopeCollection();
  for (TimerInfo timer : kTimersScopeId1) {
    collection.UpdateScopeStats(kScopeId, timer);
  }
  ASSERT_EQ(collection.GetAllProvidedScopeIds().size(), 1);

  collection.UpdateScopeStats(ScopeId(2), kTimerScopeId2);
  ASSERT_EQ(collection.GetAllProvidedScopeIds().size(), 2);

  AssertStatsAreEqual(collection.GetScopeStatsOrDefault(kScopeId), kScope1Stats);
  const auto* timer_durations = collection.GetSortedTimerDurationsForScopeId(kScopeId);
  ASSERT_NE(timer_durations, nullptr);
  ASSERT_EQ(timer_durations->size(), kNumTimers);
  ASSERT_THAT(*timer_durations, testing::ElementsAre(9, 500, 3000));
}

TEST(ScopeCollectionTest, CreateWithTimers) {
  orbit_grpc_protos::CaptureOptions capture_options;
  std::unique_ptr<ScopeIdProvider> scope_id_provider =
      NameEqualityScopeIdProvider::Create(capture_options);
  std::vector<const TimerInfo*> timers;
  for (size_t i = 0; i < kNumTimers; ++i) {
    const TimerInfo* timer = &kTimersScopeId1.at(i);
    timers.emplace_back(timer);
  }
  timers.emplace_back(&kTimerScopeId2);
  ScopeCollection collection = ScopeCollection(*scope_id_provider, timers);

  ASSERT_EQ(collection.GetAllProvidedScopeIds().size(), 2);
  AssertStatsAreEqual(collection.GetScopeStatsOrDefault(kScopeId), kScope1Stats);
  const auto* timer_durations = collection.GetSortedTimerDurationsForScopeId(kScopeId);
  ASSERT_NE(timer_durations, nullptr);
  ASSERT_EQ(timer_durations->size(), kNumTimers);
  ASSERT_THAT(*timer_durations, testing::ElementsAre(9, 500, 3000));
}

}  // namespace orbit_client_data