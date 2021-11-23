// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "ApiUtils/EncodedString.h"
#include "CaptureClient/ApiEventProcessor.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_capture_client {

using orbit_client_protos::CallstackInfo;
using orbit_client_protos::TimerInfo;

using google::protobuf::util::MessageDifferencer;
using ::testing::Invoke;
using ::testing::SaveArg;

using orbit_grpc_protos::ApiEvent;
using orbit_grpc_protos::ClientCaptureEvent;

namespace {
class MockCaptureListener : public CaptureListener {
 public:
  MOCK_METHOD(void, OnCaptureStarted,
              (const orbit_grpc_protos::CaptureStarted&, std::optional<std::filesystem::path>,
               absl::flat_hash_set<uint64_t>),
              (override));
  MOCK_METHOD(void, OnCaptureFinished, (const orbit_grpc_protos::CaptureFinished&), (override));
  MOCK_METHOD(void, OnTimer, (const TimerInfo&), (override));
  MOCK_METHOD(void, OnKeyAndString, (uint64_t /*key*/, std::string), (override));
  MOCK_METHOD(void, OnUniqueCallstack, (uint64_t /*callstack_id*/, CallstackInfo /*callstack*/),
              (override));
  MOCK_METHOD(void, OnCallstackEvent, (orbit_client_protos::CallstackEvent), (override));
  MOCK_METHOD(void, OnThreadName, (uint32_t /*thread_id*/, std::string /*thread_name*/),
              (override));
  MOCK_METHOD(void, OnThreadStateSlice, (orbit_client_protos::ThreadStateSliceInfo), (override));
  MOCK_METHOD(void, OnAddressInfo, (orbit_client_protos::LinuxAddressInfo), (override));
  MOCK_METHOD(void, OnUniqueTracepointInfo,
              (uint64_t /*key*/, orbit_grpc_protos::TracepointInfo /*tracepoint_info*/),
              (override));
  MOCK_METHOD(void, OnTracepointEvent, (orbit_client_protos::TracepointEventInfo), (override));
  MOCK_METHOD(void, OnModuleUpdate,
              (uint64_t /*timestamp_ns*/, orbit_grpc_protos::ModuleInfo /*module_info*/),
              (override));
  MOCK_METHOD(void, OnModulesSnapshot,
              (uint64_t /*timestamp_ns*/,
               std::vector<orbit_grpc_protos::ModuleInfo> /*module_infos*/),
              (override));
  MOCK_METHOD(void, OnApiStringEvent, (const orbit_client_protos::ApiStringEvent&), (override));
  MOCK_METHOD(void, OnApiTrackValue, (const orbit_client_protos::ApiTrackValue&), (override));
  MOCK_METHOD(void, OnWarningEvent, (orbit_grpc_protos::WarningEvent /*warning_event*/),
              (override));
  MOCK_METHOD(void, OnClockResolutionEvent,
              (orbit_grpc_protos::ClockResolutionEvent /*clock_resolution_event*/), (override));
  MOCK_METHOD(
      void, OnErrorsWithPerfEventOpenEvent,
      (orbit_grpc_protos::ErrorsWithPerfEventOpenEvent /*errors_with_perf_event_open_event*/),
      (override));
  MOCK_METHOD(void, OnErrorEnablingOrbitApiEvent,
              (orbit_grpc_protos::ErrorEnablingOrbitApiEvent /*error_enabling_orbit_api_event*/),
              (override));
  MOCK_METHOD(void, OnErrorEnablingUserSpaceInstrumentationEvent,
              (orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent /*error_event*/),
              (override));
  MOCK_METHOD(
      void, OnWarningInstrumentingWithUserSpaceInstrumentationEvent,
      (orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent /*warning_event*/),
      (override));
  MOCK_METHOD(void, OnLostPerfRecordsEvent,
              (orbit_grpc_protos::LostPerfRecordsEvent /*lost_perf_records_event*/), (override));
  MOCK_METHOD(
      void, OnOutOfOrderEventsDiscardedEvent,
      (orbit_grpc_protos::OutOfOrderEventsDiscardedEvent /*out_of_order_events_discarded_event*/),
      (override));
};

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

