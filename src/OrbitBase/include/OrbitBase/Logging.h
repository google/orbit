// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_LOGGING_H_
#define ORBIT_BASE_LOGGING_H_

#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <stdlib.h>

#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>

#include "OrbitBase/Result.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

constexpr const char* kLogTimeFormat = "%Y-%m-%dT%H:%M:%E6S";

#define ORBIT_LOG(format, ...)                                                               \
  do {                                                                                       \
    std::filesystem::path path__ = std::filesystem::path(__FILE__);                          \
    std::string file__;                                                                      \
    std::string dir__;                                                                       \
    auto path_it__ = path__.end();                                                           \
    if (path_it__ != path__.begin()) file__ = (--path_it__)->string();                       \
    if (path_it__ != path__.begin()) dir__ = (--path_it__)->string();                        \
    std::string file_and_line__ = absl::StrFormat("%s/%s:%d", dir__, file__, __LINE__);      \
    std::string time__ = absl::FormatTime(kLogTimeFormat, absl::Now(), absl::UTCTimeZone()); \
    std::string formatted_log__ =                                                            \
        absl::StrFormat("[%s] [%40s] " format "\n", time__, file_and_line__, ##__VA_ARGS__); \
    ORBIT_INTERNAL_PLATFORM_LOG(formatted_log__.c_str());                                    \
  } while (0)

#define ORBIT_ERROR(format, ...) ORBIT_LOG("Error: " format, ##__VA_ARGS__)

#define ORBIT_LOG_ONCE(format, ...)                                                   \
  do {                                                                                \
    static std::once_flag already_logged_flag;                                        \
    std::call_once(already_logged_flag, [&]() { ORBIT_LOG(format, ##__VA_ARGS__); }); \
  } while (0)

#define ORBIT_ERROR_ONCE(format, ...) ORBIT_LOG_ONCE("Error: " format, ##__VA_ARGS__)

#define ORBIT_FATAL(format, ...)                \
  do {                                          \
    ORBIT_LOG("Fatal: " format, ##__VA_ARGS__); \
    ORBIT_INTERNAL_PLATFORM_ABORT();            \
  } while (0)

#define ORBIT_UNREACHABLE() ORBIT_FATAL("Unreachable code")

#if defined(__GNUC__) || defined(__clang__)
#define ORBIT_LIKELY(cond) __builtin_expect(!!(cond), 1)
#define ORBIT_UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#else
#define ORBIT_LIKELY(cond) (!!(cond))
#define ORBIT_UNLIKELY(cond) (!!(cond))
#endif

#define ORBIT_FAIL_IF(condition, format, ...) \
  do {                                        \
    if (ORBIT_UNLIKELY(condition)) {          \
      ORBIT_FATAL(format, ##__VA_ARGS__);     \
    }                                         \
  } while (0)

#define ORBIT_CHECK(assertion)                   \
  do {                                           \
    if (ORBIT_UNLIKELY(!(assertion))) {          \
      ORBIT_LOG("Check failed: %s", #assertion); \
      ORBIT_INTERNAL_PLATFORM_ABORT();           \
    }                                            \
  } while (0)

#ifndef NDEBUG
#define ORBIT_DCHECK(assertion) ORBIT_CHECK(assertion)
#else
#define ORBIT_DCHECK(assertion) \
  do {                          \
  } while (0 && (assertion))
#endif

#define ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT_INDIRECT(a, b) a##b
#define ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT(a, b) \
  ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT_INDIRECT(a, b)

// Declare the class inside the macro so that file:line are the ones where ORBIT_SCOPED_TIMED_LOG is
// used.
#define ORBIT_SCOPED_TIMED_LOG(format, ...)                                                    \
  class ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT(ScopedTimedLog, __LINE__) {                     \
   public:                                                                                     \
    explicit ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT(ScopedTimedLog,                            \
                                                    __LINE__)(std::string && message)          \
        : message_{std::move(message)}, begin_{std::chrono::steady_clock::now()} {             \
      ORBIT_LOG("%s started", message_);                                                       \
    }                                                                                          \
                                                                                               \
    ~ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT(ScopedTimedLog, __LINE__)() {                      \
      auto end = std::chrono::steady_clock::now();                                             \
      auto duration =                                                                          \
          std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - begin_); \
      ORBIT_LOG("%s took %.3f ms", message_, duration.count());                                \
    }                                                                                          \
                                                                                               \
   private:                                                                                    \
    std::string message_;                                                                      \
    std::chrono::time_point<std::chrono::steady_clock> begin_;                                 \
  } ORBIT_INTERNAL_SCOPED_TIMED_LOG_CONCAT(scoped_timed_log_, __LINE__) {                      \
    absl::StrFormat(format, ##__VA_ARGS__)                                                     \
  }

// Internal.
#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
namespace orbit_base {
struct FuzzingException {};
}  // namespace orbit_base
#define ORBIT_INTERNAL_PLATFORM_LOG(message) (void)(message)  // No logging in fuzzing mode.
#define ORBIT_INTERNAL_PLATFORM_ABORT() \
  throw orbit_base::FuzzingException {}
#elif defined(_WIN32)
#define ORBIT_INTERNAL_PLATFORM_LOG(message)        \
  do {                                              \
    fprintf(stderr, "%s", message);                 \
    orbit_base_internal::OutputToDebugger(message); \
    orbit_base_internal::LogToFile(message);        \
  } while (0)
#define ORBIT_INTERNAL_PLATFORM_ABORT() \
  do {                                  \
    __debugbreak();                     \
    abort();                            \
  } while (0)
#else
#define ORBIT_INTERNAL_PLATFORM_LOG(message) \
  do {                                       \
    fprintf(stderr, "%s", message);          \
    orbit_base_internal::LogToFile(message); \
  } while (0)
#define ORBIT_INTERNAL_PLATFORM_ABORT() abort()
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace orbit_base {
std::string GetLogFileName();
// This function tries to remove log files older than kLogFileLifetime even when an error is
// returned. An error returns, for instance, if some log files to remove are used by other
// applications. If so, both the file name and the error message will be recorded in the log file.
ErrorMessageOr<void> TryRemoveOldLogFiles(const std::filesystem::path& log_dir);

void InitLogFile(const std::filesystem::path& path);

void LogStacktrace();
}  // namespace orbit_base

namespace orbit_base_internal {

void LogToFile(std::string_view message);

#ifdef _WIN32
// Add one indirection so that we can #include <Windows.h> in the .cpp instead of in this header.
void OutputToDebugger(const char* str);
#endif

}  // namespace orbit_base_internal

#endif  // ORBIT_BASE_LOGGING_H_
