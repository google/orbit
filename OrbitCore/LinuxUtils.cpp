//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "LinuxUtils.h"

#include <absl/strings/str_split.h>
#include <asm/unistd.h>
#include <cxxabi.h>
#include <dirent.h>
#include <linux/perf_event.h>
#include <linux/types.h>
#include <linux/version.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "Callstack.h"
#include "Capture.h"
#include "ConnectionManager.h"
#include "ElfFile.h"
#include "EventBuffer.h"
#include "OrbitBase/Logging.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Path.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "TcpClient.h"
#include "Utils.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/strings/strip.h"

namespace LinuxUtils {

//-----------------------------------------------------------------------------
std::vector<std::string> ListModules(pid_t a_PID) {
  std::vector<std::string> modules;
  // TODO: we should read the file directly instead or memory map it.
  std::string result =
      ExecuteCommand(absl::StrFormat("cat /proc/%u/maps", a_PID).c_str());

  std::stringstream ss(result);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    modules.push_back(line);
  }

  return modules;
}

//-----------------------------------------------------------------------------
uint64_t GetTracePointID(const char* a_Group, const char* a_Event) {
  std::string cmd = absl::StrFormat(
      "cat /sys/kernel/debug/tracing/events/%s/%s/id", a_Group, a_Event);
  std::string result = ExecuteCommand(cmd.c_str());
  trim(result);
  return std::stoull(result);
}

//-----------------------------------------------------------------------------
std::string ExecuteCommand(const char* a_Cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);
  if (!pipe) {
    std::cout << "Could not open pipe" << std::endl;
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

//-----------------------------------------------------------------------------
void ListModules(pid_t pid,
                 std::map<uint64_t, std::shared_ptr<Module>>* module_map) {
  struct AddressRange {
    uint64_t start_address;
    uint64_t end_address;
  };

  std::map<std::string, AddressRange> address_map;
  std::vector<std::string> result = ListModules(pid);

  for (const std::string& line : result) {
    std::vector<std::string> tokens =
        absl::StrSplit(line, " ", absl::SkipEmpty());

    // tokens[4] is the inode column. If inode equals 0, then the memory is not
    // mapped to a file (might be heap, stack or something else)
    if (tokens.size() != 6 || tokens[4] == "0") continue;

    const std::string& module_name = tokens[5];

    std::vector<std::string> addresses = absl::StrSplit(tokens[0], "-");
    if (addresses.size() != 2) continue;

    uint64_t start = std::stoull(addresses[0], nullptr, 16);
    uint64_t end = std::stoull(addresses[1], nullptr, 16);

    auto iter = address_map.find(module_name);
    if (iter == address_map.end()) {
      address_map[module_name] = {start, end};
    } else {
      AddressRange& address_range = iter->second;
      address_range.start_address =
          std::min(address_range.start_address, start);
      address_range.end_address = std::max(address_range.end_address, end);
    }
  }

  for (const auto& [module_name, address_range] : address_map) {
    module_map->insert_or_assign(
        address_range.start_address,
        std::make_shared<Module>(module_name, address_range.start_address,
                                 address_range.end_address));

    std::shared_ptr<Module> module = (*module_map)[address_range.start_address];

    // This filters out entries which are inaccessible
    if (module->m_PdbSize == 0) continue;

    std::unique_ptr<ElfFile> elf_file = ElfFile::Create(module->m_FullName);
    if (!elf_file) {
      ERROR("Unable to create an elf file for module %s",
            module->m_FullName.c_str());
      continue;
    }

    if (elf_file->GetBuildId().empty()) continue;

    module->m_DebugSignature = elf_file->GetBuildId();
  }
}

//-----------------------------------------------------------------------------
uint32_t GetPID(const char* a_Name) {
  std::string command = absl::StrFormat("ps -A | grep %s", a_Name);
  std::string result = ExecuteCommand(command.c_str());
  auto tokens = Tokenize(result);
  if (!tokens.empty()) return (uint32_t)atoi(tokens[0].c_str());
  std::cout << "Could not find process " << a_Name;
  return 0;
}

//-----------------------------------------------------------------------------
std::unordered_map<uint32_t, float> GetCpuUtilization() {
  std::unordered_map<uint32_t, float> processMap;
  std::string result = ExecuteCommand(
      "top -b -n 1 | sed -n '8, 1000{s/^ *//;s/ *$//;s/  */,/gp;};1000q'");
  std::stringstream ss(result);
  std::string line;

  while (std::getline(ss, line, '\n')) {
    auto tokens = Tokenize(line, ",");
    if (tokens.size() > 8) {
      uint32_t pid = atoi(tokens[0].c_str());
      const auto cpu = static_cast<float>(atof(tokens[8].c_str()));
      processMap[pid] = cpu;
    }
  }

  return processMap;
}

//-----------------------------------------------------------------------------
bool Is64Bit(pid_t a_PID) {
  std::string result =
      ExecuteCommand(absl::StrFormat("file -L /proc/%u/exe", a_PID).c_str());
  return absl::StrContains(result, "64-bit");
}

//-----------------------------------------------------------------------------
double GetSecondsFromNanos(uint64_t a_Nanos) {
  return 0.000000001 * (double)a_Nanos;
}

//-----------------------------------------------------------------------------
void DumpClocks() {
  uint64_t realTime = OrbitTicks(CLOCK_REALTIME);
  uint64_t monotonic = OrbitTicks(CLOCK_MONOTONIC);
  uint64_t monotonicRaw = OrbitTicks(CLOCK_MONOTONIC_RAW);
  uint64_t bootTime = OrbitTicks(CLOCK_BOOTTIME);
  uint64_t tai = OrbitTicks(CLOCK_TAI);

  printf("    realTime: %lu \n", realTime);
  printf("   monotonic: %lu \n", monotonic);
  printf("monotonicRaw: %lu \n", monotonicRaw);
  printf("    bootTime: %lu \n", bootTime);
  printf("         tai: %lu \n\n", tai);

  printf("    realTime: %f \n", GetSecondsFromNanos(realTime));
  printf("   monotonic: %f \n", GetSecondsFromNanos(monotonic));
  printf("monotonicRaw: %f \n", GetSecondsFromNanos(monotonicRaw));
  printf("    bootTime: %f \n", GetSecondsFromNanos(bootTime));
  printf("         tai: %f \n\n", GetSecondsFromNanos(tai));
}

//-----------------------------------------------------------------------------
void StreamCommandOutput(const char* a_Cmd,
                         std::function<void(const std::string&)> a_Callback,
                         bool* a_ExitRequested) {
  std::cout << "Starting output stream for command" << a_Cmd << std::endl;

  // TODO: Decrease the buffer to 128 again after we found a solution for
  // bpftrace.
  std::array<char, 2048> buffer;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);

  if (!pipe) {
    std::cout << "Could not open pipe" << std::endl;
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
         !(*a_ExitRequested)) {
    a_Callback(buffer.data());
  }

  std::cout << "end stream" << std::endl;
}