  static orbit_client_protos::ApiStringEvent CreateClientStringEvent(uint64_t timestamp_ns,
                                                                     int32_t process_id,
                                                                     int32_t thread_id, uint64_t id,
                                                                     const char* name) {
    orbit_client_protos::ApiStringEvent result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_process_id(process_id);
    result.set_thread_id(thread_id);
    result.set_async_scope_id(id);
    result.set_name(name);

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
  template <typename DataType>
  static orbit_client_protos::ApiTrackValue CreateClientTrackValue(
      uint64_t timestamp_ns, int32_t process_id, int32_t thread_id, const char* name,
      void (orbit_client_protos::ApiTrackValue::*set_data)(DataType), DataType data) {
    orbit_client_protos::ApiTrackValue result;
    result.set_timestamp_ns(timestamp_ns);
    result.set_process_id(process_id);
    result.set_thread_id(thread_id);
    result.set_name(name);

    (result.*set_data)(data);

    return result;
  }

  [[deprecated]] static orbit_grpc_protos::ApiEvent CreateApiEventLegacy(
      int32_t pid, int32_t tid, uint64_t timestamp_ns, orbit_api::EventType type,
      const char* name = nullptr, uint64_t data = 0, orbit_api_color color = kOrbitColorAuto) {
    orbit_api::EncodedEvent encoded_event(type, name, data, color);
    ApiEvent api_event;
    api_event.set_timestamp_ns(timestamp_ns);
    api_event.set_pid(pid);
    api_event.set_tid(tid);
    api_event.set_r0(encoded_event.args[0]);
    api_event.set_r1(encoded_event.args[1]);
    api_event.set_r2(encoded_event.args[2]);
    api_event.set_r3(encoded_event.args[3]);
    api_event.set_r4(encoded_event.args[4]);
    api_event.set_r5(encoded_event.args[5]);

    return api_event;
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

TEST_F(ApiEventProcessorTest, StringEvent) {
  auto string_event = CreateStringEvent(1, kProcessId, kThreadId1, kId1, "Some string for this id");

  auto expected_string_event =
      CreateClientStringEvent(1, kProcessId, kThreadId1, kId1, "Some string for this id");

  orbit_client_protos::ApiStringEvent actual_string_event;
  EXPECT_CALL(capture_listener_, OnApiStringEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_string_event));

  api_event_processor_.ProcessApiStringEvent(string_event);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_string_event, actual_string_event));
}

TEST_F(ApiEventProcessorTest, TrackDouble) {
  auto track_double = CreateTrackValue<double, orbit_grpc_protos::ApiTrackDouble>(
      1, kProcessId, kThreadId1, "Some name", 3.14);

  auto expected_track_value =
      CreateClientTrackValue<double>(1, kProcessId, kThreadId1, "Some name",
                                     &orbit_client_protos::ApiTrackValue::set_data_double, 3.14);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackDouble(track_double);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackFloat) {
  auto track_float = CreateTrackValue<float, orbit_grpc_protos::ApiTrackFloat>(
      1, kProcessId, kThreadId1, "Some name", 3.14f);

  auto expected_track_value =
      CreateClientTrackValue<float>(1, kProcessId, kThreadId1, "Some name",
                                    &orbit_client_protos::ApiTrackValue::set_data_float, 3.14f);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackFloat(track_float);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackInt) {
  auto track_int = CreateTrackValue<int32_t, orbit_grpc_protos::ApiTrackInt>(
      1, kProcessId, kThreadId1, "Some name", 3);

  auto expected_track_value = CreateClientTrackValue<int32_t>(
      1, kProcessId, kThreadId1, "Some name", &orbit_client_protos::ApiTrackValue::set_data_int, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackInt(track_int);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackInt64) {
  auto track_int64 = CreateTrackValue<int64_t, orbit_grpc_protos::ApiTrackInt64>(
      1, kProcessId, kThreadId1, "Some name", 3);

  auto expected_track_value =
      CreateClientTrackValue<int64_t>(1, kProcessId, kThreadId1, "Some name",
                                      &orbit_client_protos::ApiTrackValue::set_data_int64, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackInt64(track_int64);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackUint) {
  auto track_uint = CreateTrackValue<uint32_t, orbit_grpc_protos::ApiTrackUint>(
      1, kProcessId, kThreadId1, "Some name", 3);

  auto expected_track_value =
      CreateClientTrackValue<uint32_t>(1, kProcessId, kThreadId1, "Some name",
                                       &orbit_client_protos::ApiTrackValue::set_data_uint, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackUint(track_uint);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackUint64) {
  auto track_uint64 = CreateTrackValue<uint64_t, orbit_grpc_protos::ApiTrackUint64>(
      1, kProcessId, kThreadId1, "Some name", 3);

  auto expected_track_value =
      CreateClientTrackValue<uint64_t>(1, kProcessId, kThreadId1, "Some name",
                                       &orbit_client_protos::ApiTrackValue::set_data_uint64, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiTrackUint64(track_uint64);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, ScopesFromSameThreadLegacy) {
  ApiEvent start_0 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kScopeStart, "Scope0");
  ApiEvent start_1 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 2, orbit_api::EventType::kScopeStart, "Scope1");
  ApiEvent start_2 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 3, orbit_api::EventType::kScopeStart, "Scope2");

  ApiEvent stop_2 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 4, orbit_api::EventType::kScopeStop);
  ApiEvent stop_1 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 5, orbit_api::EventType::kScopeStop);
  ApiEvent stop_0 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 6, orbit_api::EventType::kScopeStop);

  EXPECT_CALL(capture_listener_, OnTimer).Times(0);
  api_event_processor_.ProcessApiEventLegacy(start_0);
  api_event_processor_.ProcessApiEventLegacy(start_1);
  api_event_processor_.ProcessApiEventLegacy(start_2);

  ::testing::Mock::VerifyAndClearExpectations(&capture_listener_);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;

  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(3)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiEventLegacy(stop_2);
  api_event_processor_.ProcessApiEventLegacy(stop_1);
  api_event_processor_.ProcessApiEventLegacy(stop_0);

  auto expected_timer_2 =
      CreateTimerInfo(3, 4, kProcessId, kThreadId1, "Scope2", 2, 0, 0, 0, TimerInfo::kApiScope);
  auto expected_timer_1 =
      CreateTimerInfo(2, 5, kProcessId, kThreadId1, "Scope1", 1, 0, 0, 0, TimerInfo::kApiScope);
  auto expected_timer_0 =
      CreateTimerInfo(1, 6, kProcessId, kThreadId1, "Scope0", 0, 0, 0, 0, TimerInfo::kApiScope);

  ASSERT_THAT(actual_timers.size(), 3);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_2, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_1, actual_timers[1]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_0, actual_timers[2]));
}

