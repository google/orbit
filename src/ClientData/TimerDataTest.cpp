// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/TimerData.h"

namespace orbit_client_data {

using orbit_client_protos::TimerInfo;

TEST(TimerData, IsEmpty) {
  TimerData timer_data;
  EXPECT_TRUE(timer_data.GetChains().empty());
  EXPECT_EQ(timer_data.GetChain(0), nullptr);
  EXPECT_EQ(timer_data.GetChain(7), nullptr);
  EXPECT_TRUE(timer_data.IsEmpty());
  EXPECT_EQ(timer_data.GetNumberOfTimers(), 0);
  EXPECT_EQ(timer_data.GetMaxTime(), std::numeric_limits<uint64_t>::min());
  EXPECT_EQ(timer_data.GetMinTime(), std::numeric_limits<uint64_t>::max());
}

TEST(TimerData, AddTimers) {
  TimerData timer_data;
  TimerInfo timer_info;
  timer_info.set_start(2);
  timer_info.set_end(5);

  timer_data.AddTimer(timer_info, 0);

  EXPECT_FALSE(timer_data.IsEmpty());
  EXPECT_EQ(timer_data.GetNumberOfTimers(), 1);
  ASSERT_NE(timer_data.GetChain(0), nullptr);
  EXPECT_EQ(timer_data.GetChain(1), nullptr);
  EXPECT_EQ(timer_data.GetChain(0)->size(), 1);

  EXPECT_EQ(timer_data.GetMaxTime(), 5);
  EXPECT_EQ(timer_data.GetMinTime(), 2);

  timer_info.set_start(8);
  timer_info.set_end(11);

  timer_data.AddTimer(timer_info, 0);

  EXPECT_FALSE(timer_data.IsEmpty());
  EXPECT_EQ(timer_data.GetNumberOfTimers(), 2);
  ASSERT_NE(timer_data.GetChain(0), nullptr);
  EXPECT_EQ(timer_data.GetChain(1), nullptr);
  EXPECT_EQ(timer_data.GetChain(0)->size(), 2);

  EXPECT_EQ(timer_data.GetMaxTime(), 11);
  EXPECT_EQ(timer_data.GetMinTime(), 2);

  timer_info.set_start(10);
  timer_info.set_end(11);

  timer_data.AddTimer(timer_info, 1);

  EXPECT_EQ(timer_data.GetNumberOfTimers(), 3);
  EXPECT_FALSE(timer_data.IsEmpty());
  ASSERT_NE(timer_data.GetChain(0), nullptr);
  ASSERT_NE(timer_data.GetChain(1), nullptr);
  EXPECT_EQ(timer_data.GetChain(0)->size(), 2);
  EXPECT_EQ(timer_data.GetChain(1)->size(), 1);

  EXPECT_EQ(timer_data.GetMaxTime(), 11);
  EXPECT_EQ(timer_data.GetMinTime(), 2);
}

// TODO(b/204173036): Make GetFirstAfterStartTime private and test GetLeft/Right/Top/Down instead.
TEST(TimerData, FindTimers) {
  TimerData timer_data;

  {
    TimerInfo timer_info;
    timer_info.set_start(2);
    timer_info.set_end(5);
    timer_data.AddTimer(timer_info, 0);

    timer_info.set_start(8);
    timer_info.set_end(11);
    timer_data.AddTimer(timer_info, 0);

    timer_info.set_start(10);
    timer_info.set_end(12);
    timer_data.AddTimer(timer_info, 0);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstAfterStartTime(4, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 8);
    EXPECT_EQ(timer_info->end(), 11);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstAfterStartTime(2, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 8);
    EXPECT_EQ(timer_info->end(), 11);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstAfterStartTime(1, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 2);
    EXPECT_EQ(timer_info->end(), 5);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstAfterStartTime(9, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 10);
    EXPECT_EQ(timer_info->end(), 12);
  }

  {
    const TimerInfo* timer_info =
        timer_data.GetFirstAfterStartTime(std::numeric_limits<uint64_t>::max(), 0);
    EXPECT_EQ(timer_info, nullptr);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstAfterStartTime(0, 1);
    EXPECT_EQ(timer_info, nullptr);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstBeforeStartTime(6, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 2);
    EXPECT_EQ(timer_info->end(), 5);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstBeforeStartTime(4, 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 2);
    EXPECT_EQ(timer_info->end(), 5);
  }

  {
    const TimerInfo* timer_info =
        timer_data.GetFirstBeforeStartTime(std::numeric_limits<uint64_t>::max(), 0);
    ASSERT_NE(timer_info, nullptr);
    EXPECT_EQ(timer_info->start(), 10);
    EXPECT_EQ(timer_info->end(), 12);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstBeforeStartTime(2, 0);
    EXPECT_EQ(timer_info, nullptr);
  }

  {
    const TimerInfo* timer_info = timer_data.GetFirstBeforeStartTime(0, 0);
    EXPECT_EQ(timer_info, nullptr);
  }
  {
    const TimerInfo* timer_info = timer_data.GetFirstBeforeStartTime(1000, 1);
    EXPECT_EQ(timer_info, nullptr);
  }
}

}  // namespace orbit_client_data