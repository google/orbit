// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_INTERFACE_ORBIT_H_
#define ORBIT_API_INTERFACE_ORBIT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =================================================================================================
// Orbit Manual Instrumentation API.
// =================================================================================================
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
// Note that all of the macros above have a "_WITH_COLOR" variant that allows users to specify
// a custom color for time slices, async strings and graph elements. A set of predefined colors can
// be found below, see "orbit_api_color". Set custom colors with the "orbit_api_color(0xff0000ff)"
// syntax (rgba).
//
// Integration:
// To integrate the manual instrumentation API in your code base, simply include this header file
// and place the ORBIT_API_INSTANTIATE macro in an implementation file. Orbit will automatically
// deploy and dynamically load liborbit.so into the target process. Orbit will then write the proper
// function addresses into the "g_orbit_api" table.
//
// NOTE: To enable manual instrumentation, please make sure that:
//       1. The "Enable Orbit Api in target" checkbox is ticked in the "Capture Options" dialog.
//       2. You have loaded debug symbols for modules in which "ORBIT_API_INSTANTIATE" was placed.
//
// Please note that this feature is still considered "experimental".
//
//
// =================================================================================================
// ORBIT_SCOPE: Profile current scope.
// =================================================================================================
//
// Overview:
// ORBIT_SCOPE will profile the time between "now" and the end of the current scope.
//
// Group id:
// This macro also has a "_WITH_GROUP_ID" and a "_WITH_COLOR_AND_GROUP_ID" variant that allows users
// to specify a group id. Scopes with the same group id are associated to each other, such that
// selecting one scope in Orbit highlights all the other scopes that are associated to the selected
// one.
//
// Example Usage: Profile sections of a function:
//
// void MyVeryLongFunction() {
//   DoSomeWork();
//   if(condition) {
//     ORBIT_SCOPE("DoSomeMoreWork");
//     DoSomeMoreWork();
//   } else {
//     ORBIT_SCOPE_WITH_COLOR("DoSomeOtherWork", kOrbitColorLightGreen);
//     DoSomeOtherWork();
//   }
// }
//
// Parameters:
// name: [const char*] Label to be displayed on current time slice.
// col: [orbit_api_color] User-defined color for the current time slice (see orbit_api_color below).
// group_id: [uint64_t] User-defined non-zero id that associates the current time slice with all the
//           other time slices with the same id.
//
//
// =================================================================================================
// ORBIT_START/ORBIT_STOP: Profile sections inside a scope.
// =================================================================================================
//
// Overview:
// Profile the time between ORBIT_START and ORBIT_STOP.
//
// Note:
// ORBIT_START and its matching ORBIT_STOP need to happen in the same thread. For start and stop
// operations that happen in different threads use ORBIT_ASYNC_START/ORBIT_ASYNC_STOP.
//
// Group id:
// The ORBIT_START macro also has a "_WITH_GROUP_ID" and a "_WITH_COLOR_AND_GROUP_ID" variant that
// allows users to specify a group id. Time slices with the same group id are associated to each
// other, such that selecting one slice in Orbit highlights all the other slices that are associated
// to the selected one.
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
//   ORBIT_START_WITH_COLOR("DoSomeOtherWork", kOrbitColorLightGreen);
//   DoSomeOtherWork();
//   ORBIT_STOP();
// }
//
// Parameters of ORBIT_START:
// name: [const char*] Label to be displayed on the current time slice.
// col: [orbit_api_color] User-defined color for the current time slice (see orbit_api_color below).
// group_id: [uint64_t] User-defined non-zero id that associates the current time slice with all the
//           other time slices with the same id.
//
//
// =================================================================================================
// ORBIT_START_ASYNC/ORBIT_STOP_ASYNC: Profile time spans across scopes or threads.
// =================================================================================================
//
// Overview:
// Async time spans can be started in one scope and stopped in another. They will be displayed
// in Orbit on a track uniquely identified by the "name" parameter. Note that those time slices
// do not represent hierarchical information.
//
// Note:
// It is possible to add per-time-slice strings using the ORBIT_ASYNC_STRING macro (see below).
//
// Example usage: Tracking "File IO" operations.
// Thread 1: ORBIT_START_ASYNC("File IO", unique_64_bit_id);  // File IO request site.
// Thread 1 or 2: ORBIT_ASYNC_STRING(unique_64_bit_id, "My very long file path");
// Thread 1 or 2: ORBIT_STOP_ASYNC(unique_64_bit_id);  // File IO result site.
// Result: Multiple time slices labeled with the results of "io_request->GetFileName()" will appear
//         on a single "async" track named "File IO".
//
// Parameters of ORBIT_START_ASYNC:
// name: [const char*] Name of the *track* that will display the async events in Orbit.
// id: [uint64_t] User-provided globally *unique* id for the time slice. This id is used to match
//     the ORBIT_START_ASYNC and ORBIT_STOP_ASYNC calls. An id needs to be unique across all tracks.
// col: [orbit_api_color] User-defined color for the current time slice (see orbit_api_color below).
//
// Parameters of ORBIT_STOP_ASYNC:
// id: [uint64_t] User-provided globally *unique* id for the time slice. This id is used to match
//     the ORBIT_START_ASYNC and ORBIT_STOP_ASYNC calls. An id needs to be unique across all tracks.
//
//
// =================================================================================================
// ORBIT_ASYNC_STRING: Provide an additional string for an async time span.
// =================================================================================================
//
// Overview:
// Provide additional string to be displayed on the time slice corresponding to "id".
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
// id: [uint64_t] User-provided unique id for the time slice.
// col: [orbit_api_color] User-defined color for the current string (see orbit_api_color below).
//
//
// =================================================================================================
// ORBIT_[type]: Graph variables.
// =================================================================================================
//
// Overview:
// Send values to be plotted over time in a track uniquely identified by "name".
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
// Result: Given that instances have unique names, as many variable tracks as there are unique
//         instances will be created, and they will graph their individual instance health over
//         time. A single "Live Allocations" track will be created and will graph the result of
//         "MemManager::GetNumLiveAllocs()" over time.
//
// Parameters:
// name: [const char*] Name of the track that will display the graph in Orbit.
// val: [int, int64_t, uint32_t, uint64_t, float, double] Value to be plotted.
// col: [orbit_api_color] User-defined color for the current value (see orbit_api_color below).

