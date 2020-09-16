// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "OrbitClientModel/CaptureDeserializer.h"
#include "absl/base/casts.h"
#include "capture_data.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureHeader;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TimerInfo;
using orbit_client_protos::TracepointEventInfo;
using orbit_grpc_protos::TracepointInfo;

using ::testing::_;
using ::testing::Assign;
using ::testing::DoAll;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Matcher;
using ::testing::NotNull;
using ::testing::SaveArg;

using ::testing::InvokeWithoutArgs;
namespace {

class MockCaptureListener : public CaptureListener {
 public:
  MOCK_METHOD(
      void, OnCaptureStarted,
      (int32_t /*process_id*/, std::string /*process_name*/, std::shared_ptr<Process> /*process*/,
       (absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>)/*selected_functions*/,
       TracepointInfoSet /*selected_tracepoints*/),
      (override));
  MOCK_METHOD(void, OnCaptureComplete, (), (override));
  MOCK_METHOD(void, OnCaptureCancelled, (), (override));
  MOCK_METHOD(void, OnCaptureFailed, (ErrorMessage), (override));
  MOCK_METHOD(void, OnTimer, (const TimerInfo&), (override));
  MOCK_METHOD(void, OnKeyAndString, (uint64_t /*key*/, std::string), (override));
  MOCK_METHOD(void, OnUniqueCallStack, (CallStack), (override));
  MOCK_METHOD(void, OnCallstackEvent, (CallstackEvent), (override));
  MOCK_METHOD(void, OnThreadName, (int32_t /*thread_id*/, std::string /*thread_name*/), (override));
  MOCK_METHOD(void, OnAddressInfo, (LinuxAddressInfo), (override));
  MOCK_METHOD(void, OnUniqueTracepointInfo, (uint64_t /*key*/, TracepointInfo /*tracepoint_info*/),
              (override));
  MOCK_METHOD(void, OnTracepointEvent, (TracepointEventInfo), (override));
};

TEST(CaptureDeserializer, LoadFileNotExists) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  EXPECT_CALL(listener, OnCaptureFailed).Times(1);
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  EXPECT_CALL(listener, OnCaptureComplete).Times(0);
  EXPECT_CALL(listener, OnCaptureCancelled).Times(0);
  capture_deserializer::Load("not_existing_test_file", &listener, &cancellation_requested);
}

TEST(CaptureDeserializer, LoadNoVersion) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  ErrorMessage actual_error;
  EXPECT_CALL(listener, OnCaptureFailed).Times(1);
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  EXPECT_CALL(listener, OnCaptureComplete).Times(0);
  EXPECT_CALL(listener, OnCaptureCancelled).Times(0);
  CaptureHeader header;
  header.set_version("");

  std::string serialized_header;
  header.SerializeToString(&serialized_header);
  int32_t size_of_header = serialized_header.size();
  std::stringstream stream;
  stream << std::string(absl::bit_cast<char*>(&size_of_header), sizeof(size_of_header))
         << serialized_header;

  // std::istream test;
  capture_deserializer::Load(stream, "not_existing_test_file", &listener, &cancellation_requested);
}

TEST(CaptureDeserializer, LoadOldVersion) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  ErrorMessage actual_error;
  EXPECT_CALL(listener, OnCaptureFailed).Times(1).WillOnce(SaveArg<0>(&actual_error));
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  EXPECT_CALL(listener, OnCaptureComplete).Times(0);
  EXPECT_CALL(listener, OnCaptureCancelled).Times(0);
  CaptureHeader header;
  header.set_version("1.51");

  std::string serialized_header;
  header.SerializeToString(&serialized_header);
  int32_t size_of_header = serialized_header.size();
  std::stringstream stream;
  stream << std::string(absl::bit_cast<char*>(&size_of_header), sizeof(size_of_header))
         << serialized_header;

  capture_deserializer::Load(stream, "not_existing_test_file", &listener, &cancellation_requested);

  EXPECT_THAT(actual_error.message(), HasSubstr("1.51"));
}

