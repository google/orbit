// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_LOGGING_H_
#define ORBIT_BASE_LOGGING_H_

#include <cstdio>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "absl/strings/str_format.h"
#include "absl/synchronization/mutex.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

template <typename... Args>
static void VARIADIC_UNUSED(Args... args) {
  ((void)args, ...);
}

#ifdef ORBIT_FUZZING
// In a fuzzing context we don't want to show standard log messages
// which get interleaved with the fuzzer's output.
#define LOG(format, ...) VARIADIC_UNUSED(format, ##__VA_ARGS__)
#else
#define LOG(format, ...)                                                      \
  do {                                                                        \
    std::string file__ = std::filesystem::path(__FILE__).filename().string(); \
    std::string file_and_line__ =                                             \
        absl::StrFormat("%s:%d", file__.c_str(), __LINE__);                   \
    if (file_and_line__.size() > 28)                                          \
      file_and_line__ =                                                       \
          "..." + file_and_line__.substr(file_and_line__.size() - 25);        \
    std::string formatted_log__ = absl::StrFormat(                            \
        "[%28s] " format "\n", file_and_line__.c_str(), ##__VA_ARGS__);       \
    PLATFORM_LOG(formatted_log__.c_str());                                    \
  } while (0)
#endif  // ORBIT_FUZZING

#if defined(_WIN32) && defined(ERROR)
#undef ERROR
#endif

#define ERROR(format, ...) LOG("Error: " format, ##__VA_ARGS__)

#ifdef ORBIT_FUZZING
// In a fuzzing context an abort is considered a crash
// and we can't continue testing. By throwing an exception
// instead we can stop the control flow and continue
// testing in the same process.
#define FATAL(format, ...)                \
  do {                                    \
    LOG("Fatal: " format, ##__VA_ARGS__); \
    throw std::runtime_error("fatal");    \
  } while (0)
#else
#define FATAL(format, ...)                \
  do {                                    \
    LOG("Fatal: " format, ##__VA_ARGS__); \
    abort();                              \
  } while (0)
#endif  // ORBIT_FUZZING

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

#ifdef ORBIT_FUZZING
// In a fuzzing context an abort is considered a crash
// and we can't continue testing. By throwing an exception
// instead we can stop the control flow and continue
// testing in the same process.
#define CHECK(assertion)                 \
  do {                                   \
    if (UNLIKELY(!(assertion))) {        \
      throw std::runtime_error("CHECK"); \
    }                                    \
  } while (0)
#else
#define CHECK(assertion)                \
  do {                                  \
    if (UNLIKELY(!(assertion))) {       \
      LOG("Check failed: " #assertion); \
      abort();                          \
    }                                   \
  } while (0)
#endif  // ORBIT_FUZZING

#ifndef NDEBUG
#define DCHECK(assertion) CHECK(assertion)
#else
#define DCHECK(assertion) \
  do {                    \
  } while (0 && (assertion))
#endif

extern std::ofstream log_file;
void InitLogFile(const std::string& path);
void LogToFile(const std::string& message);

// Internal.
#if defined(_WIN32)
#define PLATFORM_LOG(message)       \
  do {                              \
    fprintf(stderr, "%s", message); \
    OutputDebugStringA(message);    \
    LogToFile(message);             \
  } while (0)
#else
#define PLATFORM_LOG(message)       \
  do {                              \
    fprintf(stderr, "%s", message); \
    LogToFile(message);             \
  } while (0)
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif  // ORBIT_BASE_LOGGING_H_
