// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryWatchdog.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <unistd.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_linux_capture_service {

uint64_t GetPhysicalMemoryInBytes() {
  static uint64_t physical_pages = sysconf(_SC_PHYS_PAGES);
  static uint64_t page_size_bytes = sysconf(_SC_PAGESIZE);
  static uint64_t physical_memory_bytes = physical_pages * page_size_bytes;
  return physical_memory_bytes;
}

std::optional<uint64_t> ExtractRssInPagesFromProcPidStat(std::string_view proc_pid_stat) {
  static constexpr int kRssFieldIndex = 23;
  std::string rss_string;
  int spaces_found = 0;
  for (char c : proc_pid_stat) {
    if (c == ' ') {
      ++spaces_found;
      if (spaces_found > kRssFieldIndex) {
        break;
      }
      continue;
    }
    if (spaces_found == kRssFieldIndex) {
      rss_string.push_back(c);
    }
  }

  uint64_t rss_pages;
  if (!absl::SimpleAtoi(rss_string, &rss_pages)) {
    ORBIT_ERROR_ONCE("Parsing rss \"%s\"", rss_string);
    return std::nullopt;
  }
  return rss_pages;
}

std::optional<uint64_t> ReadRssInBytesFromProcPidStat() {
  static const pid_t pid = getpid();
  static const std::string proc_pid_stat_filename = absl::StrFormat("/proc/%d/stat", pid);

  ErrorMessageOr<std::string> error_or_stat = orbit_base::ReadFileToString(proc_pid_stat_filename);
  if (error_or_stat.has_error()) {
    ORBIT_ERROR_ONCE("Reading \"%s\": %s", proc_pid_stat_filename, error_or_stat.error().message());
    return std::nullopt;
  }

  std::optional<uint64_t> rss_pages = ExtractRssInPagesFromProcPidStat(error_or_stat.value());
  if (!rss_pages.has_value()) {
    ORBIT_ERROR_ONCE("Extracting rss from \"%s\"", proc_pid_stat_filename);
    return std::nullopt;
  }

  static uint64_t page_size_bytes = sysconf(_SC_PAGESIZE);
  return rss_pages.value() * page_size_bytes;
}

}  // namespace orbit_linux_capture_service
