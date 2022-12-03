// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProcessService/Process.h"

#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "ProcessServiceUtils.h"

namespace orbit_process_service_internal {

void Process::UpdateCpuUsage(Jiffies process_cpu_time, TotalCpuTime total_cpu_time) {
  const auto diff_process_cpu_time =
      static_cast<double>(process_cpu_time.value - previous_process_cpu_time_.value);
  const auto diff_total_cpu_time =
      static_cast<double>(total_cpu_time.jiffies.value - previous_total_cpu_time_.value);

  // When the counters wrap, `cpu_usage` might be smaller than 0.0 or larger than 1.0,
  // depending on the signedness of `Jiffies`. Reference implementations like top and htop usually
  // clamp in this case. So that's what we're also doing here. Since 100% is usually considered
  // the usage of a single logical core, we multiply by the number of cores (cpus) - just like
  // top and htop do as well.
  const auto cpu_usage =
      std::clamp(diff_process_cpu_time / diff_total_cpu_time, 0.0, 1.0) * total_cpu_time.cpus;

  // TODO(hebecker): Rename cpu_usage to cpu_usage_rate and normalize. Being in percent was
  // surprising
  process_info_.set_cpu_usage(cpu_usage * 100.0);

  previous_process_cpu_time_ = process_cpu_time;
  previous_total_cpu_time_ = total_cpu_time.jiffies;
}

ErrorMessageOr<Process> Process::FromPid(uint32_t pid) {
  const auto path = std::filesystem::path{"/proc"} / std::to_string(pid);

  OUTCOME_TRY(const bool is_directory, orbit_base::IsDirectory(path));
  if (!is_directory) {
    return ErrorMessage{absl::StrFormat("PID %u does not exist", pid)};
  }

  const std::filesystem::path name_file_path = path / "comm";
  auto name_file_result = orbit_base::ReadFileToString(name_file_path);
  if (name_file_result.has_error()) {
    return ErrorMessage{absl::StrFormat("Failed to read %s: %s", name_file_path.string(),
                                        name_file_result.error().message())};
  }

  std::string name = std::move(name_file_result.value());
  // Remove new line character.
  absl::StripTrailingAsciiWhitespace(&name);
  if (name.empty()) {
    return ErrorMessage{absl::StrFormat("Could not determine the process name of process %u", pid)};
  }

  Process process{};
  process.process_info_.set_pid(pid);
  process.process_info_.set_name(name);

  const auto total_cpu_time = orbit_process_service::GetCumulativeTotalCpuTime();
  const auto cpu_time =
      orbit_process_service::GetCumulativeCpuTimeFromProcess(process.process_info().pid());
  if (cpu_time && total_cpu_time) {
    process.UpdateCpuUsage(cpu_time.value(), total_cpu_time.value());
  } else {
    ORBIT_LOG("Could not update the CPU usage of process %u", process.process_info().pid());
  }

  // "The command-line arguments appear [...] as a set of strings
  // separated by null bytes ('\0')".
  const std::filesystem::path cmdline_file_path = path / "cmdline";
  auto cmdline_file_result = orbit_base::ReadFileToString(cmdline_file_path);
  if (cmdline_file_result.has_error()) {
    return ErrorMessage{absl::StrFormat("Failed to read %s: %s", cmdline_file_path.string(),
                                        cmdline_file_result.error().message())};
  }

  std::string cmdline = std::move(cmdline_file_result.value());
  std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
  process.process_info_.set_command_line(cmdline);

  auto file_path_result = orbit_base::GetExecutablePath(pid);
  if (!file_path_result.has_error()) {
    process.process_info_.set_full_path(file_path_result.value());

    const auto& elf_file = orbit_object_utils::CreateElfFile(file_path_result.value());
    if (!elf_file.has_error()) {
      process.process_info_.set_is_64_bit(elf_file.value()->Is64Bit());
      process.process_info_.set_build_id(elf_file.value()->GetBuildId());
    } else {
      ORBIT_LOG("Warning: Unable to parse the executable \"%s\" as elf file. (pid: %u): %s",
                file_path_result.value(), pid, elf_file.error().message());
    }
  }

  return process;
}

}  // namespace orbit_process_service_internal
