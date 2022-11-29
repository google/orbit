// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <absl/synchronization/mutex.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "LinuxTracingUtils.h"
#include "ModuleUtils/ReadLinuxMaps.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ModuleInfo;

namespace orbit_linux_tracing {

TEST(GetThreadState, LinuxTracingTestsMainAndAnother) {
  pid_t main_tid = syscall(SYS_gettid);

  std::optional<char> main_state_initial = GetThreadState(main_tid);
  ASSERT_TRUE(main_state_initial.has_value());
  EXPECT_EQ('R', main_state_initial.value());

  absl::Mutex mutex;
  pid_t thread_tid = -1;
  std::optional<char> thread_state_holding_mutex;
  std::optional<char> main_state_waiting_mutex;

  mutex.Lock();
  std::thread thread{[&] {
    // Make sure /proc/<pid>/stat is parsed correctly
    // even when the thread name contains spaces and parentheses.
    orbit_base::SetCurrentThreadName(") )  )()( )(  )");
    {
      absl::MutexLock lock{&mutex};
      thread_tid = syscall(SYS_gettid);
      thread_state_holding_mutex = GetThreadState(thread_tid);
      main_state_waiting_mutex = GetThreadState(main_tid);
    }
    // Let the main thread read this thread's state while this thread is in the sleep_for and verify
    // that in such a case the state is also 'S'.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }};

  mutex.Await(absl::Condition(
      +[](pid_t* tid) { return *tid != -1; }, &thread_tid));
  mutex.Unlock();

  ASSERT_TRUE(thread_state_holding_mutex.has_value());
  EXPECT_EQ('R', thread_state_holding_mutex.value());
  ASSERT_TRUE(main_state_waiting_mutex.has_value());
  EXPECT_EQ('S', main_state_waiting_mutex.value());  // Interruptible sleep

  // Make sure `thread` has had the time to call sleep_for.
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  std::optional<char> thread_state_in_sleep_for = GetThreadState(thread_tid);
  ASSERT_TRUE(thread_state_in_sleep_for.has_value());
  EXPECT_EQ('S', thread_state_in_sleep_for.value());

  thread.join();
  // Make sure the kernel has had the time to clean up `thread` from the /proc filesystem.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::optional<char> thread_state_exited = GetThreadState(thread_tid);
  EXPECT_FALSE(thread_state_exited.has_value());
}

TEST(ExtractCpusetFromCgroup, NoCpuset) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  ASSERT_FALSE(returned_cpuset.has_value());
}

TEST(ExtractCpusetFromCgroup, OnlyCpusetInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "8:cpuset:/groupname/foo\n"
      "6:cpu,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ExtractCpusetFromCgroup, CpusetLastInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuacct,cpuset:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ExtractCpusetFromCgroup, CpusetMiddleInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuset,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ParseCpusetCpus, Empty) {
  std::string cpuset_cpus_content = "";
  std::vector<int> returned_cpus = ParseCpusetCpus(cpuset_cpus_content);
  EXPECT_TRUE(returned_cpus.empty());
}

TEST(ParseCpusetCpus, SingleValuesAndRanges) {
  std::string cpuset_cpus_content = "0-2,4,7,12-14";
  std::vector<int> returned_cpus = ParseCpusetCpus(cpuset_cpus_content);
  EXPECT_THAT(returned_cpus, ::testing::ElementsAre(0, 1, 2, 4, 7, 12, 13, 14));
}

static ModuleInfo MakeModuleInfo(std::string file_path, uint64_t address_start, uint64_t load_bias,
                                 uint64_t executable_segment_offset,
                                 ModuleInfo::ObjectFileType object_file_type) {
  ModuleInfo module;
  module.set_file_path(std::move(file_path));
  module.set_address_start(address_start);
  module.set_load_bias(load_bias);
  module.set_executable_segment_offset(executable_segment_offset);
  module.set_object_file_type(object_file_type);
  return module;
}

static InstrumentedFunction MakeInstrumentedFunction(uint64_t function_id, std::string file_path,
                                                     std::string function_name,
                                                     uint64_t virtual_address,
                                                     uint64_t offset_in_file) {
  InstrumentedFunction function;
  function.set_function_id(function_id);
  function.set_file_path(std::move(file_path));
  function.set_function_name(std::move(function_name));
  function.set_function_virtual_address(virtual_address);
  function.set_file_offset(offset_in_file);
  return function;
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, NoMaps) {
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages({}, modules, instrumented_functions);

  EXPECT_EQ(function_ids_to_error_messages.size(), 2);

  auto it = function_ids_to_error_messages.begin();
  const auto& [function_id, error_message] = *it;
  EXPECT_EQ(function_id, 1);
  EXPECT_TRUE(absl::StartsWith(error_message,
                               "Function \"foo()\" belongs to module \"/path/to/pe.exe\", which is "
                               "not loaded by the process."));

  ++it;
  const auto& [function_id2, error_message2] = *it;
  EXPECT_EQ(function_id2, 2);
  EXPECT_TRUE(absl::StartsWith(error_message2,
                               "Function \"bar()\" belongs to module \"/path/to/elf\", which is "
                               "not loaded by the process."));
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, NoModules) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 00:00 0 \n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), {}, instrumented_functions);

  EXPECT_EQ(function_ids_to_error_messages.size(), 2);

  auto it = function_ids_to_error_messages.begin();
  const auto& [function_id, error_message] = *it;
  EXPECT_EQ(function_id, 1);
  EXPECT_TRUE(absl::StartsWith(error_message,
                               "Function \"foo()\" belongs to module \"/path/to/pe.exe\", which is "
                               "not loaded by the process."));

  ++it;
  const auto& [function_id2, error_message2] = *it;
  EXPECT_EQ(function_id2, 2);
  EXPECT_TRUE(absl::StartsWith(error_message2,
                               "Function \"bar()\" belongs to module \"/path/to/elf\", which is "
                               "not loaded by the process."));
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, NoFunctionsToInstrument) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 00:00 0 \n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, {});

  EXPECT_EQ(function_ids_to_error_messages.size(), 0);
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, ModuleInMapsAndFunctionInFileMap) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 103:07 6946834        /path/to/pe.exe\n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  EXPECT_EQ(function_ids_to_error_messages.size(), 0);
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, ModuleInMapsButFunctionNotInFileMapPe) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 00:00 0 \n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  ASSERT_EQ(function_ids_to_error_messages.size(), 1);
  const auto& [function_id, error_message] = *function_ids_to_error_messages.begin();
  EXPECT_EQ(function_id, 1);
  EXPECT_TRUE(absl::StartsWith(error_message,
                               "Function \"foo()\" belonging to module \"/path/to/pe.exe\" is not "
                               "(always) loaded into a file mapping. The module is a PE,"));
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, ModuleInMapsButFunctionNotInFileMapElf) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 103:07 6946834        /path/to/pe.exe\n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "high_address()", 0x101000, 0x100000)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  ASSERT_EQ(function_ids_to_error_messages.size(), 1);
  const auto& [function_id, error_message] = *function_ids_to_error_messages.begin();
  EXPECT_EQ(function_id, 2);
  EXPECT_EQ(error_message,
            "Function \"high_address()\" belonging to module \"/path/to/elf\" is not (always) "
            "loaded into a file mapping.");
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages,
     ModuleInMapsTwiceButFunctionInOnlyOneFileMap) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 103:07 6946834        /path/to/pe.exe\n"
      "150000000-150001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "150001000-150004000 r-xp 00000000 00:00 0 \n"
      "150000000-150001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/pe.exe", 0x150001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  ASSERT_EQ(function_ids_to_error_messages.size(), 1);
  const auto& [function_id, error_message] = *function_ids_to_error_messages.begin();
  EXPECT_EQ(function_id, 1);
  EXPECT_TRUE(absl::StartsWith(error_message,
                               "Function \"foo()\" belonging to module \"/path/to/pe.exe\" is not "
                               "(always) loaded into a file mapping. The module is a PE,"));
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages,
     ModuleInMapsTwiceButFunctionNotInAnyFileMap) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 00:00 0 \n"
      "150000000-150001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "150001000-150004000 r-xp 00000000 00:00 0 \n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/pe.exe", 0x150001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  ASSERT_EQ(function_ids_to_error_messages.size(), 1);
  const auto& [function_id, error_message] = *function_ids_to_error_messages.begin();
  EXPECT_EQ(function_id, 1);
  EXPECT_TRUE(absl::StartsWith(error_message,
                               "Function \"foo()\" belonging to module \"/path/to/pe.exe\" is not "
                               "(always) loaded into a file mapping. The module is a PE,"));
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages,
     FunctionFromUnalignedPeMappedTwiceButAbsoluteAddressInAnonymousMapping) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 00:00 0 \n"
      "7f3a06c57000-7f3a06c83000 r--p 00000000 103:0a 42623    /path/to/elf\n"
      "7f3a06c83000-7f3a06cb5000 r-xp 0002b000 103:0a 42623    /path/to/elf\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "mapped_twice()", 0x140001850, 0xc50),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  ASSERT_EQ(function_ids_to_error_messages.size(), 1);
  const auto& [function_id, error_message] = *function_ids_to_error_messages.begin();
  EXPECT_EQ(function_id, 1);
  EXPECT_TRUE(
      absl::StartsWith(error_message,
                       "Function \"mapped_twice()\" belonging to module \"/path/to/pe.exe\" is not "
                       "(always) loaded into a file mapping. The module is a PE,"));
}

