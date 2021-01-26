// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define ORBIT_API_INTERNAL_IMPL
#include "OrbitAPI/Orbit.h"

#include <absl/container/flat_hash_map.h>

#include "OrbitAPI/EncodedEvent.h"
#include "OrbitAPI/LockFreeApiEventProducer.h"
#include "OrbitAPI/Stubs.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

constexpr const char* kNameNullPtr = nullptr;
constexpr uint64_t kDataZero = 0;

using orbit_grpc_protos::ApiEvent;

ApiEvent::Type ApiEventTypeFromEncodedEventType(orbit_api::EventType encoded_event_type) {
  static absl::flat_hash_map<orbit_api::EventType, ApiEvent::Type> type_map{
      {orbit_api::kScopeStart, ApiEvent::kScopeStart},
      {orbit_api::kScopeStop, ApiEvent::kScopeStop},
      {orbit_api::kScopeStartAsync, ApiEvent::kScopeStartAsync},
      {orbit_api::kScopeStopAsync, ApiEvent::kScopeStopAsync},
      {orbit_api::kTrackInt, ApiEvent::kTrackInt},
      {orbit_api::kTrackInt64, ApiEvent::kTrackInt64},
      {orbit_api::kTrackUint, ApiEvent::kTrackUint},
      {orbit_api::kTrackUint64, ApiEvent::kTrackUint64},
      {orbit_api::kTrackFloat, ApiEvent::kTrackFloat},
      {orbit_api::kTrackDouble, ApiEvent::kTrackDouble},
      {orbit_api::kString, ApiEvent::kString}};
  CHECK(type_map.count(encoded_event_type) > 0);
  return type_map[encoded_event_type];
}

void EnqueueEncodedEvent(const orbit_api::EncodedEvent& event) {
  (void)event;
  static orbit_api::LockFreeApiEventProducer producer;

  orbit_grpc_protos::ApiEvent api_event;
  api_event.set_type(
      ApiEventTypeFromEncodedEventType(static_cast<orbit_api::EventType>(event.event.type)));
  api_event.set_pid(getpid());
  api_event.set_tid(orbit_base::GetCurrentThreadId());
  api_event.set_timestamp_ns(MonotonicTimestampNs());
  // api_evnet.set_depth();
  api_event.set_name(event.event.name);
  api_event.set_color(event.event.color);
  api_event.set_data(event.event.data);

  orbit_grpc_protos::CaptureEvent caputure_event;
  *caputure_event.mutable_api_event() = std::move(api_event);
  producer.EnqueueIntermediateEvent(caputure_event);
}

extern "C" {

void orbit_api_start(const char* name, orbit_api_color color) {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStart, name, kDataZero, color);
  // orbit_api::Start(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
  EnqueueEncodedEvent(e);
}

void orbit_api_stop() {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStop);
  // orbit_api::Stop(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
  EnqueueEncodedEvent(e);
}

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStartAsync, name, id, color);
  // orbit_api::StartAsync(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
  EnqueueEncodedEvent(e);
}

void orbit_api_stop_async(uint64_t id) {
  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStopAsync, kNameNullPtr, id);
  // orbit_api::StopAsync(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
  EnqueueEncodedEvent(e);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  if (str == nullptr) return;
  constexpr size_t chunk_size = orbit_api::kMaxEventStringSize - 1;
  const char* end = str + strlen(str);
  while (str < end) {
    orbit_api::EncodedEvent e(orbit_api::EventType::kString, kNameNullPtr, id, color);
    std::strncpy(e.event.name, str, chunk_size);
    e.event.name[chunk_size] = 0;
    EnqueueEncodedEvent(e);
    // orbit_api::TrackValue(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
    str += chunk_size;
  }
}

static inline void TrackValue(orbit_api::EventType type, const char* name, uint64_t value,
                              orbit_api_color color) {
  orbit_api::EncodedEvent e(type, name, value, color);
  // orbit_api::TrackValue(e.args[0], e.args[1], e.args[2], e.args[3], e.args[4], e.args[5]);
  EnqueueEncodedEvent(e);
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
