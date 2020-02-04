//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "LinuxUtils.h"

#include <asm/unistd.h>
#include <cxxabi.h>
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

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "absl/strings/str_format.h"

#include "Callstack.h"
#include "Capture.h"
#include "ConnectionManager.h"
#include "EventBuffer.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Path.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "TcpClient.h"
#include "Utils.h"

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
std::vector<pid_t> ListThreads(pid_t a_PID) {
  std::vector<pid_t> threads;
  std::string result =
      ExecuteCommand(absl::StrFormat("ls /proc/%u/task", a_PID).c_str());

  std::stringstream ss(result);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    threads.push_back(std::stol(line));
  }

  return threads;
}

//-----------------------------------------------------------------------------
std::string ReadMaps(pid_t a_PID) {
  std::ifstream maps_file{"/proc/" + std::to_string(a_PID) + "/maps"};
  if (!maps_file) {
    return "";
  }

  std::string maps_buffer;
  std::string maps_line;
  while (std::getline(maps_file, maps_line)) {
    maps_buffer.append(maps_line).append("\n");
  }
  return maps_buffer;
}

//-----------------------------------------------------------------------------
uint64_t GetTracePointID(const char* a_Group, const char* a_Event) {
  std::string cmd =
      absl::StrFormat("cat /sys/kernel/debug/tracing/events/%s/%s/id", a_Group, a_Event);
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
void ListModules(pid_t a_PID,
                 std::map<uint64_t, std::shared_ptr<Module>>& o_ModuleMap) {
  std::map<std::string, std::shared_ptr<Module>> modules;
  std::vector<std::string> result = ListModules(a_PID);
  for (const std::string& line : result) {
    std::vector<std::string> tokens = Tokenize(line);
    if (tokens.size() == 6) {
      std::string& moduleName = tokens[5];

      auto addresses = Tokenize(tokens[0], "-");
      if (addresses.size() == 2) {
        auto iter = modules.find(moduleName);
        std::shared_ptr<Module> module =
            (iter == modules.end()) ? std::make_shared<Module>() : iter->second;

        uint64_t start = std::stoull(addresses[0], nullptr, 16);
        uint64_t end = std::stoull(addresses[1], nullptr, 16);

        if (module->m_AddressStart == 0 || start < module->m_AddressStart)
          module->m_AddressStart = start;
        if (end > module->m_AddressEnd) module->m_AddressEnd = end;

        module->m_FullName = moduleName;
        module->m_Name = ws2s(Path::GetFileName(s2ws(module->m_FullName)));
        module->m_Directory =
            ws2s(Path::GetDirectory(s2ws(module->m_FullName)));
        module->m_PdbSize = Path::FileSize(moduleName);
        auto prettyName = module->GetPrettyName();
        modules[moduleName] = module;
      }
    }
  }

  for (auto& iter : modules) {
    auto module = iter.second;
    o_ModuleMap[module->m_AddressStart] = module;
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
      float cpu = atof(tokens[8].c_str());
      processMap[pid] = cpu;
    }
  }

  return processMap;
}

//-----------------------------------------------------------------------------
bool Is64Bit(pid_t a_PID) {
  std::string result =
      ExecuteCommand(absl::StrFormat("file -L /proc/%u/exe", a_PID).c_str());
  return Contains(result, "64-bit");
}

//-----------------------------------------------------------------------------
std::string Demangle(const char* name) {
  int status = 0;
  std::unique_ptr<char, void (*)(void*)> res{
      abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

  return (status == 0) ? res.get() : name;
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
  PRINT("Error: GetVersion: Invalid argument\n");
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

}  // namespace LinuxUtils