TEST(CaptureDeserializer, LoadNoCaptureInfo) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  ErrorMessage actual_error;
  EXPECT_CALL(listener, OnCaptureFailed).Times(1);
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  EXPECT_CALL(listener, OnCaptureComplete).Times(0);
  EXPECT_CALL(listener, OnCaptureCancelled).Times(0);
  CaptureHeader header;
  header.set_version(capture_deserializer::internal::kRequiredCaptureVersion);

  std::string serialized_header;
  header.SerializeToString(&serialized_header);
  int32_t size_of_header = serialized_header.size();
  std::stringstream stream;
  stream << std::string(absl::bit_cast<char*>(&size_of_header), sizeof(size_of_header))
         << serialized_header;

  capture_deserializer::Load(stream, "not_existing_test_file", &listener, &cancellation_requested);
}

TEST(CaptureDeserializer, LoadCaptureInfoOnCaptureStarted) {
  MockCaptureListener listener;
  CaptureInfo capture_info;
  capture_info.set_process_id(42);
  capture_info.set_process_name("process");
  FunctionInfo* selected_function = capture_info.add_selected_functions();
  selected_function->set_name("foo");
  selected_function->set_pretty_name("void foo()");
  selected_function->set_address(21);
  selected_function->set_size(12);
  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> actual_selected_functions;
  // There will be no call to OnCaptureStarted other then the one specified next.
  EXPECT_CALL(listener, OnCaptureStarted(_, _, _, _, _)).Times(0);
  EXPECT_CALL(listener, OnCaptureStarted(42, "process", NotNull(), _, IsEmpty()))
      .Times(1)
      .WillOnce(SaveArg<3>(&actual_selected_functions));
  EXPECT_CALL(listener, OnCaptureComplete).Times(1);
  EXPECT_CALL(listener, OnCaptureFailed).Times(0);
  EXPECT_CALL(listener, OnCaptureCancelled).Times(0);

  EXPECT_CALL(listener, OnAddressInfo).Times(0);
  EXPECT_CALL(listener, OnThreadName).Times(0);

  capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &empty_stream,
                                                  &cancellation_requested);

  ASSERT_EQ(actual_selected_functions.size(), 1);
  ASSERT_TRUE(actual_selected_functions.contains(selected_function->address()));
  FunctionInfo actual_function_info = actual_selected_functions.at(selected_function->address());

  EXPECT_EQ(actual_function_info.name(), selected_function->name());
  EXPECT_EQ(actual_function_info.pretty_name(), selected_function->pretty_name());
  EXPECT_EQ(actual_function_info.size(), selected_function->size());
}

TEST(CaptureDeserializer, LoadCaptureInfoAddressInfos) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  LinuxAddressInfo address_info_1;
  address_info_1.set_function_name("foo");
  address_info_1.set_module_path("/path");
  address_info_1.set_offset_in_function(0);
  address_info_1.set_absolute_address(123);
  capture_info.add_address_infos()->CopyFrom(address_info_1);

  LinuxAddressInfo address_info_2;
  address_info_2.set_function_name("bar");
  address_info_2.set_module_path("/path");
  address_info_2.set_offset_in_function(0);
  address_info_2.set_absolute_address(123);
  capture_info.add_address_infos()->CopyFrom(address_info_2);

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  LinuxAddressInfo actual_address_info_1;
  LinuxAddressInfo actual_address_info_2;
  EXPECT_CALL(listener, OnAddressInfo)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_address_info_1))
      .WillOnce(SaveArg<0>(&actual_address_info_2));
  EXPECT_CALL(listener, OnCaptureStarted).Times(1);
  EXPECT_CALL(listener, OnCaptureComplete).Times(1);

  capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &empty_stream,
                                                  &cancellation_requested);

  EXPECT_EQ(actual_address_info_1.function_name(), address_info_1.function_name());
  EXPECT_EQ(actual_address_info_2.function_name(), address_info_2.function_name());
  EXPECT_EQ(actual_address_info_1.module_path(), address_info_1.module_path());
  EXPECT_EQ(actual_address_info_2.module_path(), address_info_2.module_path());
  EXPECT_EQ(actual_address_info_1.offset_in_function(), address_info_1.offset_in_function());
  EXPECT_EQ(actual_address_info_2.offset_in_function(), address_info_2.offset_in_function());
  EXPECT_EQ(actual_address_info_1.absolute_address(), address_info_1.absolute_address());
  EXPECT_EQ(actual_address_info_2.absolute_address(), address_info_2.absolute_address());
}

