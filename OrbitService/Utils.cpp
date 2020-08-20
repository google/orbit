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
#include <memory>
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

namespace {

const char* kLinuxTracingEvents = "/sys/kernel/debug/tracing/events/";

ErrorMessageOr<uint64_t> FileSize(const std::string& file_path) {
  struct stat stat_buf {};
  int ret = stat(file_path.c_str(), &stat_buf);
  if (ret != 0) {
    return ErrorMessage(absl::StrFormat("Unable to call stat with file \"%s\": %s", file_path,
                                        SafeStrerror(errno)));
  }
  return stat_buf.st_size;
}

ErrorMessageOr<std::string> ExecuteCommand(const std::string& cmd) {
  // TODO (antonrohr): check exit code of executed cmd. If exit code is not 0,
  //  return failure
  std::array<char, 128> buffer{};
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) {
    ERROR("Failed to execute command \"%s\": %s", cmd.c_str(), SafeStrerror(errno));
    return ErrorMessage(
        absl::StrFormat("Failed to execute command \"%s\": %s", cmd, SafeStrerror(errno)));
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}
}  // namespace

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

ErrorMessageOr<std::unordered_map<pid_t, double>> GetCpuUtilization() {
  const std::string cmd = "top -b -n 1 -w512 | sed -n '8, 1000{s/^ *//;s/ *$//;s/  */,/gp;};1000q'";
  OUTCOME_TRY(top_data, ExecuteCommand(cmd));
  return GetCpuUtilization(top_data);
}

ErrorMessageOr<std::unordered_map<pid_t, double>> GetCpuUtilization(std::string_view top_data) {
  std::unordered_map<pid_t, double> process_map;
  for (const auto& line : absl::StrSplit(top_data, '\n')) {
    if (line.empty()) continue;
    std::vector<std::string> tokens = absl::StrSplit(line, ',');
    if (tokens.size() != 12) {
      return ErrorMessage(
          "Unable to determine cpu utilization, wrong format from top "
          "command.");
    }

    pid_t pid = strtol(tokens[0].c_str(), nullptr, 10);
    double cpu = strtod(tokens[8].c_str(), nullptr);
    process_map[pid] = cpu;
  }

  return process_map;
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
