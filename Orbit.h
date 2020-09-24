// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_H_
#define ORBIT_H_

#include <stdint.h>

#include <cstring>

// Orbit Manual Instrumentation API (header-only)
//
// While Orbit's main feature is dynamic instrumentation, manual instrumentation can be extremely
// useful. The macros below allow you to profile sections of functions, track "async" operations,
// and graph interesting values in Orbit's main capture window.
//
// Summary:
// ORBIT_SCOPE: Profile current scope.
// ORBIT_START/ORBIT_STOP: Profile sections inside a scope.
// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: Profile time spans across scopes or threads.
// ORBIT_ASYNC_STRING: Provide additional information for an async time span.
// ORBIT_INT: Graph int values.
// ORBIT_INT64: Graph int64_t values.
// ORBIT_UINT: Graph uint32_t values.
// ORBIT_UINT64: Graph uint64_t values.
// ORBIT_FLOAT: Graph float values.
// ORBIT_DOUBLE: Graph double values.
//
// Implementation:
// The manual instrumentation macros call empty "ORBIT_STUB" functions that Orbit dynamically
// instruments. For manual instrumentation to appear in your Orbit capture, make sure that symbols
// have been loaded for the manually instrumented modules.

// To disable manual instrumentation macros, define ORBIT_API_ENABLED as 0.
#define ORBIT_API_ENABLED 1

#if ORBIT_API_ENABLED

// ORBIT_SCOPE: Profile current scope.
//
// Overview:
// ORBIT_SCOPE will profile the time between "now" and the end of the current scope.
//
// Note:
// We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize". This
// limitation will be lifted as we roll out a new dynamic instrumentation implementation on Linux.
//
// Example Usage: Profile sections of a function:
//
// void MyVeryLongFunction() {
//   DoSomeWork();
//   if(condition) {
//     ORBIT_SCOPE("DoSomeMoreWork");
//     DoSomeMoreWork();
//   } else {
//     ORBIT_SCOPE_WITH_COLOR("DoSomeOtherWork", orbit::Color::kLightGreen);
//     DoSomeOtherWork();
//   }
// }
//
// Parameters:
// name: [const char*] Label to be displayed on current time slice (kMaxEventStringSize characters).
// col: [orbit::Color] User defined color for the current time slice.
//
#define ORBIT_SCOPE(name) ORBIT_SCOPE_WITH_COLOR(name, orbit::Color::kAuto)
#define ORBIT_SCOPE_WITH_COLOR(name, col) orbit_api::Scope ORBIT_VAR(name, col)

// ORBIT_START/ORBIT_STOP: Profile sections inside a scope.
//
// Overview:
// Profile the time between ORBIT_START and ORBIT_STOP.
//
// Notes:
// 1. ORBIT_START and ORBIT_STOP need to be in the same scope. For start and stop operations that
//    need to happen in different scopes or threads, use ORBIT_ASYNC_START/ORBIT_ASYNC_STOP.
//
// 2. We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize".
//    This limitation will be lifted as we roll out a new dynamic instrumentation implementation on
//    Linux.
//
// Example Usage: Profile sections of a function:
//
// void MyVeryLongFunction() {
//   DoSomeWork();
//
//   ORBIT_START("DoSomeMoreWork");
//   DoSomeMoreWork();
//   ORBIT_STOP();
//
//   ORBIT_START_WITH_COLOR("DoSomeOtherWork", orbit::Color::kLightGreen);
//   DoSomeOtherWork();
//   ORBIT_STOP();
// }
//
// Parameters:
// name: [const char*] Label to be displayed on current time slice (kMaxEventStringSize characters).
// col: [orbit::Color] User defined color for the current time slice.
//
#define ORBIT_START(name) ORBIT_START_WITH_COLOR(name, orbit::Color::kAuto)
#define ORBIT_START_WITH_COLOR(name, col) orbit_api::Start(name, col)
#define ORBIT_STOP() orbit_api::Stop()

// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: Profile time spans across scopes or threads.
//
// Overview:
// Async time spans can be started in one scope and stopped on another. They will be displayed
// in Orbit on a track uniquely identified by the "name" parameter. Note that those time slices
// do not represent hierarchical information.
//
// Note:
// We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize". This
// limitation will be lifted as we roll out a new dynamic instrumentation implementation on Linux.
// It is possible however to add per-time slice additional information using the ASYNC_STRING macro.
//
// Example usage: Tracking "File IO" operations.
// Thread 1: ORBIT_START_ASYNC("File IO", unique_64_bit_id);  // File IO request site.
// Thread 1 or 2: ORBIT_ASYNC_STRING(unique_64_bit_id, "My very long file path");
// Thread 1 or 2: ORBIT_STOP_ASYNC(unique_64_bit_id);  // File IO result site.
//
// Parameters:
// name: [const char*] Name of the *track* that will display the async events in Orbit.
// id: [uint64_t] A user-provided unique id for the time slice.
// col: [orbit::Color] User defined color for the current time slice.
//
#define ORBIT_START_ASYNC(name, id) ORBIT_START_ASYNC_WITH_COLOR(name, id, orbit::Color::kAuto)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, col) orbit_api::StartAsync(name, id, col)
#define ORBIT_STOP_ASYNC(id) orbit_api::StopAsync(id)