TEST(CaptureDeserializer, LoadCaptureInfoThreadNames) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  absl::flat_hash_map<int32_t, std::string> expected_thread_names;
  expected_thread_names[1] = "thread_a";
  expected_thread_names[2] = "thread_b";
  capture_info.mutable_thread_names()->insert(expected_thread_names.begin(),
                                              expected_thread_names.end());
  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  EXPECT_CALL(listener, OnThreadName(_, _)).Times(0);
  EXPECT_CALL(listener, OnThreadName(1, "thread_a")).Times(1);
  EXPECT_CALL(listener, OnThreadName(2, "thread_b")).Times(1);

  EXPECT_CALL(listener, OnCaptureStarted).Times(1);
  EXPECT_CALL(listener, OnCaptureComplete).Times(1);

  capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &empty_stream,
                                                  &cancellation_requested);
}

TEST(CaptureDeserializer, LoadCaptureInfoKeysAndStrings) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  absl::flat_hash_map<uint64_t, std::string> keys_and_strings;
  keys_and_strings[1] = "string_a";
  keys_and_strings[2] = "string_b";

  capture_info.mutable_key_to_string()->insert(keys_and_strings.begin(), keys_and_strings.end());
  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  EXPECT_CALL(listener, OnKeyAndString(_, _)).Times(0);
  EXPECT_CALL(listener, OnKeyAndString(1, "string_a")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(2, "string_b")).Times(1);

  EXPECT_CALL(listener, OnCaptureStarted).Times(1);
  EXPECT_CALL(listener, OnCaptureComplete).Times(1);

  capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &empty_stream,
                                                  &cancellation_requested);
}

TEST(CaptureDeserializer, LoadCaptureInfoCallstacks) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  // Add two callstacks with same hash:
  std::vector<uint64_t> callstack_data_1;
  callstack_data_1.push_back(1);
  callstack_data_1.push_back(2);
  callstack_data_1.push_back(3);
  CallStack callstack_1(std::move(callstack_data_1));
  CallstackInfo* callstack_info_1 = capture_info.add_callstacks();
  *callstack_info_1->mutable_data() = {callstack_1.GetFrames().begin(),
                                       callstack_1.GetFrames().end()};
  CallstackEvent* callstack_event_1_1 = capture_info.add_callstack_events();
  callstack_event_1_1->set_thread_id(1);
  callstack_event_1_1->set_time(1);
  callstack_event_1_1->set_callstack_hash(callstack_1.GetHash());
  CallstackEvent* callstack_event_1_2 = capture_info.add_callstack_events();
  callstack_event_1_2->set_thread_id(1);
  callstack_event_1_2->set_time(2);
  callstack_event_1_2->set_callstack_hash(callstack_1.GetHash());

  // Add one additional callstack with a different hash:
  std::vector<uint64_t> callstack_data_2;
  callstack_data_2.push_back(4);
  callstack_data_2.push_back(5);
  CallStack callstack_2(std::move(callstack_data_2));
  CallstackInfo* callstack_info_2 = capture_info.add_callstacks();
  *callstack_info_2->mutable_data() = {callstack_2.GetFrames().begin(),
                                       callstack_2.GetFrames().end()};
  CallstackEvent* callstack_event_2 = capture_info.add_callstack_events();
  callstack_event_2->set_thread_id(2);
  callstack_event_2->set_time(3);
  callstack_event_2->set_callstack_hash(callstack_1.GetHash());

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  bool hash_added_1 = false;
  bool hash_present_1_1 = false;
  bool hash_present_1_2 = false;

  bool hash_added_2 = false;
  bool hash_present_2 = false;

  CallStack actual_callstack_1;
  CallStack actual_callstack_2;
  EXPECT_CALL(listener, OnUniqueCallStack(_))
      .Times(2)
      .WillOnce(DoAll(SaveArg<0>(&actual_callstack_1), Assign(&hash_added_1, true)))
      .WillOnce(DoAll(SaveArg<0>(&actual_callstack_2), Assign(&hash_added_2, true)));

  CallstackEvent actual_callstack_event_1_1;
  CallstackEvent actual_callstack_event_1_2;
  CallstackEvent actual_callstack_event_2;

  auto check_hash_present_1 = [&hash_present_1_1, &hash_added_1]() mutable {
    hash_present_1_1 = hash_added_1;
  };
  auto check_hash_present_1_2 = [&hash_present_1_2, &hash_added_1]() mutable {
    hash_present_1_2 = hash_added_1;
  };
  auto check_hash_present_2 = [&hash_present_2, &hash_added_2]() mutable {
    hash_present_2 = hash_added_2;
  };

  EXPECT_CALL(listener, OnCallstackEvent(_))
      .Times(3)
      .WillOnce(
          DoAll(SaveArg<0>(&actual_callstack_event_1_1), InvokeWithoutArgs(check_hash_present_1)))
      .WillOnce(
          DoAll(SaveArg<0>(&actual_callstack_event_1_2), InvokeWithoutArgs(check_hash_present_1_2)))
      .WillOnce(
          DoAll(SaveArg<0>(&actual_callstack_event_2), InvokeWithoutArgs(check_hash_present_2)));

  EXPECT_CALL(listener, OnCaptureStarted).Times(1);
  EXPECT_CALL(listener, OnCaptureComplete).Times(1);

  capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &empty_stream,
                                                  &cancellation_requested);
  EXPECT_TRUE(hash_added_1);
  EXPECT_TRUE(hash_added_2);
  EXPECT_TRUE(hash_present_1_1);
  EXPECT_TRUE(hash_present_1_2);
  EXPECT_TRUE(hash_present_2);

  EXPECT_EQ(callstack_event_1_1->time(), actual_callstack_event_1_1.time());
  EXPECT_EQ(callstack_event_1_2->time(), actual_callstack_event_1_2.time());
  EXPECT_EQ(callstack_event_2->time(), actual_callstack_event_2.time());
}

