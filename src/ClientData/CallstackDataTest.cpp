// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <absl/container/flat_hash_map.h>
#include <absl/functional/bind_front.h>
#include <absl/functional/function_ref.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"

using ::testing::AnyOfArray;
using ::testing::Pointwise;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

using orbit_client_data::CallstackEvent;
using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;

namespace orbit_client_data {

namespace {

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
              Pointwise(CallstackEventEq(),
                        std::vector<CallstackEvent>{event1, event2, event3, event4, event5}));

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  tid_with_no_complete, 0, std::numeric_limits<uint64_t>::max()),
              Pointwise(CallstackEventEq(), std::vector<CallstackEvent>{event6, event7}));

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  tid_without_supermajority, 0, std::numeric_limits<uint64_t>::max()),
              Pointwise(CallstackEventEq(), std::vector<CallstackEvent>{event8, event9, event10}));
}

constexpr uint32_t kTid = 42;
constexpr uint32_t kAnotherTid = 43;

inline constexpr uint64_t kCallstackId1 = 12;
constexpr uint64_t kCallstackId2 = 13;

constexpr uint64_t kCloneAddress = 0x10;
constexpr uint64_t kBrokenAddress = 0x30;
constexpr uint64_t kFunctionToStopUnwindingAtAddress = 0x40;

