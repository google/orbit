// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_H_
#define ORBIT_H_

#include <stdint.h>

#include <cstring>

// Orbit API (header-only)
//
// While the main feature of Orbit is its ability to dynamically instrument functions, manual
// instrumentation is still possible using the macros below. These macros call empty functions that
// Orbit dynamically instruments.
//
// NOTE: We only support string literals for now. This is enforced through the ORBIT_STR() macro.

// To disable manual instrumentation macros, define ORBIT_API_ENABLED as 0.
#define ORBIT_API_ENABLED 1

#if ORBIT_API_ENABLED

// ORBIT_SCOPE: profile current scope.
#define ORBIT_SCOPE(name) ORBIT_SCOPE_WITH_COLOR(name, orbit::Color::kAuto)
#define ORBIT_SCOPE_WITH_COLOR(name, col) orbit_api::Scope ORBIT_VAR(ORBIT_STR(name), col)

// ORBIT_START/ORBIT_STOP: profile time span on a single thread.
#define ORBIT_START(name) ORBIT_START_WITH_COLOR(name, orbit::Color::kAuto)
#define ORBIT_START_WITH_COLOR(name, col) orbit_api::Start(ORBIT_STR(name), 0, col)
#define ORBIT_STOP() orbit_api::Stop()

// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: profile time span across threads.
#define ORBIT_START_ASYNC(name, id) ORBIT_START_ASYNC_WITH_COLOR(name, id, orbit::Color::kAuto)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, col) orbit_api::StartAsync(ORBIT_STR(name), id, col)
#define ORBIT_STOP_ASYNC(id) orbit_api::StopAsync(id)

// ORBIT_[type]: graph variables.
#define ORBIT_INT(name, val) ORBIT_INT_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_INT64(name, val) ORBIT_INT64_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_UINT(name, val) ORBIT_UINT_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_UINT64(name, val) ORBIT_UINT64_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_FLOAT(name, val) ORBIT_FLOAT_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_DOUBLE(name, val) ORBIT_DOUBLE_WITH_COLOR(name, val, orbit::Color::kAuto)

// ORBIT_[type]_WITH_COLOR: graph variables with color.
#define ORBIT_INT_WITH_COLOR(name, val, col) orbit_api::TrackInt(ORBIT_STR(name), val, col)
#define ORBIT_INT64_WITH_COLOR(name, val, col) orbit_api::TrackInt64(ORBIT_STR(name), val, col)
#define ORBIT_UINT_WITH_COLOR(name, val, col) orbit_api::TrackUint(ORBIT_STR(name), val, col)
#define ORBIT_UINT64_WITH_COLOR(name, val, col) orbit_api::TrackUint64(ORBIT_STR(name), val, col)
#define ORBIT_FLOAT_WITH_COLOR(name, val, col) orbit_api::TrackFloat(ORBIT_STR(name), val, col)
#define ORBIT_DOUBLE_WITH_COLOR(name, val, col) orbit_api::TrackDouble(ORBIT_STR(name), val, col)

namespace orbit {

// Material Design Colors #500
enum class Color : uint32_t {
  kAuto = 0x00000001,
  kRed = 0xf44336ff,
  kPink = 0xe91e63ff,
  kPurple = 0x9c27b0ff,
  kDeepPurple = 0x673ab7ff,
  kIndigo = 0x3f51b5ff,
  kBlue = 0x2196f3ff,
  kLightBlue = 0x03a9f4ff,
  kCyan = 0x00bcd4ff,
  kTeal = 0x009688ff,
  kGreen = 0x4caf50ff,
  kLightGreen = 0x8bc34aff,
  kLime = 0xcddc39ff,
  kYellow = 0xffeb3bff,
  kAmber = 0xffc107ff,
  kOrange = 0xff9800ff,
  kDeepOrange = 0xff5722ff,
  kBrown = 0x795548ff,
  kGrey = 0x9e9e9eff,
  kBlueGrey = 0x607d8bff
};

}  // namespace orbit

#else

