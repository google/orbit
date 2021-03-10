// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "CaptureSerializationTestMatchers.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientModel/CaptureDeserializer.h"
#include "capture.pb.h"
#include "capture_data.pb.h"

using orbit_client_data::ModuleManager;
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureHeader;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::ProcessInfo;
using orbit_client_protos::ThreadStateSliceInfo;
using orbit_client_protos::TimerInfo;
using orbit_client_protos::TracepointEventInfo;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_grpc_protos::TracepointInfo;

using ::testing::_;
using ::testing::Assign;
using ::testing::DoAll;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::InvokeWithoutArgs;
using ::testing::IsEmpty;
using ::testing::Matcher;
using ::testing::SaveArg;
using ::testing::Unused;

namespace {

class MockCaptureListener : public CaptureListener {
 public:
  MOCK_METHOD(void, OnCaptureStarted,
              (ProcessData&& /*process*/,
               (absl::flat_hash_map<uint64_t, InstrumentedFunction>)/*instrumented_functions*/,
               TracepointInfoSet /*selected_tracepoints*/,
               absl::flat_hash_set<uint64_t> /*frame_track_function_ids*/),
              (override));
  MOCK_METHOD(void, OnTimer, (const TimerInfo&), (override));
  MOCK_METHOD(void, OnSystemMemoryUsage, (const SystemMemoryUsage&), (override));
  MOCK_METHOD(void, OnKeyAndString, (uint64_t /*key*/, std::string), (override));
  MOCK_METHOD(void, OnUniqueCallStack, (CallStack), (override));
  MOCK_METHOD(void, OnCallstackEvent, (CallstackEvent), (override));
  MOCK_METHOD(void, OnModuleUpdate, (uint64_t /*timestamp_ns*/, ModuleInfo /*module_info*/),
              (override));
  MOCK_METHOD(void, OnModulesSnapshot,
              (uint64_t /*timestamp_ns*/, std::vector<ModuleInfo> /*module_infos*/), (override));
  MOCK_METHOD(void, OnThreadName, (int32_t /*thread_id*/, std::string /*thread_name*/), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (ThreadStateSliceInfo), (override));
  MOCK_METHOD(void, OnAddressInfo, (LinuxAddressInfo), (override));
  MOCK_METHOD(void, OnUniqueTracepointInfo, (uint64_t /*key*/, TracepointInfo /*tracepoint_info*/),
              (override));
  MOCK_METHOD(void, OnTracepointEvent, (TracepointEventInfo), (override));
};

TEST(CaptureDeserializer, LoadFileNotExists) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result = capture_deserializer::Load(
      "not_existing_test_file", &listener, &module_manager, &cancellation_requested);
  ASSERT_TRUE(result.has_error());

  EXPECT_EQ(result.error().message(),
            "Unable to open file \"not_existing_test_file\": No such file or directory");
}

TEST(CaptureDeserializer, LoadNoVersion) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  CaptureHeader header;
  header.set_version("");

  std::string serialized_header;
  header.SerializeToString(&serialized_header);
  int32_t size_of_header = serialized_header.size();

  std::string buffer;
  buffer.append(absl::bit_cast<char*>(&size_of_header), sizeof(size_of_header))
      .append(serialized_header);

  google::protobuf::io::ArrayInputStream array_input_stream(buffer.data(),
                                                            static_cast<int>(buffer.size()));
  google::protobuf::io::CodedInputStream coded_input_stream(&array_input_stream);

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result = capture_deserializer::Load(
      &coded_input_stream, "file_name", &listener, &module_manager, &cancellation_requested);
  ASSERT_TRUE(result.has_error());

  std::string expected_error_message =
      "Error parsing the capture from \"file_name\".\nNote: If the capture "
      "was taken with a previous Orbit version, it could be incompatible. "
      "Please check release notes for more information.";

  EXPECT_EQ(result.error().message(), expected_error_message);
}

