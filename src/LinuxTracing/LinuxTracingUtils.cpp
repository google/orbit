// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracingUtils.h"

#include <absl/container/flat_hash_map.h>
#include <absl/meta/type_traits.h>
#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <errno.h>
#include <sys/resource.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "ModuleUtils/ReadLinuxMaps.h"
#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"
#include "OrbitBase/ExecuteCommand.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"

namespace fs = std::filesystem;

using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ModuleInfo;
using orbit_module_utils::LinuxMemoryMapping;

namespace orbit_linux_tracing {

std::optional<char> GetThreadState(pid_t tid) {
  fs::path stat{fs::path{"/proc"} / std::to_string(tid) / "stat"};
  if (!fs::exists(stat)) {
    return std::nullopt;
  }

  ErrorMessageOr<std::string> file_content_or_error = orbit_base::ReadFileToString(stat);
  if (file_content_or_error.has_error()) {
    ORBIT_ERROR("Could not open \"%s\": %s", stat.string(),
                file_content_or_error.error().message());
    return std::nullopt;
  }

  std::vector<std::string> lines =
      absl::StrSplit(file_content_or_error.value(), absl::MaxSplits('\n', 1));
  if (lines.empty()) {
    ORBIT_ERROR("Empty \"%s\" file", stat.string());
    return std::nullopt;
  }
  std::string first_line = lines.at(0);

  // Remove fields up to comm (process name) as this, enclosed in parentheses, could contain spaces.
  size_t last_closed_paren_index = first_line.find_last_of(')');
  if (last_closed_paren_index == std::string::npos) {
    return std::nullopt;
  }
  std::string_view first_line_excl_pid_comm =
      std::string_view{first_line}.substr(last_closed_paren_index + 1);

  std::vector<std::string_view> fields_excl_pid_comm =
      absl::StrSplit(first_line_excl_pid_comm, ' ', absl::SkipWhitespace{});

  constexpr size_t kCommIndex = 1;
  constexpr size_t kStateIndex = 2;
  constexpr size_t kStateIndexExclPidComm = kStateIndex - kCommIndex - 1;
  if (fields_excl_pid_comm.size() <= kStateIndexExclPidComm) {
    return std::nullopt;
  }
  return fields_excl_pid_comm[kStateIndexExclPidComm][0];
}

int GetNumCores() {
  int hw_conc = static_cast<int>(std::thread::hardware_concurrency());
  // Some compilers do not support std::thread::hardware_concurrency().
  if (hw_conc != 0) {
    return hw_conc;
  }

  std::optional<std::string> nproc_str = orbit_base::ExecuteCommand("nproc");
  if (nproc_str.has_value() && !nproc_str.value().empty()) {
    return std::stoi(nproc_str.value());
  }

  return 1;
}

// Read /proc/<pid>/cgroup.
static ErrorMessageOr<std::string> ReadCgroupContent(pid_t pid) {
  std::string cgroup_filename = absl::StrFormat("/proc/%d/cgroup", pid);
  return orbit_base::ReadFileToString(cgroup_filename);
}

// Extract the cpuset entry from the content of /proc/<pid>/cgroup.
std::optional<std::string> ExtractCpusetFromCgroup(std::string_view cgroup_content) {
  for (std::string_view cgroup_line : absl::StrSplit(cgroup_content, '\n')) {
    if (absl::StrContains(cgroup_line, "cpuset:") || absl::StrContains(cgroup_line, "cpuset,")) {
      // For example "8:cpuset:/" or "8:cpuset:/game", but potentially also
      // "5:cpuacct,cpu,cpuset:/daemons".
      return std::string{cgroup_line.substr(cgroup_line.find_last_of(':') + 1)};
    }
  }

  return std::nullopt;
}

// Read /sys/fs/cgroup/cpuset/<cgroup>/cpuset.cpus.
static ErrorMessageOr<std::string> ReadCpusetCpusContent(std::string_view cgroup_cpuset) {
  std::string cpuset_cpus_filename = absl::StrFormat("/sys/fs/cgroup/cpuset%s/cpuset.cpus",
                                                     cgroup_cpuset == "/" ? "" : cgroup_cpuset);
  return orbit_base::ReadFileToString(cpuset_cpus_filename);
}

std::vector<int> ParseCpusetCpus(std::string_view cpuset_cpus_content) {
  std::vector<int> cpuset_cpus{};
  // Example of format: "0-2,7,12-14".
  for (const auto& range : absl::StrSplit(cpuset_cpus_content, ',', absl::SkipEmpty())) {
    std::vector<std::string> values = absl::StrSplit(range, '-');
    if (values.size() == 1) {
      int cpu = std::stoi(values[0]);
      cpuset_cpus.push_back(cpu);
    } else if (values.size() == 2) {
      for (int cpu = std::stoi(values[0]); cpu <= std::stoi(values[1]); ++cpu) {
        cpuset_cpus.push_back(cpu);
      }
    }
  }
  return cpuset_cpus;
}

// Read and parse /sys/fs/cgroup/cpuset/<cgroup_cpuset>/cpuset.cpus for the
// cgroup cpuset of the process with this pid.
// An empty result indicates an error, as trying to start a process with an
// empty cpuset fails with message "cgroup change of group failed".
std::vector<int> GetCpusetCpus(pid_t pid) {
  ErrorMessageOr<std::string> cgroup_content_or_error = ReadCgroupContent(pid);
  if (cgroup_content_or_error.has_error()) {
    return {};
  }

  // For example "/" or "/game".
  std::optional<std::string> cgroup_cpuset_opt =
      ExtractCpusetFromCgroup(cgroup_content_or_error.value());
  if (!cgroup_cpuset_opt.has_value()) {
    return {};
  }

  // For example "0-2,7,12-14".
  ErrorMessageOr<std::string> cpuset_cpus_content_or_error =
      ReadCpusetCpusContent(cgroup_cpuset_opt.value());
  if (cpuset_cpus_content_or_error.has_error()) {
    return {};
  }

  return ParseCpusetCpus(cpuset_cpus_content_or_error.value());
}

int GetTracepointId(const char* tracepoint_category, const char* tracepoint_name) {
  std::string filename = absl::StrFormat("/sys/kernel/debug/tracing/events/%s/%s/id",
                                         tracepoint_category, tracepoint_name);

  ErrorMessageOr<std::string> file_content_or_error = orbit_base::ReadFileToString(filename);
  if (file_content_or_error.has_error()) {
    ORBIT_ERROR("Reading tracepoint id of %s:%s: %s", tracepoint_category, tracepoint_name,
                file_content_or_error.error().message());
    return -1;
  }

  int tp_id = -1;
  if (!absl::SimpleAtoi(file_content_or_error.value(), &tp_id)) {
    ORBIT_ERROR("Parsing tracepoint id for: %s:%s", tracepoint_category, tracepoint_name);
    return -1;
  }
  return tp_id;
}

uint64_t GetMaxOpenFilesHardLimit() {
  rlimit limit{};
  int ret = getrlimit(RLIMIT_NOFILE, &limit);
  if (ret != 0) {
    ORBIT_ERROR("getrlimit: %s", SafeStrerror(errno));
    return 0;
  }
  return limit.rlim_max;
}

bool SetMaxOpenFilesSoftLimit(uint64_t soft_limit) {
  uint64_t hard_limit = GetMaxOpenFilesHardLimit();
  if (hard_limit == 0) {
    return false;
  }
  rlimit limit{soft_limit, hard_limit};
  int ret = setrlimit(RLIMIT_NOFILE, &limit);
  if (ret != 0) {
    ORBIT_ERROR("setrlimit: %s", SafeStrerror(errno));
    return false;
  }
  return true;
}

// Check that all mappings containing the absolute addresses of the function are file mappings.
// Plural, because we have to consider the possibility that the module may be mapped multiple times,
// and hence that the function may have multiple absolute addresses.
//
// Note: A more naive solution would be to look for a map containing the file offset for the
// function, hence not involving absolute addresses and modules at all. For misaligned PEs, this can
// cause false negatives, because a function can be mapped twice, in a file mapping and in an
// anonymous (executable) mapping, but with the actual absolute address of the function being in the
// anonymous (executable) mapping.
//
// Example: Consider a PE with one section, the .text section, at offset in the file 0x400 and
// relative virtual address 0x1000.
// The maps could look like this:
// 140000000-140001000 r--p 00000000 103:07 6946834    /path/to/pe.exe
// 140001000-140004000 r-xp 00000000 00:00 0
// The first map corresponds to the headers, however, it also covers all functions with offsets in
// the file from 0x400 to 0x1000 (i.e., with RVAs from 0x1000 to 0x1c00). But those functions are
// mapped again in the anonymous map, and that's where they actually have their absolute address,
// i.e., where they actually get executed.
static bool FunctionIsAlwaysInFileMapping(absl::Span<const LinuxMemoryMapping> file_path_maps,
                                          absl::Span<const ModuleInfo> file_path_modules,
                                          const InstrumentedFunction& function) {
  for (const ModuleInfo& module : file_path_modules) {
    ORBIT_CHECK(module.file_path() == function.file_path());
    const uint64_t function_absolute_address =
        orbit_module_utils::SymbolVirtualAddressToAbsoluteAddress(
            function.function_virtual_address(), module.address_start(), module.load_bias(),
            module.executable_segment_offset());
    const std::string& function_file_path = function.file_path();
    if (!std::any_of(
            file_path_maps.begin(), file_path_maps.end(),
            [&function_absolute_address, &function_file_path](const LinuxMemoryMapping& map) {
              ORBIT_CHECK(map.pathname() == function_file_path);
              return map.start_address() <= function_absolute_address &&
                     function_absolute_address < map.end_address();
            })) {
      return false;
    }
  }
  return true;
}

std::map<uint64_t, std::string> FindFunctionsThatUprobesCannotInstrumentWithMessages(
    absl::Span<const LinuxMemoryMapping> maps, absl::Span<const ModuleInfo> modules,
    absl::Span<const InstrumentedFunction> functions) {
  absl::flat_hash_map<std::string, std::vector<LinuxMemoryMapping>> file_paths_to_maps;
  for (const LinuxMemoryMapping& map : maps) {
    if (map.inode() == 0 || map.pathname().empty()) continue;
    file_paths_to_maps[map.pathname()].emplace_back(map);
  }

  absl::flat_hash_map<std::string, std::vector<ModuleInfo>> file_paths_to_modules;
  for (const ModuleInfo& module : modules) {
    file_paths_to_modules[module.file_path()].emplace_back(module);
  }

  constexpr const char* kModuleNotLoadedMessageFormat =
      "Function \"%s\" belongs to module \"%s\", which is not loaded by the process. If the module "
      "gets loaded during the capture, the function will get instrumented automatically.";
  constexpr const char* kFunctionInAnonymousMapMessageFormatGeneric =
      "Function \"%s\" belonging to module \"%s\" is not (always) loaded into a file mapping.";
  constexpr const char* kFunctionInAnonymousMapMessageFormatForPe =
      "Function \"%s\" belonging to module \"%s\" is not (always) loaded into a file mapping. The "
      "module is a PE, so Wine might have loaded its text section into an anonymous mapping "
      "instead.";

  std::map<uint64_t, std::string> function_ids_to_error_messages;
  for (const InstrumentedFunction& function : functions) {
    auto file_path_modules_it = file_paths_to_modules.find(function.file_path());
    if (file_path_modules_it == file_paths_to_modules.end()) {
      // The module of this function is not loaded by the process.
      std::string message = absl::StrFormat(kModuleNotLoadedMessageFormat, function.function_name(),
                                            function.file_path());
      function_ids_to_error_messages.emplace(function.function_id(), std::move(message));
      continue;
    }

    auto file_path_maps_it = file_paths_to_maps.find(function.file_path());
    if (file_path_maps_it == file_paths_to_maps.end()) {
      // The module of this function is not in the maps. Note: this is generally unexpected if the
      // condition above was false, i.e., if we detected that the module is loaded by the process.
      std::string message = absl::StrFormat(kModuleNotLoadedMessageFormat, function.function_name(),
                                            function.file_path());
      function_ids_to_error_messages.emplace(function.function_id(), std::move(message));
      continue;
    }

    const std::vector<LinuxMemoryMapping>& file_path_maps = file_path_maps_it->second;
    const std::vector<ModuleInfo>& file_path_modules = file_path_modules_it->second;

    if (FunctionIsAlwaysInFileMapping(file_path_maps, file_path_modules, function)) {
      // This function is mapped into a file mapping (for each time its module is loaded).
      continue;
    }

    // The module of this function is loaded by the process, but the address of the function itself
    // doesn't appear in any file mapping.
    const bool module_is_pe = std::any_of(
        file_path_modules.begin(), file_path_modules.end(), [](const ModuleInfo& module) {
          return module.object_file_type() == ModuleInfo::kCoffFile;
        });
    // When the module is a PE, the message will contain a note regarding Wine.
    std::string message = module_is_pe
                              ? absl::StrFormat(kFunctionInAnonymousMapMessageFormatForPe,
                                                function.function_name(), function.file_path())
                              : absl::StrFormat(kFunctionInAnonymousMapMessageFormatGeneric,
                                                function.function_name(), function.file_path());
    function_ids_to_error_messages.emplace(function.function_id(), std::move(message));
  }

  if (!function_ids_to_error_messages.empty()) {
    std::string log_message = "Uprobes likely failed to instrument some functions:\n";
    for (const auto& [unused_function_id, message] : function_ids_to_error_messages) {
      log_message.append("* ").append(message).push_back('\n');
    }
    ORBIT_ERROR("%s", log_message);
  }

  return function_ids_to_error_messages;
}

}  // namespace orbit_linux_tracing
