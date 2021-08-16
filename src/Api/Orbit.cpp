// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define ORBIT_API_INTERNAL_IMPL
#include "ApiInterface/Orbit.h"

#include <absl/base/casts.h>

#include "ApiUtils/Event.h"
#include "LockFreeApiEventProducer.h"
#include "OrbitApiVersions.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

// `__builtin_return_address(0)` will return us the (possibly encoded) return address of the
// current function (level "0" refers to this frame, "1" would be the callers return address and
// so on).
// To decode the return address, we call `__builtin_extract_return_addr`.
#define ORBIT_GET_CALLER_PC() \
  absl::bit_cast<uint64_t>(__builtin_extract_return_addr(__builtin_return_address(0)))

namespace {
orbit_api::LockFreeApiEventProducer& GetEventProducer() {
  static orbit_api::LockFreeApiEventProducer producer;
  return producer;
}

template <typename Event, typename... Types>
void EnqueueApiEvent(Types... args) {
  orbit_api::LockFreeApiEventProducer& producer = GetEventProducer();

  if (!producer.IsCapturing()) return;

  static pid_t pid = orbit_base::GetCurrentProcessId();
  thread_local pid_t tid = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  Event event{pid, tid, timestamp_ns, args...};
  producer.EnqueueIntermediateEvent(event);
}

void orbit_api_start_v1(const char* name, orbit_api_color color, uint64_t group_id) {
  uint64_t return_address = ORBIT_GET_CALLER_PC();
  EnqueueApiEvent<orbit_api::ApiScopeStart>(name, color, group_id, return_address);
}

void orbit_api_start(const char* name, orbit_api_color color) {
  uint64_t return_address = ORBIT_GET_CALLER_PC();
  EnqueueApiEvent<orbit_api::ApiScopeStart>(name, color, kOrbitDefaultGroupId, return_address);
}

void orbit_api_stop() { EnqueueApiEvent<orbit_api::ApiScopeStop>(); }

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  uint64_t return_address = ORBIT_GET_CALLER_PC();
  EnqueueApiEvent<orbit_api::ApiScopeStartAsync>(name, id, color, return_address);
}

void orbit_api_stop_async(uint64_t id) { EnqueueApiEvent<orbit_api::ApiScopeStopAsync>(id); }

void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
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

template <typename OrbitApiCurrentT>
void orbit_api_initialize_and_set_enabled(
    OrbitApiCurrentT* api, void (*orbit_api_initialize_function_table)(OrbitApiCurrentT*),
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
  // By the time we reach this, the "initialized" guard variable has been set to 1 and we know
  // that all function pointers have been written to and published to other cores by the use of
  // acquire/release fences. The "enabled" flag serves as a global toggle which is always used in
  // conjunction with the "initialized" flag to determine if the Api is active. See
  // orbit_api_active() in Orbit.h.
  api->enabled = static_cast<uint32_t>(enabled);
}

void orbit_api_initialize_v0(orbit_api_v0* api) {
  api->start = &orbit_api_start;
  api->stop = &orbit_api_stop;
  api->start_async = &orbit_api_start_async;
  api->stop_async = &orbit_api_stop_async;
  api->async_string = &orbit_api_async_string;
  api->track_int = &orbit_api_track_int;
  api->track_int64 = &orbit_api_track_int64;
  api->track_uint = &orbit_api_track_uint;
  api->track_uint64 = &orbit_api_track_uint64;
  api->track_float = &orbit_api_track_float;
  api->track_double = &orbit_api_track_double;
}

void orbit_api_initialize_v1(orbit_api_v1* api) {
  api->start = &orbit_api_start_v1;
  api->stop = &orbit_api_stop;
  api->start_async = &orbit_api_start_async;
  api->stop_async = &orbit_api_stop_async;
  api->async_string = &orbit_api_async_string;
  api->track_int = &orbit_api_track_int;
  api->track_int64 = &orbit_api_track_int64;
  api->track_uint = &orbit_api_track_uint;
  api->track_uint64 = &orbit_api_track_uint64;
  api->track_float = &orbit_api_track_float;
  api->track_double = &orbit_api_track_double;
}

}  // namespace

extern "C" {

// The "orbit_api_set_enabled" function is called remotely by OrbitService on every capture start
// for all api function tables. It is also called on every capture stop to disable the api so that
// the api calls early out at the call site.
void orbit_api_set_enabled(uint64_t address, uint64_t api_version, bool enabled) {
  LOG("%s Orbit API at address %#x, version %u", enabled ? "Enabling" : "Disabling", address,
      api_version);
  if (api_version > kOrbitApiVersion) {
    ERROR(
        "Orbit API version in tracee (%u) is newer than the max supported version (%u). "
        "Some features will be unavailable.",
        api_version, kOrbitApiVersion);
  }

  switch (api_version) {
    case 0: {
      auto* api_v0 = absl::bit_cast<orbit_api_v0*>(address);
      orbit_api_initialize_and_set_enabled(api_v0, &orbit_api_initialize_v0, enabled);
      return;
    }
    case 1: {
      auto* api_v1 = absl::bit_cast<orbit_api_v1*>(address);
      orbit_api_initialize_and_set_enabled(api_v1, &orbit_api_initialize_v1, enabled);
      return;
    }
    default:
      UNREACHABLE();
  }
}
}