#define ORBIT_SCOPE(name)
#define ORBIT_START(name)
#define ORBIT_STOP()
#define ORBIT_START_ASYNC(name, id)
#define ORBIT_STOP_ASYNC(id)
#define ORBIT_INT(name, value)
#define ORBIT_INT64(name, value)
#define ORBIT_UINT(name, value)
#define ORBIT_UINT64(name, value)
#define ORBIT_FLOAT(name, value)
#define ORBIT_DOUBLE(name, value)

#define ORBIT_SCOPE_WITH_COLOR(name, color)
#define ORBIT_START_WITH_COLOR(name, color)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, color)
#define ORBIT_INT_WITH_COLOR(name, value, color)
#define ORBIT_INT64_WITH_COLOR(name, value, color)
#define ORBIT_UINT_WITH_COLOR(name, value, color)
#define ORBIT_UINT64_WITH_COLOR(name, value, color)
#define ORBIT_FLOAT_WITH_COLOR(name, value, color)
#define ORBIT_DOUBLE_WITH_COLOR(name, value, color)

#endif

// NOTE: Do not use any of the code below directly.
#if ORBIT_API_ENABLED

// Internal macros.
#define ORBIT_STR(x) ("" x)
#define ORBIT_CONCAT_IND(x, y) (x##y)
#define ORBIT_CONCAT(x, y) ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x) ORBIT_CONCAT(x, __COUNTER__)
#define ORBIT_VAR ORBIT_UNIQUE(ORB)
#define ORBIT_NOOP()       \
  do {                     \
    static volatile int x; \
    x;                     \
  } while (0)

#if _WIN32
#define ORBIT_STUB inline __declspec(noinline)
#else
#define ORBIT_STUB inline __attribute__((noinline))
#endif

namespace orbit_api {

// NOTE: Do not use these directly, use corresponding macros instead.
#ifdef ORBIT_API_INTERNAL_IMPL
void Start(const char* name, uint64_t dummy, orbit::Color color);
void Stop();
void StartAsync(const char* name, uint64_t value, orbit::Color color);
void StopAsync(uint64_t);
void TrackInt(const char* name, int32_t value, orbit::Color color);
void TrackInt64(const char* name, int64_t value, orbit::Color color);
void TrackUint(const char* name, uint32_t value, orbit::Color color);
void TrackUint64(const char* name, uint64_t value, orbit::Color color);
void TrackFloat(const char* name, float value, orbit::Color color);
void TrackDouble(const char* name, double value, orbit::Color color);
#else
ORBIT_STUB void Start(const char*, uint8_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void Stop() { ORBIT_NOOP(); }
ORBIT_STUB void StartAsync(const char*, uint64_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void StopAsync(uint64_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackInt(const char*, int32_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void TrackInt64(const char*, int64_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void TrackUint(const char*, uint32_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void TrackUint64(const char*, uint64_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void TrackFloatAsInt(const char*, int32_t, orbit::Color) { ORBIT_NOOP(); }
ORBIT_STUB void TrackDoubleAsInt64(const char*, int64_t, orbit::Color) { ORBIT_NOOP(); }

// Convert floating point arguments to integer arguments as we can't access
// XMM registers with our current dynamic instrumentation on Linux (uprobes).
inline void TrackFloat(const char* name, float value, orbit::Color color) {
  int int_value;
  static_assert(sizeof(value) == sizeof(int_value));
  std::memcpy(&int_value, &value, sizeof(value));
  TrackFloatAsInt(name, int_value, color);
}

inline void TrackDouble(const char* name, double value, orbit::Color color) {
  int64_t int_value;
  static_assert(sizeof(value) == sizeof(int_value));
  std::memcpy(&int_value, &value, sizeof(value));
  TrackDoubleAsInt64(name, int_value, color);
}
#endif  // ORBIT_API_INTERNAL_IMPL

struct Scope {
  Scope(const char* name, orbit::Color color) { Start(name, 0, color); }
  ~Scope() { Stop(); }
};

}  // namespace orbit_api

#endif  // ORBIT_API_ENABLED

#endif  // ORBIT_H_
