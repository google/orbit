// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_ORBIT_H_
#define ORBIT_API_ORBIT_H_

#include <stdint.h>

// Orbit Manual Instrumentation API.
//
// While dynamic instrumentation is one of Orbit's core features, manual instrumentation can also be
// extremely useful. The macros below allow you to profile sections of functions, track "async"
// operations, and graph interesting values directly in Orbit's main capture window.
//
// API Summary:
// ORBIT_SCOPE: Profile current scope.
// ORBIT_START/ORBIT_STOP: Profile sections inside a scope.
// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: Profile time spans across scopes or threads.
// ORBIT_ASYNC_STRING: Provide custom string for an async time span.
// ORBIT_INT: Graph int values.
// ORBIT_INT64: Graph int64_t values.
// ORBIT_UINT: Graph uint32_t values.
// ORBIT_UINT64: Graph uint64_t values.
// ORBIT_FLOAT: Graph float values.
// ORBIT_DOUBLE: Graph double values.
//
// Colors:
// Note that all of the macros above have a "_WITH_COLOR" variant that allow users to specify
// a custom color for time slices, async strings and graph elements. A set of predefined colors can
// be found below, see "orbit_api_color". Set custom colors with the "orbit_api_color(0xff0000ff)"
// syntax (rgba).
//
// Implementation:
// The manual instrumentation macros call empty "ORBIT_STUB" functions that Orbit dynamically
// instruments. For manual instrumentation to appear in your Orbit capture, make sure that symbols
// have been loaded for the manually instrumented modules.
//
// Performance:
// On Linux/Stadia, our current dynamic instrumentation implementation, which relies on uprobes and
// uretprobes, incur some non-negligible overhead (>5us per instrumented function call). Please
// note that instrumenting too many functions will possibly cause some noticeable performance
// degradation. Reducing overhead is our highest priority and we are actively working on a new
// implementation that should be at least one order of magnitude faster.
//
// Integration:
// To integrate the manual instrumentation API in your code base, simply include this header file.
//
// Please note that this feature is still considered "experimental".

// To disable manual instrumentation macros, define ORBIT_API_ENABLED as 0.
#define ORBIT_API_ENABLED 1

#if ORBIT_API_ENABLED

// Call once at application start, this is only needed when using the Api from pure C code.
#define ORBIT_API_INIT() orbit_api_init()

// Call once at application exit.
#define ORBIT_API_DEINIT() orbit_api_deinit()

// ORBIT_SCOPE: Profile current scope.
//
// Overview:
// ORBIT_SCOPE will profile the time between "now" and the end of the current scope.
//
// Note:
// We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize". This
// limitation may be lifted as we roll out a new dynamic instrumentation implementation.
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
// col: [orbit::Color] User-defined color for the current time slice (see orbit::Color below).
//
#ifdef __cplusplus
#define ORBIT_SCOPE(name) ORBIT_SCOPE_WITH_COLOR(name, kOrbitColorAuto)
#define ORBIT_SCOPE_WITH_COLOR(name, col) orbit_api::Scope ORBIT_VAR(name, col)
#endif

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
//    This limitation may be lifted as we roll out a new dynamic instrumentation implementation.
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
//   ORBIT_START_WITH_COLOR("DoSomeOtherWork", KOrbitColorLightGreen);
//   DoSomeOtherWork();
//   ORBIT_STOP();
// }
//
// Parameters:
// name: [const char*] Label to be displayed on current time slice (kMaxEventStringSize characters).
// col: [orbit_api_color] User-defined color for the current time slice (see orbit_api_color below).
//
#define ORBIT_START(name) orbit_api_start(name, kOrbitColorAuto)
#define ORBIT_START_WITH_COLOR(name, color) orbit_api_start(name, color)
#define ORBIT_STOP() orbit_api_stop()

// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: Profile time spans across scopes or threads.
//
// Overview:
// Async time spans can be started in one scope and stopped in another. They will be displayed
// in Orbit on a track uniquely identified by the "name" parameter. Note that those time slices
// do not represent hierarchical information.
//
// Note:
// We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize". This
// limitation may be lifted as we roll out a new dynamic instrumentation implementation.
// It is possible however to add per-time-slice strings using the ASYNC_STRING macro.
//
// Example usage: Tracking "File IO" operations.
// Thread 1: ORBIT_START_ASYNC("File IO", unique_64_bit_id);  // File IO request site.
// Thread 1 or 2: ORBIT_ASYNC_STRING(unique_64_bit_id, "My very long file path");
// Thread 1 or 2: ORBIT_STOP_ASYNC(unique_64_bit_id);  // File IO result site.
// Result: Multiple time slices labeled with the results of "io_request->GetFileName()" will appear
//         on a single "async" track named "File IO".
//
// Parameters:
// name: [const char*] Name of the *track* that will display the async events in Orbit.
// id: [uint64_t] A user-provided unique id for the time slice. This unique id is used to match the
//     ORBIT_START_ASYNC and ORBIT_STOP_ASYNC calls. An id needs to be unique for the current track.
// col: [orbit_api_color] User-defined color for the current time slice (see orbit_api_color below).
//
#define ORBIT_START_ASYNC(name, id) orbit_api_start_async(name, id, kOrbitColorAuto)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, color) orbit_api_start_async(name, id, color)
#define ORBIT_STOP_ASYNC(id) orbit_api_stop_async(id)

// ORBIT_ASYNC_STRING: Provide an additional string for an async time span.
//
// Overview:
// Provide additional string to be displayed on the time slice corresponding to "id".
//
// Note: There is a performance overhead incurred by using the ASYNC_STRING macro. The arbitrarily
//       long input string will be chunked into substrings of "kMaxEventStringSize" length that will
//       individually be emitted as multiple profiling events.
//
// Example usage: Tracking "File IO" operations.
// Thread 1: ORBIT_START_ASYNC("File IO", unique_64_bit_id);  // File IO request site.
// Thread 1 or 2: ORBIT_ASYNC_STRING(unique_64_bit_id, "My very long file path");
// Thread 1 or 2: ORBIT_STOP_ASYNC(unique_64_bit_id);  // File IO result site.
// Result: Multiple time slices labeled with the results of "io_request->GetFileName()" will appear
//         on a single "async" track named "File IO".
//
// Parameters:
// str: [const char*] String of arbitrary length to display in the time slice corresponding to "id".
// id: [uint64_t] A user-provided unique id for the time slice.
// col: [orbit_api_color] User-defined color for the current string (see orbit_api_color below).
//
#define ORBIT_ASYNC_STRING(string, id) orbit_api_async_string(string, id, kOrbitColorAuto)
#define ORBIT_ASYNC_STRING_WITH_COLOR(string, id, color) orbit_api_async_string(string, id, color)

// ORBIT_[type]: Graph variables.
//
// Overview:
// Send values to be plotted over time in a track uniquely identified by "name".
//
// Note:
// We limit the maximum number of characters of the "name" parameter to "kMaxEventStringSize". This
// limitation may be lifted as we roll out a new dynamic instrumentation implementation.
//
// Example usage: Graph the state of interesting variables over time:
//
// void MainLoop() {
//   for(instance : instances_) {
//     ORBIT_FLOAT(instance->GetName(), instance->GetHealth());
//   }
//
//   ORBIT_UINT64("Live Allocations", MemManager::GetNumLiveAllocs());
// }
// Result: Given that instances have unique names, as many graph tracks as there are unique
//         instances will be created and they will graph their individual instance health over time.
//         A single "Live Allocations" track will be created and will graph the the result of
//         "MemManager::GetNumLiveAllocs()" over time.
//
// Parameters:
// name: [const char*] Name of the track that will display the graph in Orbit.
// val: [int, int64_t, uint32_t, uint64_t, float, double] Value to be plotted.
// col: [orbit_api_color] User-defined color for the current value (see orbit_api_color below).
//
#define ORBIT_INT(name, value) orbit_api_track_int(name, value, kOrbitColorAuto)
#define ORBIT_INT64(name, value) orbit_api_track_int64(name, value, kOrbitColorAuto)
#define ORBIT_UINT(name, value) orbit_api_track_uint(name, value, kOrbitColorAuto)
#define ORBIT_UINT64(name, value) orbit_api_track_uint64(name, value, kOrbitColorAuto)
#define ORBIT_FLOAT(name, value) orbit_api_track_float(name, value, kOrbitColorAuto)
#define ORBIT_DOUBLE(name, value) orbit_api_track_double(name, value, kOrbitColorAuto)

