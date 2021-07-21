// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/TrackData.h"

namespace orbit_client_data {

TEST(TrackData, IsEmpty) {
  TrackData track_data;
  EXPECT_TRUE(track_data.GetChains().empty());
  EXPECT_EQ(track_data.GetChain(0), nullptr);
  EXPECT_EQ(track_data.GetChain(7), nullptr);
  EXPECT_TRUE(track_data.IsEmpty());
  EXPECT_EQ(track_data.GetNumberOfTimers(), 0);
  EXPECT_EQ(track_data.GetMaxTime(), std::numeric_limits<uint64_t>::min());
  EXPECT_EQ(track_data.GetMinTime(), std::numeric_limits<uint64_t>::max());
}

TEST(TrackData, AddTimers) {
  TrackData track_data;
  orbit_client_protos::TimerInfo timer_info;
  timer_info.set_start(2);
  timer_info.set_end(5);

  track_data.AddTimer(0, timer_info);

  EXPECT_FALSE(track_data.IsEmpty());
  EXPECT_EQ(track_data.GetNumberOfTimers(), 1);
  ASSERT_NE(track_data.GetChain(0), nullptr);
  EXPECT_EQ(track_data.GetChain(1), nullptr);
  EXPECT_EQ(track_data.GetChain(0)->size(), 1);

  EXPECT_EQ(track_data.GetMaxTime(), 5);
  EXPECT_EQ(track_data.GetMinTime(), 2);

  timer_info.set_start(8);
  timer_info.set_end(11);

  track_data.AddTimer(0, timer_info);

  EXPECT_FALSE(track_data.IsEmpty());
  EXPECT_EQ(track_data.GetNumberOfTimers(), 2);
  ASSERT_NE(track_data.GetChain(0), nullptr);
  EXPECT_EQ(track_data.GetChain(1), nullptr);
  EXPECT_EQ(track_data.GetChain(0)->size(), 2);

  EXPECT_EQ(track_data.GetMaxTime(), 11);
  EXPECT_EQ(track_data.GetMinTime(), 2);

  timer_info.set_start(10);
  timer_info.set_end(11);

  track_data.AddTimer(1, timer_info);

  EXPECT_EQ(track_data.GetNumberOfTimers(), 3);
  EXPECT_FALSE(track_data.IsEmpty());
  ASSERT_NE(track_data.GetChain(0), nullptr);
  ASSERT_NE(track_data.GetChain(1), nullptr);
  EXPECT_EQ(track_data.GetChain(0)->size(), 2);
  EXPECT_EQ(track_data.GetChain(1)->size(), 1);

  EXPECT_EQ(track_data.GetMaxTime(), 11);
  EXPECT_EQ(track_data.GetMinTime(), 2);
}

}  // namespace orbit_client_data