TEST(CaptureDeserializer, LoadOldVersion) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  CaptureHeader header;
  header.set_version("1.51");

  std::string serialized_header;
  header.SerializeToString(&serialized_header);
  int32_t size_of_header = serialized_header.size();

  std::string buffer;
  buffer.append(absl::bit_cast<char*>(&size_of_header), sizeof(size_of_header))
      .append(serialized_header);

  google::protobuf::io::ArrayInputStream array_input_stream(buffer.data(),
                                                            static_cast<int>(buffer.size()));
  google::protobuf::io::CodedInputStream coded_input_stream(&array_input_stream);

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result = capture_deserializer::Load(
      &coded_input_stream, "file_name", &listener, &module_manager, &cancellation_requested);
  ASSERT_TRUE(result.has_error());

  EXPECT_THAT(result.error().message(), HasSubstr("1.51"));
}

TEST(CaptureDeserializer, LoadNoCaptureInfo) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  EXPECT_CALL(listener, OnCaptureStarted).Times(0);
  CaptureHeader header;
  header.set_version(capture_deserializer::internal::kRequiredCaptureVersion);

  std::string serialized_header;
  header.SerializeToString(&serialized_header);
  int32_t size_of_header = serialized_header.size();

  std::string buffer;
  buffer.append(absl::bit_cast<char*>(&size_of_header), sizeof(size_of_header))
      .append(serialized_header);

  google::protobuf::io::ArrayInputStream array_input_stream(buffer.data(),
                                                            static_cast<int>(buffer.size()));
  google::protobuf::io::CodedInputStream coded_input_stream(&array_input_stream);

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result = capture_deserializer::Load(
      &coded_input_stream, "file_name", &listener, &module_manager, &cancellation_requested);
  ASSERT_TRUE(result.has_error());
}

TEST(CaptureDeserializer, LoadCaptureInfoOnCaptureStarted) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  ProcessInfo* process_info = capture_info.mutable_process();
  process_info->set_pid(42);
  process_info->set_name("process");

  constexpr uint64_t kLoadBias = 5;
  constexpr const char* kModuleBuildId = "build_id";
  orbit_client_protos::ModuleInfo* module_info = capture_info.add_modules();
  module_info->set_load_bias(kLoadBias);
  module_info->set_name("module");
  module_info->set_file_path("path/to/module");
  module_info->set_build_id(kModuleBuildId);
  module_info->set_address_start(10);
  module_info->set_address_end(123);

  constexpr uint64_t kInstrumentedFunctionId = 1;
  FunctionInfo instrumented_function;
  instrumented_function.set_name("foo");
  instrumented_function.set_pretty_name("void foo()");
  instrumented_function.set_module_path("path/to/module");
  instrumented_function.set_module_build_id(kModuleBuildId);
  instrumented_function.set_address(21);
  instrumented_function.set_size(12);
  (*capture_info.mutable_instrumented_functions())[kInstrumentedFunctionId] = instrumented_function;

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  ModuleManager module_manager;

  EXPECT_CALL(listener, OnCaptureStarted(_, _, IsEmpty(), _))
      .Times(1)
      .WillOnce([&instrumented_function, load_bias = kLoadBias, kInstrumentedFunctionId,
                 &module_manager](
                    ProcessData&& process,
                    absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>
                        actual_instrumented_functions,
                    Unused, Unused) {
        EXPECT_EQ(process.name(), "process");
        EXPECT_EQ(process.pid(), 42);
        EXPECT_EQ(process.GetModuleBaseAddress("path/to/module"), 10);

        ASSERT_EQ(actual_instrumented_functions.size(), 1);
        ASSERT_TRUE(actual_instrumented_functions.contains(kInstrumentedFunctionId));
        const InstrumentedFunction& actual_function_info =
            actual_instrumented_functions.at(kInstrumentedFunctionId);

        EXPECT_EQ(actual_function_info.function_name(), instrumented_function.pretty_name());
        EXPECT_EQ(actual_function_info.file_path(), instrumented_function.module_path());
        EXPECT_EQ(actual_function_info.file_build_id(), instrumented_function.module_build_id());
        EXPECT_EQ(actual_function_info.file_offset(), instrumented_function.address() - load_bias);

        // Also check that we can find corresponding function_infos using module_path/build_id and
        // offset
        const ModuleData* module = module_manager.GetModuleByPathAndBuildId(
            actual_function_info.file_path(), actual_function_info.file_build_id());
        ASSERT_NE(module, nullptr);
        const FunctionInfo* function_info = module->FindFunctionByElfAddress(
            module->load_bias() + actual_function_info.file_offset(), true);
        ASSERT_NE(function_info, nullptr);

        EXPECT_EQ(function_info->name(), instrumented_function.name());
        EXPECT_EQ(function_info->pretty_name(), instrumented_function.pretty_name());
        EXPECT_EQ(function_info->module_path(), instrumented_function.module_path());
        EXPECT_EQ(function_info->module_build_id(), instrumented_function.module_build_id());
        EXPECT_EQ(function_info->address(), instrumented_function.address());
        EXPECT_EQ(function_info->size(), instrumented_function.size());
      });

  EXPECT_CALL(listener, OnAddressInfo).Times(0);
  EXPECT_CALL(listener, OnThreadName).Times(0);

  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);
}

