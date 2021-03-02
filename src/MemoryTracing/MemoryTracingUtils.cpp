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
  int mem_total, mem_free, mem_available, mem_buffers, mem_cached;
  for (const std::string& line : top_lines) {
    std::vector<std::string> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
    if (splits.size() < 2) continue;
    // TODO: (http://b/181637734) Also parse the unit in splits[2] and update the unit tests.
    if (splits[0] == "MemTotal:" && absl::SimpleAtoi(splits[1], &mem_total)) {
      memory_info.set_total_kb(mem_total);
    } else if (splits[0] == "MemFree:" && absl::SimpleAtoi(splits[1], &mem_free)) {
      memory_info.set_free_kb(mem_free);
    } else if (splits[0] == "MemAvailable:" && absl::SimpleAtoi(splits[1], &mem_available)) {
      memory_info.set_available_kb(mem_available);
    } else if (splits[0] == "Buffers:" && absl::SimpleAtoi(splits[1], &mem_buffers)) {
      memory_info.set_buffers_kb(mem_buffers);
    } else if (splits[0] == "Cached:" && absl::SimpleAtoi(splits[1], &mem_cached)) {
      memory_info.set_cached_kb(mem_cached);
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