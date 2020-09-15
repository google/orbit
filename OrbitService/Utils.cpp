// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Utils.h"

#include <absl/base/casts.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <cxxabi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <memory>
#include <numeric>
#include <string>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "absl/strings/str_format.h"
#include "absl/strings/strip.h"

namespace orbit_service::utils {

namespace fs = std::filesystem;

using ::ElfUtils::ElfFile;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::TracepointInfo;

static const char* kLinuxTracingEvents = "/sys/kernel/debug/tracing/events/";

ErrorMessageOr<uint64_t> FileSize(const std::string& file_path) {
  struct stat stat_buf {};
  int ret = stat(file_path.c_str(), &stat_buf);
  if (ret != 0) {
    return ErrorMessage(absl::StrFormat("Unable to call stat with file \"%s\": %s", file_path,
                                        SafeStrerror(errno)));
  }
  return stat_buf.st_size;
}

ErrorMessageOr<std::vector<ModuleInfo>> ReadModules(int32_t pid) {
  std::filesystem::path proc_maps_path{absl::StrFormat("/proc/%d/maps", pid)};
  OUTCOME_TRY(proc_maps_data, ReadFileToString(proc_maps_path));
  return ParseMaps(proc_maps_data);
}

ErrorMessageOr<std::vector<ModuleInfo>> ParseMaps(std::string_view proc_maps_data) {
  struct AddressRange {
    uint64_t start_address;
    uint64_t end_address;
    bool is_executable;
  };

  const std::vector<std::string> proc_maps = absl::StrSplit(proc_maps_data, '\n');

  std::map<std::string, AddressRange> address_map;
  for (const std::string& line : proc_maps) {
    std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    // tokens[4] is the inode column. If inode equals 0, then the memory is not
    // mapped to a file (might be heap, stack or something else)
    if (tokens.size() != 6 || tokens[4] == "0") continue;

    const std::string& module_path = tokens[5];

    std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;

    uint64_t start = std::stoull(addresses[0], nullptr, 16);
    uint64_t end = std::stoull(addresses[1], nullptr, 16);
    bool is_executable = tokens[1].size() == 4 && tokens[1][2] == 'x';

    auto iter = address_map.find(module_path);
    if (iter == address_map.end()) {
      address_map[module_path] = {start, end, is_executable};
    } else {
      AddressRange& address_range = iter->second;
      address_range.start_address = std::min(address_range.start_address, start);
      address_range.end_address = std::max(address_range.end_address, end);
      address_range.is_executable |= is_executable;
    }
  }

  std::vector<ModuleInfo> result;
  for (const auto& [module_path, address_range] : address_map) {
    // Filter out entries which are not executable
    if (!address_range.is_executable) continue;
    if (!std::filesystem::exists(module_path)) continue;
    ErrorMessageOr<uint64_t> file_size = FileSize(module_path);
    if (!file_size) continue;

    ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file = ElfFile::Create(module_path);
    if (!elf_file) {
      // TODO: Shouldn't this result in ErrorMessage?
      ERROR("Unable to load module \"%s\": %s - will ignore.", module_path,
            elf_file.error().message());
      continue;
    }

    ErrorMessageOr<uint64_t> load_bias = elf_file.value()->GetLoadBias();
    // Every loadable module contains a load bias.
    if (!load_bias) {
      ERROR("No load bias found for module %s", module_path.c_str());
      continue;
    }

    ModuleInfo module_info;
    module_info.set_name(std::filesystem::path{module_path}.filename());
    module_info.set_file_path(module_path);
    module_info.set_file_size(file_size.value());
    module_info.set_address_start(address_range.start_address);
    module_info.set_address_end(address_range.end_address);
    module_info.set_build_id(elf_file.value()->GetBuildId());
    module_info.set_load_bias(load_bias.value());

    result.push_back(module_info);
  }

  return result;
}

ErrorMessageOr<std::vector<orbit_grpc_protos::TracepointInfo>> ReadTracepoints() {
  std::vector<TracepointInfo> result;

  std::error_code error_code_category, error_code_name;
  std::error_code no_err;
  std::string error_category_message, error_name_message;

  for (const auto& category : fs::directory_iterator(kLinuxTracingEvents, error_code_category)) {
    if (fs::is_directory(category)) {
      for (const auto& name : fs::directory_iterator(category, error_code_name)) {
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

static std::optional<pid_t> ProcEntryToPid(const std::filesystem::directory_entry& entry) {
  if (!entry.is_directory()) {
    return std::nullopt;
  }

  int potential_pid;
  if (!absl::SimpleAtoi(entry.path().filename().string(), &potential_pid)) {
    return std::nullopt;
  }

  if (potential_pid <= 0) {
    return std::nullopt;
  }

  return static_cast<pid_t>(potential_pid);
}

std::vector<pid_t> GetAllPids() {
  const auto emplace_back_if_has_value = [](auto vec, const auto& val) {
    if (val.has_value()) {
      vec.emplace_back(std::move(val.value()));
    }

    return std::move(vec);
  };

  fs::directory_iterator proc{"/proc"};
  return std::transform_reduce(begin(proc), end(proc), std::vector<pid_t>{},
                               emplace_back_if_has_value, ProcEntryToPid);
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

  std::vector<std::string_view> fields = absl::StrSplit(first_line, ' ', absl::SkipWhitespace{});

  constexpr size_t kUtimeIndex = 13;
  constexpr size_t kStimeIndex = 14;

  if (fields.size() <= std::max(kUtimeIndex, kStimeIndex)) {
    return {};
  }

  size_t utime{};
  if (!absl::SimpleAtoi(fields[kUtimeIndex], &utime)) {
    return {};
  }

  size_t stime{};
  if (!absl::SimpleAtoi(fields[kStimeIndex], &stime)) {
    return {};
  }

  return Jiffies{utime + stime};
}

std::optional<Jiffies> GetCumulativeTotalCpuTime() {
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
  std::string current_line;
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

  return Jiffies{std::accumulate(splits.begin() + 1, splits.end(), 0ul,
                                 [](auto sum, const auto& str) {
                                   int potential_time = 0;
                                   if (absl::SimpleAtoi(str, &potential_time)) {
                                     sum += potential_time;
                                   }

                                   return sum;
                                 }) /
                 cpus};
}

ErrorMessageOr<Path> GetExecutablePath(int32_t pid) {
  char buffer[PATH_MAX];

  ssize_t length = readlink(absl::StrFormat("/proc/%d/exe", pid).c_str(), buffer, sizeof(buffer));
  if (length == -1) {
    return ErrorMessage(absl::StrFormat("Unable to get executable path of process with pid %d: %s",
                                        pid, SafeStrerror(errno)));
  }

  return Path{std::string(buffer, length)};
}

ErrorMessageOr<std::string> ReadFileToString(const std::filesystem::path& file_name) {
  std::ifstream file_stream(file_name);
  if (file_stream.fail()) {
    return ErrorMessage(
        absl::StrFormat("Unable to read file %s: %s", file_name, SafeStrerror(errno)));
  }
  return std::string{std::istreambuf_iterator<char>{file_stream}, std::istreambuf_iterator<char>{}};
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
