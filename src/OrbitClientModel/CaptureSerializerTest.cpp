// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "CaptureSerializationTestMatchers.h"
#include "CoreUtils.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientModel/CaptureData.h"
#include "OrbitClientModel/CaptureSerializer.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "process.pb.h"
#include "tracepoint.pb.h"

using orbit_client_data::ModuleManager;
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TracepointEventInfo;
using orbit_grpc_protos::TracepointInfo;
using ::testing::ElementsAreArray;

TEST(CaptureSerializer, GetCaptureFileName) {
  int32_t process_id = 42;
  std::string process_name = "p";
  orbit_grpc_protos::ProcessInfo process_info;
  process_info.set_name(process_name);
  process_info.set_pid(process_id);
  ProcessData process(process_info);

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_load_bias(0);
  module_info.set_file_path("path/to/module");
  module_info.set_address_start(15);
  module_info.set_address_end(1000);
  ModuleData module(module_info);

  std::vector<orbit_grpc_protos::ModuleInfo> module_infos{module_info};
  process.UpdateModuleInfos(module_infos);
  ModuleManager module_manager;
  module_manager.AddOrUpdateModules(module_infos);

  CaptureData capture_data{std::move(process), &module_manager, {}, {}, {}};

  time_t timestamp = std::chrono::system_clock::to_time_t(capture_data.capture_start_time());
  std::string expected_file_name = absl::StrCat("p_", orbit_core::FormatTime(timestamp), ".orbit");
  EXPECT_EQ(expected_file_name, capture_serializer::GetCaptureFileName(capture_data));
}

TEST(CaptureSerializer, IncludeOrbitExtensionInFile) {
  std::string file_name_with_extension = "process_000.orbit";
  std::string expected_file_name = file_name_with_extension;
  capture_serializer::IncludeOrbitExtensionInFile(file_name_with_extension);
  EXPECT_EQ(expected_file_name, file_name_with_extension);

  std::string file_name_without_extension = "process_000";
  capture_serializer::IncludeOrbitExtensionInFile(file_name_without_extension);
  EXPECT_EQ(expected_file_name, file_name_without_extension);
}

