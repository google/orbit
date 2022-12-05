// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ApiUtils/EncodedString.h"
#include "CaptureClient/ApiEventProcessor.h"
#include "ClientData/ApiStringEvent.h"
#include "ClientData/ApiTrackValue.h"
#include "ClientData/CallstackInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "MockCaptureListener.h"

namespace orbit_capture_client {

using orbit_client_data::ApiStringEvent;
using orbit_client_data::ApiTrackValue;
using orbit_client_data::CallstackInfo;

using orbit_client_protos::TimerInfo;

using google::protobuf::util::MessageDifferencer;
using ::testing::AllOf;
using ::testing::DoubleEq;
using ::testing::Invoke;
using ::testing::Property;
using ::testing::SaveArg;

using orbit_grpc_protos::ClientCaptureEvent;

namespace {

class ApiEventProcessorTest : public ::testing::Test {
 public:
  ApiEventProcessorTest() : api_event_processor_{&capture_listener_} {}

 protected:
  void SetUp() override {}

  void TearDown() override {}

  static orbit_grpc_protos::ApiScopeStart CreateStartScope(
      const char* name, uint64_t timestamp_ns, int32_t process_id, int32_t thread_id,
      uint64_t group_id, uint64_t address_in_function, orbit_api_color color = kOrbitColorAuto) {
    orbit_grpc_protos::ApiScopeStart result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_pid(process_id);
    result.set_tid(thread_id);
    result.set_color_rgba(color);
    result.set_group_id(group_id);
    result.set_address_in_function(address_in_function);

    orbit_api::EncodeString(name, &result);

    return result;
  }

  static orbit_grpc_protos::ApiScopeStop CreateStopScope(uint64_t timestamp_ns, int32_t process_id,
                                                         int32_t thread_id) {
    orbit_grpc_protos::ApiScopeStop result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_pid(process_id);
    result.set_tid(thread_id);
    return result;
  }

  static orbit_grpc_protos::ApiScopeStartAsync CreateStartScopeAsync(
      const char* name, uint64_t timestamp_ns, int32_t process_id, int32_t thread_id, uint64_t id,
      uint64_t address_in_function, orbit_api_color color = kOrbitColorAuto) {
    orbit_grpc_protos::ApiScopeStartAsync result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_pid(process_id);
    result.set_tid(thread_id);
    result.set_color_rgba(color);
    result.set_id(id);
    result.set_address_in_function(address_in_function);

    orbit_api::EncodeString(name, &result);

    return result;
  }

  static orbit_grpc_protos::ApiScopeStopAsync CreateStopScopeAsync(uint64_t timestamp_ns,
                                                                   int32_t process_id,
                                                                   int32_t thread_id, uint64_t id) {
    orbit_grpc_protos::ApiScopeStopAsync result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_pid(process_id);
    result.set_tid(thread_id);
    result.set_id(id);

    return result;
  }

  static orbit_grpc_protos::ApiStringEvent CreateStringEvent(uint64_t timestamp_ns,
                                                             int32_t process_id, int32_t thread_id,
                                                             uint64_t id, const char* name) {
    orbit_grpc_protos::ApiStringEvent result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_pid(process_id);
    result.set_tid(thread_id);
    result.set_id(id);

    orbit_api::EncodeString(name, &result);

    return result;
  }

  template <typename DataType, typename ProtoType>
  static ProtoType CreateTrackValue(uint64_t timestamp_ns, int32_t process_id, int32_t thread_id,
                                    const char* name, DataType data) {
    ProtoType result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_pid(process_id);
    result.set_tid(thread_id);
    result.set_data(data);

    orbit_api::EncodeString(name, &result);

    return result;
  }