#define ORBIT_INT_WITH_COLOR(name, value, color) orbit_api_track_int(name, value, color)
#define ORBIT_INT64_WITH_COLOR(name, value, color) orbit_api_track_int64(name, value, color)
#define ORBIT_UINT_WITH_COLOR(name, value, color) orbit_api_track_uint(name, value, color)
#define ORBIT_UINT64_WITH_COLOR(name, value, color) orbit_api_track_uint64(name, value, color)
#define ORBIT_FLOAT_WITH_COLOR(name, value, color) orbit_api_track_float(name, value, color)
#define ORBIT_DOUBLE_WITH_COLOR(name, value, color) orbit_api_track_double(name, value, color)

#else  // ORBIT_API_ENABLED

#define ORBIT_API_INIT()
#define ORBIT_API_DEINIT()
#define ORBIT_SCOPE(name)
#define ORBIT_START(name)
#define ORBIT_STOP()
#define ORBIT_START_ASYNC(name, id)
#define ORBIT_STOP_ASYNC(id)
#define ORBIT_ASYNC_STRING(str, id)
#define ORBIT_INT(name, value)
#define ORBIT_INT64(name, value)
#define ORBIT_UINT(name, value)
#define ORBIT_UINT64(name, value)
#define ORBIT_FLOAT(name, value)
#define ORBIT_DOUBLE(name, value)

#define ORBIT_SCOPE_WITH_COLOR(name, color)
#define ORBIT_START_WITH_COLOR(name, color)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, color)
#define ORBIT_ASYNC_STRING_WITH_COLOR(str, id, col)
#define ORBIT_INT_WITH_COLOR(name, value, color)
#define ORBIT_INT64_WITH_COLOR(name, value, color)
#define ORBIT_UINT_WITH_COLOR(name, value, color)
#define ORBIT_UINT64_WITH_COLOR(name, value, color)
#define ORBIT_FLOAT_WITH_COLOR(name, value, color)
#define ORBIT_DOUBLE_WITH_COLOR(name, value, color)

#endif  // ORBIT_API_ENABLED

// Material Design Colors #500
typedef enum {
  kOrbitColorAuto = 0x00000000,
  kOrbitColorRed = 0xf44336ff,
  kOrbitColorPink = 0xe91e63ff,
  kOrbitColorPurple = 0x9c27b0ff,
  kOrbitColorDeepPurple = 0x673ab7ff,
  kOrbitColorIndigo = 0x3f51b5ff,
  kOrbitColorBlue = 0x2196f3ff,
  kOrbitColorLightBlue = 0x03a9f4ff,
  kOrbitColorCyan = 0x00bcd4ff,
  kOrbitColorTeal = 0x009688ff,
  kOrbitColorGreen = 0x4caf50ff,
  kOrbitColorLightGreen = 0x8bc34aff,
  kOrbitColorLime = 0xcddc39ff,
  kOrbitColorYellow = 0xffeb3bff,
  kOrbitColorAmber = 0xffc107ff,
  kOrbitColorOrange = 0xff9800ff,
  kOrbitColorDeepOrange = 0xff5722ff,
  kOrbitColorBrown = 0x795548ff,
  kOrbitColorGrey = 0x9e9e9eff,
  kOrbitColorBlueGrey = 0x607d8bff
} orbit_api_color;

#ifndef __cplusplus
// The C implementation of the API lives in Orbit.c.
#define ORBIT_API_INTERNAL_IMPL 1
#endif

#ifdef ORBIT_API_INTERNAL_IMPL