TEST(CaptureSerializer, GenerateCaptureInfo) {
  int32_t process_id = 42;
  std::string process_name = "p";
  orbit_grpc_protos::ProcessInfo process_info;
  process_info.set_name(process_name);
  process_info.set_pid(process_id);
  ProcessData process(process_info);

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_load_bias(0);
  module_info.set_file_path("path/to/module");
  module_info.set_address_start(15);
  module_info.set_address_end(1000);
  ModuleData module(module_info);

  std::vector<orbit_grpc_protos::ModuleInfo> module_infos{module_info};
  process.UpdateModuleInfos(module_infos);
  ModuleManager module_manager;
  module_manager.AddOrUpdateModules(module_infos);

  constexpr uint64_t kInstrumentedFunctionId = 23;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> instrumented_functions;
  FunctionInfo instrumented_function;
  instrumented_function.set_name("foo");
  instrumented_function.set_address(123);
  instrumented_function.set_loaded_module_path("path/to/module");
  uint64_t selected_function_absolute_address = 123 + 15 - 0;
  instrumented_functions[kInstrumentedFunctionId] = instrumented_function;

  TracepointInfoSet selected_tracepoints;
  TracepointInfo selected_tracepoint_info;
  selected_tracepoint_info.set_category("sched");
  selected_tracepoint_info.set_name("sched_switch");
  selected_tracepoints.insert(selected_tracepoint_info);

  absl::flat_hash_set<uint64_t> frame_track_function_ids;
  frame_track_function_ids.insert(kInstrumentedFunctionId);

  CaptureData capture_data{std::move(process), &module_manager, instrumented_functions,
                           selected_tracepoints, frame_track_function_ids};

  capture_data.AddOrAssignThreadName(42, "t42");
  capture_data.AddOrAssignThreadName(43, "t43");

  orbit_client_protos::ThreadStateSliceInfo thread_state_slice0;
  thread_state_slice0.set_tid(42);
  thread_state_slice0.set_thread_state(orbit_client_protos::ThreadStateSliceInfo::kRunnable);
  thread_state_slice0.set_begin_timestamp_ns(1000);
  thread_state_slice0.set_end_timestamp_ns(2000);
  capture_data.AddThreadStateSlice(thread_state_slice0);
  orbit_client_protos::ThreadStateSliceInfo thread_state_slice1;

  thread_state_slice1.set_tid(42);
  thread_state_slice1.set_thread_state(
      orbit_client_protos::ThreadStateSliceInfo::kInterruptibleSleep);
  thread_state_slice1.set_begin_timestamp_ns(3000);
  thread_state_slice1.set_end_timestamp_ns(4000);
  capture_data.AddThreadStateSlice(thread_state_slice1);

  LinuxAddressInfo address_info;
  address_info.set_absolute_address(987);
  address_info.set_offset_in_function(0);
  capture_data.InsertAddressInfo(address_info);

  std::vector<uint64_t> addresses;
  addresses.push_back(1);
  addresses.push_back(2);
  addresses.push_back(3);
  CallStack callstack(1, std::move(addresses));
  capture_data.AddUniqueCallStack(callstack);

  CallstackEvent callstack_event;
  callstack_event.set_time(1);
  callstack_event.set_thread_id(123);
  callstack_event.set_callstack_id(callstack.id());
  capture_data.AddCallstackEvent(callstack_event);

  capture_data.AddUniqueTracepointEventInfo(1, selected_tracepoint_info);

  TracepointEventInfo tracepoint_event;
  tracepoint_event.set_tracepoint_info_key(1);
  tracepoint_event.set_pid(0);
  tracepoint_event.set_tid(1);
  tracepoint_event.set_cpu(2);
  tracepoint_event.set_time(3);
  capture_data.AddTracepointEventAndMapToThreads(
      tracepoint_event.time(), tracepoint_event.tracepoint_info_key(), tracepoint_event.pid(),
      tracepoint_event.tid(), tracepoint_event.cpu(), true);

  capture_data.UpdateFunctionStats(instrumented_function, 100);
  capture_data.UpdateFunctionStats(instrumented_function, 110);
  capture_data.UpdateFunctionStats(instrumented_function, 120);

  absl::flat_hash_map<uint64_t, std::string> key_to_string_map;
  key_to_string_map[0] = "a";
  key_to_string_map[1] = "b";
  key_to_string_map[2] = "c";

  CaptureInfo capture_info =
      capture_serializer::internal::GenerateCaptureInfo(capture_data, key_to_string_map);

  ASSERT_EQ(1, capture_info.instrumented_functions_size());
  const FunctionInfo& actual_selected_function =
      capture_info.instrumented_functions().begin()->second;
  EXPECT_EQ(instrumented_function.address(), actual_selected_function.address());
  EXPECT_EQ(instrumented_function.name(), actual_selected_function.name());

  EXPECT_EQ(process_id, capture_info.process().pid());
  EXPECT_EQ(process_name, capture_info.process().name());

  ASSERT_EQ(1, capture_info.modules_size());
  const orbit_client_protos::ModuleInfo& actual_module = capture_info.modules(0);
  EXPECT_EQ(module_info.name(), actual_module.name());
  EXPECT_EQ(module_info.file_path(), actual_module.file_path());
  EXPECT_EQ(module_info.load_bias(), actual_module.load_bias());
  EXPECT_EQ(module_info.address_start(), actual_module.address_start());
  EXPECT_EQ(module_info.address_end(), actual_module.address_end());

  EXPECT_THAT(
      capture_info.thread_names(),
      ::testing::UnorderedElementsAre(::testing::Pair(42, "t42"), ::testing::Pair(43, "t43")));

  EXPECT_THAT(
      capture_info.thread_state_slices(),
      ::testing::Pointwise(ThreadStateSliceInfoEq(), {thread_state_slice0, thread_state_slice1}));

  ASSERT_EQ(1, capture_info.address_infos_size());
  const LinuxAddressInfo& actual_address_info = capture_info.address_infos(0);
  EXPECT_EQ(address_info.absolute_address(), actual_address_info.absolute_address());
  EXPECT_EQ(address_info.offset_in_function(), actual_address_info.offset_in_function());

  ASSERT_EQ(1, capture_info.callstacks_size());
  const CallstackInfo& actual_callstack = capture_info.callstacks().at(1);
  std::vector<uint64_t> actual_callstack_data{actual_callstack.data().begin(),
                                              actual_callstack.data().end()};
  EXPECT_THAT(actual_callstack_data, ElementsAreArray(callstack.frames()));

  ASSERT_EQ(1, capture_info.callstack_events_size());
  const CallstackEvent& actual_callstack_event = capture_info.callstack_events(0);
  EXPECT_EQ(callstack_event.thread_id(), actual_callstack_event.thread_id());
  EXPECT_EQ(callstack_event.time(), actual_callstack_event.time());
  EXPECT_EQ(callstack_event.callstack_id(), actual_callstack_event.callstack_id());

  ASSERT_EQ(1, capture_info.tracepoint_infos_size());
  const orbit_client_protos::TracepointInfo& actual_tracepoint_info =
      capture_info.tracepoint_infos(0);
  ASSERT_EQ(actual_tracepoint_info.category(), selected_tracepoint_info.category());
  ASSERT_EQ(actual_tracepoint_info.name(), selected_tracepoint_info.name());
  ASSERT_EQ(actual_tracepoint_info.tracepoint_info_key(), 1);

  ASSERT_EQ(1, capture_info.tracepoint_event_infos_size());
  const orbit_client_protos::TracepointEventInfo& actual_tracepoint_event =
      capture_info.tracepoint_event_infos(0);
  EXPECT_EQ(tracepoint_event.tid(), actual_tracepoint_event.tid());
  EXPECT_EQ(tracepoint_event.time(), actual_tracepoint_event.time());
  EXPECT_EQ(tracepoint_event.tracepoint_info_key(), actual_tracepoint_event.tracepoint_info_key());

  ASSERT_EQ(1, capture_info.user_defined_capture_info()
                   .frame_tracks_info()
                   .frame_track_function_ids()
                   .size());
  uint64_t actual_frame_track_function_id =
      capture_info.user_defined_capture_info().frame_tracks_info().frame_track_function_ids(0);
  EXPECT_EQ(actual_frame_track_function_id, kInstrumentedFunctionId);

  ASSERT_EQ(1, capture_info.function_stats_size());
  ASSERT_TRUE(capture_info.function_stats().contains(selected_function_absolute_address));
  const FunctionStats& actual_function_stats =
      capture_info.function_stats().at(selected_function_absolute_address);
  const FunctionStats& expected_function_stats =
      capture_data.GetFunctionStatsOrDefault(instrumented_function);
  EXPECT_EQ(expected_function_stats.count(), actual_function_stats.count());
  EXPECT_EQ(expected_function_stats.total_time_ns(), actual_function_stats.total_time_ns());
  EXPECT_EQ(expected_function_stats.average_time_ns(), actual_function_stats.average_time_ns());
  EXPECT_EQ(expected_function_stats.min_ns(), actual_function_stats.min_ns());
  EXPECT_EQ(expected_function_stats.max_ns(), actual_function_stats.max_ns());

  ASSERT_EQ(key_to_string_map.size(), capture_info.key_to_string_size());
  for (const auto& expected_key_to_string : key_to_string_map) {
    ASSERT_TRUE(capture_info.key_to_string().contains(expected_key_to_string.first));
    EXPECT_EQ(expected_key_to_string.second,
              capture_info.key_to_string().at(expected_key_to_string.first));
  }
}