// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/ApiEventProcessor.h"

#include "../OrbitAPI/include/OrbitAPI/EncodedEvent.h"  // IWYU pragma: export
#include "OrbitBase/Logging.h"

using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::ApiEvent;

// constexpr const char* kNameNullPtr = nullptr;
constexpr uint64_t kDataZero = 0;

ApiEventProcessor::ApiEventProcessor(CaptureListener* listener) : capture_listener_(listener) {}

void ApiEventProcessor::ProcessApiEvent(const ApiEvent& api_event) {
  switch (api_event.type()) {
    case ApiEvent::kScopeStart:
      ProcessStartEvent(api_event);
      break;
    case ApiEvent::kScopeStop:
      ProcessStopEvent(api_event);
      break;
    case ApiEvent::kScopeStartAsync:
      ProcessAsyncStartEvent(api_event);
      break;
    case ApiEvent::kScopeStopAsync:
      ProcessAsyncStopEvent(api_event);
      break;

    case ApiEvent::kTrackInt:
    case ApiEvent::kTrackInt64:
    case ApiEvent::kTrackUint:
    case ApiEvent::kTrackUint64:
    case ApiEvent::kTrackFloat:
    case ApiEvent::kTrackDouble:
    case ApiEvent::kString:
      ProcessTrackingEvent(api_event);
      break;
    default:
      ERROR("ApiEvent::EVENT_NOT_SET read from Capture's gRPC stream");
  }
}

void ApiEventProcessor::ProcessStartEvent(const ApiEvent& api_event) {
  synchronous_event_stack_by_tid_[api_event.tid()].emplace_back(api_event);
}

void ApiEventProcessor::ProcessStopEvent(const ApiEvent& stop_event) {
  std::vector<ApiEvent>& event_stack = synchronous_event_stack_by_tid_[stop_event.tid()];
  if (event_stack.empty()) {
    // We received a stop event with no matching start event, which is possible
    // if the capture was started after the start and before the stop.
    return;
  }

  const ApiEvent& start_event = event_stack.back();

  TimerInfo timer_info;
  timer_info.set_start(start_event.timestamp_ns());
  timer_info.set_end(stop_event.timestamp_ns());
  timer_info.set_process_id(stop_event.pid());
  timer_info.set_thread_id(stop_event.tid());
  timer_info.set_depth(event_stack.size() - 1);
  timer_info.set_type(TimerInfo::kApiEvent);

  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStart, start_event.name().c_str(),
                            kDataZero, static_cast<orbit_api_color>(start_event.color()));
  timer_info.mutable_registers()->Reserve(6);
  timer_info.add_registers(e.args[0]);
  timer_info.add_registers(e.args[1]);
  timer_info.add_registers(e.args[2]);
  timer_info.add_registers(e.args[3]);
  timer_info.add_registers(e.args[4]);
  timer_info.add_registers(e.args[5]);

  static uint64_t count = 0;
  LOG("API SCOPE EVENT %u", ++count);
  capture_listener_->OnTimer(timer_info);
  event_stack.pop_back();
}

void ApiEventProcessor::ProcessAsyncStartEvent(const ApiEvent& start_event) {
  const uint64_t event_id = start_event.event_id();
  asynchronous_events_by_id_[event_id] = start_event;
}
void ApiEventProcessor::ProcessAsyncStopEvent(const ApiEvent& stop_event) {
  const uint64_t event_id = stop_event.event_id();
  if (asynchronous_events_by_id_.count(event_id) == 0) {
    return;
  }

  ApiEvent& start_event = asynchronous_events_by_id_[event_id];

  TimerInfo timer_info;
  timer_info.set_start(start_event.timestamp_ns());
  timer_info.set_end(stop_event.timestamp_ns());
  timer_info.set_process_id(stop_event.pid());
  timer_info.set_thread_id(stop_event.tid());
  timer_info.set_type(TimerInfo::kApiEvent);

  orbit_api::EncodedEvent e(orbit_api::EventType::kScopeStartAsync, start_event.name().c_str(),
                            kDataZero, static_cast<orbit_api_color>(start_event.color()));
  timer_info.mutable_registers()->Reserve(6);
  timer_info.add_registers(e.args[0]);
  timer_info.add_registers(e.args[1]);
  timer_info.add_registers(e.args[2]);
  timer_info.add_registers(e.args[3]);
  timer_info.add_registers(e.args[4]);
  timer_info.add_registers(e.args[5]);

  capture_listener_->OnTimer(timer_info);

  asynchronous_events_by_id_.erase(event_id);
}

orbit_api::EventType EncodedEventTypeFromApiEventType(ApiEvent::Type api_event_type) {
  static absl::flat_hash_map<ApiEvent::Type, orbit_api::EventType> type_map{
      {ApiEvent::kScopeStart, orbit_api::kScopeStart},
      {ApiEvent::kScopeStop, orbit_api::kScopeStop},
      {ApiEvent::kScopeStartAsync, orbit_api::kScopeStartAsync},
      {ApiEvent::kScopeStopAsync, orbit_api::kScopeStopAsync},
      {ApiEvent::kTrackInt, orbit_api::kTrackInt},
      {ApiEvent::kTrackInt64, orbit_api::kTrackInt64},
      {ApiEvent::kTrackUint, orbit_api::kTrackUint},
      {ApiEvent::kTrackUint64, orbit_api::kTrackUint64},
      {ApiEvent::kTrackFloat, orbit_api::kTrackFloat},
      {ApiEvent::kTrackDouble, orbit_api::kTrackDouble},
      {ApiEvent::kString, orbit_api::kString}};
  CHECK(type_map.count(api_event_type) > 0);
  return type_map[api_event_type];
}

void ApiEventProcessor::ProcessTrackingEvent(const ApiEvent& api_event) {
  TimerInfo timer_info;
  timer_info.set_start(api_event.timestamp_ns());
  timer_info.set_process_id(api_event.pid());
  timer_info.set_thread_id(api_event.tid());
  timer_info.set_type(TimerInfo::kApiEvent);

  orbit_api::EventType event_type = EncodedEventTypeFromApiEventType(api_event.type());
  orbit_api::EncodedEvent e(event_type, api_event.name().c_str(), api_event.data(),
                            static_cast<orbit_api_color>(api_event.color()));

  timer_info.mutable_registers()->Reserve(6);
  timer_info.add_registers(e.args[0]);
  timer_info.add_registers(e.args[1]);
  timer_info.add_registers(e.args[2]);
  timer_info.add_registers(e.args[3]);
  timer_info.add_registers(e.args[4]);
  timer_info.add_registers(e.args[5]);

  capture_listener_->OnTimer(timer_info);
}