// To disable manual instrumentation macros, define ORBIT_API_ENABLED as 0.
#define ORBIT_API_ENABLED 1

#if ORBIT_API_ENABLED

#ifdef __cplusplus

#define ORBIT_SCOPE(name) ORBIT_SCOPE_WITH_COLOR(name, kOrbitColorAuto)
#define ORBIT_SCOPE_WITH_COLOR(name, col) \
  ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID(name, col, kOrbitDefaultGroupId)
#define ORBIT_SCOPE_WITH_GROUP_ID(name, group_id) \
  ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID(name, kOrbitColorAuto, group_id)
#ifdef _WIN32
#define ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID(name, col, group_id) \
  orbit_api::Scope ORBIT_VAR(name, col, group_id)
#else
#define ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID(name, col, group_id) \
  ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID_INTERNAL(name, col, group_id, ORBIT_VAR)
#endif  // _WIN32

#endif  // __cplusplus

#define ORBIT_START(name) \
  ORBIT_CALL(start, name, kOrbitColorAuto, kOrbitDefaultGroupId, kOrbitCallerAddressAuto)
#define ORBIT_STOP() ORBIT_CALL(stop, )
#define ORBIT_START_ASYNC(name, id) \
  ORBIT_CALL(start_async, name, id, kOrbitColorAuto, kOrbitCallerAddressAuto)
#define ORBIT_STOP_ASYNC(id) ORBIT_CALL(stop_async, id)
#define ORBIT_ASYNC_STRING(string, id) ORBIT_CALL(async_string, string, id, kOrbitColorAuto)
#define ORBIT_INT(name, value) ORBIT_CALL(track_int, name, value, kOrbitColorAuto)
#define ORBIT_INT64(name, value) ORBIT_CALL(track_int64, name, value, kOrbitColorAuto)
#define ORBIT_UINT(name, value) ORBIT_CALL(track_uint, name, value, kOrbitColorAuto)
#define ORBIT_UINT64(name, value) ORBIT_CALL(track_uint64, name, value, kOrbitColorAuto)
#define ORBIT_FLOAT(name, value) ORBIT_CALL(track_float, name, value, kOrbitColorAuto)
#define ORBIT_DOUBLE(name, value) ORBIT_CALL(track_double, name, value, kOrbitColorAuto)

