#ifndef ORBIT_BASE_LOGGING_H_
#define ORBIT_BASE_LOGGING_H_

#include <cstdio>
#include <filesystem>

#include "absl/strings/str_format.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#define LOG(format, ...)                                               \
  do {                                                                 \
    std::string file_and_line__ = absl::StrFormat(                     \
        "%s:%d", std::filesystem::path(__FILE__).filename().c_str(),   \
        __LINE__);                                                     \
    if (file_and_line__.size() > 28)                                   \
      file_and_line__ =                                                \
          "..." + file_and_line__.substr(file_and_line__.size() - 25); \
    fprintf(stderr, "[%28s] " format "\n", file_and_line__.c_str(),    \
            ##__VA_ARGS__);                                            \
  } while (0)

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

#define FAIL_IF(assertion, format, ...)  \
  do {                                   \
    if (UNLIKELY(assertion)) {           \
      FATAL(format, ##__VA_ARGS__);      \
    }                                    \
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

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif  // ORBIT_BASE_LOGGING_H_