  static orbit_client_protos::TimerInfo CreateTimerInfo(uint64_t start, uint64_t end,
                                                        int32_t process_id, int32_t thread_id,
                                                        const char* name, uint32_t depth,
                                                        uint64_t group_id, uint64_t async_scope_id,
                                                        uint64_t address_in_function,
                                                        TimerInfo::Type type) {
    orbit_client_protos::TimerInfo timer;
    timer.set_start(start);
    timer.set_end(end);
    timer.set_process_id(process_id);
    timer.set_thread_id(thread_id);
    timer.set_api_scope_name(name);
    timer.set_type(type);
    timer.set_group_id(group_id);
    timer.set_api_async_scope_id(async_scope_id);
    timer.set_address_in_function(address_in_function);
    timer.set_depth(depth);

    return timer;
  }

  MockCaptureListener capture_listener_;
  ApiEventProcessor api_event_processor_;

  static constexpr int32_t kProcessId = 42;
  static constexpr int32_t kThreadId1 = 12;
  static constexpr int32_t kThreadId2 = 13;
  static constexpr uint64_t kGroupId = 77;
  static constexpr uint64_t kId1 = 89;
  static constexpr uint64_t kId2 = 99;
  static constexpr uint64_t kId3 = 109;
  static constexpr uint64_t kAddressInFunction = 111;
};

auto ApiStringEventEq(const ApiStringEvent& expected) {
  return AllOf(Property(&ApiStringEvent::async_scope_id, expected.async_scope_id()),
               Property(&ApiStringEvent::name, expected.name()),
               Property(&ApiStringEvent::should_concatenate, expected.should_concatenate()));
}

auto ApiTrackValueEq(const ApiTrackValue& expected) {
  return AllOf(Property("process_id()", &ApiTrackValue::process_id, expected.process_id()),
               Property("thread_id()", &ApiTrackValue::thread_id, expected.thread_id()),
               Property("timestamp_ns()", &ApiTrackValue::timestamp_ns, expected.timestamp_ns()),
               Property("track_name()", &ApiTrackValue::track_name, expected.track_name()),
               Property("value()", &ApiTrackValue::value, DoubleEq(expected.value())));
}

}  // namespace

TEST_F(ApiEventProcessorTest, ScopesFromSameThread) {
  auto start_0 =
      CreateStartScope("Scope0", 1, kProcessId, kThreadId1, kGroupId, kAddressInFunction);
  auto start_1 =
      CreateStartScope("Scope1", 2, kProcessId, kThreadId1, kGroupId, kAddressInFunction);
  auto start_2 =
      CreateStartScope("Scope2", 3, kProcessId, kThreadId1, kGroupId, kAddressInFunction);
  auto stop_2 = CreateStopScope(4, kProcessId, kThreadId1);
  auto stop_1 = CreateStopScope(5, kProcessId, kThreadId1);
  auto stop_0 = CreateStopScope(6, kProcessId, kThreadId1);

  EXPECT_CALL(capture_listener_, OnTimer).Times(0);
  api_event_processor_.ProcessApiScopeStart(start_0);
  api_event_processor_.ProcessApiScopeStart(start_1);
  api_event_processor_.ProcessApiScopeStart(start_2);

  ::testing::Mock::VerifyAndClearExpectations(&capture_listener_);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;

  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(3)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiScopeStop(stop_2);
  api_event_processor_.ProcessApiScopeStop(stop_1);
  api_event_processor_.ProcessApiScopeStop(stop_0);

  auto expected_timer_2 = CreateTimerInfo(3, 4, kProcessId, kThreadId1, "Scope2", 2, kGroupId, 0,
                                          kAddressInFunction, TimerInfo::kApiScope);
  auto expected_timer_1 = CreateTimerInfo(2, 5, kProcessId, kThreadId1, "Scope1", 1, kGroupId, 0,
                                          kAddressInFunction, TimerInfo::kApiScope);
  auto expected_timer_0 = CreateTimerInfo(1, 6, kProcessId, kThreadId1, "Scope0", 0, kGroupId, 0,
                                          kAddressInFunction, TimerInfo::kApiScope);

  ASSERT_THAT(actual_timers.size(), 3);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_2, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_1, actual_timers[1]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_0, actual_timers[2]));
}

