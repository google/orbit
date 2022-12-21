// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ApiInterface/Orbit.h"

#include <absl/base/casts.h>

#include <utility>

#include "ApiUtils/Event.h"
#include "LockFreeApiEventProducer.h"
#include "OrbitApiVersions.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

#ifdef _WIN32
#include "ApiUtils/ApiEnableInfo.h"
#endif

namespace {
orbit_api::LockFreeApiEventProducer& GetCaptureEventProducer() {
  static orbit_api::LockFreeApiEventProducer producer;
  return producer;
}

template <typename Event, typename... Types>
void EnqueueApiEvent(Types... args) {
  orbit_api::LockFreeApiEventProducer& producer = GetCaptureEventProducer();

  if (!producer.IsCapturing()) return;

  static uint32_t pid = orbit_base::GetCurrentProcessId();
  thread_local uint32_t tid = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  Event event{pid, tid, timestamp_ns, args...};
  producer.EnqueueIntermediateEvent(event);
}

void orbit_api_start_v1(const char* name, orbit_api_color color, uint64_t group_id,
                        uint64_t caller_address) {
  if (caller_address == kOrbitCallerAddressAuto) {
    caller_address = ORBIT_GET_CALLER_PC();
  }
  EnqueueApiEvent<orbit_api::ApiScopeStart>(name, color, group_id, caller_address);
}

[[deprecated]] void orbit_api_start(const char* name, orbit_api_color color) {
  uint64_t return_address = ORBIT_GET_CALLER_PC();
  EnqueueApiEvent<orbit_api::ApiScopeStart>(
      name, color, static_cast<uint64_t>(kOrbitDefaultGroupId), return_address);
}

void orbit_api_stop() { EnqueueApiEvent<orbit_api::ApiScopeStop>(); }

void orbit_api_start_async_v1(const char* name, uint64_t id, orbit_api_color color,
                              uint64_t caller_address) {
  if (caller_address == kOrbitCallerAddressAuto) {
    caller_address = ORBIT_GET_CALLER_PC();
  }
  EnqueueApiEvent<orbit_api::ApiScopeStartAsync>(name, id, color, caller_address);
}

[[deprecated]] void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  uint64_t return_address = ORBIT_GET_CALLER_PC();
  EnqueueApiEvent<orbit_api::ApiScopeStartAsync>(name, id, color, return_address);
}

void orbit_api_stop_async(uint64_t id) { EnqueueApiEvent<orbit_api::ApiScopeStopAsync>(id); }

void orbit_api_track_int(const char* name, int32_t value, orbit_api_color color) {
  EnqueueApiEvent<orbit_api::ApiTrackInt>(name, value, color);
}

void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  EnqueueApiEvent<orbit_api::ApiTrackInt64>(name, value, color);
}

void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  EnqueueApiEvent<orbit_api::ApiTrackUint>(name, value, color);
}

void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  EnqueueApiEvent<orbit_api::ApiTrackUint64>(name, value, color);
}

void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  EnqueueApiEvent<orbit_api::ApiTrackFloat>(name, value, color);
}

void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  EnqueueApiEvent<orbit_api::ApiTrackDouble>(name, value, color);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  if (str == nullptr) return;
  EnqueueApiEvent<orbit_api::ApiStringEvent>(str, id, color);
}

template <typename OrbitApiT>
void orbit_api_initialize_and_set_enabled(OrbitApiT* api,
                                          void (*orbit_api_initialize_function_table)(OrbitApiT*),
                                          bool enabled) {
  if (api->initialized == 0u) {
    // The api function table is accessed by user code using this pattern:
    //
    // bool initialized = api.initialized;
    // std::atomic_thread_fence(std::memory_order_acquire)
    // if (initialized && api->enabled && api->orbit_api_function_name)
    // api->orbit_api_function_name()
    //
    // We use acquire and release semantics to make sure that when api->initialized is set, all the
    // function pointers have been assigned and are visible to other cores.
    (*orbit_api_initialize_function_table)(api);
    std::atomic_thread_fence(std::memory_order_release);
    api->initialized = 1;
  }
  // By the time we reach this, the "initialized" guard variable has been set to 1, and we know
  // that all function pointers have been written to and published to other cores by the use of
  // acquire/release fences. The "enabled" flag serves as a global toggle which is always used in
  // conjunction with the "initialized" flag to determine if the Api is active. See
  // orbit_api_active() in Orbit.h.
  api->enabled = static_cast<uint32_t>(enabled);
}

