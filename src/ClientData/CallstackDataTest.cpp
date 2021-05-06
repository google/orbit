// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

#include "ClientData/Callstack.h"
#include "ClientData/CallstackData.h"
#include "capture_data.pb.h"

namespace orbit_client_data {

MATCHER(CallstackEventEq, "") {
  const orbit_client_protos::CallstackEvent& a = std::get<0>(arg);
  const orbit_client_protos::CallstackEvent& b = std::get<1>(arg);
  return a.time() == b.time() && a.callstack_id() == b.callstack_id() &&
         a.thread_id() == b.thread_id();
}

TEST(CallstackData, FilterCallstackEventsBasedOnMajorityStart) {
  CallstackData callstack_data;

  const int32_t tid = 42;
  const int32_t tid_with_broken_only = 43;
  const int32_t tid_without_supermajority = 44;

  const uint64_t cs1_id = 12;
  const uint64_t cs1_outer = 0x10;
  const uint64_t cs1_inner = 0x11;
  const CallStack cs1{cs1_id, {cs1_inner, cs1_outer}};
  callstack_data.AddUniqueCallStack(cs1);

  const uint64_t cs2_id = 13;
  const uint64_t cs2_outer = 0x10;
  const uint64_t cs2_inner = 0x21;
  const CallStack cs2{cs2_id, {cs2_inner, cs2_outer}};
  callstack_data.AddUniqueCallStack(cs2);

  const uint64_t broken_cs_id = 81;
  const uint64_t broken_cs_outer = 0x30;
  const uint64_t broken_cs_inner = 0x31;
  const CallStack broken_cs{broken_cs_id, {broken_cs_inner, broken_cs_outer}};
  callstack_data.AddUniqueCallStack(broken_cs);

  const uint64_t time1 = 100;
  orbit_client_protos::CallstackEvent event1;
  event1.set_time(time1);
  event1.set_thread_id(tid);
  event1.set_callstack_id(cs1_id);
  callstack_data.AddCallstackEvent(event1);

  const uint64_t time2 = 200;
  orbit_client_protos::CallstackEvent event2;
  event2.set_time(time2);
  event2.set_thread_id(tid);
  event2.set_callstack_id(broken_cs_id);
  callstack_data.AddCallstackEvent(event2);

  const uint64_t time3 = 300;
  orbit_client_protos::CallstackEvent event3;
  event3.set_time(time3);
  event3.set_thread_id(tid);
  event3.set_callstack_id(cs2_id);
  callstack_data.AddCallstackEvent(event3);

  const uint64_t time4 = 400;
  orbit_client_protos::CallstackEvent event4;
  event4.set_time(time4);
  event4.set_thread_id(tid);
  event4.set_callstack_id(cs1_id);
  callstack_data.AddCallstackEvent(event4);

  const uint64_t time5 = 500;
  orbit_client_protos::CallstackEvent event5;
  event5.set_time(time5);
  event5.set_thread_id(tid_with_broken_only);
  event5.set_callstack_id(broken_cs_id);
  callstack_data.AddCallstackEvent(event5);

  const uint64_t time6 = 600;
  orbit_client_protos::CallstackEvent event6;
  event6.set_time(time6);
  event6.set_thread_id(tid_without_supermajority);
  event6.set_callstack_id(cs1_id);
  callstack_data.AddCallstackEvent(event6);

  const uint64_t time7 = 700;
  orbit_client_protos::CallstackEvent event7;
  event7.set_time(time7);
  event7.set_thread_id(tid_without_supermajority);
  event7.set_callstack_id(broken_cs_id);
  callstack_data.AddCallstackEvent(event7);

  callstack_data.FilterCallstackEventsBasedOnMajorityStart();

  EXPECT_THAT(
      callstack_data.GetCallstackEventsOfTidInTimeRange(tid, 0,
                                                        std::numeric_limits<uint64_t>::max()),
      testing::Pointwise(CallstackEventEq(),
                         std::vector<orbit_client_protos::CallstackEvent>{event1, event3, event4}));

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  tid_with_broken_only, 0, std::numeric_limits<uint64_t>::max()),
              testing::Pointwise(CallstackEventEq(),
                                 std::vector<orbit_client_protos::CallstackEvent>{event5}));

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  tid_without_supermajority, 0, std::numeric_limits<uint64_t>::max()),
              testing::Pointwise(CallstackEventEq(),
                                 std::vector<orbit_client_protos::CallstackEvent>{event6, event7}));
}

}  // namespace orbit_client_data