#define ORBIT_START_WITH_COLOR(name, color) \
  ORBIT_CALL(start, name, color, kOrbitDefaultGroupId, kOrbitCallerAddressAuto)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, color) \
  ORBIT_CALL(start_async, name, id, color, kOrbitCallerAddressAuto)
#define ORBIT_ASYNC_STRING_WITH_COLOR(string, id, color) ORBIT_CALL(async_string, string, id, color)
#define ORBIT_INT_WITH_COLOR(name, value, color) ORBIT_CALL(track_int, name, value, color)
#define ORBIT_INT64_WITH_COLOR(name, value, color) ORBIT_CALL(track_int64, name, value, color)
#define ORBIT_UINT_WITH_COLOR(name, value, color) ORBIT_CALL(track_uint, name, value, color)
#define ORBIT_UINT64_WITH_COLOR(name, value, color) ORBIT_CALL(track_uint64, name, value, color)
#define ORBIT_FLOAT_WITH_COLOR(name, value, color) ORBIT_CALL(track_float, name, value, color)
#define ORBIT_DOUBLE_WITH_COLOR(name, value, color) ORBIT_CALL(track_double, name, value, color)

#define ORBIT_START_WITH_GROUP_ID(name, group_id) \
  ORBIT_CALL(start, name, kOrbitColorAuto, group_id, kOrbitCallerAddressAuto)
#define ORBIT_START_WITH_COLOR_AND_GROUP_ID(name, color, group_id) \
  ORBIT_CALL(start, name, color, group_id, kOrbitCallerAddressAuto)

#else  // ORBIT_API_ENABLED

#ifdef __cplusplus
#define ORBIT_SCOPE(name)
#define ORBIT_SCOPE_WITH_COLOR(name, color)
#define ORBIT_SCOPE_WITH_GROUP_ID(name, group_id)
#define ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID(name, col, group_id)
#endif

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

#define ORBIT_START_WITH_COLOR(name, color)
#define ORBIT_START_ASYNC_WITH_COLOR(name, id, color)
#define ORBIT_ASYNC_STRING_WITH_COLOR(str, id, col)
#define ORBIT_INT_WITH_COLOR(name, value, color)
#define ORBIT_INT64_WITH_COLOR(name, value, color)
#define ORBIT_UINT_WITH_COLOR(name, value, color)
#define ORBIT_UINT64_WITH_COLOR(name, value, color)
#define ORBIT_FLOAT_WITH_COLOR(name, value, color)
#define ORBIT_DOUBLE_WITH_COLOR(name, value, color)

#define ORBIT_START_WITH_GROUP_ID(name, group_id)
#define ORBIT_START_WITH_COLOR_AND_GROUP_ID(name, color, group_id)

#endif  // ORBIT_API_ENABLED

#ifdef __cplusplus
#include <atomic>

#define ORBIT_THREAD_FENCE_ACQUIRE() std::atomic_thread_fence(std::memory_order_acquire)
#else
#if __STDC_VERSION__ >= 201112L
#include <stdatomic.h>

#define ORBIT_THREAD_FENCE_ACQUIRE() atomic_thread_fence(memory_order_acquire)
#elif defined(_WIN32)
#include <windows.h>

// In this case we only have a full (read and write) barrier available.
#define ORBIT_THREAD_FENCE_ACQUIRE() MemoryBarrier()
#else
#define ORBIT_THREAD_FENCE_ACQUIRE() __ATOMIC_ACQUIRE
#endif
#endif

#ifdef __linux
#define ORBIT_EXPORT __attribute__((visibility("default")))
#else
#define ORBIT_EXPORT __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

enum { kOrbitDefaultGroupId = 0ULL };
enum { kOrbitCallerAddressAuto = 0ULL };

enum { kOrbitApiVersion = 2 };

struct orbit_api_v2 {
  uint32_t enabled;
  uint32_t initialized;
  void (*start)(const char* name, orbit_api_color color, uint64_t group_id,
                uint64_t caller_address);
  void (*stop)();
  void (*start_async)(const char* name, uint64_t id, orbit_api_color color,
                      uint64_t caller_address);
  void (*stop_async)(uint64_t id);
  void (*async_string)(const char* str, uint64_t id, orbit_api_color color);
  void (*track_int)(const char* name, int value, orbit_api_color color);
  void (*track_int64)(const char* name, int64_t value, orbit_api_color color);
  void (*track_uint)(const char* name, uint32_t value, orbit_api_color color);
  void (*track_uint64)(const char* name, uint64_t value, orbit_api_color color);
  void (*track_float)(const char* name, float value, orbit_api_color color);
  void (*track_double)(const char* name, double value, orbit_api_color color);
};

