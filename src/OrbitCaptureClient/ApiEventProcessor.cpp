// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureClient/ApiEventProcessor.h"

#include "OrbitBase/Logging.h"

using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::ApiEvent;

// constexpr const char* kNameNullPtr = nullptr;
// constexpr uint64_t kDataZero = 0;

ApiEventProcessor::ApiEventProcessor(CaptureListener* listener) : capture_listener_(listener) {}

void CheckThreadMonotonicity(const orbit_api::ApiEvent& api_event) {
  int32_t tid = api_event.tid;
  uint64_t time_stamp = api_event.timestamp_ns;

  static uint64_t total_count = 0;
  ++total_count;

  static absl::flat_hash_map<int32_t, uint64_t> last_timestamps_per_tid;
  uint64_t last_time_stamp = last_timestamps_per_tid[tid];
  if (last_time_stamp > time_stamp) {
    static uint64_t count = 0;
    ++count;
    LOG("[%lu/%lu] %lu > %lu diff ms = %f (%f pct)", count, total_count, last_time_stamp,
        time_stamp, (last_time_stamp - time_stamp) * 0.000001,
        double(count) / double(total_count) * 100.0);
  }
  last_timestamps_per_tid[tid] = time_stamp;
}

void ApiEventProcessor::ProcessApiEvent(const ApiEvent& event_buffer) {
  const orbit_api::ApiEvent* raw_event_buffer =
      reinterpret_cast<const orbit_api::ApiEvent*>(event_buffer.raw_data().data());
  CHECK(event_buffer.num_raw_events() * sizeof(orbit_api::ApiEvent) ==
        event_buffer.raw_data().size() * sizeof(uint64_t));

  for (size_t i = 0; i < event_buffer.num_raw_events(); ++i) {
    const orbit_api::ApiEvent& api_event = raw_event_buffer[i];
    ProcessApiEvent(api_event);
  }
}

/*
message ApiEventFixed {
  uint64 timestamp_ns = 1;
  int32 pid = 2;
  int32 tid = 3;
  uint32 type = 4;
  uint32 color = 5;
  uint64 data = 6;
  fixed64 d0 = 7;
  fixed64 d1 = 8;
  fixed64 d2 = 9;
  fixed64 d3 = 10;
  fixed64 d4 = 11;
}
*/
void ApiEventProcessor::ProcessApiEvent(const orbit_grpc_protos::ApiEventFixed& event_fixed) {
  orbit_api::ApiEvent event;
  event.timestamp_ns = event_fixed.timestamp_ns();
  event.pid = event_fixed.pid();
  event.tid = event_fixed.tid();
  event.encoded_event.event.type = event_fixed.type();
  event.encoded_event.event.color = static_cast<orbit_api_color>(event_fixed.color());
  event.encoded_event.event.data = event_fixed.data();
  uint64_t str_as_uint64[4];
  str_as_uint64[0] = event_fixed.d0();
  str_as_uint64[1] = event_fixed.d1();
  str_as_uint64[2] = event_fixed.d2();
  str_as_uint64[3] = event_fixed.d3();
  size_t num_chars = sizeof(str_as_uint64);
  CHECK(num_chars < orbit_api::kMaxEventStringSize);
  std::memcpy(event.encoded_event.event.name, &str_as_uint64, num_chars);
  event.encoded_event.event.name[num_chars] = '\0';
  ProcessApiEvent(event);
}