TEST(CaptureDeserializer, LoadCaptureInfoNoBuildIdInFunctionInfo) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  ProcessInfo* process_info = capture_info.mutable_process();
  process_info->set_pid(42);
  process_info->set_name("process");

  constexpr uint64_t kLoadBias = 5;
  constexpr const char* kModuleBuildId = "build_id";
  orbit_client_protos::ModuleInfo* module_info = capture_info.add_modules();
  module_info->set_load_bias(kLoadBias);
  module_info->set_name("module");
  module_info->set_file_path("path/to/module");
  module_info->set_build_id(kModuleBuildId);
  module_info->set_address_start(10);
  module_info->set_address_end(123);

  constexpr uint64_t kInstrumentedFunctionId = 1;
  FunctionInfo instrumented_function;
  instrumented_function.set_name("foo");
  instrumented_function.set_pretty_name("void foo()");
  instrumented_function.set_module_path("path/to/module");
  instrumented_function.set_module_build_id("");
  instrumented_function.set_address(21);
  instrumented_function.set_size(12);
  (*capture_info.mutable_instrumented_functions())[kInstrumentedFunctionId] = instrumented_function;

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  ModuleManager module_manager;
  EXPECT_CALL(listener, OnCaptureStarted(_, _, IsEmpty(), _))
      .Times(1)
      .WillOnce([&instrumented_function, load_bias = kLoadBias, kInstrumentedFunctionId,
                 kModuleBuildId, &module_manager](
                    ProcessData&& process,
                    absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>
                        actual_instrumented_functions,
                    Unused, Unused) {
        EXPECT_EQ(process.name(), "process");
        EXPECT_EQ(process.pid(), 42);
        EXPECT_EQ(process.GetModuleBaseAddress("path/to/module"), 10);

        ASSERT_EQ(actual_instrumented_functions.size(), 1);
        ASSERT_TRUE(actual_instrumented_functions.contains(kInstrumentedFunctionId));
        const InstrumentedFunction& actual_function_info =
            actual_instrumented_functions.at(kInstrumentedFunctionId);

        EXPECT_EQ(actual_function_info.function_name(), instrumented_function.pretty_name());
        EXPECT_EQ(actual_function_info.file_path(), instrumented_function.module_path());
        EXPECT_EQ(actual_function_info.file_build_id(), kModuleBuildId);
        EXPECT_EQ(actual_function_info.file_offset(), instrumented_function.address() - load_bias);

        // Also check that we can find corresponding function_infos using module_path/build_id and
        // offset
        const ModuleData* module = module_manager.GetModuleByPathAndBuildId(
            actual_function_info.file_path(), actual_function_info.file_build_id());
        ASSERT_NE(module, nullptr);
        const FunctionInfo* function_info = module->FindFunctionByElfAddress(
            module->load_bias() + actual_function_info.file_offset(), true);
        ASSERT_NE(function_info, nullptr);

        EXPECT_EQ(function_info->name(), instrumented_function.name());
        EXPECT_EQ(function_info->pretty_name(), instrumented_function.pretty_name());
        EXPECT_EQ(function_info->module_path(), instrumented_function.module_path());
        EXPECT_EQ(function_info->module_build_id(), kModuleBuildId);
        EXPECT_EQ(function_info->address(), instrumented_function.address());
        EXPECT_EQ(function_info->size(), instrumented_function.size());
      });

  EXPECT_CALL(listener, OnAddressInfo).Times(0);
  EXPECT_CALL(listener, OnThreadName).Times(0);

  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);
}