#if __cplusplus >= 201103L  // C++11
static_assert(sizeof(struct orbit_api_v2) == 96, "struct orbit_api_v2 has an unexpected layout");
#elif __STDC_VERSION__ >= 201112L  // C11
_Static_assert(sizeof(struct orbit_api_v2) == 96, "struct orbit_api_v2 has an unexpected layout");
#endif

extern struct orbit_api_v2 g_orbit_api;

// User needs to place "ORBIT_API_INSTANTIATE" in an implementation file.
// We use a different name per platform for the "orbit_api_get_function_table_address_..._v#"
// function, so that we can easily distinguish what platform the binary was built for.
#ifdef _WIN32
extern ORBIT_EXPORT void* orbit_api_get_function_table_address_win_v2();

#define ORBIT_API_INSTANTIATE      \
  struct orbit_api_v2 g_orbit_api; \
  void* orbit_api_get_function_table_address_win_v2() { return &g_orbit_api; }
#else
extern ORBIT_EXPORT void* orbit_api_get_function_table_address_v2();

#define ORBIT_API_INSTANTIATE      \
  struct orbit_api_v2 g_orbit_api; \
  void* orbit_api_get_function_table_address_v2() { return &g_orbit_api; }
#endif  // _WIN32

#ifndef __cplusplus
// In C, `inline` alone doesn't generate an out-of-line definition, causing a linker error if the
// function is not inlined. We need to make the function `static`.
static
#endif
    inline bool
    orbit_api_active() {
  bool initialized = g_orbit_api.initialized != 0u;
  ORBIT_THREAD_FENCE_ACQUIRE();
  return initialized && (g_orbit_api.enabled != 0u);
}

#define ORBIT_CALL(function_name, ...)                                                           \
  do {                                                                                           \
    if (orbit_api_active() && g_orbit_api.function_name) g_orbit_api.function_name(__VA_ARGS__); \
  } while (0)

#ifdef __cplusplus
}  // extern "C"

// Internal macros.
#define ORBIT_CONCAT_IND(x, y) x##y
#define ORBIT_CONCAT(x, y) ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x) ORBIT_CONCAT(x, __COUNTER__)
#define ORBIT_VAR ORBIT_UNIQUE(ORB)

#ifdef _WIN32
#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

#define ORBIT_GET_CALLER_PC() reinterpret_cast<uint64_t>(_ReturnAddress())
#else
// `__builtin_return_address(0)` will return us the (possibly encoded) return address of the current
// function (level "0" refers to this frame, "1" would be the caller's return address and so on).
// To decode the return address, we call `__builtin_extract_return_addr`.
#define ORBIT_GET_CALLER_PC() \
  reinterpret_cast<uint64_t>(__builtin_extract_return_addr(__builtin_return_address(0)))
#endif  // _WIN32

#ifdef _WIN32
namespace orbit_api {
struct Scope {
  __declspec(noinline) Scope(const char* name, orbit_api_color color, uint64_t group_id) {
    uint64_t return_address = ORBIT_GET_CALLER_PC();
    ORBIT_CALL(start, name, color, group_id, return_address);
  }
  ~Scope() { ORBIT_CALL(stop); }
};
}  // namespace orbit_api
#else
// Retrieve the program counter with inline assembly, instead of using ORBIT_GET_CALLER_PC() in
// Scope::Scope and forcing that constructor to be noinline.
#define ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID_INTERNAL(name, col, group_id, pc_name) \
  uint64_t pc_name;                                                                \
  asm("lea (%%rip), %0" : "=r"(pc_name) : :);                                      \
  orbit_api::Scope ORBIT_VAR(name, col, group_id, pc_name)

namespace orbit_api {
struct Scope {
  Scope(const char* name, orbit_api_color color, uint64_t group_id, uint64_t pc) {
    ORBIT_CALL(start, name, color, group_id, pc);
  }
  ~Scope() { ORBIT_CALL(stop); }
};
}  // namespace orbit_api
#endif  // _WIN32

#endif  // __cplusplus

#endif  // ORBIT_API_INTERFACE_ORBIT_H_
