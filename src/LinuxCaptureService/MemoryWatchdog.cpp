// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryWatchdog.h"

#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <unistd.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_linux_capture_service {

std::optional<uint64_t> ExtractMemTotalInKbFromProcMeminfo(std::string_view proc_meminfo) {
  std::string line;
  for (size_t i = 0; i <= proc_meminfo.size(); ++i) {
    if (i == proc_meminfo.size() || proc_meminfo[i] == '\n') {
      if (!absl::StartsWith(line, "MemTotal:")) {
        line.clear();
        continue;
      }

      std::vector<std::string_view> splits = absl::StrSplit(line, ' ', absl::SkipWhitespace{});
      if (splits.size() < 3 || splits[2] != "kB") {
        ERROR("Extracting MemTotal from \"%s\"", line);
        return std::nullopt;
      }

      uint64_t mem_total_kb;
      if (!absl::SimpleAtoi(splits[1], &mem_total_kb)) {
        ERROR("Parsing MemTotal \"%s\"", splits[1]);
        return std::nullopt;
      }

      return mem_total_kb;
    }

    line.push_back(proc_meminfo[i]);
  }

  ERROR("Could not find MemTotal in file");
  return std::nullopt;
}

std::optional<uint64_t> ReadMemTotalInBytesFromProcMeminfo() {
  static constexpr std::string_view kProcMeminfoFilename = "/proc/meminfo";

  ErrorMessageOr<std::string> error_or_meminfo = orbit_base::ReadFileToString(kProcMeminfoFilename);
  if (error_or_meminfo.has_error()) {
    ERROR("Reading \"%s\": %s", kProcMeminfoFilename, error_or_meminfo.error().message());
    return std::nullopt;
  }

  std::optional<uint64_t> mem_total_kb =
      ExtractMemTotalInKbFromProcMeminfo(error_or_meminfo.value());
  if (!mem_total_kb.has_value()) {
    ERROR("Extracting MemTotal from \"%s\"", kProcMeminfoFilename);
    return std::nullopt;
  }

  return mem_total_kb.value() * 1024;
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
    ERROR_ONCE("Parsing rss \"%s\"", rss_string);
    return std::nullopt;
  }
  return rss_pages;
}

std::optional<uint64_t> ReadRssInBytesFromProcPidStat() {
  static const pid_t pid = getpid();
  static const std::string proc_pid_stat_filename = absl::StrFormat("/proc/%d/stat", pid);

  ErrorMessageOr<std::string> error_or_stat = orbit_base::ReadFileToString(proc_pid_stat_filename);
  if (error_or_stat.has_error()) {
    ERROR_ONCE("Reading \"%s\": %s", proc_pid_stat_filename, error_or_stat.error().message());
    return std::nullopt;
  }

  std::optional<uint64_t> rss_pages = ExtractRssInPagesFromProcPidStat(error_or_stat.value());
  if (!rss_pages.has_value()) {
    ERROR_ONCE("Extracting rss from \"%s\"", proc_pid_stat_filename);
    return std::nullopt;
  }

  static uint64_t page_size_bytes = sysconf(_SC_PAGESIZE);
  return rss_pages.value() * page_size_bytes;
}

}  // namespace orbit_linux_capture_service
