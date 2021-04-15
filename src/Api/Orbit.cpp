// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define ORBIT_API_INTERNAL_IMPL
#include "Api/Orbit.h"

#include <absl/base/casts.h>

#include "Api/EncodedEvent.h"
#include "Api/LockFreeApiEventProducer.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

static void EnqueueApiEvent(orbit_api::EventType type, const char* name = nullptr,
                            uint64_t data = 0, orbit_api_color color = kOrbitColorAuto) {
  static orbit_api::LockFreeApiEventProducer producer;
  if (!producer.IsCapturing()) return;

  static pid_t pid = orbit_base::GetCurrentProcessId();
  thread_local uint32_t tid = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();

  orbit_api::ApiEvent api_event(pid, tid, timestamp_ns, type, name, data, color);
  producer.EnqueueIntermediateEvent(api_event);
}

extern "C" {

void orbit_api_start(const char* name, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::EventType::kScopeStart, name, /*data=*/0, color);
}

void orbit_api_stop() { EnqueueApiEvent(orbit_api::EventType::kScopeStop); }

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::EventType::kScopeStartAsync, name, id, color);
}

void orbit_api_stop_async(uint64_t id) {
  EnqueueApiEvent(orbit_api::EventType::kScopeStopAsync, /*name=*/nullptr, id);
}

void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::kTrackInt, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::kTrackInt64, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::kTrackUint, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::kTrackUint64, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::kTrackFloat, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::kTrackDouble, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  if (str == nullptr) return;
  char buffer[orbit_api::kMaxEventStringSize] = {};
  constexpr size_t chunk_size = orbit_api::kMaxEventStringSize - 1;
  const char* end = str + strlen(str);
  while (str < end) {
    std::strncpy(buffer, str, chunk_size);
    buffer[chunk_size] = 0;
    EnqueueApiEvent(orbit_api::EventType::kString, buffer, id, color);
    str += chunk_size;
  }
}

static void orbit_api_initialize_v0(uint64_t address) {
  // The api function table is accessed by user code using this pattern:
  //
  // std::atomic_thread_fence(std::memory_order_acquire)
  // if(api->active && api->orbit_api_function_name) api->orbit_api_function_name(...);
  //
  // We use acquire and release semantics to make sure that when api->active is set, all the
  // function pointers have been assigned and are visible to other cores.

  orbit_api_v0* api = absl::bit_cast<orbit_api_v0*>(address);
  if (api->initialized != 0) return;
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
  std::atomic_thread_fence(std::memory_order_release);
  api->initialized = 1;
  api->active = 1;
}

void orbit_api_initialize(uint64_t address, uint64_t api_version) {
  LOG("Initializing Orbit API at address 0x%x, version %u", address, api_version);
  if (api_version > kOrbitApiVersion) {
    ERROR(
        "Orbit API version in tracee (%u) is newer than the max supported version (%u). "
        "Some features will be unavailable.",
        api_version, kOrbitApiVersion);
  }

  if (api_version >= 0) orbit_api_initialize_v0(address);
}
}
