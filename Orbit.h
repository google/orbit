// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_H_
#define ORBIT_H_

#include <stdint.h>

// Orbit API (header-only)
//
// While the main feature of Orbit is its ability to dynamically instrument
// functions, manual instrumentation is still possible using the macros below.
// These macros call empty functions that Orbit dynamically instruments.

// To disable manual instrumentation macros, set ORBIT_API_ENABLED to 0.
#define ORBIT_API_ENABLED 1

#if ORBIT_API_ENABLED

// ORBIT_SCOPE: profile current scope.
#define ORBIT_SCOPE(name) orbit::Scope ORBIT_UNIQUE(ORB)(ORBIT_LITERAL(name))

// ORBIT_START/ORBIT_STOP: profile time span on a single thread.
#define ORBIT_START(name) orbit::Start(ORBIT_LITERAL(name))
#define ORBIT_STOP orbit::Stop()

// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: profile time span across threads.
#define ORBIT_START_ASYNC(name, id) orbit::StartAsync(ORBIT_LITERAL(name), id)
#define ORBIT_STOP_ASYNC(id) orbit::StopAsync(id)

// ORBIT_[type]: graph variables.
#define ORBIT_INT(name, value) orbit::TrackInt(ORBIT_LITERAL(name), value)
#define ORBIT_INT64(name, value) orbit::TrackInt64(ORBIT_LITERAL(name), value)
#define ORBIT_UINT(name, value) orbit::TrackUint(ORBIT_LITERAL(name), value)
#define ORBIT_UINT64(name, value) orbit::TrackUint64(ORBIT_LITERAL(name), value)
#define ORBIT_FLOAT(name, value) orbit::TrackFloat(ORBIT_LITERAL(name), value)
#define ORBIT_DOUBLE(name, value) orbit::TrackDouble(ORBIT_LITERAL(name), value)

#else

#define ORBIT_SCOPE(name)
#define ORBIT_START(name)
#define ORBIT_STOP
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
#define ORBIT_LITERAL(x) ("" x)
#define ORBIT_CONCAT_IND(x, y) (x##y)
#define ORBIT_CONCAT(x, y) ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x) ORBIT_CONCAT(x, __COUNTER__)
#define ORBIT_UNUSED(x) (void)(x)
#define ORBIT_NOOP       \
  static volatile int x; \
  x;

#if _WIN32
#define NO_INLINE __declspec(noinline)
#else
#define NO_INLINE __attribute__((noinline))
#endif

namespace orbit {

// NOTE: Do not use these directly, use corresponding macros instead.
inline void NO_INLINE Start(const char*) { ORBIT_NOOP; }
inline void NO_INLINE Stop() { ORBIT_NOOP; }
inline void NO_INLINE StartAsync(const char*, uint64_t) { ORBIT_NOOP; }
inline void NO_INLINE StopAsync(uint64_t) { ORBIT_NOOP; }
inline void NO_INLINE TrackInt(const char*, int32_t) { ORBIT_NOOP; }
inline void NO_INLINE TrackInt64(const char*, int64_t) { ORBIT_NOOP; }
inline void NO_INLINE TrackUint(const char*, uint32_t) { ORBIT_NOOP; }
inline void NO_INLINE TrackUint64(const char*, uint64_t) { ORBIT_NOOP; }
inline void NO_INLINE TrackFloatAsInt(const char*, int32_t) { ORBIT_NOOP; }
inline void NO_INLINE TrackDoubleAsInt64(const char*, int64_t) { ORBIT_NOOP; }

// Convert floating point arguments to integer arguments as we can't access
// XMM registers with our current dynamic instrumentation on Linux (uprobes).
inline void TrackFloat(const char* name, float value) {
  TrackFloatAsInt(name, *reinterpret_cast<int32_t*>(&value));
}
inline void TrackDouble(const char* name, double value) {
  TrackDoubleAsInt64(name, *reinterpret_cast<int64_t*>(&value));
}

struct Scope {
  Scope(const char* name) { Start(name); }
  ~Scope() { Stop(); }
};

}  // namespace orbit

#endif  // ORBIT_H_