TEST_F(ApiEventProcessorTest, ScopesFromDifferentThreadsLegacy) {
  auto start_0 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kScopeStart, "Scope0");
  auto start_1 =
      CreateApiEventLegacy(kProcessId, kThreadId2, 2, orbit_api::EventType::kScopeStart, "Scope1");
  auto stop_0 = CreateApiEventLegacy(kProcessId, kThreadId1, 4, orbit_api::EventType::kScopeStop);
  auto stop_1 = CreateApiEventLegacy(kProcessId, kThreadId2, 5, orbit_api::EventType::kScopeStop);

  EXPECT_CALL(capture_listener_, OnTimer).Times(0);
  api_event_processor_.ProcessApiEventLegacy(start_0);
  api_event_processor_.ProcessApiEventLegacy(start_1);

  ::testing::Mock::VerifyAndClearExpectations(&capture_listener_);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;

  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(2)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiEventLegacy(stop_0);
  api_event_processor_.ProcessApiEventLegacy(stop_1);

  auto expected_timer_0 =
      CreateTimerInfo(1, 4, kProcessId, kThreadId1, "Scope0", 0, 0, 0, 0, TimerInfo::kApiScope);
  auto expected_timer_1 =
      CreateTimerInfo(2, 5, kProcessId, kThreadId2, "Scope1", 0, 0, 0, 0, TimerInfo::kApiScope);

  ASSERT_THAT(actual_timers.size(), 2);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_0, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_1, actual_timers[1]));
}

TEST_F(ApiEventProcessorTest, AsyncScopesLegacy) {
  auto start_0 = CreateApiEventLegacy(kProcessId, kThreadId1, 1,
                                      orbit_api::EventType::kScopeStartAsync, "AsyncScope0", kId1);
  auto start_1 = CreateApiEventLegacy(kProcessId, kThreadId1, 2,
                                      orbit_api::EventType::kScopeStartAsync, "AsyncScope1", kId2);
  auto start_2 = CreateApiEventLegacy(kProcessId, kThreadId1, 3,
                                      orbit_api::EventType::kScopeStartAsync, "AsyncScope2", kId3);

  auto stop_2 = CreateApiEventLegacy(kProcessId, kThreadId1, 4,
                                     orbit_api::EventType::kScopeStopAsync, nullptr, kId3);
  auto stop_1 = CreateApiEventLegacy(kProcessId, kThreadId1, 5,
                                     orbit_api::EventType::kScopeStopAsync, nullptr, kId2);
  auto stop_0 = CreateApiEventLegacy(kProcessId, kThreadId1, 6,
                                     orbit_api::EventType::kScopeStopAsync, nullptr, kId1);

  EXPECT_CALL(capture_listener_, OnTimer).Times(0);
  api_event_processor_.ProcessApiEventLegacy(start_0);
  api_event_processor_.ProcessApiEventLegacy(start_1);
  api_event_processor_.ProcessApiEventLegacy(start_2);

  ::testing::Mock::VerifyAndClearExpectations(&capture_listener_);

  std::vector<orbit_client_protos::TimerInfo> actual_timers;

  EXPECT_CALL(capture_listener_, OnTimer)
      .Times(3)
      .WillRepeatedly(
          Invoke([&actual_timers](const TimerInfo& timer) { actual_timers.push_back(timer); }));

  api_event_processor_.ProcessApiEventLegacy(stop_2);
  api_event_processor_.ProcessApiEventLegacy(stop_1);
  api_event_processor_.ProcessApiEventLegacy(stop_0);

  auto expected_timer_2 = CreateTimerInfo(3, 4, kProcessId, kThreadId1, "AsyncScope2", 0, 0, kId3,
                                          0, TimerInfo::kApiScopeAsync);
  auto expected_timer_1 = CreateTimerInfo(2, 5, kProcessId, kThreadId1, "AsyncScope1", 0, 0, kId2,
                                          0, TimerInfo::kApiScopeAsync);
  auto expected_timer_0 = CreateTimerInfo(1, 6, kProcessId, kThreadId1, "AsyncScope0", 0, 0, kId1,
                                          0, TimerInfo::kApiScopeAsync);

  ASSERT_THAT(actual_timers.size(), 3);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_2, actual_timers[0]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_1, actual_timers[1]));
  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_timer_0, actual_timers[2]));
}