// ORBIT_ASYNC_STRING: Provide additional information for an async time span.
//
// Overview:
// Provide additional information to be displayed on the time slice corresponding to "id".
//
// Example usage: Tracking "File IO" operations.
// Thread 1: ORBIT_START_ASYNC("File IO", unique_64_bit_id);
// Thread 1 or 2: ORBIT_ASYNC_STRING(unique_64_bit_id, "My very long file path");
// Thread 1 or 2: ORBIT_STOP_ASYNC(unique_64_bit_id);
//
// Parameters:
// str: [const char*] Arbitrary length string to display in the time slice corresponding to "id".
// id: [uint64_t] A user-provided unique id for the time slice.
// col: [orbit::Color] User defined color for the current string.
//
#define ORBIT_ASYNC_STRING(str, id) orbit_api::AsyncString(str, id, orbit::Color::kAuto)
#define ORBIT_ASYNC_STRING_WITH_COLOR(str, id, col) orbit_api::AsyncString(str, id, col)

// ORBIT_[type]: Graph variables.
//
// Overview:
// Send values to be plotted in a track uniquely identified by "name".
//
// Note:
// We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize". This
// limitation will be lifted as we roll out a new dynamic instrumentation implementation on Linux.
//
// Example usage: Graph state of interesting variables over time:
//
// void MainLoop() {
//   for(instance : instances_) {
//     ORBIT_FLOAT(instance->GetName(), instance->GetHealth());
//   }
//
//   ORBIT_UINT64("LiveAllocations", MemManager::GetNumLiveAllocs());
// }
//
// Parameters:
// name: [const char*] Name of the track that will display the graph in Orbit.
// val: [int, int64_t, uint32_t, uint64_t, float, double] Value to be plotted.
// col: [orbit::Color] User defined color for the current value.
//
#define ORBIT_INT(name, val) ORBIT_INT_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_INT64(name, val) ORBIT_INT64_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_UINT(name, val) ORBIT_UINT_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_UINT64(name, val) ORBIT_UINT64_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_FLOAT(name, val) ORBIT_FLOAT_WITH_COLOR(name, val, orbit::Color::kAuto)
#define ORBIT_DOUBLE(name, val) ORBIT_DOUBLE_WITH_COLOR(name, val, orbit::Color::kAuto)

#define ORBIT_INT_WITH_COLOR(name, val, col) ORBIT_TRACK(orbit_api::kTrackInt, name, val, col)
#define ORBIT_INT64_WITH_COLOR(name, val, col) ORBIT_TRACK(orbit_api::kTrackInt64, name, val, col)
#define ORBIT_UINT_WITH_COLOR(name, val, col) ORBIT_TRACK(orbit_api::kTrackUint, name, val, col)
#define ORBIT_UINT64_WITH_COLOR(name, val, col) ORBIT_TRACK(orbit_api::kTrackUint64, name, val, col)
#define ORBIT_FLOAT_WITH_COLOR(name, val, col) ORBIT_TRACK(orbit_api::kTrackFloat, name, val, col)
#define ORBIT_DOUBLE_WITH_COLOR(name, val, col) ORBIT_TRACK(orbit_api::kTrackDouble, name, val, col)

