// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define ORBIT_API_INTERNAL_IMPL
#include "OrbitAPI/Orbit.h"

#include "OrbitAPI/EncodedEvent.h"
#include "OrbitAPI/Stubs.h"

constexpr const char* kNameNullPtr = nullptr;
constexpr uint64_t kDataZero = 0;

extern "C" {

void orbit_api_start(const char* name, orbit_api_color color) {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStart, name, kDataZero, color);
  orbit_api::Start(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

void orbit_api_stop() {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStop);
  orbit_api::Stop(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStartAsync, name, id, color);
  orbit_api::StartAsync(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

void orbit_api_stop_async(uint64_t id) {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStopAsync, kNameNullPtr, id);
  orbit_api::StopAsync(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  if (str == nullptr) return;
  constexpr size_t chunk_size = orbit_api::kMaxEventStringSize - 1;
  const char* end = str + strlen(str);
  while (str < end) {
    orbit_api::EncodedEvent e(orbit_api::EventType::kString, kNameNullPtr, id, color);
    std::strncpy(e.event.name, str, chunk_size);
    e.event.name[chunk_size] = 0;
    orbit_api::TrackValue(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
    str += chunk_size;
  }
}

static inline void TrackValue(orbit_api::EventType type, const char* name, uint64_t value,
                              orbit_api_color color) {
  orbit_api::EncodedEvent e(type, name, value, color);
  orbit_api::TrackValue(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
}

void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackInt, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackInt64, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackUint, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackUint64, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackFloat, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackDouble, name, orbit_api::Encode<uint64_t>(value), color);
}
}
