// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CaptureStats.h"
#include "SchedulerTrack.h"
#include "SchedulingStats.h"
#include "TextBox.h"

TextBox* CreateScope(int32_t pid, int32_t tid, int32_t cpu, uint64_t start, uint64_t end) {
  const size_t kNumScopes = 1024;
  static TextBox scope_buffer[kNumScopes];
  static size_t index = 0;
  EXPECT_TRUE(index < kNumScopes - 2);
  TextBox& scope = scope_buffer[index++];
  orbit_client_protos::TimerInfo& timer_info = scope.GetMutableTimerInfo();
  timer_info.set_start(start);
  timer_info.set_end(end);
  timer_info.set_thread_id(tid);
  timer_info.set_process_id(pid);
  timer_info.set_processor(cpu);
  return &scope;
}

TEST(CaptureStats, NullCaptureWindow) {
  CaptureStats capture_stats;
  auto result = capture_stats.Generate(/*capture_window=*/nullptr, 0, 0);
  EXPECT_EQ(result.has_error(), true);
}

TEST(SchedulingStats, ZeroSchedulingScopes) {
  std::vector<const TextBox*> scheduling_scopes;
  SchedulingStats::ThreadNameProvider thread_name_provider = [](int32_t thread_id) {
    return std::to_string(thread_id);
  };
  SchedulingStats scheduling_stats(scheduling_scopes, thread_name_provider, 0, 0);

  EXPECT_EQ(scheduling_stats.GetTimeRangeMs(), 0);
  EXPECT_EQ(scheduling_stats.GetTimeOnCoreNs(), 0);
  EXPECT_EQ(scheduling_stats.GetTimeOnCoreNsByCore().size(), 0);
  EXPECT_EQ(scheduling_stats.GetProcessStatsByPid().size(), 0);
  EXPECT_EQ(scheduling_stats.GetProcessStatsSortedByTimeOnCore().size(), 0);
  EXPECT_STREQ(scheduling_stats.ToString().c_str(), "");
}

TEST(SchedulingStats, SchedulingStats) {
  std::vector<const TextBox*> scopes;
  SchedulingStats::ThreadNameProvider thread_name_provider = [](int32_t thread_id) {
    return std::to_string(thread_id);
  };

  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/0, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/1, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/2, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/3, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/4, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/5, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/6, /*start_ns=*/0, /*end_ns=*/10));
  scopes.push_back(CreateScope(/*pid=*/0, /*tid=*/1, /*cpu=*/7, /*start_ns=*/0, /*end_ns=*/10));

  {
    SchedulingStats scheduling_stats(scopes, thread_name_provider, /*start_ns=*/0, /*end_ns*/ 1000);

    EXPECT_EQ(scheduling_stats.GetTimeRangeMs(), 0.001);
    EXPECT_EQ(scheduling_stats.GetTimeOnCoreNs(), 80);
    EXPECT_EQ(scheduling_stats.GetTimeOnCoreNsByCore().size(), 8);
    EXPECT_EQ(scheduling_stats.GetProcessStatsByPid().size(), 1);
    EXPECT_EQ(scheduling_stats.GetProcessStatsSortedByTimeOnCore().size(), 1);
  }

  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/0, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/1, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/2, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/3, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/4, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/5, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/6, /*start_ns=*/0, /*end_ns=*/1000));
  scopes.push_back(CreateScope(/*pid=*/1, /*tid=*/2, /*cpu=*/7, /*start_ns=*/0, /*end_ns=*/1000));

  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/0, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/1, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/2, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/3, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/4, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/5, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/6, /*start_ns=*/0, /*end_ns=*/100));
  scopes.push_back(CreateScope(/*pid=*/2, /*tid=*/3, /*cpu=*/7, /*start_ns=*/0, /*end_ns=*/100));

  {
    SchedulingStats scheduling_stats(scopes, thread_name_provider, /*start_ns=*/0, /*end_ns*/ 1000);

    EXPECT_EQ(scheduling_stats.GetTimeRangeMs(), 0.001);
    EXPECT_EQ(scheduling_stats.GetTimeOnCoreNsByCore().size(), 8);
    EXPECT_EQ(scheduling_stats.GetProcessStatsByPid().size(), 3);
    EXPECT_EQ(scheduling_stats.GetProcessStatsSortedByTimeOnCore().size(), 3);

    EXPECT_EQ(scheduling_stats.GetProcessStatsSortedByTimeOnCore()[0]->pid, 1);
    EXPECT_EQ(scheduling_stats.GetProcessStatsSortedByTimeOnCore()[1]->pid, 2);
    EXPECT_EQ(scheduling_stats.GetProcessStatsSortedByTimeOnCore()[2]->pid, 0);
  }
}