TEST_F(ApiEventProcessorTest, ScopesFromDifferentThreads) {
  auto start_0 =
      CreateStartScope("Scope0", 1, kProcessId, kThreadId1, kGroupId, kAddressInFunction);
  auto start_1 =
      CreateStartScope("Scope1", 2, kProcessId, kThreadId2, kGroupId, kAddressInFunction);
  auto stop_2 = CreateStopScope(4, kProcessId, kThreadId1);
  auto stop_1 = CreateStopScope(5, kProcessId, kThreadId2);

  EXPECT_CALL(capture_listener_, OnTimer).Times(0);
  api_event_processor_.ProcessApiScopeStart(start_0);
  api_event_processor_.ProcessApiScopeStart(start_1);

  ::testing::Mock::VerifyAndClearExpectations(&capture_listener_);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;

  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(2)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiScopeStop(stop_2);
  api_event_processor_.ProcessApiScopeStop(stop_1);

  auto expected_timer_0 = CreateTimerInfo(1, 4, kProcessId, kThreadId1, "Scope0", 0, kGroupId, 0,
                                          kAddressInFunction, TimerInfo::kApiScope);
  auto expected_timer_1 = CreateTimerInfo(2, 5, kProcessId, kThreadId2, "Scope1", 0, kGroupId, 0,
                                          kAddressInFunction, TimerInfo::kApiScope);

  ASSERT_THAT(actual_timers.size(), 2);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_0, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_1, actual_timers[1]));
}

TEST_F(ApiEventProcessorTest, AsyncScopes) {
  auto start_0 =
      CreateStartScopeAsync("AsyncScope0", 1, kProcessId, kThreadId1, kId1, kAddressInFunction);
  auto start_1 =
      CreateStartScopeAsync("AsyncScope1", 2, kProcessId, kThreadId1, kId2, kAddressInFunction);
  auto start_2 =
      CreateStartScopeAsync("AsyncScope2", 3, kProcessId, kThreadId1, kId3, kAddressInFunction);
  auto stop_2 = CreateStopScopeAsync(4, kProcessId, kThreadId1, kId3);
  auto stop_1 = CreateStopScopeAsync(5, kProcessId, kThreadId1, kId2);
  auto stop_0 = CreateStopScopeAsync(6, kProcessId, kThreadId1, kId1);

  EXPECT_CALL(capture_listener_, OnTimer).Times(0);
  api_event_processor_.ProcessApiScopeStartAsync(start_0);
  api_event_processor_.ProcessApiScopeStartAsync(start_1);
  api_event_processor_.ProcessApiScopeStartAsync(start_2);

  ::testing::Mock::VerifyAndClearExpectations(&capture_listener_);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;

  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(3)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiScopeStopAsync(stop_2);
  api_event_processor_.ProcessApiScopeStopAsync(stop_1);
  api_event_processor_.ProcessApiScopeStopAsync(stop_0);

  auto expected_timer_2 = CreateTimerInfo(3, 4, kProcessId, kThreadId1, "AsyncScope2", 0, 0, kId3,
                                          kAddressInFunction, TimerInfo::kApiScopeAsync);
  auto expected_timer_1 = CreateTimerInfo(2, 5, kProcessId, kThreadId1, "AsyncScope1", 0, 0, kId2,
                                          kAddressInFunction, TimerInfo::kApiScopeAsync);
  auto expected_timer_0 = CreateTimerInfo(1, 6, kProcessId, kThreadId1, "AsyncScope0", 0, 0, kId1,
                                          kAddressInFunction, TimerInfo::kApiScopeAsync);

  ASSERT_THAT(actual_timers.size(), 3);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_2, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_1, actual_timers[1]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_0, actual_timers[2]));
}

