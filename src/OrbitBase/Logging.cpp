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
#include <errno.h>

#include <array>
#include <cstdio>
#include <type_traits>
#include <vector>

#include "LoggingUtils.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_base {

static absl::Mutex log_file_mutex(absl::kConstInit);
static orbit_base::unique_resource log_file{static_cast<std::FILE*>(nullptr), [](std::FILE* f) {
                                              if (f != nullptr) std::fclose(f);
                                            }};
std::string GetLogFileName() {
  std::string timestamp_string = absl::FormatTime(orbit_base_internal::kLogFileNameTimeFormat,
                                                  absl::Now(), absl::UTCTimeZone());
  return absl::StrFormat(orbit_base_internal::kLogFileNameDelimiter, timestamp_string,
                         orbit_base::GetCurrentProcessId());
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
  if (log_file.get() != nullptr) {
    ORBIT_INTERNAL_PLATFORM_ABORT();
  }

  // O_WRONLY, O_CLOEXEC for glibc, O_BINARY for windows
#if defined(_WIN32)
  log_file.reset(std::fopen(path.string().c_str(), "wb"));
#else
  log_file.reset(std::fopen(path.string().c_str(), "wbe"));
#endif

  if (log_file.get() == nullptr) {
    // Log a error (to stderr)
    std::fprintf(stderr, "Error: Unable to open logfile \"%s\": %s\n", path.string().c_str(),
                 SafeStrerror(errno));
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
    ORBIT_LOG("  %p: %s\n", raw_stack[i], symbol);
  }
}

}  // namespace orbit_base

namespace orbit_base_internal {

void LogToFile(std::string_view message) {
  absl::MutexLock lock(&orbit_base::log_file_mutex);
  if (orbit_base::log_file.get() != nullptr) {
    // Ignore any errors that can happen, we cannot do anything about them at this point anyways.
    std::fwrite(message.data(), message.size(), 1, orbit_base::log_file.get());
    std::fflush(orbit_base::log_file.get());
  }
}

}  // namespace orbit_base_internal