void orbit_api_initialize_v0(orbit_api_v0* api_v0) {
  api_v0->start = &orbit_api_start;
  api_v0->stop = &orbit_api_stop;
  api_v0->start_async = &orbit_api_start_async;
  api_v0->stop_async = &orbit_api_stop_async;
  api_v0->async_string = &orbit_api_async_string;
  api_v0->track_int = &orbit_api_track_int;
  api_v0->track_int64 = &orbit_api_track_int64;
  api_v0->track_uint = &orbit_api_track_uint;
  api_v0->track_uint64 = &orbit_api_track_uint64;
  api_v0->track_float = &orbit_api_track_float;
  api_v0->track_double = &orbit_api_track_double;
}

void orbit_api_initialize_v1(orbit_api_v1* api_v1) {
  api_v1->start = &orbit_api_start_v1;
  api_v1->stop = &orbit_api_stop;
  api_v1->start_async = &orbit_api_start_async;
  api_v1->stop_async = &orbit_api_stop_async;
  api_v1->async_string = &orbit_api_async_string;
  api_v1->track_int = &orbit_api_track_int;
  api_v1->track_int64 = &orbit_api_track_int64;
  api_v1->track_uint = &orbit_api_track_uint;
  api_v1->track_uint64 = &orbit_api_track_uint64;
  api_v1->track_float = &orbit_api_track_float;
  api_v1->track_double = &orbit_api_track_double;
}

void orbit_api_initialize_v2(orbit_api_v2* api_v2) {
  api_v2->start = &orbit_api_start_v1;
  api_v2->stop = &orbit_api_stop;
  api_v2->start_async = &orbit_api_start_async_v1;
  api_v2->stop_async = &orbit_api_stop_async;
  api_v2->async_string = &orbit_api_async_string;
  api_v2->track_int = &orbit_api_track_int;
  api_v2->track_int64 = &orbit_api_track_int64;
  api_v2->track_uint = &orbit_api_track_uint;
  api_v2->track_uint64 = &orbit_api_track_uint64;
  api_v2->track_float = &orbit_api_track_float;
  api_v2->track_double = &orbit_api_track_double;
}

#ifdef __linux

// The functions that follow, with `__attribute__((ms_abi))`, are used to fill the function table
// `g_orbit_api` when the target was built for Windows and is running on Wine. They simply forward
// to the Linux versions, and the compiler takes care of converting between calling conventions.

__attribute__((ms_abi)) void orbit_api_start_wine_v1(const char* name, orbit_api_color color,
                                                     uint64_t group_id, uint64_t caller_address) {
  if (caller_address == kOrbitCallerAddressAuto) {
    caller_address = ORBIT_GET_CALLER_PC();
  }
  orbit_api_start_v1(name, color, group_id, caller_address);
}

__attribute__((ms_abi)) void orbit_api_stop_wine() { orbit_api_stop(); }

__attribute__((ms_abi)) void orbit_api_start_async_wine_v1(const char* name, uint64_t id,
                                                           orbit_api_color color,
                                                           uint64_t caller_address) {
  if (caller_address == kOrbitCallerAddressAuto) {
    caller_address = ORBIT_GET_CALLER_PC();
  }
  orbit_api_start_async_v1(name, id, color, caller_address);
}

__attribute__((ms_abi)) void orbit_api_stop_async_wine(uint64_t id) { orbit_api_stop_async(id); }

__attribute__((ms_abi)) void orbit_api_async_string_wine(const char* str, uint64_t id,
                                                         orbit_api_color color) {
  orbit_api_async_string(str, id, color);
}

__attribute__((ms_abi)) void orbit_api_track_int_wine(const char* name, int32_t value,
                                                      orbit_api_color color) {
  orbit_api_track_int(name, value, color);
}

__attribute__((ms_abi)) void orbit_api_track_int64_wine(const char* name, int64_t value,
                                                        orbit_api_color color) {
  orbit_api_track_int64(name, value, color);
}

__attribute__((ms_abi)) void orbit_api_track_uint_wine(const char* name, uint32_t value,
                                                       orbit_api_color color) {
  orbit_api_track_uint(name, value, color);
}

__attribute__((ms_abi)) void orbit_api_track_uint64_wine(const char* name, uint64_t value,
                                                         orbit_api_color color) {
  orbit_api_track_uint64(name, value, color);
}

__attribute__((ms_abi)) void orbit_api_track_float_wine(const char* name, float value,
                                                        orbit_api_color color) {
  orbit_api_track_float(name, value, color);
}

__attribute__((ms_abi)) void orbit_api_track_double_wine(const char* name, double value,
                                                         orbit_api_color color) {
  orbit_api_track_double(name, value, color);
}

