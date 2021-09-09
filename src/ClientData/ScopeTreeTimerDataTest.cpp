// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ScopeTreeTimerData.h"

namespace orbit_client_data {

TEST(ScopeTreeTimerData, IsEmpty) {
  ScopeTreeTimerData scope_tree_timer_data;
  EXPECT_TRUE(scope_tree_timer_data.IsEmpty());
  EXPECT_TRUE(scope_tree_timer_data.GetTimers().empty());
}

TEST(ScopeTreeTimerData, AddTimers) {
  ScopeTreeTimerData scope_tree_timer_data;
  orbit_client_protos::TimerInfo timer_info;
  timer_info.set_start(2);
  timer_info.set_end(5);

  scope_tree_timer_data.AddTimer(timer_info);

  EXPECT_FALSE(scope_tree_timer_data.IsEmpty());
  EXPECT_EQ(scope_tree_timer_data.GetNumberOfTimers(), 1);
  EXPECT_EQ(scope_tree_timer_data.GetMaxDepth(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetMinTime(), 2);
  EXPECT_EQ(scope_tree_timer_data.GetMaxTime(), 5);

  timer_info.set_start(8);
  timer_info.set_end(11);

  scope_tree_timer_data.AddTimer(timer_info);

  EXPECT_EQ(scope_tree_timer_data.GetNumberOfTimers(), 2);
  EXPECT_EQ(scope_tree_timer_data.GetMaxDepth(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetMinTime(), 2);
  EXPECT_EQ(scope_tree_timer_data.GetMaxTime(), 11);

  timer_info.set_start(10);
  timer_info.set_end(11);

  scope_tree_timer_data.AddTimer(timer_info);

  EXPECT_EQ(scope_tree_timer_data.GetNumberOfTimers(), 3);
  EXPECT_EQ(scope_tree_timer_data.GetMaxDepth(), 1);
  EXPECT_EQ(scope_tree_timer_data.GetMinTime(), 2);
  EXPECT_EQ(scope_tree_timer_data.GetMaxTime(), 11);
}

TEST(ScopeTreeTimerData, GetTimers) {
  ScopeTreeTimerData scope_tree_timer_data;
  orbit_client_protos::TimerInfo timer_info;

  timer_info.set_start(2);
  timer_info.set_end(5);
  scope_tree_timer_data.AddTimer(timer_info);

  timer_info.set_start(8);
  timer_info.set_end(11);
  scope_tree_timer_data.AddTimer(timer_info);

  timer_info.set_start(10);
  timer_info.set_end(11);
  scope_tree_timer_data.AddTimer(timer_info);

  EXPECT_EQ(scope_tree_timer_data.GetTimers(0, 1).size(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetTimers(20, 30).size(), 0);
  EXPECT_EQ(scope_tree_timer_data.GetTimers(1, 3).size(), 1);   // (2,5)
  EXPECT_EQ(scope_tree_timer_data.GetTimers(8, 9).size(), 1);   // (8,11)
  EXPECT_EQ(scope_tree_timer_data.GetTimers(8, 11).size(), 2);  // (8,11) , (10,11)
  EXPECT_EQ(scope_tree_timer_data.GetTimers(4, 11).size(), 3);
  EXPECT_EQ(scope_tree_timer_data.GetTimers().size(), 3);
}

bool AreSameTimer(const orbit_client_protos::TimerInfo& timer_1,
                  const orbit_client_protos::TimerInfo& timer_2) {
  return timer_1.start() == timer_2.start() && timer_1.end() && timer_2.end();
}

TEST(ScopeTreeTimerData, GetLeftRightUpDown) {
  ScopeTreeTimerData scope_tree_timer_data;
  orbit_client_protos::TimerInfo timer_info_left;
  timer_info_left.set_start(2);
  timer_info_left.set_end(5);
  scope_tree_timer_data.AddTimer(timer_info_left);

  orbit_client_protos::TimerInfo timer_info_middle;
  timer_info_middle.set_start(8);
  timer_info_middle.set_end(11);
  scope_tree_timer_data.AddTimer(timer_info_middle);

  orbit_client_protos::TimerInfo timer_info_down;
  timer_info_down.set_start(10);
  timer_info_down.set_end(11);
  scope_tree_timer_data.AddTimer(timer_info_down);

  orbit_client_protos::TimerInfo timer_info_right;
  timer_info_right.set_start(13);
  timer_info_right.set_end(15);
  scope_tree_timer_data.AddTimer(timer_info_right);

  AreSameTimer(scope_tree_timer_data.GetLeft(timer_info_middle), timer_info_left);
  AreSameTimer(scope_tree_timer_data.GetLeft(timer_info_right), timer_info_middle);
  AreSameTimer(scope_tree_timer_data.GetRight(timer_info_left), timer_info_middle);
  AreSameTimer(scope_tree_timer_data.GetRight(timer_info_middle), timer_info_right);
  AreSameTimer(scope_tree_timer_data.GetUp(timer_info_down), timer_info_middle);
  AreSameTimer(scope_tree_timer_data.GetDown(timer_info_middle), timer_info_down);
}

}  // namespace orbit_client_data