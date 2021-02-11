// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define ORBIT_API_INTERNAL_IMPL
#include "OrbitAPI/Orbit.h"

#include "OrbitAPI/EncodedEvent.h"
#include "OrbitAPI/LockFreeApiEventProducer.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

constexpr const char* kNameNullPtr = nullptr;
constexpr uint64_t kDataZero = 0;

static void EnqueueApiEvent(orbit_api::EventType type, const char* name = nullptr,
                            uint64_t data = kDataZero, orbit_api_color color = kOrbitColorAuto) {
  static orbit_api::LockFreeApiEventBulkProducer producer;
  static pid_t pid = orbit_base::GetCurrentProcessId();
  thread_local uint32_t tid = orbit_base::GetCurrentThreadId();

  orbit_api::ApiEvent api_event(pid, tid, MonotonicTimestampNs(), type, name, data, color);
  producer.EnqueueIntermediateEvent(api_event);
}

extern "C" {

void orbit_api_start(const char* name, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::EventType::kScopeStart, name, kDataZero, color);
}

void orbit_api_stop() { EnqueueApiEvent(orbit_api::EventType::kScopeStop); }

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  EnqueueApiEvent(orbit_api::EventType::kScopeStartAsync, name, id, color);
}

void orbit_api_stop_async(uint64_t id) {
  EnqueueApiEvent(orbit_api::EventType::kScopeStopAsync, kNameNullPtr, id);
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
}