TEST_F(ApiEventProcessorTest, AsyncScopesOverwrittenStartAndRepeatedStop) {
  auto start0 =
      CreateStartScopeAsync("AsyncTrack", 1, kProcessId, kThreadId1, kId1, kAddressInFunction);
  auto start1 =
      CreateStartScopeAsync("AsyncTrack", 2, kProcessId, kThreadId1, kId1, kAddressInFunction);
  auto stop0 = CreateStopScopeAsync(3, kProcessId, kThreadId1, kId1);
  auto stop1 = CreateStopScopeAsync(4, kProcessId, kThreadId1, kId1);

  orbit_client_protos::TimerInfo actual_timer;
  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(1)
      .WillRepeatedly(Invoke([&actual_timer](const TimerInfo& timer) { actual_timer = timer; }));

  api_event_processor_.ProcessApiScopeStartAsync(start0);
  api_event_processor_.ProcessApiScopeStartAsync(start1);
  api_event_processor_.ProcessApiScopeStopAsync(stop0);
  api_event_processor_.ProcessApiScopeStopAsync(stop1);

  EXPECT_TRUE(MessageDifferencer::Equivalent(
      actual_timer, CreateTimerInfo(2, 3, kProcessId, kThreadId1, "AsyncTrack", 0, 0, kId1,
                                    kAddressInFunction, TimerInfo::kApiScopeAsync)));
}

TEST_F(ApiEventProcessorTest, AsyncScopesWithIdsDifferingOnlyInUpperHalf) {
  static constexpr uint64_t kShortId = 0x1D;
  static constexpr uint64_t kLongId = 0xFF0000001D;
  auto start0 =
      CreateStartScopeAsync("AsyncTrack", 1, kProcessId, kThreadId1, kShortId, kAddressInFunction);
  auto start1 =
      CreateStartScopeAsync("AsyncTrack", 2, kProcessId, kThreadId1, kLongId, kAddressInFunction);
  auto stop1 = CreateStopScopeAsync(3, kProcessId, kThreadId1, kLongId);
  auto stop0 = CreateStopScopeAsync(4, kProcessId, kThreadId1, kShortId);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;
  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(2)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiScopeStartAsync(start0);
  api_event_processor_.ProcessApiScopeStartAsync(start1);

  api_event_processor_.ProcessApiScopeStopAsync(stop1);
  api_event_processor_.ProcessApiScopeStopAsync(stop0);

  ASSERT_THAT(actual_timers.size(), 2);
  EXPECT_TRUE(MessageDifferencer::Equivalent(
      actual_timers[0], CreateTimerInfo(2, 3, kProcessId, kThreadId1, "AsyncTrack", 0, 0, kLongId,
                                        kAddressInFunction, TimerInfo::kApiScopeAsync)));
  EXPECT_TRUE(MessageDifferencer::Equivalent(
      actual_timers[1], CreateTimerInfo(1, 4, kProcessId, kThreadId1, "AsyncTrack", 0, 0, kShortId,
                                        kAddressInFunction, TimerInfo::kApiScopeAsync)));
}

TEST_F(ApiEventProcessorTest, StringEvent) {
  auto string_event = CreateStringEvent(1, kProcessId, kThreadId1, kId1, "Some string for this id");

  ApiStringEvent expected_string_event{kId1, "Some string for this id",
                                       /*should_concatenate=*/false};

  std::optional<ApiStringEvent> actual_string_event;
  EXPECT_CALL(capture_listener_, OnApiStringEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_string_event));

  api_event_processor_.ProcessApiStringEvent(string_event);

  ASSERT_TRUE(actual_string_event.has_value());
  EXPECT_THAT(actual_string_event.value(), ApiStringEventEq(expected_string_event));
}