#ifdef __cplusplus
extern "C" {
#endif

void orbit_api_init();
void orbit_api_deinit();
void orbit_api_start(const char* name, orbit_api_color color);
void orbit_api_stop();
void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color);
void orbit_api_stop_async(uint64_t id);
void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color);
void orbit_api_track_int(const char* name, int value, orbit_api_color color);
void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color);
void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color);
void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color);
void orbit_api_track_float(const char* name, float value, orbit_api_color color);
void orbit_api_track_double(const char* name, double value, orbit_api_color color);

#ifdef __cplusplus
}
#endif

#else  // ORBIT_API_INTERNAL_IMPL

#if __linux__

#include <dlfcn.h>
#include <stdio.h>

inline void* orbit_api_get_lib_orbit() {
  static void* liborbit = dlopen("./liborbit.so", RTLD_LAZY);
  if (liborbit == 0) printf("%s", "ERROR. liborbit.so not found, Orbit API will be disabled.\n");
  return liborbit;
}

inline void* orbit_api_get_proc_address(const char* name) {
  static void* liborbit = orbit_api_get_lib_orbit();
  void* address = liborbit != 0 ? dlsym(liborbit, name) : 0;
  printf("orbit_api_get_proc_address for %s : %p\n", name, address);
  return address;
}

#else

inline void* orbit_api_get_proc_address(const char*) {
  printf("%s", "ERROR. Platform not supported, Orbit API will be disabled.\n");
  return nullptr;
}

#endif

template <typename OrbitFunctionType>
class OrbitFunctor {
 public:
  OrbitFunctor() = delete;
  explicit OrbitFunctor(const char* proc_name)
      : func_(reinterpret_cast<OrbitFunctionType>(orbit_api_get_proc_address(proc_name))) {}

  template <typename... Args>
  inline void operator()(const Args&... args) {
    if (func_ != nullptr) func_(args...);
  }

 private:
  OrbitFunctionType func_;
};

extern "C" {

inline void orbit_api_init() {}

inline void orbit_api_deinit() {
  void* liborbit = orbit_api_get_lib_orbit();
  if (liborbit != nullptr) {
    dlclose(liborbit);
  }
}

inline void orbit_api_start(const char* name, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, orbit_api_color)> f("orbit_api_start");
  f(name, color);
}

inline void orbit_api_stop() {
  static OrbitFunctor<void (*)()> f("orbit_api_stop");
  f();
}

inline void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, uint64_t, orbit_api_color)> f("orbit_api_start_async");
  f(name, id, color);
}

inline void orbit_api_stop_async(uint64_t id) {
  static OrbitFunctor<void (*)(uint64_t)> f("orbit_api_stop_async");
  f(id);
}

inline void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, uint64_t, orbit_api_color)> f("orbit_api_async_string");
  f(str, id, color);
}

inline void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, int, orbit_api_color)> f("orbit_api_track_int");
  f(name, value, color);
}

inline void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, int64_t, orbit_api_color)> f("orbit_api_track_int64");
  f(name, value, color);
}

inline void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, uint32_t, orbit_api_color)> f("orbit_api_track_uint");
  f(name, value, color);
}

inline void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, uint64_t, orbit_api_color)> f("orbit_api_track_uint64");
  f(name, value, color);
}

inline void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, float, orbit_api_color)> f("orbit_api_track_float");
  f(name, value, color);
}

inline void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  static OrbitFunctor<void (*)(const char*, double, orbit_api_color)> f("orbit_api_track_double");
  f(name, value, color);
}

}  // extern "C"

#endif  // ORBIT_API_INTERNAL_IMPL


#ifdef __cplusplus

// Internal macros.
#define ORBIT_CONCAT_IND(x, y) (x##y)
#define ORBIT_CONCAT(x, y) ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x) ORBIT_CONCAT(x, __COUNTER__)
#define ORBIT_VAR ORBIT_UNIQUE(ORB)

namespace orbit_api {
struct Scope {
  Scope(const char* name, orbit_api_color color) { orbit_api_start(name, color); }
  ~Scope() { orbit_api_stop(); }
};
}  // namespace orbit_api

#endif  // __cplusplus

#endif  // ORBIT_API_ORBIT_H
