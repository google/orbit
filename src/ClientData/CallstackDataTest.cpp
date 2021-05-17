// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

#include "ClientData/CallstackData.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;

namespace orbit_client_data {

MATCHER(CallstackEventEq, "") {
  const CallstackEvent& a = std::get<0>(arg);
  const CallstackEvent& b = std::get<1>(arg);
  return a.time() == b.time() && a.callstack_id() == b.callstack_id() &&
         a.thread_id() == b.thread_id();
}

TEST(CallstackData, FilterCallstackEventsBasedOnMajorityStart) {
  CallstackData callstack_data;

  const int32_t tid = 42;
  const int32_t tid_with_no_complete = 43;
  const int32_t tid_without_supermajority = 44;

  const uint64_t cs1_id = 12;
  const uint64_t cs1_outer = 0x10;
  const uint64_t cs1_inner = 0x11;
  CallstackInfo cs1;
  cs1.add_frames(cs1_inner);
  cs1.add_frames(cs1_outer);
  cs1.set_type(CallstackInfo::kComplete);
  callstack_data.AddUniqueCallstack(cs1_id, cs1);

  const uint64_t cs2_id = 13;
  const uint64_t cs2_outer = 0x10;
  const uint64_t cs2_inner = 0x21;
  CallstackInfo cs2;
  cs2.add_frames(cs2_inner);
  cs2.add_frames(cs2_outer);
  cs2.set_type(CallstackInfo::kComplete);
  callstack_data.AddUniqueCallstack(cs2_id, cs2);

  const uint64_t broken_cs_id = 81;
  const uint64_t broken_cs_outer = 0x30;
  const uint64_t broken_cs_inner = 0x31;
  CallstackInfo broken_cs;
  broken_cs.add_frames(broken_cs_inner);
  broken_cs.add_frames(broken_cs_outer);
  broken_cs.set_type(CallstackInfo::kComplete);
  callstack_data.AddUniqueCallstack(broken_cs_id, broken_cs);

  const uint64_t non_complete_cs_id = 91;
  const uint64_t non_complete_cs_outer = 0x40;
  const uint64_t non_complete_cs_inner = 0x41;
  CallstackInfo non_complete_cs;
  non_complete_cs.add_frames(non_complete_cs_inner);
  non_complete_cs.add_frames(non_complete_cs_outer);
  non_complete_cs.set_type(CallstackInfo::kDwarfUnwindingError);
  callstack_data.AddUniqueCallstack(non_complete_cs_id, non_complete_cs);

  const uint64_t time1 = 142;
  CallstackEvent event1;
  event1.set_time(time1);
  event1.set_thread_id(tid);
  event1.set_callstack_id(cs1_id);
  callstack_data.AddCallstackEvent(event1);

  const uint64_t time2 = 242;
  CallstackEvent event2;
  event2.set_time(time2);
  event2.set_thread_id(tid);
  event2.set_callstack_id(broken_cs_id);
  callstack_data.AddCallstackEvent(event2);

  const uint64_t time3 = 342;
  CallstackEvent event3;
  event3.set_time(time3);
  event3.set_thread_id(tid);
  event3.set_callstack_id(cs2_id);
  callstack_data.AddCallstackEvent(event3);

  const uint64_t time4 = 442;
  CallstackEvent event4;
  event4.set_time(time4);
  event4.set_thread_id(tid);
  event4.set_callstack_id(cs1_id);
  callstack_data.AddCallstackEvent(event4);

  const uint64_t time5 = 542;
  CallstackEvent event5;
  event5.set_time(time5);
  event5.set_thread_id(tid);
  event5.set_callstack_id(non_complete_cs_id);
  callstack_data.AddCallstackEvent(event5);

  const uint64_t time6 = 143;
  CallstackEvent event6;
  event6.set_time(time6);
  event6.set_thread_id(tid_with_no_complete);
  event6.set_callstack_id(broken_cs_id);
  callstack_data.AddCallstackEvent(event6);

  const uint64_t time7 = 243;
  CallstackEvent event7;
  event7.set_time(time7);
  event7.set_thread_id(tid_with_no_complete);
  event7.set_callstack_id(non_complete_cs_id);
  callstack_data.AddCallstackEvent(event7);

  const uint64_t time8 = 144;
  CallstackEvent event8;
  event8.set_time(time8);
  event8.set_thread_id(tid_without_supermajority);
  event8.set_callstack_id(cs1_id);
  callstack_data.AddCallstackEvent(event8);

  const uint64_t time9 = 244;
  CallstackEvent event9;
  event9.set_time(time9);
  event9.set_thread_id(tid_without_supermajority);
  event9.set_callstack_id(broken_cs_id);
  callstack_data.AddCallstackEvent(event9);

  const uint64_t time10 = 344;
  CallstackEvent event10;
  event10.set_time(time10);
  event10.set_thread_id(tid_without_supermajority);
  event10.set_callstack_id(non_complete_cs_id);
  callstack_data.AddCallstackEvent(event10);

  callstack_data.UpdateCallstackTypeBasedOnMajorityStart();

  EXPECT_EQ(callstack_data.GetCallstack(cs1_id)->type(), CallstackInfo::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(cs2_id)->type(), CallstackInfo::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(broken_cs_id)->type(),
            CallstackInfo::kFilteredByMajorityOutermostFrame);
  EXPECT_EQ(callstack_data.GetCallstack(non_complete_cs_id)->type(),
            CallstackInfo::kDwarfUnwindingError);

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  tid, 0, std::numeric_limits<uint64_t>::max()),
              testing::Pointwise(CallstackEventEq(), std::vector<CallstackEvent>{
                                                         event1, event2, event3, event4, event5}));

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  tid_with_no_complete, 0, std::numeric_limits<uint64_t>::max()),
              testing::Pointwise(CallstackEventEq(), std::vector<CallstackEvent>{event6, event7}));

  EXPECT_THAT(
      callstack_data.GetCallstackEventsOfTidInTimeRange(tid_without_supermajority, 0,
                                                        std::numeric_limits<uint64_t>::max()),
      testing::Pointwise(CallstackEventEq(), std::vector<CallstackEvent>{event8, event9, event10}));
}

}  // namespace orbit_client_data
