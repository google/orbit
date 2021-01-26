// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_LOGGING_H_
#define ORBIT_BASE_LOGGING_H_

#include <stdlib.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "absl/strings/str_format.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

constexpr const char* kLogTimeFormat = "%Y-%m-%dT%H:%M:%E6S";

#define LOG(format, ...)                                                                     \
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
    PLATFORM_LOG(formatted_log__.c_str());                                                   \
  } while (0)

#if defined(_WIN32) && defined(ERROR)
#undef ERROR
#endif

#define ERROR(format, ...) LOG("Error: " format, ##__VA_ARGS__)

#define FATAL(format, ...)                \
  do {                                    \
    LOG("Fatal: " format, ##__VA_ARGS__); \
    PLATFORM_ABORT();                     \
  } while (0)

#define UNREACHABLE() FATAL("Unreachable code")

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(cond) __builtin_expect(!!(cond), 1)
#define UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#else
#define LIKELY(cond) (!!(cond))
#define UNLIKELY(cond) (!!(cond))
#endif

#define FAIL_IF(condition, format, ...) \
  do {                                  \
    if (UNLIKELY(condition)) {          \
      FATAL(format, ##__VA_ARGS__);     \
    }                                   \
  } while (0)

#define CHECK(assertion)                \
  do {                                  \
    if (UNLIKELY(!(assertion))) {       \
      LOG("Check failed: " #assertion); \
      PLATFORM_ABORT();                 \
    }                                   \
  } while (0)

#ifndef NDEBUG
#define DCHECK(assertion) CHECK(assertion)
#else
#define DCHECK(assertion) \
  do {                    \
  } while (0 && (assertion))
#endif

#define SCOPED_TIMED_LOG_CONCAT_INDIRECT_(a, b) a##b
#define SCOPED_TIMED_LOG_CONCAT_(a, b) SCOPED_TIMED_LOG_CONCAT_INDIRECT_(a, b)

// Declare the class inside the macro so that file:line are the ones where SCOPED_TIMED_LOG is used.
#define SCOPED_TIMED_LOG(format, ...)                                                          \
  class SCOPED_TIMED_LOG_CONCAT_(ScopedTimedLog, __LINE__) {                                   \
   public:                                                                                     \
    explicit SCOPED_TIMED_LOG_CONCAT_(ScopedTimedLog, __LINE__)(std::string && message)        \
        : message_{std::move(message)}, begin_{std::chrono::steady_clock::now()} {             \
      LOG("%s started", message_);                                                             \
    }                                                                                          \
                                                                                               \
    ~SCOPED_TIMED_LOG_CONCAT_(ScopedTimedLog, __LINE__)() {                                    \
      auto end = std::chrono::steady_clock::now();                                             \
      auto duration =                                                                          \
          std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - begin_); \
      LOG("%s took %.3f ms", message_, duration.count());                                      \
    }                                                                                          \
                                                                                               \
   private:                                                                                    \
    std::string message_;                                                                      \
    std::chrono::time_point<std::chrono::steady_clock> begin_;                                 \
  } SCOPED_TIMED_LOG_CONCAT_(scoped_timed_log_, __LINE__) {                                    \
    absl::StrFormat(format, ##__VA_ARGS__)                                                     \
  }

extern std::ofstream log_file;
void InitLogFile(const std::filesystem::path& path);
void LogToFile(const std::string& message);

// Internal.
#if defined(_WIN32)
#define PLATFORM_LOG(message)       \
  do {                              \
    fprintf(stderr, "%s", message); \
    OutputDebugStringA(message);    \
    LogToFile(message);             \
  } while (0)
#define PLATFORM_ABORT() \
  do {                   \
    __debugbreak();      \
    abort();             \
  } while (0)
#else
#define PLATFORM_LOG(message)       \
  do {                              \
    fprintf(stderr, "%s", message); \
    LogToFile(message);             \
  } while (0)
#define PLATFORM_ABORT() abort()
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

void LogStacktrace();

#endif  // ORBIT_BASE_LOGGING_H_
