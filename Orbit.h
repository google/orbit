// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_H_
#define ORBIT_H_

#include <stdint.h>

#include <cstring>

// Orbit API (header-only)
//
// While the main feature of Orbit is its ability to dynamically instrument
// functions, manual instrumentation is still possible using the macros below.
// These macros call empty functions that Orbit dynamically instruments.

// To disable manual instrumentation macros, define ORBIT_API_ENABLED as 0.
#define ORBIT_API_ENABLED 1

#if ORBIT_API_ENABLED

// ORBIT_SCOPE: profile current scope.
#define ORBIT_SCOPE(name) orbit_api::Scope ORBIT_VAR(ORBIT_STR(name))

// ORBIT_START/ORBIT_STOP: profile time span on a single thread.
#define ORBIT_START(name) orbit_api::Start(ORBIT_STR(name))
#define ORBIT_STOP() orbit_api::Stop()

// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: profile time span across threads.
#define ORBIT_START_ASYNC(name, id) orbit_api::StartAsync(ORBIT_STR(name), id)
#define ORBIT_STOP_ASYNC(id) orbit_api::StopAsync(id)

// ORBIT_[type]: graph variables.
#define ORBIT_INT(name, val) orbit_api::TrackInt(ORBIT_STR(name), val)
#define ORBIT_INT64(name, val) orbit_api::TrackInt64(ORBIT_STR(name), val)
#define ORBIT_UINT(name, val) orbit_api::TrackUint(ORBIT_STR(name), val)
#define ORBIT_UINT64(name, val) orbit_api::TrackUint64(ORBIT_STR(name), val)
#define ORBIT_FLOAT(name, val) orbit_api::TrackFloat(ORBIT_STR(name), val)
#define ORBIT_DOUBLE(name, val) orbit_api::TrackDouble(ORBIT_STR(name), val)

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

#endif

// NOTE: Do not use any of the code below directly.

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
ORBIT_STUB void Start(const char*) { ORBIT_NOOP(); }
ORBIT_STUB void Stop() { ORBIT_NOOP(); }
ORBIT_STUB void StartAsync(const char*, uint64_t) { ORBIT_NOOP(); }
ORBIT_STUB void StopAsync(uint64_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackInt(const char*, int32_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackInt64(const char*, int64_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackUint(const char*, uint32_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackUint64(const char*, uint64_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackFloatAsInt(const char*, int32_t) { ORBIT_NOOP(); }
ORBIT_STUB void TrackDoubleAsInt64(const char*, int64_t) { ORBIT_NOOP(); }

// Convert floating point arguments to integer arguments as we can't access
// XMM registers with our current dynamic instrumentation on Linux (uprobes).
inline void TrackFloat(const char* name, float value) {
  int int_value;
  static_assert(sizeof(value) == sizeof(int_value));
  std::memcpy(&int_value, &value, sizeof(value));
  TrackFloatAsInt(name, int_value);
}
inline void TrackDouble(const char* name, double value) {
  int64_t int_value;
  static_assert(sizeof(value) == sizeof(int_value));
  std::memcpy(&int_value, &value, sizeof(value));
  TrackDoubleAsInt64(name, int_value);
}

struct Scope {
  Scope(const char* name) { Start(name); }
  ~Scope() { Stop(); }
};

}  // namespace orbit_api

#endif  // ORBIT_H_