namespace orbit {

// Material Design Colors #500
enum class Color : uint32_t {
  kAuto = 0x00000000,
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
#define ORBIT_ASYNC_STRING(str, id)
#define ORBIT_ASYNC_STRING_WITH_COLOR(str, id, col)
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
#define ORBIT_CONCAT_IND(x, y) (x##y)
#define ORBIT_CONCAT(x, y) ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x) ORBIT_CONCAT(x, __COUNTER__)
#define ORBIT_VAR ORBIT_UNIQUE(ORB)
#define ORBIT_TRACK(type, name, val, col) \
  orbit_api::TrackValue(type, name, orbit_api::Encode<uint64_t>(val), col)

#if _WIN32
#define ORBIT_STUB inline __declspec(noinline)
#else
#define ORBIT_STUB inline __attribute__((noinline))
#endif

namespace orbit_api {

constexpr uint8_t kVersion = 1;

enum EventType : uint8_t {
  kNone = 0,
  kScopeStart = 1,
  kScopeStop = 2,
  kScopeStartAsync = 3,
  kScopeStopAsync = 4,
  kTrackInt = 5,
  kTrackInt64 = 6,
  kTrackUint = 7,
  kTrackUint64 = 8,
  kTrackFloat = 9,
  kTrackDouble = 10,
  kString = 11,
};

constexpr size_t kMaxEventStringSize = 34;
struct Event {
  uint8_t version;                 // 1
  uint8_t type;                    // 1
  char name[kMaxEventStringSize];  // 34
  orbit::Color color;              // 4
  uint64_t data;                   // 8
};

union EncodedEvent {
  EncodedEvent(orbit_api::EventType type, const char* name = nullptr, uint64_t data = 0,
               orbit::Color color = orbit::Color::kAuto) {
    static_assert(sizeof(EncodedEvent) == 48, "orbit_api::EncodedEvent should be 48 bytes.");
    static_assert(sizeof(Event) == 48, "orbit_api::Event should be 48 bytes.");
    event.version = kVersion;
    event.type = static_cast<uint8_t>(type);
    memset(event.name, 0, kMaxEventStringSize);
    if (name != nullptr) {
      std::strncpy(event.name, name, kMaxEventStringSize - 1);
      event.name[kMaxEventStringSize - 1] = 0;
    }
    event.data = data;
    event.color = color;
  }

  EncodedEvent(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
    args[0] = a0;
    args[1] = a1;
    args[2] = a2;
    args[3] = a3;
    args[4] = a4;
    args[5] = a5;
  }
  Event event;
  uint64_t args[6];
};

template <typename Dest, typename Source>
inline Dest Encode(const Source& source) {
  static_assert(sizeof(Source) <= sizeof(Dest));
  Dest dest = 0;
  std::memcpy(&dest, &source, sizeof(Source));
  return dest;
}

template <typename Dest, typename Source>
inline Dest Decode(const Source& source) {
  static_assert(sizeof(Dest) <= sizeof(Source));
  Dest dest = 0;
  std::memcpy(&dest, &source, sizeof(Dest));
  return dest;
}

// Used to prevent compiler from stripping out empty function.
inline void Noop() {
  static volatile int x;
  x;
}

// The stub functions below are automatically dynamically instrumented.
ORBIT_STUB void Start(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) { Noop(); }
ORBIT_STUB void Stop(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) { Noop(); }
ORBIT_STUB void StartAsync(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) { Noop(); }
ORBIT_STUB void StopAsync(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) { Noop(); }
ORBIT_STUB void TrackValue(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) { Noop(); }

// NOTE: Do not use these directly, use corresponding macros instead.
#ifndef ORBIT_API_INTERNAL_IMPL

// Default values.
constexpr const char* kNameNullPtr = nullptr;
constexpr uint64_t kDataZero = 0;
constexpr orbit::Color kColorAuto = orbit::Color::kAuto;

inline void Start(const char* name, orbit::Color color) {
  EncodedEvent e(EventType::kScopeStart, name, kDataZero, color);
  Start(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

inline void Stop() {
  EncodedEvent e(EventType::kScopeStop);
  Stop(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

inline void StartAsync(const char* name, uint64_t id, orbit::Color color) {
  EncodedEvent e(EventType::kScopeStartAsync, name, id, color);
  StartAsync(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

inline void StopAsync(uint64_t id) {
  EncodedEvent e(EventType::kScopeStopAsync, kNameNullPtr, id, kColorAuto);
  StopAsync(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

inline void AsyncString(const char* str, uint64_t id, orbit::Color color) {
  if (str == nullptr) return;
  constexpr size_t chunk_size = kMaxEventStringSize - 1;
  const char* end = str + strlen(str);
  while (str < end) {
    EncodedEvent e(EventType::kString, kNameNullPtr, id, color);
    std::strncpy(e.event.name, str, chunk_size);
    e.event.name[chunk_size] = 0;
    TrackValue(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
    str += chunk_size;
  }
}

inline void TrackValue(EventType type, const char* name, uint64_t value, orbit::Color color) {
  EncodedEvent e(type, name, value, color);
  TrackValue(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

#else

void Start(const char* name, orbit::Color color);
void Stop();
void StartAsync(const char* name, uint64_t id, orbit::Color color);
void StopAsync(uint64_t id);
void AsyncString(const char* str, uint64_t id);
void TrackValue(EventType type, const char* name, uint64_t value, orbit::Color color);

#endif  // ORBIT_API_INTERNAL_IMPL

struct Scope {
  Scope(const char* name, orbit::Color color) { Start(name, color); }
  ~Scope() { Stop(); }
};

}  // namespace orbit_api

#endif  // ORBIT_API_ENABLED

#endif  // ORBIT_H_