TEST(FindFunctionsThatUprobesCannotInstrumentWithMessages, ModuleNotInMaps) {
  constexpr const char* kProcPidMapsContent{
      "140000000-140001000 r--p 00000000 103:07 6946834        /path/to/pe.exe\n"
      "140001000-140004000 r-xp 00000000 103:07 6946834        /path/to/pe.exe\n"};
  const std::vector<ModuleInfo> modules{
      MakeModuleInfo("/path/to/pe.exe", 0x140001000, 0x140000000, 0x1000, ModuleInfo::kCoffFile),
      MakeModuleInfo("/path/to/elf", 0x00007f3a06c83000, 0x1000, 0x2b060, ModuleInfo::kElfFile),
  };
  const std::vector<InstrumentedFunction> instrumented_functions{
      MakeInstrumentedFunction(1, "/path/to/pe.exe", "foo()", 0x1400027e0, 0x1be0),
      MakeInstrumentedFunction(2, "/path/to/elf", "bar()", 0x56290, 0x55290)};
  const std::map<uint64_t, std::string> function_ids_to_error_messages =
      FindFunctionsThatUprobesCannotInstrumentWithMessages(
          orbit_module_utils::ParseMaps(kProcPidMapsContent), modules, instrumented_functions);

  ASSERT_EQ(function_ids_to_error_messages.size(), 1);
  const auto& [function_id, error_message] = *function_ids_to_error_messages.begin();
  EXPECT_EQ(function_id, 2);
  EXPECT_TRUE(absl::StartsWith(error_message,
                               "Function \"bar()\" belongs to module \"/path/to/elf\", which is "
                               "not loaded by the process."));
}

}  // namespace orbit_linux_tracing