TEST(CaptureDeserializer, LoadCaptureInfoModuleManager) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  ProcessInfo* process_info = capture_info.mutable_process();
  process_info->set_pid(42);
  process_info->set_name("process");

  std::string module_path = "path/to/module";
  constexpr const char* kBuildId = "build id 42";
  orbit_client_protos::ModuleInfo* module_info = capture_info.add_modules();
  module_info->set_name("module");
  module_info->set_file_path(module_path);
  module_info->set_file_size(300);
  module_info->set_build_id(kBuildId);
  module_info->set_load_bias(0x400);

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  // There will be no call to OnCaptureStarted other then the one specified next.
  EXPECT_CALL(listener, OnCaptureStarted).Times(1);

  EXPECT_CALL(listener, OnAddressInfo).Times(0);
  EXPECT_CALL(listener, OnThreadName).Times(0);

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);

  const ModuleData* module = module_manager.GetModuleByPathAndBuildId(module_path, kBuildId);
  ASSERT_NE(module, nullptr);
  EXPECT_EQ(module->name(), module_info->name());
  EXPECT_EQ(module->file_path(), module_info->file_path());
  EXPECT_EQ(module->file_size(), module_info->file_size());
  EXPECT_EQ(module->build_id(), module_info->build_id());
  EXPECT_EQ(module->load_bias(), module_info->load_bias());
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
  address_info_2.set_offset_in_function(6);
  address_info_2.set_absolute_address(243);
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

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);

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

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);
}

TEST(CaptureDeserializer, LoadCaptureInfoThreadStateSlices) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  orbit_client_protos::ThreadStateSliceInfo thread_state_slice0;
  thread_state_slice0.set_tid(42);
  thread_state_slice0.set_thread_state(orbit_client_protos::ThreadStateSliceInfo::kRunnable);
  thread_state_slice0.set_begin_timestamp_ns(1000);
  thread_state_slice0.set_end_timestamp_ns(2000);
  capture_info.add_thread_state_slices()->CopyFrom(thread_state_slice0);

  orbit_client_protos::ThreadStateSliceInfo thread_state_slice1;
  thread_state_slice1.set_tid(42);
  thread_state_slice1.set_thread_state(
      orbit_client_protos::ThreadStateSliceInfo::kInterruptibleSleep);
  thread_state_slice1.set_begin_timestamp_ns(3000);
  thread_state_slice1.set_end_timestamp_ns(4000);
  capture_info.add_thread_state_slices()->CopyFrom(thread_state_slice1);

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  {
    InSequence sequence;
    EXPECT_CALL(listener, OnThreadStateSlice(ThreadStateSliceInfoEq(thread_state_slice0))).Times(1);
    EXPECT_CALL(listener, OnThreadStateSlice(ThreadStateSliceInfoEq(thread_state_slice1))).Times(1);
  }

  EXPECT_CALL(listener, OnCaptureStarted).Times(1);

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);
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

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);
}

TEST(CaptureDeserializer, LoadCaptureInfoCallstacks) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  // Add two callstacks with same hash:
  std::vector<uint64_t> callstack_data_1;
  callstack_data_1.push_back(1);
  callstack_data_1.push_back(2);
  callstack_data_1.push_back(3);
  CallStack callstack_1(1, std::move(callstack_data_1));
  CallstackInfo callstack_info_1;
  *callstack_info_1.mutable_data() = {callstack_1.frames().begin(), callstack_1.frames().end()};
  (*capture_info.mutable_callstacks())[1] = callstack_info_1;
  CallstackEvent* callstack_event_1_1 = capture_info.add_callstack_events();
  callstack_event_1_1->set_thread_id(1);
  callstack_event_1_1->set_time(1);
  callstack_event_1_1->set_callstack_id(callstack_1.id());
  CallstackEvent* callstack_event_1_2 = capture_info.add_callstack_events();
  callstack_event_1_2->set_thread_id(1);
  callstack_event_1_2->set_time(2);
  callstack_event_1_2->set_callstack_id(callstack_1.id());

  // Add one additional callstack with a different hash:
  std::vector<uint64_t> callstack_data_2;
  callstack_data_2.push_back(4);
  callstack_data_2.push_back(5);
  CallStack callstack_2(2, std::move(callstack_data_2));
  CallstackInfo callstack_info_2;
  *callstack_info_2.mutable_data() = {callstack_2.frames().begin(), callstack_2.frames().end()};
  (*capture_info.mutable_callstacks())[2] = callstack_info_2;
  CallstackEvent* callstack_event_2 = capture_info.add_callstack_events();
  callstack_event_2->set_thread_id(2);
  callstack_event_2->set_time(3);
  callstack_event_2->set_callstack_id(callstack_2.id());

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

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);

  EXPECT_TRUE(hash_added_1);
  EXPECT_TRUE(hash_added_2);
  EXPECT_TRUE(hash_present_1_1);
  EXPECT_TRUE(hash_present_1_2);
  EXPECT_TRUE(hash_present_2);

  EXPECT_EQ(callstack_event_1_1->time(), actual_callstack_event_1_1.time());
  EXPECT_EQ(callstack_event_1_2->time(), actual_callstack_event_1_2.time());
  EXPECT_EQ(callstack_event_2->time(), actual_callstack_event_2.time());
}

