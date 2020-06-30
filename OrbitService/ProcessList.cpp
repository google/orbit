// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessList.h"

#include <absl/strings/ascii.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>

#include <filesystem>
#include <unordered_map>

#include "LinuxUtils.h"
#include "OrbitBase/Logging.h"
#include "Utils.h"

outcome::result<void, std::string> ProcessList::Refresh() {
  std::unordered_map<int32_t, double> cpu_usage_map =
      LinuxUtils::GetCpuUtilization();

  std::vector<ProcessInfo> updated_processes;

  for (const auto& directory_entry :
       std::filesystem::directory_iterator("/proc")) {
    if (!directory_entry.is_directory()) continue;

    std::string path = directory_entry.path().string();
    std::string folder_name = directory_entry.path().filename().string();

    uint32_t pid;
    if (!absl::SimpleAtoi(folder_name, &pid)) continue;

    auto iter = processes_map_.find(pid);
    if (iter != processes_map_.end()) {
      ProcessInfo& process(*(iter->second));
      process.set_cpu_usage(cpu_usage_map[process.pid()]);
      updated_processes.push_back(process);
      continue;
    }

    std::string name = FileToString(path + "/comm");
    // Remove new line character.
    absl::StripTrailingAsciiWhitespace(&name);
    if (name.empty()) continue;

    ProcessInfo process;
    process.set_pid(pid);
    process.set_name(name);
    process.set_cpu_usage(cpu_usage_map[pid]);

    // "The command-line arguments appear [...] as a set of strings
    // separated by null bytes ('\0')".
    std::string cmdline = FileToString(path + "/cmdline");
    process.set_full_path(cmdline.substr(0, cmdline.find('\0')));

    std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
    process.set_command_line(cmdline);

    process.set_is_64_bit(LinuxUtils::Is64Bit(pid));

    updated_processes.push_back(process);
  }

  processes_ = std::move(updated_processes);
  processes_map_.clear();
  for (auto& process : processes_) {
    processes_map_[process.pid()] = &process;
  }

  return outcome::success();
}