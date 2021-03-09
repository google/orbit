// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracingUtils.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <stdlib.h>

#include <string>

#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_memory_tracing {

using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::SystemMemoryUsage;

std::optional<SystemMemoryUsage> ParseMemInfo(const std::string& meminfo_content) {
  if (meminfo_content.empty()) return std::nullopt;

  std::vector<std::string> lines = absl::StrSplit(meminfo_content, '\n');
  if (lines.empty()) return std::nullopt;

  SystemMemoryUsage memory_info;
  memory_info.set_total_kb(kMissingInfo);
  memory_info.set_free_kb(kMissingInfo);
  memory_info.set_available_kb(kMissingInfo);
  memory_info.set_buffers_kb(kMissingInfo);
  memory_info.set_cached_kb(kMissingInfo);

  constexpr size_t kNumLines = 5;
  std::vector<std::string> top_lines(
      lines.begin(), lines.begin() + (lines.size() > kNumLines ? kNumLines : lines.size()));
  for (const std::string& line : top_lines) {
    // Each line of the /proc/meminfo file consists of a parameter name, followed by a colon, the
    // value of the parameter, and an option unit of measurement (e.g., "kB").
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 3) continue;

    // According to the kernel code https://github.com/torvalds/linux/blob/master/fs/proc/meminfo.c,
    // the size unit in the file /proc/meminfo is fixed to "kB", which implies 1024 Bytes. And this
    // is different from the definitions in http://en.wikipedia.org/wiki/Kilobyte. We keep
    // consistent with the definition in /proc/meminfo: we report in "kB" and consider 1 kB = 1
    // KiloBytes = 1024 Bytes.
    CHECK(splits[2] == "kB");

    int64_t memory_size_value;
    if (!absl::SimpleAtoi(splits[1], &memory_size_value)) continue;

    if (splits[0] == "MemTotal:") {
      memory_info.set_total_kb(memory_size_value);
    } else if (splits[0] == "MemFree:") {
      memory_info.set_free_kb(memory_size_value);
    } else if (splits[0] == "MemAvailable:") {
      memory_info.set_available_kb(memory_size_value);
    } else if (splits[0] == "Buffers:") {
      memory_info.set_buffers_kb(memory_size_value);
    } else if (splits[0] == "Cached:") {
      memory_info.set_cached_kb(memory_size_value);
    }
  }

  return memory_info;
}

std::optional<SystemMemoryUsage> GetSystemMemoryUsage() noexcept {
  uint64_t current_timestamp_ns = orbit_base::CaptureTimestampNs();

  ErrorMessageOr<std::string> reading_result = orbit_base::ReadFileToString("/proc/meminfo");
  if (!reading_result.has_value()) {
    ERROR("%s", reading_result.error().message());
    return std::nullopt;
  }

  std::optional<SystemMemoryUsage> parsing_result = ParseMemInfo(reading_result.value());
  if (parsing_result.has_value()) {
    parsing_result.value().set_timestamp_ns(current_timestamp_ns);
  }
  return parsing_result;
}

}  // namespace orbit_memory_tracing