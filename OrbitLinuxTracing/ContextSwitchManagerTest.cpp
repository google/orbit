// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "ContextSwitchManager.h"

namespace LinuxTracing {

using orbit_grpc_protos::SchedulingSlice;

TEST(ContextSwitchManager, OneCoreMatch) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid, kTid, kCore, 100);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid, kTid, kCore, 101);
  ASSERT_TRUE(processed_scheduling_slice.has_value());
  EXPECT_EQ(processed_scheduling_slice.value().pid(), kPid);
  EXPECT_EQ(processed_scheduling_slice.value().tid(), kTid);
  EXPECT_EQ(processed_scheduling_slice.value().core(), kCore);
  EXPECT_EQ(processed_scheduling_slice.value().in_timestamp_ns(), 100);
  EXPECT_EQ(processed_scheduling_slice.value().out_timestamp_ns(), 101);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid, kTid, kCore, 102);
  ASSERT_FALSE(processed_scheduling_slice.has_value());
}

TEST(ContextSwitchManager, OneCoreThreadExit) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid, kTid, kCore, 100);

  processed_scheduling_slice = context_switch_manager.ProcessContextSwitchOut(-1, -1, kCore, 101);
  ASSERT_TRUE(processed_scheduling_slice.has_value());
  EXPECT_EQ(processed_scheduling_slice.value().pid(), kPid);
  EXPECT_EQ(processed_scheduling_slice.value().tid(), kTid);
  EXPECT_EQ(processed_scheduling_slice.value().core(), kCore);
  EXPECT_EQ(processed_scheduling_slice.value().in_timestamp_ns(), 100);
  EXPECT_EQ(processed_scheduling_slice.value().out_timestamp_ns(), 101);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid, kTid, kCore, 102);
  ASSERT_FALSE(processed_scheduling_slice.has_value());
}

TEST(ContextSwitchManager, OneCoreInMissing) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid, kTid, kCore, 101);
  ASSERT_FALSE(processed_scheduling_slice.has_value());
}

TEST(ContextSwitchManager, OneCoreMismatch) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid, kTid, kCore, 100);

  processed_scheduling_slice = context_switch_manager.ProcessContextSwitchOut(kPid, 77, kCore, 101);
  ASSERT_FALSE(processed_scheduling_slice.has_value());
}

TEST(ContextSwitchManager, OneCoreTwoMatches) {
  constexpr pid_t kPid1 = 42;
  constexpr pid_t kTid1 = 43;
  constexpr pid_t kPid2 = 52;
  constexpr pid_t kTid2 = 53;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid1, kTid1, kCore, 100);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid1, kTid1, kCore, 101);
  ASSERT_TRUE(processed_scheduling_slice.has_value());
  EXPECT_EQ(processed_scheduling_slice.value().pid(), kPid1);
  EXPECT_EQ(processed_scheduling_slice.value().tid(), kTid1);
  EXPECT_EQ(processed_scheduling_slice.value().core(), kCore);
  EXPECT_EQ(processed_scheduling_slice.value().in_timestamp_ns(), 100);
  EXPECT_EQ(processed_scheduling_slice.value().out_timestamp_ns(), 101);

  context_switch_manager.ProcessContextSwitchIn(kPid2, kTid2, kCore, 102);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid2, kTid2, kCore, 103);
  ASSERT_TRUE(processed_scheduling_slice.has_value());
  EXPECT_EQ(processed_scheduling_slice.value().pid(), kPid2);
  EXPECT_EQ(processed_scheduling_slice.value().tid(), kTid2);
  EXPECT_EQ(processed_scheduling_slice.value().core(), kCore);
  EXPECT_EQ(processed_scheduling_slice.value().in_timestamp_ns(), 102);
  EXPECT_EQ(processed_scheduling_slice.value().out_timestamp_ns(), 103);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid2, kTid2, kCore, 104);
  ASSERT_FALSE(processed_scheduling_slice.has_value());
}

TEST(ContextSwitchManager, TwoCoresMatches) {
  constexpr pid_t kPid1 = 42;
  constexpr pid_t kTid1 = 43;
  constexpr uint16_t kCore1 = 1;
  constexpr pid_t kPid2 = 52;
  constexpr pid_t kTid2 = 53;
  constexpr uint16_t kCore2 = 2;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid1, kTid1, kCore1, 100);

  context_switch_manager.ProcessContextSwitchIn(kPid2, kTid2, kCore2, 101);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid2, kTid2, kCore2, 103);
  ASSERT_TRUE(processed_scheduling_slice.has_value());
  EXPECT_EQ(processed_scheduling_slice.value().pid(), kPid2);
  EXPECT_EQ(processed_scheduling_slice.value().tid(), kTid2);
  EXPECT_EQ(processed_scheduling_slice.value().core(), kCore2);
  EXPECT_EQ(processed_scheduling_slice.value().in_timestamp_ns(), 101);
  EXPECT_EQ(processed_scheduling_slice.value().out_timestamp_ns(), 103);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid1, kTid1, kCore1, 102);
  ASSERT_TRUE(processed_scheduling_slice.has_value());
  EXPECT_EQ(processed_scheduling_slice.value().pid(), kPid1);
  EXPECT_EQ(processed_scheduling_slice.value().tid(), kTid1);
  EXPECT_EQ(processed_scheduling_slice.value().core(), kCore1);
  EXPECT_EQ(processed_scheduling_slice.value().in_timestamp_ns(), 100);
  EXPECT_EQ(processed_scheduling_slice.value().out_timestamp_ns(), 102);

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid1, kTid1, kCore1, 104);
  ASSERT_FALSE(processed_scheduling_slice.has_value());

  processed_scheduling_slice =
      context_switch_manager.ProcessContextSwitchOut(kPid2, kTid2, kCore2, 105);
  ASSERT_FALSE(processed_scheduling_slice.has_value());

  processed_scheduling_slice = context_switch_manager.ProcessContextSwitchOut(62, 63, 3, 106);
  ASSERT_FALSE(processed_scheduling_slice.has_value());
}

TEST(ContextSwitchManager, TwoCoresOutOnDifferentCore) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid, kTid, kCore, 100);

  processed_scheduling_slice = context_switch_manager.ProcessContextSwitchOut(kPid, kTid, 2, 101);
}

TEST(ContextSwitchManager, OneCoreOutOfOrder) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint16_t kCore = 1;
  std::optional<SchedulingSlice> processed_scheduling_slice;
  ContextSwitchManager context_switch_manager;

  context_switch_manager.ProcessContextSwitchIn(kPid, kTid, kCore, 100);

  EXPECT_DEATH(context_switch_manager.ProcessContextSwitchOut(52, 53, kCore, 99), "");
}

}  // namespace LinuxTracing
