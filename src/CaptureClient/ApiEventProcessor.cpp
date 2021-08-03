// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/ApiEventProcessor.h"

#include "Api/EncodedString.h"
#include "OrbitBase/Logging.h"

namespace orbit_capture_client {

using orbit_client_protos::ApiStringEvent;
using orbit_client_protos::ApiTrackValue;
using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::ApiEvent;
using orbit_grpc_protos::ApiScopeStart;
using orbit_grpc_protos::ApiScopeStartAsync;

ApiEventProcessor::ApiEventProcessor(CaptureListener* listener) : capture_listener_(listener) {
  CHECK(listener != nullptr);
}

static inline TimerInfo TimerInfoFromEncodedEvent(const orbit_api::EncodedEvent& encoded_event,
                                                  uint64_t start, uint64_t end, int32_t pid,
                                                  int32_t tid, uint32_t depth) {
  TimerInfo timer_info;
  timer_info.set_start(start);
  timer_info.set_end(end);
  timer_info.set_process_id(pid);
  timer_info.set_thread_id(tid);
  timer_info.set_depth(depth);
  timer_info.set_type(TimerInfo::kApiEvent);
  timer_info.mutable_registers()->Reserve(6);
  timer_info.add_registers(encoded_event.args[0]);
  timer_info.add_registers(encoded_event.args[1]);
  timer_info.add_registers(encoded_event.args[2]);
  timer_info.add_registers(encoded_event.args[3]);
  timer_info.add_registers(encoded_event.args[4]);
  timer_info.add_registers(encoded_event.args[5]);
  return timer_info;
}

void ApiEventProcessor::ProcessApiEvent(const orbit_grpc_protos::ApiEvent& grpc_api_event) {
  orbit_api::ApiEvent api_event;
  api_event.pid = grpc_api_event.pid();
  api_event.tid = grpc_api_event.tid();
  api_event.timestamp_ns = grpc_api_event.timestamp_ns();
  api_event.encoded_event.args[0] = grpc_api_event.r0();
  api_event.encoded_event.args[1] = grpc_api_event.r1();
  api_event.encoded_event.args[2] = grpc_api_event.r2();
  api_event.encoded_event.args[3] = grpc_api_event.r3();
  api_event.encoded_event.args[4] = grpc_api_event.r4();
  api_event.encoded_event.args[5] = grpc_api_event.r5();
  ProcessApiEvent(api_event);
}

void ApiEventProcessor::ProcessApiEvent(const orbit_api::ApiEvent& api_event) {
  orbit_api::EventType event_type = api_event.Type();

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
      UNREACHABLE();
  }
}

void ApiEventProcessor::ProcessStartEvent(const orbit_api::ApiEvent& api_event) {
  synchronous_event_stack_by_tid_[api_event.tid].emplace_back(api_event);
}

void ApiEventProcessor::ProcessStopEvent(const orbit_api::ApiEvent& stop_event) {
  std::vector<orbit_api::ApiEvent>& event_stack = synchronous_event_stack_by_tid_[stop_event.tid];
  if (event_stack.empty()) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  const orbit_api::ApiEvent& start_event = event_stack.back();
  TimerInfo timer_info = TimerInfoFromEncodedEvent(
      start_event.encoded_event, start_event.timestamp_ns, stop_event.timestamp_ns, stop_event.pid,
      stop_event.tid, /*depth=*/event_stack.size() - 1);
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
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  orbit_api::ApiEvent& start_event = asynchronous_events_by_id_[event_id];
  TimerInfo timer_info = TimerInfoFromEncodedEvent(
      start_event.encoded_event, start_event.timestamp_ns, stop_event.timestamp_ns, stop_event.pid,
      stop_event.tid, /*depth=*/0);
  capture_listener_->OnTimer(timer_info);

  asynchronous_events_by_id_.erase(event_id);
}

void ApiEventProcessor::ProcessTrackingEvent(const orbit_api::ApiEvent& api_event) {
  TimerInfo timer_info =
      TimerInfoFromEncodedEvent(api_event.encoded_event, api_event.timestamp_ns,
                                api_event.timestamp_ns, api_event.pid, api_event.tid, /*depth=*/0);
  capture_listener_->OnTimer(timer_info);
}

namespace {
void EncodedColorToColor(uint32_t encoded_color, orbit_client_protos::Color* color) {
  color->set_red((0xff000000 & encoded_color) >> 24);
  color->set_green((0x00ff0000 & encoded_color) >> 16);
  color->set_blue((0x0000ff00 & encoded_color) >> 8);
  color->set_alpha(0x000000ff & encoded_color);
}
}  // namespace

void ApiEventProcessor::ProcessApiScopeStart(const orbit_grpc_protos::ApiScopeStart& event_buffer) {
  synchronous_scopes_stack_by_tid_[event_buffer.tid()].emplace_back(event_buffer);
}

