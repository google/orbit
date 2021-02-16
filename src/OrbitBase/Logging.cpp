// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Logging.h"

#include <absl/base/const_init.h>
#include <absl/debugging/stacktrace.h>
#include <absl/debugging/symbolize.h>
#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>

#include <array>
#include <system_error>

#include "LoggingUtils.h"
#include "OrbitBase/ThreadUtils.h"

static absl::Mutex log_file_mutex(absl::kConstInit);
std::ofstream log_file;

std::string GetLogFileName() {
  std::string timestamp_string = absl::FormatTime(orbit_base_internal::kLogFileNameTimeFormat,
                                                  absl::Now(), absl::UTCTimeZone());
  return absl::StrFormat(orbit_base_internal::kLogFileNameDelimiter, timestamp_string,
                         static_cast<uint32_t>(orbit_base::GetCurrentProcessId()));
}

ErrorMessageOr<void> TryRemoveOldLogFiles(const std::filesystem::path& log_dir) {
  std::vector<std::filesystem::path> log_file_paths =
      orbit_base_internal::ListFilesRecursivelyIgnoreErrors(log_dir);
  std::vector<std::filesystem::path> old_files =
      orbit_base_internal::FindOldLogFiles(log_file_paths);
  return orbit_base_internal::RemoveFiles(old_files);
}

void InitLogFile(const std::filesystem::path& path) {
  absl::MutexLock lock(&log_file_mutex);
  // Do not call CHECK here - it will end up calling LogToFile,
  // which tries to lock on the same mutex a second time. This will
  // lead to an error since the mutex is not recursive.
  if (log_file.is_open()) {
    PLATFORM_ABORT();
  }
  log_file.open(path, std::ofstream::out);
}

void LogToFile(const std::string& message) {
  absl::MutexLock lock(&log_file_mutex);
  if (log_file.is_open()) {
    log_file << message;
    log_file.flush();
  }
}

void LogStacktrace() {
  constexpr size_t kMaxDepth = 64;
  std::array<void*, kMaxDepth> raw_stack = {};
  const int raw_stack_size = absl::GetStackTrace(&raw_stack[0], kMaxDepth, 1);
  for (int i = 0; i < raw_stack_size; ++i) {
    std::array<char, 1024> buf = {};
    const char* symbol = "(unknown)";
    if (absl::Symbolize(raw_stack[i], buf.data(), buf.size())) {
      symbol = buf.data();
    }
    LOG("  %p: %s\n", raw_stack[i], symbol);
  }
}
