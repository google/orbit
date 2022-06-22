// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

// clang-format off
#include <libloaderapi.h>
// clang-format on

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/StringConversion.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/ProcessList.h"

namespace orbit_windows_utils {

TEST(ProcessList, ContainsCurrentProcess) {
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  std::vector<const Process*> processes = process_list->GetProcesses();
  EXPECT_NE(processes.size(), 0);

  wchar_t this_exe_file_name[MAX_PATH] = {0};
  GetModuleFileNameW(NULL, this_exe_file_name, MAX_PATH);
  std::string file_name = orbit_base::ToStdString(this_exe_file_name);

  bool found_this_exe = false;
  for (const Process* process : processes) {
    if (process->full_path == file_name) {
      found_this_exe = true;
      break;
    }
  }

  EXPECT_TRUE(found_this_exe);
}

TEST(ProcessList, CpuUsage) {
  std::unique_ptr<ProcessList> process_list = ProcessList::Create();
  std::vector<const Process*> processes = process_list->GetProcesses();
  EXPECT_NE(processes.size(), 0);

  for (const Process* process : processes) {
    EXPECT_EQ(process->cpu_usage_percentage, 0);
  }

  constexpr uint64_t kBusyLoopTimeMs = 200;
  constexpr uint64_t kBusyLoopTimeNs = kBusyLoopTimeMs * 1'000'000;
  uint64_t end_time = orbit_base::CaptureTimestampNs() + kBusyLoopTimeNs;

  // Busy loop.
  while (orbit_base::CaptureTimestampNs() < end_time) {
  }

  uint32_t pid = orbit_base::GetCurrentProcessId();

  // Refresh must be called to generate cpu usage values.
  ErrorMessageOr<void> result = process_list->Refresh();
  EXPECT_FALSE(result.has_error());

  bool found_this_process = false;
  double this_process_cpu_usage = 0;
  for (const Process* process : process_list->GetProcesses()) {
    if (process->pid == pid) {
      found_this_process = true;
      this_process_cpu_usage = process->cpu_usage_percentage;
    }
  }

  EXPECT_EQ(found_this_process, true);

  // For short tests, the interval at which Windows updates process times become an issue.
  // Assuming an interval of 16ms and a test period of 200ms, we have a potential error of 20%.
  // Choose a conservative threshold.
  constexpr double kMinExpectedBusyLoopCpuPercentage = 70.0;
  EXPECT_GT(this_process_cpu_usage, kMinExpectedBusyLoopCpuPercentage);
}

}  // namespace orbit_windows_utils