TEST_F(ApiEventProcessorTest, TrackDouble) {
  auto track_double = CreateTrackValue<double, orbit_grpc_protos::ApiTrackDouble>(
      1, kProcessId, kThreadId1, "Some name", 3.14);

  ApiTrackValue expected_track_value{kProcessId, kThreadId1, 1, "Some name", 3.14};

  std::optional<ApiTrackValue> actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackDouble(track_double);

  ASSERT_TRUE(actual_track_value.has_value());
  EXPECT_THAT(actual_track_value.value(), ApiTrackValueEq(expected_track_value));
}

TEST_F(ApiEventProcessorTest, TrackFloat) {
  constexpr float kValue = 3.14f;
  auto track_float = CreateTrackValue<float, orbit_grpc_protos::ApiTrackFloat>(
      1, kProcessId, kThreadId1, "Some name", kValue);

  ApiTrackValue expected_track_value{kProcessId, kThreadId1, 1, "Some name",
                                     static_cast<double>(kValue)};

  std::optional<ApiTrackValue> actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackFloat(track_float);

  ASSERT_TRUE(actual_track_value.has_value());
  EXPECT_THAT(actual_track_value.value(), ApiTrackValueEq(expected_track_value));
}

TEST_F(ApiEventProcessorTest, TrackInt) {
  constexpr int32_t kValue = 3;
  auto track_int = CreateTrackValue<int32_t, orbit_grpc_protos::ApiTrackInt>(
      1, kProcessId, kThreadId1, "Some name", kValue);

  ApiTrackValue expected_track_value{kProcessId, kThreadId1, 1, "Some name",
                                     static_cast<double>(kValue)};

  std::optional<ApiTrackValue> actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackInt(track_int);

  ASSERT_TRUE(actual_track_value.has_value());
  EXPECT_THAT(actual_track_value.value(), ApiTrackValueEq(expected_track_value));
}

TEST_F(ApiEventProcessorTest, TrackInt64) {
  constexpr int64_t kValue = std::numeric_limits<int64_t>::max();
  auto track_int64 = CreateTrackValue<int64_t, orbit_grpc_protos::ApiTrackInt64>(
      1, kProcessId, kThreadId1, "Some name", kValue);

  ApiTrackValue expected_track_value{kProcessId, kThreadId1, 1, "Some name",
                                     static_cast<double>(kValue)};

  std::optional<ApiTrackValue> actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackInt64(track_int64);

  ASSERT_TRUE(actual_track_value.has_value());
  EXPECT_THAT(actual_track_value.value(), ApiTrackValueEq(expected_track_value));
}

TEST_F(ApiEventProcessorTest, TrackUint) {
  constexpr uint32_t kValue = std::numeric_limits<uint32_t>::max();
  auto track_uint = CreateTrackValue<uint32_t, orbit_grpc_protos::ApiTrackUint>(
      1, kProcessId, kThreadId1, "Some name", kValue);

  ApiTrackValue expected_track_value{kProcessId, kThreadId1, 1, "Some name",
                                     static_cast<double>(kValue)};

  std::optional<ApiTrackValue> actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackUint(track_uint);

  ASSERT_TRUE(actual_track_value.has_value());
  EXPECT_THAT(actual_track_value.value(), ApiTrackValueEq(expected_track_value));
}

TEST_F(ApiEventProcessorTest, TrackUint64) {
  constexpr uint64_t kValue = std::numeric_limits<uint64_t>::max();
  auto track_uint64 = CreateTrackValue<uint64_t, orbit_grpc_protos::ApiTrackUint64>(
      1, kProcessId, kThreadId1, "Some name", kValue);

  ApiTrackValue expected_track_value{kProcessId, kThreadId1, 1, "Some name",
                                     static_cast<double>(kValue)};

  std::optional<ApiTrackValue> actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackUint64(track_uint64);

  ASSERT_TRUE(actual_track_value.has_value());
  EXPECT_THAT(actual_track_value.value(), ApiTrackValueEq(expected_track_value));
}

}  // namespace orbit_capture_client
