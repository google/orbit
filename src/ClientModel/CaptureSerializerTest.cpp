// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "CaptureSerializationTestMatchers.h"
#include "ClientData/Callstack.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TracepointCustom.h"
#include "ClientModel/CaptureData.h"
#include "ClientModel/CaptureSerializer.h"
#include "CoreUtils.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "tracepoint.pb.h"

using orbit_client_data::ModuleManager;
using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TracepointEventInfo;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::TracepointInfo;
using ::testing::ElementsAreArray;

namespace orbit_client_model {

TEST(CaptureSerializer, GenerateCaptureFileName) {
  constexpr int32_t kProcessId = 42;

  CaptureStarted capture_started;
  capture_started.set_process_id(kProcessId);
  capture_started.set_capture_start_timestamp_ns(1'392'033'600'000'000);
  capture_started.set_executable_path("/path/to/p");

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_load_bias(0);
  module_info.set_file_path("path/to/module");
  module_info.set_address_start(15);
  module_info.set_address_end(1000);

  ModuleManager module_manager;

  CaptureData capture_data{&module_manager, capture_started, {}};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());
  capture_data.mutable_process()->UpdateModuleInfos({module_info});

  std::string expected_file_name = absl::StrCat(
      "p_", orbit_core::FormatTime(capture_data.capture_start_time()), "_suffix.orbit");
  EXPECT_EQ(expected_file_name,
            capture_serializer::GenerateCaptureFileName(
                capture_data.process_name(), capture_data.capture_start_time(), "_suffix"));
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
  constexpr int32_t kProcessId = 42;
  constexpr const char* kExpectedProcessName = "p";

  CaptureStarted capture_started;
  capture_started.set_process_id(kProcessId);
  capture_started.set_capture_start_timestamp_ns(1'392'033'600'000'000);
  capture_started.set_executable_path("/path/to/p");

  constexpr uint64_t kModuleLoadBias = 10;
  constexpr uint64_t kModuleBaseAddress = 15;

  constexpr const char* kModulePath = "path/to/module";
  constexpr const char* kModuleBuildId = "build_id_id";

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_load_bias(kModuleLoadBias);
  module_info.set_file_path(kModulePath);
  module_info.set_build_id(kModuleBuildId);
  module_info.set_address_start(kModuleBaseAddress);
  module_info.set_address_end(1000);

  constexpr uint64_t kInstrumentedFunctionId = 23;
  constexpr uint64_t kInstrumentedFunctionAddress = 123;

  orbit_grpc_protos::InstrumentedFunction instrumented_function;
  instrumented_function.set_function_id(kInstrumentedFunctionId);
  instrumented_function.set_function_name("foo");
  instrumented_function.set_file_offset(kInstrumentedFunctionAddress - kModuleLoadBias);
  instrumented_function.set_file_path(kModulePath);
  instrumented_function.set_file_build_id(kModuleBuildId);

  capture_started.mutable_capture_options()->add_instrumented_functions()->CopyFrom(
      instrumented_function);

  constexpr uint64_t kSelectedFunctionAbsoluteAddress =
      kInstrumentedFunctionAddress + kModuleBaseAddress - kModuleLoadBias;

  orbit_grpc_protos::TracepointInfo tracepoint_info;
  tracepoint_info.set_category("sched");
  tracepoint_info.set_name("sched_switch");
  capture_started.mutable_capture_options()->add_instrumented_tracepoint()->CopyFrom(
      tracepoint_info);

  absl::flat_hash_set<uint64_t> frame_track_function_ids;
  frame_track_function_ids.insert(kInstrumentedFunctionId);

  ModuleManager module_manager;
  CaptureData capture_data{&module_manager, capture_started, frame_track_function_ids};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());
  capture_data.mutable_process()->UpdateModuleInfos({module_info});

  orbit_grpc_protos::ModuleSymbols symbols;
  symbols.set_symbols_file_path("path/to/symbols");
  symbols.set_load_bias(kModuleLoadBias);
  orbit_grpc_protos::SymbolInfo* symbol_info = symbols.add_symbol_infos();
  symbol_info->set_demangled_name("foo");
  symbol_info->set_name("foo");
  symbol_info->set_address(kInstrumentedFunctionAddress);
  symbol_info->set_size(10);

  module_manager.GetMutableModuleByPathAndBuildId(kModulePath, kModuleBuildId)->AddSymbols(symbols);

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

  capture_data.AddUniqueTracepointEventInfo(1, tracepoint_info);

  TracepointEventInfo tracepoint_event;
  tracepoint_event.set_tracepoint_info_key(1);
  tracepoint_event.set_pid(0);
  tracepoint_event.set_tid(1);
  tracepoint_event.set_cpu(2);
  tracepoint_event.set_time(3);
  capture_data.AddTracepointEventAndMapToThreads(
      tracepoint_event.time(), tracepoint_event.tracepoint_info_key(), tracepoint_event.pid(),
      tracepoint_event.tid(), tracepoint_event.cpu(), true);

  capture_data.UpdateFunctionStats(instrumented_function.function_id(), 100);
  capture_data.UpdateFunctionStats(instrumented_function.function_id(), 110);
  capture_data.UpdateFunctionStats(instrumented_function.function_id(), 120);

  absl::flat_hash_map<uint64_t, std::string> key_to_string_map;
  key_to_string_map[0] = "a";
  key_to_string_map[1] = "b";
  key_to_string_map[2] = "c";

  CaptureInfo capture_info =
      capture_serializer::internal::GenerateCaptureInfo(capture_data, key_to_string_map);

  ASSERT_EQ(1, capture_info.instrumented_functions_size());
  const FunctionInfo& actual_selected_function =
      capture_info.instrumented_functions().begin()->second;
  EXPECT_EQ(instrumented_function.file_offset(),
            actual_selected_function.address() - kModuleLoadBias);
  EXPECT_EQ(instrumented_function.file_path(), actual_selected_function.module_path());
  EXPECT_EQ(instrumented_function.function_name(), actual_selected_function.pretty_name());

  EXPECT_EQ(capture_info.process().pid(), kProcessId);
  EXPECT_EQ(capture_info.process().name(), kExpectedProcessName);

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
  ASSERT_EQ(actual_tracepoint_info.category(), tracepoint_info.category());
  ASSERT_EQ(actual_tracepoint_info.name(), tracepoint_info.name());
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
  ASSERT_TRUE(capture_info.function_stats().contains(kSelectedFunctionAbsoluteAddress));
  const FunctionStats& actual_function_stats =
      capture_info.function_stats().at(kSelectedFunctionAbsoluteAddress);
  const FunctionStats& expected_function_stats =
      capture_data.GetFunctionStatsOrDefault(instrumented_function.function_id());
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

}  // namespace orbit_client_model
