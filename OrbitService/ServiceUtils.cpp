// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ServiceUtils.h"

#include <absl/base/casts.h>
#include <absl/base/macros.h>
#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <sys/uio.h>

#include <algorithm>
#include <filesystem>
#include <iosfwd>
#include <map>
#include <memory>
#include <numeric>
#include <outcome.hpp>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "absl/strings/str_format.h"
#include "module.pb.h"

namespace orbit_service::utils {

namespace fs = std::filesystem;

using ::orbit_elf_utils::ElfFile;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::TracepointInfo;

static const char* kLinuxTracingEvents = "/sys/kernel/debug/tracing/events/";

ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> ReadTracepoints() {
  std::vector<TracepointInfo> result;

  std::error_code error_code_category, error_code_name;
  std::error_code no_err;
  std::string error_category_message, error_name_message;

  for (const auto& category : fs::directory_iterator(kLinuxTracingEvents, error_code_category)) {
    if (fs::is_directory(category)) {
      for (const auto& name : fs::directory_iterator(category, error_code_name)) {
        if (fs::path(name).filename() == "enable" || fs::path(name).filename() == "filter") {
          continue;
        }
        TracepointInfo tracepoint_info;
        tracepoint_info.set_name(fs::path(name).filename());
        tracepoint_info.set_category(fs::path(category).filename());
        result.emplace_back(tracepoint_info);
      }
    }
  }

  error_category_message = error_code_category.message();
  error_name_message = error_code_name.message();
  if (error_category_message.compare(no_err.message()) != 0) {
    return ErrorMessage(error_category_message.c_str());
  }
  if (error_name_message.compare(no_err.message()) != 0) {
    return ErrorMessage(error_name_message.c_str());
  }

  return result;
}

std::optional<Jiffies> GetCumulativeCpuTimeFromProcess(pid_t pid) {
  const auto stat = std::filesystem::path{"/proc"} / std::to_string(pid) / "stat";

  // /proc/[pid]/stat looks like so (example - all in one line):
  // 1395261 (sleep) S 5273 1160 1160 0 -1 1077936128 101 0 0 0 0 0 0 0 20 0 1 0 42187401 5431296
  // 131 18446744073709551615 94702955896832 94702955911385 140735167078224 0 0 0 0 0 0 0 0 0 17 10
  // 0 0 0 0 0 94702955928880 94702955930112 94702967197696 140735167083224 140735167083235
  // 140735167083235 140735167086569 0
  //
  // This code reads field 13 (user time) and 14 (kernel time) to determine the process's cpu usage.
  // Older kernels might have less fields than in the example. Over time fields had been added to
  // the end, but field indexes stayed stable.

  if (!std::filesystem::exists(stat)) {
    return {};
  }

  std::ifstream stream{stat.string()};
  if (!stream.good()) {
    LOG("Could not open %s", stat.string());
    return {};
  }

  std::string first_line{};
  std::getline(stream, first_line);

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
  constexpr size_t kUtimeIndex = 13;
  constexpr size_t kUtimeIndexExclPidComm = kUtimeIndex - kCommIndex - 1;
  constexpr size_t kStimeIndex = 14;
  constexpr size_t kStimeIndexExclPidComm = kStimeIndex - kCommIndex - 1;

  if (fields_excl_pid_comm.size() <= std::max(kUtimeIndex, kStimeIndex)) {
    return {};
  }

  size_t utime{};
  if (!absl::SimpleAtoi(fields_excl_pid_comm[kUtimeIndexExclPidComm], &utime)) {
    return {};
  }

  size_t stime{};
  if (!absl::SimpleAtoi(fields_excl_pid_comm[kStimeIndexExclPidComm], &stime)) {
    return {};
  }

  return Jiffies{utime + stime};
}

std::optional<TotalCpuTime> GetCumulativeTotalCpuTime() {
  std::ifstream stat_stream{"/proc/stat"};

  // /proc/stat looks like so (example):
  // cpu  2939645 2177780 3213131 495750308 128031 0 469660 0 0 0
  // cpu0 238392 136574 241698 41376123 10562 0 285529 0 0 0
  // cpu1 250552 255075 339032 41161047 10580 0 74924 0 0 0
  // cpu2 259751 189964 284201 41275484 10515 0 25803 0 0 0
  // cpu3 262709 274244 360158 41021080 11391 0 49734 0 0 0
  // cpu4 259346 334285 391229 41021734 10923 0 6862 0 0 0
  // cpu5 257605 236852 317990 41186809 11006 0 4687 0 0 0
  // cpu6 244450 197610 258522 41315244 10772 0 3679 0 0 0
  // cpu7 239533 118254 209752 41453567 10417 0 3216 0 0 0
  // cpu8 228352 104140 203956 41495612 9605 0 2898 0 0 0
  // cpu9 231930 91346 199315 41507207 10363 0 2620 0 0 0
  // cpu10 231707 130839 201517 41467968 10920 0 2616 0 0 0
  // cpu11 235314 108593 205757 41468427 10972 0 7087 0 0 0
  // intr 1137887578 7 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ...
  // ctxt 2193055270
  // btime 1599751494
  // processes 1402492
  // procs_running 3
  // procs_blocked 0
  // softirq 786377709 150 321427815 783165 48655285 46 0 1068082 323211116 5742 91226308
  //
  // This code reads the first line to determine the overall amount of Jiffies that have been
  // counted. It also reads the lines beginning with "cpu*" to determine the number of logical CPUs
  // in the system.

  std::string first_line;
  std::getline(stat_stream, first_line);

  if (!absl::StartsWith(first_line, "cpu ")) {
    return {};
  }

  // This is counting the number of CPUs
  size_t cpus = 0;
  while (true) {
    std::string current_line;
    std::getline(stat_stream, current_line);
    if (!absl::StartsWith(current_line, "cpu")) {
      break;
    }
    cpus++;
  }

  if (cpus == 0) {
    return {};
  }

  std::vector<std::string_view> splits = absl::StrSplit(first_line, ' ', absl::SkipWhitespace{});

  const Jiffies jiffies{
      std::accumulate(splits.begin() + 1, splits.end(), 0ul, [](auto sum, const auto& str) {
        int potential_time = 0;
        if (absl::SimpleAtoi(str, &potential_time)) {
          sum += potential_time;
        }

        return sum;
      })};

  return TotalCpuTime{jiffies, cpus};
}

ErrorMessageOr<Path> FindSymbolsFilePath(const Path& module_path,
                                         const std::vector<Path>& search_directories) {
  OUTCOME_TRY(module_elf_file, ElfFile::Create(module_path.string()));
  if (module_elf_file->HasSymtab()) {
    return module_path;
  }

  if (module_elf_file->GetBuildId().empty()) {
    return ErrorMessage(absl::StrFormat(
        "Unable to find symbols for module \"%s\". Module does not contain a build id",
        module_path));
  }

  const Path& filename = module_path.filename();
  Path filename_dot_debug = filename;
  filename_dot_debug.replace_extension(".debug");
  Path filename_plus_debug = filename;
  filename_plus_debug.replace_extension(filename.extension().string() + ".debug");

  std::set<Path> search_paths;
  for (const auto& directory : search_directories) {
    search_paths.insert(directory / filename_dot_debug);
    search_paths.insert(directory / filename_plus_debug);
    search_paths.insert(directory / filename);
  }

  std::vector<std::string> error_messages;

  for (const auto& symbols_path : search_paths) {
    if (!std::filesystem::exists(symbols_path)) continue;

    ErrorMessageOr<std::unique_ptr<ElfFile>> symbols_file = ElfFile::Create(symbols_path.string());
    if (!symbols_file) {
      std::string error_message =
          absl::StrFormat("Potential symbols file \"%s\" cannot be read as an elf file: %s",
                          symbols_path, symbols_file.error().message());
      LOG("%s", error_message);
      error_messages.emplace_back("* " + std::move(error_message));
      continue;
    }
    if (!symbols_file.value()->HasSymtab()) {
      std::string error_message =
          absl::StrFormat("Potential symbols file \"%s\" does not contain symbols.", symbols_path);
      LOG("%s (It does not contain a .symtab section)", error_message);
      error_messages.emplace_back("* " + std::move(error_message));
      continue;
    }
    if (symbols_file.value()->GetBuildId().empty()) {
      std::string error_message =
          absl::StrFormat("Potential symbols file \"%s\" does not have a build id", symbols_path);
      LOG("%s", error_message);
      error_messages.emplace_back("* " + std::move(error_message));
      continue;
    }
    const std::string& build_id = symbols_file.value()->GetBuildId();
    if (build_id != module_elf_file->GetBuildId()) {
      std::string error_message = absl::StrFormat(
          "Potential symbols file \"%s\" has a different build id than the module requested by the "
          "client. \"%s\" != \"%s\"",
          symbols_path, build_id, module_elf_file->GetBuildId());
      LOG("%s", error_message);
      error_messages.emplace_back("* " + std::move(error_message));
      continue;
    }

    return symbols_path;
  }

  std::string error_message_for_client{absl::StrFormat(
      "Unable to find debug symbols on the instance for module \"%s\". ", module_path)};
  if (!error_messages.empty()) {
    absl::StrAppend(&error_message_for_client, "\nDetails:\n", absl::StrJoin(error_messages, "\n"));
  }
  return ErrorMessage(error_message_for_client);
}

bool ReadProcessMemory(int32_t pid, uintptr_t address, void* buffer, uint64_t size,
                       uint64_t* num_bytes_read) {
  iovec local_iov[] = {{buffer, size}};
  iovec remote_iov[] = {{absl::bit_cast<void*>(address), size}};
  *num_bytes_read = process_vm_readv(pid, local_iov, ABSL_ARRAYSIZE(local_iov), remote_iov,
                                     ABSL_ARRAYSIZE(remote_iov), 0);
  return *num_bytes_read == size;
}

}  // namespace orbit_service::utils