TEST_F(ApiEventProcessorTest, StringEventLegacy) {
  auto string_event = CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kString,
                                           "Some string for this id", kId1);

  auto expected_string_event =
      CreateClientStringEvent(1, kProcessId, kThreadId1, kId1, "Some string for this id");

  orbit_client_protos::ApiStringEvent actual_string_event;
  EXPECT_CALL(capture_listener_, OnApiStringEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_string_event));

  api_event_processor_.ProcessApiEventLegacy(string_event);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_string_event, actual_string_event));
}

TEST_F(ApiEventProcessorTest, TrackDoubleLegacy) {
  auto track_double =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kTrackDouble,
                           "Some name", orbit_api::Encode<uint64_t>(3.14));

  auto expected_track_value =
      CreateClientTrackValue<double>(1, kProcessId, kThreadId1, "Some name",
                                     &orbit_client_protos::ApiTrackValue::set_data_double, 3.14);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiEventLegacy(track_double);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackFloatLegacy) {
  auto track_float =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kTrackFloat,
                           "Some name", orbit_api::Encode<uint64_t>(3.14f));

  auto expected_track_value =
      CreateClientTrackValue<float>(1, kProcessId, kThreadId1, "Some name",
                                    &orbit_client_protos::ApiTrackValue::set_data_float, 3.14f);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiEventLegacy(track_float);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackIntLegacy) {
  auto track_int = CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kTrackInt,
                                        "Some name", orbit_api::Encode<uint64_t>(3));

  auto expected_track_value = CreateClientTrackValue<int32_t>(
      1, kProcessId, kThreadId1, "Some name", &orbit_client_protos::ApiTrackValue::set_data_int, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiEventLegacy(track_int);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackInt64Legacy) {
  auto track_int64 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kTrackInt64,
                           "Some name", orbit_api::Encode<uint64_t>(3));

  auto expected_track_value =
      CreateClientTrackValue<int64_t>(1, kProcessId, kThreadId1, "Some name",
                                      &orbit_client_protos::ApiTrackValue::set_data_int64, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiEventLegacy(track_int64);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackUintLegacy) {
  auto track_uint =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kTrackUint, "Some name",
                           orbit_api::Encode<uint64_t>(3));

  auto expected_track_value =
      CreateClientTrackValue<uint32_t>(1, kProcessId, kThreadId1, "Some name",
                                       &orbit_client_protos::ApiTrackValue::set_data_uint, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiEventLegacy(track_uint);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

TEST_F(ApiEventProcessorTest, TrackUint64Legacy) {
  auto track_uint64 =
      CreateApiEventLegacy(kProcessId, kThreadId1, 1, orbit_api::EventType::kTrackUint64,
                           "Some name", orbit_api::Encode<uint64_t>(3));

  auto expected_track_value =
      CreateClientTrackValue<uint64_t>(1, kProcessId, kThreadId1, "Some name",
                                       &orbit_client_protos::ApiTrackValue::set_data_uint64, 3);

  orbit_client_protos::ApiTrackValue actual_track_value;
  EXPECT_CALL(capture_listener_, OnApiTrackValue)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_track_value));

  api_event_processor_.ProcessApiEventLegacy(track_uint64);

  EXPECT_TRUE(MessageDifferencer::Equivalent(expected_track_value, actual_track_value));
}

}  // namespace orbit_capture_client
