// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "ClientData/DataManager.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_client_protos {
static bool operator==(const CallstackEvent& lhs, const CallstackEvent& rhs) {
  return lhs.thread_id() == rhs.thread_id() && lhs.callstack_id() == rhs.callstack_id();
}
}  // namespace orbit_client_protos

namespace orbit_client_data {

using orbit_client_protos::CallstackEvent;
using std::vector;

TEST(DataManager, CallstackEventSelection) {
  DataManager manager;
  EXPECT_EQ(0, manager.GetSelectedCallstackEvents(orbit_base::kAllProcessThreadsTid).size());

  const uint32_t t0_id = 0, t1_id = 1, e0_id = 0, e1_id = 1;

  CallstackEvent e0, e1;
  e0.set_callstack_id(e0_id);
  e0.set_thread_id(t0_id);

  e1.set_callstack_id(e1_id);
  e1.set_thread_id(t1_id);

  const vector<CallstackEvent> t0_events{e0};
  const vector<CallstackEvent> t1_events{e1};

  manager.SelectCallstackEvents(t0_events);
  EXPECT_EQ(t0_events, manager.GetSelectedCallstackEvents(t0_id));
  EXPECT_EQ(t0_events, manager.GetSelectedCallstackEvents(orbit_base::kAllProcessThreadsTid));

  manager.SelectCallstackEvents(t1_events);
  EXPECT_EQ(t1_events, manager.GetSelectedCallstackEvents(t1_id));
  EXPECT_EQ(0, manager.GetSelectedCallstackEvents(t0_id).size());
  EXPECT_EQ(t1_events, manager.GetSelectedCallstackEvents(orbit_base::kAllProcessThreadsTid));
}

TEST(DataManager, CallstackEventSelectionCanOnlyBeUsedFromMainThread) {
  DataManager manager;
  EXPECT_DEATH(
      {
        std::thread will_die([&]() { manager.SelectCallstackEvents({CallstackEvent()}); });
        will_die.join();
      },
      "Check failed");
}
}  // namespace orbit_client_data