TEST(CaptureDeserializer, LoadCaptureInfoTimers) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  CaptureInfo empty_capture_info;
  ErrorMessage actual_error;
  EXPECT_CALL(listener, OnCaptureStarted).Times(1);
  EXPECT_CALL(listener, OnCaptureComplete).Times(1);
  std::stringstream stream;

  TimerInfo timer_1;
  timer_1.set_start(0);
  timer_1.set_end(1);
  timer_1.set_process_id(42);

  std::string serialized_timer_1;
  timer_1.SerializeToString(&serialized_timer_1);
  int32_t size_of_timer_1 = serialized_timer_1.size();
  stream << std::string(absl::bit_cast<char*>(&size_of_timer_1), sizeof(size_of_timer_1))
         << serialized_timer_1;

  TimerInfo timer_2;
  timer_2.set_start(3);
  timer_2.set_end(5);
  timer_2.set_process_id(2);

  std::string serialized_timer_2;
  timer_2.SerializeToString(&serialized_timer_2);
  int32_t size_of_timer_2 = serialized_timer_2.size();
  stream << std::string(absl::bit_cast<char*>(&size_of_timer_2), sizeof(size_of_timer_2))
         << serialized_timer_2;

  TimerInfo actual_timer_1;
  TimerInfo actual_timer_2;
  EXPECT_CALL(listener, OnTimer)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_timer_1))
      .WillOnce(SaveArg<0>(&actual_timer_2));

  google::protobuf::io::IstreamInputStream input_stream(&stream);
  google::protobuf::io::CodedInputStream coded_input(&input_stream);

  capture_deserializer::internal::LoadCaptureInfo(empty_capture_info, &listener, &coded_input,
                                                  &cancellation_requested);

  EXPECT_EQ(timer_1.start(), actual_timer_1.start());
  EXPECT_EQ(timer_2.start(), actual_timer_2.start());
  EXPECT_EQ(timer_1.end(), actual_timer_1.end());
  EXPECT_EQ(timer_2.end(), actual_timer_2.end());
  EXPECT_EQ(timer_1.process_id(), actual_timer_1.process_id());
  EXPECT_EQ(timer_2.process_id(), actual_timer_2.process_id());
}

}  // namespace