void orbit_api_initialize_wine_v2(orbit_api_win_v2* api_win_v2) {
  api_win_v2->start = &orbit_api_start_wine_v1;
  api_win_v2->stop = &orbit_api_stop_wine;
  api_win_v2->start_async = &orbit_api_start_async_wine_v1;
  api_win_v2->stop_async = &orbit_api_stop_async_wine;
  api_win_v2->async_string = &orbit_api_async_string_wine;
  api_win_v2->track_int = &orbit_api_track_int_wine;
  api_win_v2->track_int64 = &orbit_api_track_int64_wine;
  api_win_v2->track_uint = &orbit_api_track_uint_wine;
  api_win_v2->track_uint64 = &orbit_api_track_uint64_wine;
  api_win_v2->track_float = &orbit_api_track_float_wine;
  api_win_v2->track_double = &orbit_api_track_double_wine;
}

#endif  // __linux

}  // namespace

extern "C" {

// The "orbit_api_set_enabled" function is called remotely by OrbitService on every capture start
// for all api function tables. It is also called on every capture stop to disable the api so that
// the api calls early out at the call site.
ORBIT_EXPORT void orbit_api_set_enabled(uint64_t address, uint64_t api_version, bool enabled) {
  ORBIT_LOG("%s Orbit API at address %#x, version %u", enabled ? "Enabling" : "Disabling", address,
            api_version);

  if (api_version > kOrbitApiVersion) {
    ORBIT_ERROR(
        "Orbit API version in tracee (%u) is newer than the max supported version (%u). "
        "Some features will be unavailable.",
        api_version, kOrbitApiVersion);
  }

  switch (api_version) {
    case 0: {
      auto* api_v0 = absl::bit_cast<orbit_api_v0*>(address);
      orbit_api_initialize_and_set_enabled(api_v0, &orbit_api_initialize_v0, enabled);
    } break;
    case 1: {
      auto* api_v1 = absl::bit_cast<orbit_api_v1*>(address);
      orbit_api_initialize_and_set_enabled(api_v1, &orbit_api_initialize_v1, enabled);
    } break;
    case 2: {
      auto* api_v2 = absl::bit_cast<orbit_api_v2*>(address);
      orbit_api_initialize_and_set_enabled(api_v2, &orbit_api_initialize_v2, enabled);
    } break;
    default:
      ORBIT_UNREACHABLE();
  }

  // Initialize `LockFreeApiEventProducer` and establish the connection to OrbitService now instead
  // of waiting for the first call to `EnqueueApiEvent`. As it takes some time to establish the
  // connection, `producer.IsCapturing()` would otherwise always be false with at least the first
  // event (but possibly more), causing it to be missed even if it comes a long time after calling
  // `orbit_api_set_enabled`.
  // TODO(b/206359125): The fix involving calling GetCaptureEventProducer() here was removed because
  //  of b/209560448 (we could have interrupted a malloc, which is not re-entrant, so we need to
  //  avoid any memory allocation). Re-add the call once we have a solution to allow re-entrancy.
}

#ifdef __linux

void orbit_api_set_enabled_wine(uint64_t address, uint64_t api_version, bool enabled) {
  ORBIT_LOG("%s Orbit API at address %#x, for Windows", enabled ? "Enabling" : "Disabling",
            address);
  static constexpr uint64_t kOrbitApiForWineMinVersion = 2;
  if (api_version < kOrbitApiForWineMinVersion) {
    // This is unexpected because `orbit_api_get_function_table_address_win_v#` wasn't present in
    // Orbit.h before v2.
    ORBIT_ERROR(
        "Orbit API version in tracee (%u) is older than the min supported version (%u) for "
        "Wine.",
        api_version, kOrbitApiForWineMinVersion);
    return;
  }

  if (api_version > kOrbitApiVersion) {
    ORBIT_ERROR(
        "Orbit API version in tracee (%u) is newer than the max supported version (%u). "
        "Some features will be unavailable.",
        api_version, kOrbitApiVersion);
  }

  switch (api_version) {
    case 2: {
      auto* api_win = absl::bit_cast<orbit_api_win_v2*>(address);
      orbit_api_initialize_and_set_enabled(api_win, &orbit_api_initialize_wine_v2, enabled);
    } break;
    default:
      ORBIT_UNREACHABLE();
  }

  // TODO(b/206359125): Re-add GetCaptureEventProducer() once possible. See above.
}

#endif  // __linux

#ifdef _WIN32

// This function is a wrapper around "orbit_api_set_enabled" that takes in a single parameter.
// It is needed on Windows as our method for remote code execution is based on "CreateRemoteThread"
// which takes in a single parameter.
ORBIT_EXPORT void orbit_api_set_enabled_from_struct(orbit_api::ApiEnableInfo* info) {
  void* (*orbit_api_get_address_of_function_table)() =
      absl::bit_cast<void* (*)()>(info->orbit_api_function_address);
  void* api_function_table_address = orbit_api_get_address_of_function_table();
  orbit_api_set_enabled(absl::bit_cast<uint64_t>(api_function_table_address), info->api_version,
                        info->api_enabled);
}

#endif  // _WIN32

}  // extern "C"