void ApiEventProcessor::ProcessApiScopeStop(const orbit_grpc_protos::ApiScopeStop& event_buffer) {
  std::vector<ApiScopeStart>& event_stack = synchronous_scopes_stack_by_tid_[event_buffer.tid()];
  if (event_stack.empty()) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  const ApiScopeStart& start_event = event_stack.back();
  TimerInfo timer_info;

  timer_info.set_start(start_event.timestamp_ns());
  timer_info.set_end(event_buffer.timestamp_ns());
  timer_info.set_process_id(event_buffer.pid());
  timer_info.set_thread_id(event_buffer.tid());
  timer_info.set_depth(event_stack.size() - 1);
  timer_info.set_type(TimerInfo::kApiScope);

  EncodedColorToColor(start_event.color_rgba(), timer_info.mutable_color());

  timer_info.set_group_id(start_event.group_id());
  timer_info.set_address_in_function(start_event.address_in_function());

  timer_info.set_name(orbit_api::DecodeString(
      start_event.encoded_name_1(), start_event.encoded_name_2(), start_event.encoded_name_3(),
      start_event.encoded_name_4(), start_event.encoded_name_5(), start_event.encoded_name_6(),
      start_event.encoded_name_7(), start_event.encoded_name_8(),
      start_event.encoded_name_additional().data(), start_event.encoded_name_additional_size()));

  capture_listener_->OnTimer(timer_info);
  event_stack.pop_back();
}

void ApiEventProcessor::ProcessApiScopeStartAsync(
    const orbit_grpc_protos::ApiScopeStartAsync& event_buffer) {
  asynchronous_scopes_by_id_[event_buffer.id()] = event_buffer;
}

void ApiEventProcessor::ProcessApiScopeStopAsync(
    const orbit_grpc_protos::ApiScopeStopAsync& event_buffer) {
  uint64_t event_id = event_buffer.id();
  if (!asynchronous_scopes_by_id_.contains(event_id)) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  ApiScopeStartAsync& start_event = asynchronous_scopes_by_id_[event_id];
  TimerInfo timer_info;

  timer_info.set_start(start_event.timestamp_ns());
  timer_info.set_end(event_buffer.timestamp_ns());
  timer_info.set_process_id(event_buffer.pid());
  timer_info.set_thread_id(event_buffer.tid());
  timer_info.set_depth(0);
  timer_info.set_type(TimerInfo::kApiScopeAsync);

  EncodedColorToColor(start_event.color_rgba(), timer_info.mutable_color());

  timer_info.set_id(event_id);
  timer_info.set_address_in_function(start_event.address_in_function());

  timer_info.set_name(orbit_api::DecodeString(
      start_event.encoded_name_1(), start_event.encoded_name_2(), start_event.encoded_name_3(),
      start_event.encoded_name_4(), start_event.encoded_name_5(), start_event.encoded_name_6(),
      start_event.encoded_name_7(), start_event.encoded_name_8(),
      start_event.encoded_name_additional().data(), start_event.encoded_name_additional_size()));

  capture_listener_->OnTimer(timer_info);
  asynchronous_events_by_id_.erase(event_id);
}

void ApiEventProcessor::ProcessApiStringEvent(
    const orbit_grpc_protos::ApiStringEvent& event_buffer) {
  ApiStringEvent api_string_event;
  api_string_event.set_process_id(event_buffer.pid());
  api_string_event.set_thread_id(event_buffer.tid());
  api_string_event.set_timestamp_ns(event_buffer.timestamp_ns());
  api_string_event.set_id(event_buffer.id());

  api_string_event.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));
  capture_listener_->OnApiStringEvent(api_string_event);
}

void ApiEventProcessor::ProcessApiTrackDouble(
    const orbit_grpc_protos::ApiTrackDouble& event_buffer) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(event_buffer.pid());
  api_track_value.set_thread_id(event_buffer.tid());
  api_track_value.set_timestamp_ns(event_buffer.timestamp_ns());
  api_track_value.set_data_double(event_buffer.data());

  api_track_value.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackFloat(const orbit_grpc_protos::ApiTrackFloat& event_buffer) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(event_buffer.pid());
  api_track_value.set_thread_id(event_buffer.tid());
  api_track_value.set_timestamp_ns(event_buffer.timestamp_ns());
  api_track_value.set_data_float(event_buffer.data());

  api_track_value.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackInt(const orbit_grpc_protos::ApiTrackInt& event_buffer) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(event_buffer.pid());
  api_track_value.set_thread_id(event_buffer.tid());
  api_track_value.set_timestamp_ns(event_buffer.timestamp_ns());
  api_track_value.set_data_int(event_buffer.data());

  api_track_value.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackInt64(const orbit_grpc_protos::ApiTrackInt64& event_buffer) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(event_buffer.pid());
  api_track_value.set_thread_id(event_buffer.tid());
  api_track_value.set_timestamp_ns(event_buffer.timestamp_ns());
  api_track_value.set_data_int64(event_buffer.data());

  api_track_value.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackUint(const orbit_grpc_protos::ApiTrackUint& event_buffer) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(event_buffer.pid());
  api_track_value.set_thread_id(event_buffer.tid());
  api_track_value.set_timestamp_ns(event_buffer.timestamp_ns());
  api_track_value.set_data_uint(event_buffer.data());

  api_track_value.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackUint64(
    const orbit_grpc_protos::ApiTrackUint64& event_buffer) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(event_buffer.pid());
  api_track_value.set_thread_id(event_buffer.tid());
  api_track_value.set_timestamp_ns(event_buffer.timestamp_ns());
  api_track_value.set_data_uint64(event_buffer.data());

  api_track_value.set_name(orbit_api::DecodeString(
      event_buffer.encoded_name_1(), event_buffer.encoded_name_2(), event_buffer.encoded_name_3(),
      event_buffer.encoded_name_4(), event_buffer.encoded_name_5(), event_buffer.encoded_name_6(),
      event_buffer.encoded_name_7(), event_buffer.encoded_name_8(),
      event_buffer.encoded_name_additional().data(), event_buffer.encoded_name_additional_size()));

  capture_listener_->OnApiTrackValue(api_track_value);
}

}  // namespace orbit_capture_client
