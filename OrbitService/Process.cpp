// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Process.h"

#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>

#include "ElfUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "Utils.h"

namespace orbit_service {

void Process::UpdateCpuUsage(utils::Jiffies process_cpu_time, utils::Jiffies total_cpu_time) {
  const auto diff_process_cpu_time =
      static_cast<double>(process_cpu_time.value - previous_process_cpu_time_.value);
  const auto diff_total_cpu_time =
      static_cast<double>(total_cpu_time.value - previous_total_cpu_time_.value);

  // When the counters wrap, `cpu_usage` might be smaller than 0.0 or larger than 1.0,
  // depending on the signedness of `Jiffies`. Reference implementations like top and htop usually
  // clamp in this case. So that's what we're also doing here.
  const auto cpu_usage = std::clamp(diff_process_cpu_time / diff_total_cpu_time, 0.0, 1.0);

  // TODO(hebecker): Rename cpu_usage to cpu_usage_rate and normalize. Being in percent was
  // surprising
  set_cpu_usage(cpu_usage * 100.0);

  previous_process_cpu_time_ = process_cpu_time;
  previous_total_cpu_time_ = total_cpu_time;
}

ErrorMessageOr<Process> Process::FromPid(pid_t pid) {
  const auto path = std::filesystem::path{"/proc"} / std::to_string(pid);

  if (!std::filesystem::is_directory(path)) {
    return ErrorMessage{absl::StrFormat("PID %d does not exist", pid)};
  }

  const std::filesystem::path name_file_path = path / "comm";
  auto name_file_result = utils::ReadFileToString(name_file_path);
  if (!name_file_result) {
    return ErrorMessage{absl::StrFormat("Failed to read %s: %s", name_file_path.string(),
                                        name_file_result.error().message())};
  }

  std::string name = std::move(name_file_result.value());
  // Remove new line character.
  absl::StripTrailingAsciiWhitespace(&name);
  if (name.empty()) {
    return ErrorMessage{absl::StrFormat("Could not determine the process name of process %d", pid)};
  }

  Process process{};
  process.set_pid(pid);
  process.set_name(name);

  const auto total_cpu_time = utils::GetCumulativeTotalCpuTime();
  const auto cpu_time = utils::GetCumulativeCpuTimeFromProcess(process.pid());
  if (cpu_time && total_cpu_time) {
    process.UpdateCpuUsage(cpu_time.value(), total_cpu_time.value());
  } else {
    LOG("Could not update the CPU usage of process %d", process.pid());
  }

  // "The command-line arguments appear [...] as a set of strings
  // separated by null bytes ('\0')".
  const std::filesystem::path cmdline_file_path = path / "cmdline";
  auto cmdline_file_result = utils::ReadFileToString(cmdline_file_path);
  if (!cmdline_file_result) {
    return ErrorMessage{absl::StrFormat("Failed to read %s: %s", cmdline_file_path.string(),
                                        name_file_result.error().message())};
  }

  std::string cmdline = std::move(cmdline_file_result.value());
  std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
  process.set_command_line(cmdline);

  auto file_path_result = utils::GetExecutablePath(pid);
  if (file_path_result) {
    process.set_full_path(std::move(file_path_result.value()));

    const auto& elf_file = ElfUtils::ElfFile::Create(file_path_result.value().string());
    if (elf_file) {
      process.set_is_64_bit(elf_file.value()->Is64Bit());
    } else {
      LOG("Warning: Unable to parse the executable \"%s\" as elf file. (pid: %d)",
          file_path_result.value(), pid);
    }
  }

  return process;
}

}  // namespace orbit_service
