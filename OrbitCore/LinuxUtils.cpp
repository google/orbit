// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



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
std::vector<std::string> ListModules(pid_t pid) {
  std::vector<std::string> modules;
  // TODO: we should read the file directly instead or memory map it.
  std::string result =
      ExecuteCommand(absl::StrFormat("cat /proc/%d/maps", pid).c_str());

  std::stringstream ss(result);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    modules.push_back(line);
  }

  return modules;
}

//-----------------------------------------------------------------------------
std::string ExecuteCommand(const char* a_Cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);
  if (!pipe) {
    LOG("Could not open pipe");
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
    bool is_executable;
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
    bool is_executable = tokens[1].size() == 4 && tokens[1][2] == 'x';

    auto iter = address_map.find(module_name);
    if (iter == address_map.end()) {
      address_map[module_name] = {start, end, is_executable};
    } else {
      AddressRange& address_range = iter->second;
      address_range.start_address =
          std::min(address_range.start_address, start);
      address_range.end_address = std::max(address_range.end_address, end);
      address_range.is_executable |= is_executable;
    }
  }

  for (const auto& [module_name, address_range] : address_map) {
    // Filter out entries which are not executable
    if (!address_range.is_executable) continue;

    std::shared_ptr<Module> module = std::make_shared<Module>(
        module_name, address_range.start_address, address_range.end_address);

    // This filters out entries which are inaccessible
    if (module->m_PdbSize == 0) continue;

    std::unique_ptr<ElfFile> elf_file = ElfFile::Create(module->m_FullName);
    if (!elf_file) {
      ERROR("Unable to create an elf file for module %s",
            module->m_FullName.c_str());
      continue;
    }

    module_map->insert_or_assign(address_range.start_address, module);

    if (!elf_file->GetBuildId().empty()) {
      module->m_DebugSignature = elf_file->GetBuildId();
    }
  }
}

//-----------------------------------------------------------------------------
std::unordered_map<pid_t, float> GetCpuUtilization() {
  std::unordered_map<pid_t, float> process_map;
  std::string result = ExecuteCommand(
      "top -b -n 1 | sed -n '8, 1000{s/^ *//;s/ *$//;s/  */,/gp;};1000q'");
  std::stringstream ss(result);
  std::string line;

  while (std::getline(ss, line, '\n')) {
    std::vector<std::string> tokens = absl::StrSplit(line, ",");
    if (tokens.size() > 8) {
      pid_t pid = atoi(tokens[0].c_str());
      auto cpu = static_cast<float>(atof(tokens[8].c_str()));
      process_map[pid] = cpu;
    }
  }

  return process_map;
}

//-----------------------------------------------------------------------------
bool Is64Bit(pid_t pid) {
  std::string result =
      ExecuteCommand(absl::StrFormat("file -L /proc/%d/exe", pid).c_str());
  return absl::StrContains(result, "64-bit");
}

}  // namespace LinuxUtils