void ApiEventProcessor::ProcessApiEvent(const orbit_api::ApiEvent& api_event) {
  orbit_api::EventType event_type = api_event.Type();

  CheckThreadMonotonicity(api_event);

  switch (event_type) {
    case orbit_api::kScopeStart:
      ProcessStartEvent(api_event);
      break;
    case orbit_api::kScopeStop:
      ProcessStopEvent(api_event);
      break;
    case orbit_api::kScopeStartAsync:
      ProcessAsyncStartEvent(api_event);
      break;
    case orbit_api::kScopeStopAsync:
      ProcessAsyncStopEvent(api_event);
      break;

    case orbit_api::kTrackInt:
    case orbit_api::kTrackInt64:
    case orbit_api::kTrackUint:
    case orbit_api::kTrackUint64:
    case orbit_api::kTrackFloat:
    case orbit_api::kTrackDouble:
    case orbit_api::kString:
      ProcessTrackingEvent(api_event);
      break;
    case orbit_api::kNone:
      LOG("orbit_api::kNone");
      break;
    default:
      ERROR("ApiEvent::EVENT_NOT_SET read from Capture's gRPC stream");
  }
}

void ApiEventProcessor::ProcessStartEvent(const orbit_api::ApiEvent& api_event) {
  synchronous_event_stack_by_tid_[api_event.tid].emplace_back(api_event);
}

void ApiEventProcessor::ProcessStopEvent(const orbit_api::ApiEvent& stop_event) {
  std::vector<orbit_api::ApiEvent>& event_stack = synchronous_event_stack_by_tid_[stop_event.tid];
  if (event_stack.empty()) {
    // We received a stop event with no matching start event, which is possible
    // if the capture was started after the start and before the stop.
    return;
  }

  const orbit_api::ApiEvent& start_event = event_stack.back();

  TimerInfo timer_info;
  timer_info.set_start(start_event.timestamp_ns);
  timer_info.set_end(stop_event.timestamp_ns);
  timer_info.set_process_id(stop_event.pid);
  timer_info.set_thread_id(stop_event.tid);
  timer_info.set_depth(event_stack.size() - 1);
  timer_info.set_type(TimerInfo::kApiEvent);

  const orbit_api::EncodedEvent& e = start_event.encoded_event;
  timer_info.mutable_registers()->Reserve(6);
  timer_info.add_registers(e.args[0]);
  timer_info.add_registers(e.args[1]);
  timer_info.add_registers(e.args[2]);
  timer_info.add_registers(e.args[3]);
  timer_info.add_registers(e.args[4]);
  timer_info.add_registers(e.args[5]);

  capture_listener_->OnTimer(timer_info);
  event_stack.pop_back();
}

void ApiEventProcessor::ProcessAsyncStartEvent(const orbit_api::ApiEvent& start_event) {
  const uint64_t event_id = start_event.encoded_event.event.data;
  asynchronous_events_by_id_[event_id] = start_event;
}
void ApiEventProcessor::ProcessAsyncStopEvent(const orbit_api::ApiEvent& stop_event) {
  const uint64_t event_id = stop_event.encoded_event.event.data;
  if (asynchronous_events_by_id_.count(event_id) == 0) {
    return;
  }

  orbit_api::ApiEvent& start_event = asynchronous_events_by_id_[event_id];

  TimerInfo timer_info;
  timer_info.set_start(start_event.timestamp_ns);
  timer_info.set_end(stop_event.timestamp_ns);
  timer_info.set_process_id(stop_event.pid);
  timer_info.set_thread_id(stop_event.tid);
  timer_info.set_type(TimerInfo::kApiEvent);

  const orbit_api::EncodedEvent& e = start_event.encoded_event;
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

void ApiEventProcessor::ProcessTrackingEvent(const orbit_api::ApiEvent& api_event) {
  TimerInfo timer_info;
  timer_info.set_start(api_event.timestamp_ns);
  timer_info.set_process_id(api_event.pid);
  timer_info.set_thread_id(api_event.tid);
  timer_info.set_type(TimerInfo::kApiEvent);

  const orbit_api::EncodedEvent& e = api_event.encoded_event;
  timer_info.mutable_registers()->Reserve(6);
  timer_info.add_registers(e.args[0]);
  timer_info.add_registers(e.args[1]);
  timer_info.add_registers(e.args[2]);
  timer_info.add_registers(e.args[3]);
  timer_info.add_registers(e.args[4]);
  timer_info.add_registers(e.args[5]);

  capture_listener_->OnTimer(timer_info);
}
