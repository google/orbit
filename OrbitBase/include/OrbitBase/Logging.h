#ifndef ORBIT_BASE_LOGGING_H_
#define ORBIT_BASE_LOGGING_H_

#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "absl/strings/str_format.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

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

#if defined(_WIN32) && defined(ERROR)
#undef ERROR
#endif

#define ERROR(format, ...) LOG("Error: " format, ##__VA_ARGS__)

#define FATAL(format, ...)                \
  do {                                    \
    LOG("Fatal: " format, ##__VA_ARGS__); \
    abort();                              \
  } while (0)

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(cond) __builtin_expect(!!(cond), 1)
#define UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#else
#define LIKELY(cond) (!!(cond))
#define UNLIKELY(cond) (!!(cond))
#endif

#define FAIL_IF(assertion, format, ...) \
  do {                                  \
    if (UNLIKELY(assertion)) {          \
      FATAL(format, ##__VA_ARGS__);     \
    }                                   \
  } while (0)

#define CHECK(assertion)                \
  do {                                  \
    if (UNLIKELY(!(assertion))) {       \
      LOG("Check failed: " #assertion); \
      abort();                          \
    }                                   \
  } while (0)

#ifndef NDEBUG
#define DCHECK(assertion) CHECK(assertion)
#else
#define DCHECK(assertion) \
  do {                    \
  } while (0 && (assertion))
#endif

// Internal.
#if defined(_WIN32)
#define PLATFORM_LOG(message)       \
  do {                              \
    fprintf(stderr, "%s", message); \
    OutputDebugStringA(message);    \
  } while (0)
#else
#define PLATFORM_LOG(message) fprintf(stderr, "%s", message)
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif  // ORBIT_BASE_LOGGING_H_
