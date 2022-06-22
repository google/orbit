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
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"

using orbit_client_data::CallstackEvent;
using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;

namespace orbit_client_data {

MATCHER(CallstackEventEq, "") {
  const CallstackEvent& a = std::get<0>(arg);
  const CallstackEvent& b = std::get<1>(arg);
  return a.timestamp_ns() == b.timestamp_ns() && a.callstack_id() == b.callstack_id() &&
         a.thread_id() == b.thread_id();
}

TEST(CallstackData, FilterCallstackEventsBasedOnMajorityStart) {
  CallstackData callstack_data;

  const uint32_t tid = 42;
  const uint32_t tid_with_no_complete = 43;
  const uint32_t tid_without_supermajority = 44;

  const uint64_t cs1_id = 12;
  const uint64_t cs1_outer = 0x10;
  const uint64_t cs1_inner = 0x11;
  CallstackInfo cs1{{cs1_inner, cs1_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(cs1_id, std::move(cs1));

  const uint64_t cs2_id = 13;
  const uint64_t cs2_outer = 0x10;
  const uint64_t cs2_inner = 0x21;
  CallstackInfo cs2{{cs2_inner, cs2_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(cs2_id, std::move(cs2));

  const uint64_t broken_cs_id = 81;
  const uint64_t broken_cs_outer = 0x30;
  const uint64_t broken_cs_inner = 0x31;
  CallstackInfo broken_cs{{broken_cs_inner, broken_cs_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(broken_cs_id, std::move(broken_cs));

  const uint64_t non_complete_cs_id = 91;
  const uint64_t non_complete_cs_outer = 0x40;
  const uint64_t non_complete_cs_inner = 0x41;
  CallstackInfo non_complete_cs{{non_complete_cs_inner, non_complete_cs_outer},
                                CallstackType::kDwarfUnwindingError};
  callstack_data.AddUniqueCallstack(non_complete_cs_id, std::move(non_complete_cs));

  const uint64_t time1 = 142;
  CallstackEvent event1{time1, cs1_id, tid};
  callstack_data.AddCallstackEvent(event1);

  const uint64_t time2 = 242;
  CallstackEvent event2{time2, broken_cs_id, tid};
  callstack_data.AddCallstackEvent(event2);

  const uint64_t time3 = 342;
  CallstackEvent event3{time3, cs2_id, tid};
  callstack_data.AddCallstackEvent(event3);

  const uint64_t time4 = 442;
  CallstackEvent event4{time4, cs1_id, tid};
  callstack_data.AddCallstackEvent(event4);

  const uint64_t time5 = 542;
  CallstackEvent event5{time5, non_complete_cs_id, tid};
  callstack_data.AddCallstackEvent(event5);

  const uint64_t time6 = 143;
  CallstackEvent event6{time6, broken_cs_id, tid_with_no_complete};
  callstack_data.AddCallstackEvent(event6);

  const uint64_t time7 = 243;
  CallstackEvent event7{time7, non_complete_cs_id, tid_with_no_complete};
  callstack_data.AddCallstackEvent(event7);

  const uint64_t time8 = 144;
  CallstackEvent event8{time8, cs1_id, tid_without_supermajority};
  callstack_data.AddCallstackEvent(event8);

  const uint64_t time9 = 244;
  CallstackEvent event9{time9, broken_cs_id, tid_without_supermajority};
  callstack_data.AddCallstackEvent(event9);

  const uint64_t time10 = 344;
  CallstackEvent event10{time10, non_complete_cs_id, tid_without_supermajority};
  callstack_data.AddCallstackEvent(event10);

  callstack_data.UpdateCallstackTypeBasedOnMajorityStart({});

  EXPECT_EQ(callstack_data.GetCallstack(cs1_id)->type(), CallstackType::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(cs2_id)->type(), CallstackType::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(broken_cs_id)->type(),
            CallstackType::kFilteredByMajorityOutermostFrame);
  EXPECT_EQ(callstack_data.GetCallstack(non_complete_cs_id)->type(),
            CallstackType::kDwarfUnwindingError);

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

TEST(CallstackData, FilterCallstackEventsBasedOnMajorityStartExcludesFunctionToStopUnwindingAt) {
  CallstackData callstack_data;

  constexpr uint32_t kTid = 42;

  constexpr uint64_t kCloneAddress = 0x10;
  constexpr uint64_t kBrokenAddress = 0x30;
  constexpr uint64_t kFunctionToStopUnwindingAtAddress = 0x40;

  const uint64_t cs1_id = 12;
  const uint64_t cs1_outer = kCloneAddress;
  const uint64_t cs1_inner = 0x11;
  CallstackInfo cs1{{cs1_inner, cs1_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(cs1_id, std::move(cs1));

  const uint64_t cs2_id = 13;
  const uint64_t cs2_outer = kCloneAddress;
  const uint64_t cs2_inner = 0x21;
  CallstackInfo cs2{{cs2_inner, cs2_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(cs2_id, std::move(cs2));

  const uint64_t broken_cs_id = 81;
  const uint64_t broken_cs_outer = kBrokenAddress;
  const uint64_t broken_cs_inner = 0x31;
  CallstackInfo broken_cs{{broken_cs_inner, broken_cs_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(broken_cs_id, std::move(broken_cs));

  const uint64_t function_to_stop_unwinding_at_cs_id = 91;
  const uint64_t function_to_stop_unwinding_at_cs_outer = kFunctionToStopUnwindingAtAddress;
  const uint64_t function_to_stop_unwinding_at_cs_inner = 0x41;
  CallstackInfo function_to_stop_unwinding_at_cs{
      {function_to_stop_unwinding_at_cs_inner, function_to_stop_unwinding_at_cs_outer},
      CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(function_to_stop_unwinding_at_cs_id,
                                    std::move(function_to_stop_unwinding_at_cs));

  const uint64_t time1 = 142;
  CallstackEvent event1{time1, cs1_id, kTid};
  callstack_data.AddCallstackEvent(event1);

  const uint64_t time2 = 242;
  CallstackEvent event2{time2, broken_cs_id, kTid};
  callstack_data.AddCallstackEvent(event2);

  const uint64_t time3 = 342;
  CallstackEvent event3{time3, cs2_id, kTid};
  callstack_data.AddCallstackEvent(event3);

  const uint64_t time4 = 442;
  CallstackEvent event4{time4, cs1_id, kTid};
  callstack_data.AddCallstackEvent(event4);

  const uint64_t time5 = 542;
  CallstackEvent event5{time5, function_to_stop_unwinding_at_cs_id, kTid};
  callstack_data.AddCallstackEvent(event5);

  const uint64_t time6 = 642;
  CallstackEvent event6{time6, cs2_id, kTid};
  callstack_data.AddCallstackEvent(event6);

  const uint64_t time7 = 742;
  CallstackEvent event7{time7, cs1_id, kTid};
  callstack_data.AddCallstackEvent(event7);

  const uint64_t time8 = 842;
  CallstackEvent event8{time8, cs1_id, kTid};
  callstack_data.AddCallstackEvent(event8);

  callstack_data.UpdateCallstackTypeBasedOnMajorityStart({{kFunctionToStopUnwindingAtAddress, 10}});

  EXPECT_EQ(callstack_data.GetCallstack(cs1_id)->type(), CallstackType::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(cs2_id)->type(), CallstackType::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(broken_cs_id)->type(),
            CallstackType::kFilteredByMajorityOutermostFrame);
  EXPECT_EQ(callstack_data.GetCallstack(function_to_stop_unwinding_at_cs_id)->type(),
            CallstackType::kComplete);

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  kTid, 0, std::numeric_limits<uint64_t>::max()),
              testing::Pointwise(CallstackEventEq(),
                                 std::vector<CallstackEvent>{event1, event2, event3, event4, event5,
                                                             event6, event7, event8}));
}

}  // namespace orbit_client_data