TEST(CaptureDeserializer, LoadCaptureInfoTracepoints) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  // Add two tracepoints with same hash:
  orbit_client_protos::TracepointInfo* tracepoint_info_1 = capture_info.add_tracepoint_infos();
  tracepoint_info_1->set_tracepoint_info_key(1);

  TracepointEventInfo* tracepoint_event_1_1 = capture_info.add_tracepoint_event_infos();
  tracepoint_event_1_1->set_tracepoint_info_key(tracepoint_info_1->tracepoint_info_key());
  tracepoint_event_1_1->set_pid(1);
  tracepoint_event_1_1->set_tid(2);
  tracepoint_event_1_1->set_time(3);
  tracepoint_event_1_1->set_cpu(4);

  TracepointEventInfo* tracepoint_event_1_2 = capture_info.add_tracepoint_event_infos();
  tracepoint_event_1_2->set_tracepoint_info_key(tracepoint_info_1->tracepoint_info_key());
  tracepoint_event_1_2->set_pid(5);
  tracepoint_event_1_2->set_tid(6);
  tracepoint_event_1_2->set_time(7);
  tracepoint_event_1_2->set_cpu(8);

  // Add one additional tracepoint event with a different hash:
  orbit_client_protos::TracepointInfo* tracepoint_info_2 = capture_info.add_tracepoint_infos();
  tracepoint_info_2->set_tracepoint_info_key(2);

  TracepointEventInfo* tracepoint_event_2 = capture_info.add_tracepoint_event_infos();
  tracepoint_event_2->set_tracepoint_info_key(tracepoint_info_2->tracepoint_info_key());
  tracepoint_event_2->set_pid(9);
  tracepoint_event_2->set_tid(10);
  tracepoint_event_2->set_time(11);
  tracepoint_event_2->set_cpu(12);

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  bool hash_added_1 = false;
  bool hash_present_1_1 = false;
  bool hash_present_1_2 = false;

  bool hash_added_2 = false;
  bool hash_present_2 = false;

  TracepointInfo actual_tracepoint_1;
  TracepointInfo actual_tracepoint_2;
  EXPECT_CALL(listener, OnUniqueTracepointInfo(_, _))
      .Times(2)
      .WillOnce(DoAll(SaveArg<1>(&actual_tracepoint_1), Assign(&hash_added_1, true)))
      .WillOnce(DoAll(SaveArg<1>(&actual_tracepoint_2), Assign(&hash_added_2, true)));

  TracepointEventInfo actual_tracepoint_event_1_1;
  TracepointEventInfo actual_tracepoint_event_1_2;
  TracepointEventInfo actual_tracepoint_event_2;

  auto check_hash_present_1 = [&hash_present_1_1, &hash_added_1]() mutable {
    hash_present_1_1 = hash_added_1;
  };
  auto check_hash_present_1_2 = [&hash_present_1_2, &hash_added_1]() mutable {
    hash_present_1_2 = hash_added_1;
  };
  auto check_hash_present_2 = [&hash_present_2, &hash_added_2]() mutable {
    hash_present_2 = hash_added_2;
  };

  EXPECT_CALL(listener, OnTracepointEvent(_))
      .Times(3)
      .WillOnce(
          DoAll(SaveArg<0>(&actual_tracepoint_event_1_1), InvokeWithoutArgs(check_hash_present_1)))
      .WillOnce(DoAll(SaveArg<0>(&actual_tracepoint_event_1_2),
                      InvokeWithoutArgs(check_hash_present_1_2)))
      .WillOnce(
          DoAll(SaveArg<0>(&actual_tracepoint_event_2), InvokeWithoutArgs(check_hash_present_2)));

  EXPECT_CALL(listener, OnCaptureStarted).Times(1);

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);

  EXPECT_TRUE(hash_added_1);
  EXPECT_TRUE(hash_added_2);
  EXPECT_TRUE(hash_present_1_1);
  EXPECT_TRUE(hash_present_1_2);
  EXPECT_TRUE(hash_present_2);

  auto check_tracepoint_equality = [](const auto& tracepoint_event_info_lhs,
                                      const auto& tracepoint_event_info_rhs) {
    return tracepoint_event_info_lhs->pid() == tracepoint_event_info_rhs.pid() &&
           tracepoint_event_info_lhs->tid() == tracepoint_event_info_rhs.tid() &&
           tracepoint_event_info_lhs->time() == tracepoint_event_info_rhs.time() &&
           tracepoint_event_info_lhs->cpu() == tracepoint_event_info_rhs.cpu();
  };

  EXPECT_TRUE(check_tracepoint_equality(tracepoint_event_1_1, actual_tracepoint_event_1_1));
  EXPECT_TRUE(check_tracepoint_equality(tracepoint_event_1_2, actual_tracepoint_event_1_2));
  EXPECT_TRUE(check_tracepoint_equality(tracepoint_event_2, actual_tracepoint_event_2));
}