TEST(CallstackData, FilterCallstackEventsBasedOnMajorityStartExcludesFunctionToStopUnwindingAt) {
  CallstackData callstack_data;

  const uint64_t cs1_outer = kCloneAddress;
  const uint64_t cs1_inner = 0x11;
  CallstackInfo cs1{{cs1_inner, cs1_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(kCallstackId1, std::move(cs1));

  const uint64_t cs2_outer = kCloneAddress;
  const uint64_t cs2_inner = 0x21;
  CallstackInfo cs2{{cs2_inner, cs2_outer}, CallstackType::kComplete};
  callstack_data.AddUniqueCallstack(kCallstackId2, std::move(cs2));

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
  CallstackEvent event1{time1, kCallstackId1, kTid};
  callstack_data.AddCallstackEvent(event1);

  const uint64_t time2 = 242;
  CallstackEvent event2{time2, broken_cs_id, kTid};
  callstack_data.AddCallstackEvent(event2);

  const uint64_t time3 = 342;
  CallstackEvent event3{time3, kCallstackId2, kTid};
  callstack_data.AddCallstackEvent(event3);

  const uint64_t time4 = 442;
  CallstackEvent event4{time4, kCallstackId1, kTid};
  callstack_data.AddCallstackEvent(event4);

  const uint64_t time5 = 542;
  CallstackEvent event5{time5, function_to_stop_unwinding_at_cs_id, kTid};
  callstack_data.AddCallstackEvent(event5);

  const uint64_t time6 = 642;
  CallstackEvent event6{time6, kCallstackId2, kTid};
  callstack_data.AddCallstackEvent(event6);

  callstack_data.UpdateCallstackTypeBasedOnMajorityStart({{kFunctionToStopUnwindingAtAddress, 10}});

  EXPECT_EQ(callstack_data.GetCallstack(kCallstackId1)->type(), CallstackType::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(kCallstackId2)->type(), CallstackType::kComplete);
  EXPECT_EQ(callstack_data.GetCallstack(broken_cs_id)->type(),
            CallstackType::kFilteredByMajorityOutermostFrame);
  EXPECT_EQ(callstack_data.GetCallstack(function_to_stop_unwinding_at_cs_id)->type(),
            CallstackType::kComplete);

  EXPECT_THAT(callstack_data.GetCallstackEventsOfTidInTimeRange(
                  kTid, 0, std::numeric_limits<uint64_t>::max()),
              Pointwise(CallstackEventEq(), std::vector<CallstackEvent>{event1, event2, event3,
                                                                        event4, event5, event6}));
}

const std::vector<uint32_t> kTids = {kTid, kTid, kAnotherTid, kTid};
const std::vector<uint64_t> kTimestamps = {142, 242, 342, 442};
const std::vector<CallstackEvent> kAllEvents = [] {
  std::vector<CallstackEvent> events;
  absl::c_transform(kTids, kTimestamps, std::back_inserter(events),
                    [](uint32_t tid, uint64_t timestamp) {
                      return CallstackEvent{timestamp, kCallstackId1, tid};
                    });
  return events;
}();

constexpr auto kGetTestName = [](const auto& info) { return info.param.test_name; };

std::unique_ptr<CallstackData> kCallstackDataWithEvents = [] {
  auto result = std::make_unique<CallstackData>();
  CallstackInfo cs{{0x11, 0x10}, CallstackType::kComplete};
  result->AddUniqueCallstack(kCallstackId1, std::move(cs));
  absl::c_for_each(kAllEvents, absl::bind_front(&CallstackData::AddCallstackEvent, result.get()));
  return result;
}();

template <typename T>
[[nodiscard]] static std::vector<T> Slice(const std::vector<T>& v, std::vector<int> indices) {
  std::vector<T> slice;
  absl::c_transform(indices, std::back_inserter(slice), [&](int i) { return v[i]; });
  return slice;
}

struct ForEachCallstackEventOfTidInTimeRangeDiscretizedTestCase {
  std::string test_name;
  uint32_t tid;
  uint64_t start_ns;
  uint64_t end_ns;
  uint32_t resolution;
  std::vector<int> expected_event_ids;
};

using ForEachCallstackEventOfTidInTimeRangeDiscretizedTest =
    TestWithParam<ForEachCallstackEventOfTidInTimeRangeDiscretizedTestCase>;

TEST_P(ForEachCallstackEventOfTidInTimeRangeDiscretizedTest, IterationIsCorrect) {
  const ForEachCallstackEventOfTidInTimeRangeDiscretizedTestCase& test_case = GetParam();
  const CallstackData* callstack_data = kCallstackDataWithEvents.get();

  std::vector<CallstackEvent> visited_callstack_list;
  auto visit_callstack = [&](const CallstackEvent& event) {
    visited_callstack_list.push_back(event);
  };
  callstack_data->ForEachCallstackEventOfTidInTimeRangeDiscretized(
      test_case.tid, test_case.start_ns, test_case.end_ns, test_case.resolution, visit_callstack);
  EXPECT_THAT(visited_callstack_list,
              Pointwise(CallstackEventEq(), Slice(kAllEvents, test_case.expected_event_ids)));
}

constexpr uint64_t kStartNs = 0;
constexpr uint64_t kEndNs = 1000;
constexpr uint64_t kMaxNs = std::numeric_limits<uint64_t>::max();
constexpr uint32_t kResolution = 2000;

INSTANTIATE_TEST_SUITE_P(
    ForEachCallstackEventOfTidInTimeRangeDiscretizedTests,
    ForEachCallstackEventOfTidInTimeRangeDiscretizedTest,
    ValuesIn<ForEachCallstackEventOfTidInTimeRangeDiscretizedTestCase>({
        {"NormalTimeRange", kTid, kStartNs, kEndNs, kResolution, {0, 1, 3}},
        {"DifferentTid", kAnotherTid, kStartNs, kEndNs, kResolution, {2}},
        {"SmallTimeRange", kTid, kStartNs, kTimestamps[2] - 1, kResolution, {0, 1}},
        // When max_timestamp is std::numeric_limits<uint64_t>::max(), each callstack will be drawn
        // in the first pixel, and therefore only one will be visible.
        {"InfiniteTimeRange", kTid, kStartNs, kMaxNs, kResolution, {0}},
        // With one pixel on the screen we should only see one event.
        {"OnePixel", kTid, kStartNs, kEndNs, /*resolution=*/1, {0}},
    }),
    kGetTestName);

constexpr auto kExpectAll = [](const auto& actual, const auto& expected) {
  EXPECT_THAT(actual, Pointwise(CallstackEventEq(), expected));
};

constexpr auto kExpectAny = [](const auto& actual, const auto& expected) {
  EXPECT_EQ(actual.size(), 1);
  EXPECT_THAT(actual[0], AnyOfArray(expected));
};

struct ForEachCallstackEventInTimeRangeDiscretizedTestCase {
  std::string test_name;
  absl::FunctionRef<void(std::vector<CallstackEvent>, std::vector<CallstackEvent>)> expect;
  uint64_t start_ns;
  uint64_t end_ns;
  uint32_t resolution;
  std::vector<int> expected_event_ids;
};

using ForEachCallstackEventInTimeRangeDiscretizedTest =
    TestWithParam<ForEachCallstackEventInTimeRangeDiscretizedTestCase>;
TEST_P(ForEachCallstackEventInTimeRangeDiscretizedTest, IterationIsCorrect) {
  const ForEachCallstackEventInTimeRangeDiscretizedTestCase& test_case = GetParam();
  const CallstackData* callstack_data = kCallstackDataWithEvents.get();

  std::vector<CallstackEvent> visited_callstack_list;
  auto visit_callstack = [&](const CallstackEvent& event) {
    visited_callstack_list.push_back(event);
  };
  callstack_data->ForEachCallstackEventInTimeRangeDiscretized(
      test_case.start_ns, test_case.end_ns, test_case.resolution, visit_callstack);
  test_case.expect(visited_callstack_list, Slice(kAllEvents, test_case.expected_event_ids));
}

INSTANTIATE_TEST_SUITE_P(
    ForEachCallstackEventInTimeRangeDiscretizedTests,
    ForEachCallstackEventInTimeRangeDiscretizedTest,
    ValuesIn<ForEachCallstackEventInTimeRangeDiscretizedTestCase>({
        {"NormalTimeRange", kExpectAll, kStartNs, kEndNs, kResolution, {0, 1, 2, 3}},
        {"SmallTimeRange", kExpectAll, kStartNs, kTimestamps[2] - 1, kResolution, {0, 1}},
        // When max_timestamp is std::numeric_limits<uint64_t>::max(), each callstack should be draw
        // in the first pixel, and therefore only one will be visible. It should be the first of
        // some of the threads.
        {"InfiniteTimeRange", kExpectAny, kStartNs, kMaxNs, kResolution, {0, 2}},
        // With one pixel on the screen we should only see one event. It should be the first of some
        // of the threads.
        {"OnePixel", kExpectAny, kStartNs, kEndNs, /*resolution=*/1, {0, 2}},
    }),
    kGetTestName);

}  // namespace

}  // namespace orbit_client_data
