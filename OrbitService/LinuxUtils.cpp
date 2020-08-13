// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxUtils.h"

#include <absl/base/casts.h>
#include <absl/strings/str_split.h>
#include <cxxabi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "absl/strings/str_format.h"
#include "absl/strings/strip.h"

namespace LinuxUtils {

using ::ElfUtils::ElfFile;
using orbit_grpc_protos::ModuleInfo;

namespace {
uint64_t FileSize(const std::string& file) {
  struct stat stat_buf;
  int ret = stat(file.c_str(), &stat_buf);
  return ret == 0 ? stat_buf.st_size : 0;
}
}  // namespace

outcome::result<std::vector<std::string>> ReadProcMaps(pid_t pid) {
  std::filesystem::path maps_path{absl::StrFormat("/proc/%d/maps", pid)};
  OUTCOME_TRY(maps_string, FileToString(maps_path));
  return absl::StrSplit(maps_string, '\n');
}

outcome::result<std::string> ExecuteCommand(const std::string& cmd) {
  // TODO (antonrohr): check exit code of executed cmd. If exit code is not 0,
  //  return failure
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) {
    ERROR("Failed to execute command \"%s\": %s", cmd.c_str(), SafeStrerror(errno));
    return outcome::failure(static_cast<std::errc>(errno));
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return outcome::success(result);
}

ErrorMessageOr<std::vector<ModuleInfo>> ListModules(int32_t pid) {
  struct AddressRange {
    uint64_t start_address;
    uint64_t end_address;
    bool is_executable;
  };

  std::map<std::string, AddressRange> address_map;

  const auto proc_maps = ReadProcMaps(pid);
  if (!proc_maps) {
    return ErrorMessage(
        absl::StrFormat("Unable to read /proc/%d/maps: %s", pid, proc_maps.error().message()));
  }

  for (const std::string& line : proc_maps.value()) {
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
    uint64_t file_size = FileSize(module_path);
    if (file_size == 0) continue;

    ErrorMessageOr<std::unique_ptr<ElfFile>> elf_file = ElfFile::Create(module_path);
    if (!elf_file) {
      // TODO: Shouldn't this result in ErrorMessage?
      ERROR("Unable to load module \"%s\": %s - will ignore.", module_path,
            elf_file.error().message());
      continue;
    }

    ModuleInfo module_info;
    module_info.set_name(std::filesystem::path{module_path}.filename());
    module_info.set_file_path(module_path);
    module_info.set_file_size(file_size);
    module_info.set_address_start(address_range.start_address);
    module_info.set_address_end(address_range.end_address);
    module_info.set_build_id(elf_file.value()->GetBuildId());

    result.push_back(module_info);
  }

  return result;
}

outcome::result<std::unordered_map<pid_t, double>> GetCpuUtilization() {
  const std::string cmd = "top -b -n 1 | sed -n '8, 1000{s/^ *//;s/ *$//;s/  */,/gp;};1000q'";
  OUTCOME_TRY(result, ExecuteCommand(cmd));

  std::unordered_map<pid_t, double> process_map;

  for (const auto& line : absl::StrSplit(result, '\n')) {
    std::vector<std::string> tokens = absl::StrSplit(line, ',');
    if (tokens.size() > 8) {
      pid_t pid = strtol(tokens[0].c_str(), nullptr, 10);
      double cpu = strtod(tokens[8].c_str(), nullptr);
      process_map[pid] = cpu;
    }
  }

  return process_map;
}

outcome::result<bool> Is64Bit(pid_t pid) {
  // TODO(161196904): Do this in a more reliable way. It does not work for a lot
  //  of processes.
  OUTCOME_TRY(result, ExecuteCommand(absl::StrFormat("file -L /proc/%d/exe", pid)));
  return absl::StrContains(result, "64-bit");
}

ErrorMessageOr<std::string> GetExecutablePath(int32_t pid) {
  char buffer[PATH_MAX];

  ssize_t length = readlink(absl::StrFormat("/proc/%d/exe", pid).c_str(), buffer, sizeof(buffer));
  if (length == -1) {
    return ErrorMessage(absl::StrFormat("Unable to get executable path of process with pid %d: %s",
                                        pid, SafeStrerror(errno)));
  }

  return std::string(buffer, length);
}

outcome::result<std::string> FileToString(
    const std::filesystem::path& file_name) {
  std::ifstream file_stream(file_name);
  if (file_stream.fail()) {
    return outcome::failure(static_cast<std::errc>(errno));
  }
  return outcome::success(
      std::string{std::istreambuf_iterator<char>{file_stream},
                  std::istreambuf_iterator<char>{}});
}

bool ReadProcessMemory(int32_t pid, uintptr_t address, void* buffer,
                       uint64_t size, uint64_t* num_bytes_read) {
  iovec local_iov[] = {{buffer, size}};
  iovec remote_iov[] = {{absl::bit_cast<void*>(address), size}};
  *num_bytes_read = process_vm_readv(pid, local_iov, ABSL_ARRAYSIZE(local_iov),
                                     remote_iov, ABSL_ARRAYSIZE(remote_iov), 0);
  return *num_bytes_read == size;
}

}  // namespace LinuxUtils