TEST(CaptureDeserializer, LoadCaptureInfoTimers) {
  MockCaptureListener listener;
  std::atomic<bool> cancellation_requested = false;
  CaptureInfo empty_capture_info;
  ErrorMessage actual_error;
  EXPECT_CALL(listener, OnCaptureStarted).Times(1);
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

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(
          empty_capture_info, &listener, &module_manager, &coded_input, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);

  EXPECT_EQ(timer_1.start(), actual_timer_1.start());
  EXPECT_EQ(timer_2.start(), actual_timer_2.start());
  EXPECT_EQ(timer_1.end(), actual_timer_1.end());
  EXPECT_EQ(timer_2.end(), actual_timer_2.end());
  EXPECT_EQ(timer_1.process_id(), actual_timer_1.process_id());
  EXPECT_EQ(timer_2.process_id(), actual_timer_2.process_id());
}

TEST(CaptureDeserializer, LoadCaptureInfoUserDefinedCaptureData) {
  MockCaptureListener listener;
  CaptureInfo capture_info;

  uint64_t frame_track_function_id = 42;
  capture_info.mutable_user_defined_capture_info()
      ->mutable_frame_tracks_info()
      ->add_frame_track_function_ids(frame_track_function_id);

  std::atomic<bool> cancellation_requested = false;
  uint8_t empty_data = 0;
  google::protobuf::io::CodedInputStream empty_stream(&empty_data, 0);

  // There will be no call to OnCaptureStarted other then the one specified next.
  absl::flat_hash_set<uint64_t> actual_frame_track_function_ids;
  EXPECT_CALL(listener, OnCaptureStarted)
      .Times(1)
      .WillOnce(SaveArg<3>(&actual_frame_track_function_ids));

  ModuleManager module_manager;
  ErrorMessageOr<CaptureListener::CaptureOutcome> result =
      capture_deserializer::internal::LoadCaptureInfo(capture_info, &listener, &module_manager,
                                                      &empty_stream, &cancellation_requested);
  ASSERT_TRUE(result.has_value()) << result.error().message();
  EXPECT_EQ(result.value(), CaptureListener::CaptureOutcome::kComplete);

  ASSERT_EQ(1, actual_frame_track_function_ids.size());
  EXPECT_TRUE(actual_frame_track_function_ids.contains(frame_track_function_id));
}

}  // namespace