//-----------------------------------------------------------------------------
std::string GetKernelVersionStr() {
  struct utsname buffer;
  if (uname(&buffer) != 0) {
    return "unknown version";
  }

  std::string version = buffer.release;
  version = version.substr(0, version.find("-"));
  return version;
}

//-----------------------------------------------------------------------------
uint32_t GetVersion(const std::string& a_Version) {
  std::vector<std::string> v = Tokenize(a_Version, ".");
  if (v.size() == 3)
    return KERNEL_VERSION(std::stoi(v[0]), std::stoi(v[1]), std::stoi(v[2]));
  LOG("Error: GetVersion: Invalid argument");
  return 0;
}

//-----------------------------------------------------------------------------
uint32_t GetKernelVersion() {
  static uint32_t version = GetVersion(GetKernelVersionStr());
  return version;
}

//-----------------------------------------------------------------------------
bool IsKernelOlderThan(const char* a_Version) {
  return GetKernelVersion() < GetVersion(a_Version);
}

//-----------------------------------------------------------------------------
std::string GetProcessDir(pid_t process_id) {
  return "/proc/" + std::to_string(process_id) + "/";
}

//-----------------------------------------------------------------------------
std::map<uint32_t, std::string> GetThreadNames(pid_t process_id) {
  // We use std::map rather the flat_hash_map to allow "cereal" serialization
  // and to get sorted pairs.
  std::map<uint32_t, std::string> thread_ids_to_name;
  std::string threads_dir = GetProcessDir(process_id) + "task/";
  struct dirent* dir_entry = nullptr;
  DIR* dir = nullptr;

  dir = opendir(threads_dir.c_str());
  if (dir == nullptr) {
    ERROR("Couldn't open %s\n", threads_dir.c_str());
    return thread_ids_to_name;
  }

  while ((dir_entry = readdir(dir))) {
    if (dir_entry->d_type == DT_DIR && IsAllDigits(dir_entry->d_name)) {
      std::string thread_file = threads_dir + dir_entry->d_name + "/comm";
      std::string thread_name = FileToString(thread_file);
      absl::StripTrailingAsciiWhitespace(&thread_name);  // Remove new-line.
      pid_t tid = 0;
      if (!thread_name.empty() && absl::SimpleAtoi(dir_entry->d_name, &tid)) {
        thread_ids_to_name[tid] = thread_name;
      }
    }
  }

  closedir(dir);
  return thread_ids_to_name;
}

}  // namespace LinuxUtils
