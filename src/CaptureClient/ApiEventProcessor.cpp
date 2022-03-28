// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/ApiEventProcessor.h"

#include "ApiUtils/EncodedString.h"
#include "ClientData/ApiStringEvent.h"
#include "OrbitBase/Logging.h"

namespace orbit_capture_client {

using orbit_client_data::ApiStringEvent;
using orbit_client_protos::ApiTrackValue;
using orbit_client_protos::TimerInfo;
using orbit_grpc_protos::ApiEvent;
using orbit_grpc_protos::ApiScopeStart;
using orbit_grpc_protos::ApiScopeStartAsync;

namespace {
template <typename Source>
inline std::string DecodeString(const Source& encoded_source) {
  return orbit_api::DecodeString(encoded_source.encoded_name_1(), encoded_source.encoded_name_2(),
                                 encoded_source.encoded_name_3(), encoded_source.encoded_name_4(),
                                 encoded_source.encoded_name_5(), encoded_source.encoded_name_6(),
                                 encoded_source.encoded_name_7(), encoded_source.encoded_name_8(),
                                 encoded_source.encoded_name_additional().data(),
                                 encoded_source.encoded_name_additional_size());
}

void EncodedColorToColor(uint32_t encoded_color, orbit_client_protos::Color* color) {
  color->set_red((encoded_color & (0xff << 24)) >> 24);
  color->set_green((encoded_color & (0xff << 16)) >> 16);
  color->set_blue((encoded_color & (0xff << 8)) >> 8);
  color->set_alpha(encoded_color & 0xff);
}
}  // namespace

ApiEventProcessor::ApiEventProcessor(CaptureListener* listener) : capture_listener_(listener) {
  ORBIT_CHECK(listener != nullptr);
}

void ApiEventProcessor::ProcessApiEventLegacy(const orbit_grpc_protos::ApiEvent& grpc_api_event) {
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
  ProcessApiEventLegacy(api_event);
}

void ApiEventProcessor::ProcessApiEventLegacy(const orbit_api::ApiEvent& api_event) {
  orbit_api::EventType event_type = api_event.Type();

  switch (event_type) {
    case orbit_api::kScopeStart:
      ProcessStartEventLegacy(api_event);
      break;
    case orbit_api::kScopeStop:
      ProcessStopEventLegacy(api_event);
      break;
    case orbit_api::kScopeStartAsync:
      ProcessAsyncStartEventLegacy(api_event);
      break;
    case orbit_api::kScopeStopAsync:
      ProcessAsyncStopEventLegacy(api_event);
      break;
    case orbit_api::kString:
      ProcessStringEventLegacy(api_event);
      break;
    case orbit_api::kTrackInt:
    case orbit_api::kTrackInt64:
    case orbit_api::kTrackUint:
    case orbit_api::kTrackUint64:
    case orbit_api::kTrackFloat:
    case orbit_api::kTrackDouble:
      ProcessTrackingEventLegacy(api_event);
      break;
    case orbit_api::kNone:
      ORBIT_UNREACHABLE();
  }
}

void ApiEventProcessor::ProcessStartEventLegacy(const orbit_api::ApiEvent& api_event) {
  synchronous_legacy_event_stack_by_tid_[api_event.tid].emplace_back(api_event);
}

void ApiEventProcessor::ProcessStopEventLegacy(const orbit_api::ApiEvent& api_event) {
  std::vector<orbit_api::ApiEvent>& event_stack =
      synchronous_legacy_event_stack_by_tid_[api_event.tid];
  if (event_stack.empty()) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  const orbit_api::ApiEvent& start_event = event_stack.back();

  TimerInfo timer_info;
  timer_info.set_start(start_event.timestamp_ns);
  timer_info.set_end(api_event.timestamp_ns);
  timer_info.set_process_id(api_event.pid);
  timer_info.set_thread_id(api_event.tid);
  timer_info.set_depth(event_stack.size() - 1);
  timer_info.set_type(TimerInfo::kApiScope);

  if (start_event.encoded_event.event.color != kOrbitColorAuto) {
    EncodedColorToColor(start_event.encoded_event.event.color, timer_info.mutable_color());
  }

  timer_info.set_api_scope_name(start_event.encoded_event.event.name);

  timer_info.set_group_id(0);
  timer_info.set_address_in_function(0);

  capture_listener_->OnTimer(timer_info);
  event_stack.pop_back();
}

void ApiEventProcessor::ProcessAsyncStartEventLegacy(const orbit_api::ApiEvent& api_event) {
  const uint64_t event_id = api_event.encoded_event.event.data;
  asynchronous_legacy_events_by_id_[event_id] = api_event;
}

void ApiEventProcessor::ProcessAsyncStopEventLegacy(const orbit_api::ApiEvent& api_event) {
  const uint64_t event_id = api_event.encoded_event.event.data;
  if (asynchronous_legacy_events_by_id_.count(event_id) == 0) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  orbit_api::ApiEvent& start_event = asynchronous_legacy_events_by_id_[event_id];
  TimerInfo timer_info;
  timer_info.set_start(start_event.timestamp_ns);
  timer_info.set_end(api_event.timestamp_ns);
  timer_info.set_process_id(api_event.pid);
  timer_info.set_thread_id(api_event.tid);
  timer_info.set_depth(0);
  timer_info.set_type(TimerInfo::kApiScopeAsync);

  if (start_event.encoded_event.event.color != kOrbitColorAuto) {
    EncodedColorToColor(start_event.encoded_event.event.color, timer_info.mutable_color());
  }
  timer_info.set_api_async_scope_id(api_event.encoded_event.event.data);
  timer_info.set_api_scope_name(start_event.encoded_event.event.name);

  timer_info.set_address_in_function(0);

  capture_listener_->OnTimer(timer_info);

  asynchronous_legacy_events_by_id_.erase(event_id);
}

void ApiEventProcessor::ProcessStringEventLegacy(const orbit_api::ApiEvent& api_event) {
  ApiStringEvent api_string_event{api_event.encoded_event.event.data,
                                  api_event.encoded_event.event.name, /*should_concatenate=*/true};
  capture_listener_->OnApiStringEvent(api_string_event);
}

void ApiEventProcessor::ProcessTrackingEventLegacy(const orbit_api::ApiEvent& api_event) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(api_event.pid);
  api_track_value.set_thread_id(api_event.tid);
  api_track_value.set_timestamp_ns(api_event.timestamp_ns);

  api_track_value.set_name(api_event.encoded_event.event.name);

  switch (api_event.Type()) {
    case orbit_api::kTrackInt:
      api_track_value.set_data_int(orbit_api::Decode<int32_t>(api_event.encoded_event.event.data));
      break;
    case orbit_api::kTrackInt64:
      api_track_value.set_data_int64(
          orbit_api::Decode<int64_t>(api_event.encoded_event.event.data));
      break;
    case orbit_api::kTrackUint:
      api_track_value.set_data_uint(
          orbit_api::Decode<uint32_t>(api_event.encoded_event.event.data));
      break;
    case orbit_api::kTrackUint64:
      api_track_value.set_data_uint64(
          orbit_api::Decode<uint64_t>(api_event.encoded_event.event.data));
      break;
    case orbit_api::kTrackFloat:
      api_track_value.set_data_float(orbit_api::Decode<float>(api_event.encoded_event.event.data));
      break;
    case orbit_api::kTrackDouble:
      api_track_value.set_data_double(
          orbit_api::Decode<double>(api_event.encoded_event.event.data));
      break;

    case orbit_api::kNone:
    case orbit_api::kScopeStart:
    case orbit_api::kScopeStop:
    case orbit_api::kScopeStartAsync:
    case orbit_api::kScopeStopAsync:
    case orbit_api::kString:
      ORBIT_UNREACHABLE();
  }

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiScopeStart(
    const orbit_grpc_protos::ApiScopeStart& api_scope_start) {
  synchronous_scopes_stack_by_tid_[api_scope_start.tid()].emplace_back(api_scope_start);
}

void ApiEventProcessor::ProcessApiScopeStop(
    const orbit_grpc_protos::ApiScopeStop& grpc_api_scope_stop) {
  std::vector<ApiScopeStart>& event_stack =
      synchronous_scopes_stack_by_tid_[grpc_api_scope_stop.tid()];
  if (event_stack.empty()) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  const ApiScopeStart& start_event = event_stack.back();
  TimerInfo timer_info;

  timer_info.set_start(start_event.timestamp_ns());
  timer_info.set_end(grpc_api_scope_stop.timestamp_ns());
  timer_info.set_process_id(grpc_api_scope_stop.pid());
  timer_info.set_thread_id(grpc_api_scope_stop.tid());
  timer_info.set_depth(event_stack.size() - 1);
  timer_info.set_type(TimerInfo::kApiScope);

  if (start_event.color_rgba() != kOrbitColorAuto) {
    EncodedColorToColor(start_event.color_rgba(), timer_info.mutable_color());
  }

  timer_info.set_group_id(start_event.group_id());
  timer_info.set_address_in_function(start_event.address_in_function());

  timer_info.set_api_scope_name(DecodeString(start_event));

  capture_listener_->OnTimer(timer_info);
  event_stack.pop_back();
}

void ApiEventProcessor::ProcessApiScopeStartAsync(
    const orbit_grpc_protos::ApiScopeStartAsync& grpc_api_scope_start_async) {
  asynchronous_scopes_by_id_[grpc_api_scope_start_async.id()] = grpc_api_scope_start_async;
}

void ApiEventProcessor::ProcessApiScopeStopAsync(
    const orbit_grpc_protos::ApiScopeStopAsync& grpc_api_scope_stop_async) {
  uint64_t event_id = grpc_api_scope_stop_async.id();
  if (!asynchronous_scopes_by_id_.contains(event_id)) {
    // We received a stop event with no matching start event, which is possible if the capture was
    // started between the event's start and stop times.
    return;
  }

  ApiScopeStartAsync& start_event = asynchronous_scopes_by_id_[event_id];
  TimerInfo timer_info;

  timer_info.set_start(start_event.timestamp_ns());
  timer_info.set_end(grpc_api_scope_stop_async.timestamp_ns());
  timer_info.set_process_id(grpc_api_scope_stop_async.pid());
  timer_info.set_thread_id(grpc_api_scope_stop_async.tid());
  timer_info.set_depth(0);
  timer_info.set_type(TimerInfo::kApiScopeAsync);

  if (start_event.color_rgba() != kOrbitColorAuto) {
    EncodedColorToColor(start_event.color_rgba(), timer_info.mutable_color());
  }

  timer_info.set_api_async_scope_id(event_id);
  timer_info.set_address_in_function(start_event.address_in_function());

  timer_info.set_api_scope_name(DecodeString(start_event));

  capture_listener_->OnTimer(timer_info);
  asynchronous_scopes_by_id_.erase(event_id);
}

void ApiEventProcessor::ProcessApiStringEvent(
    const orbit_grpc_protos::ApiStringEvent& grpc_api_string_event) {
  ApiStringEvent api_string_event{grpc_api_string_event.id(), DecodeString(grpc_api_string_event),
                                  /*should_concatenate=*/false};
  capture_listener_->OnApiStringEvent(api_string_event);
}

void ApiEventProcessor::ProcessApiTrackDouble(
    const orbit_grpc_protos::ApiTrackDouble& grpc_api_track_double) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(grpc_api_track_double.pid());
  api_track_value.set_thread_id(grpc_api_track_double.tid());
  api_track_value.set_timestamp_ns(grpc_api_track_double.timestamp_ns());
  api_track_value.set_data_double(grpc_api_track_double.data());

  api_track_value.set_name(DecodeString(grpc_api_track_double));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackFloat(
    const orbit_grpc_protos::ApiTrackFloat& grpc_api_track_float) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(grpc_api_track_float.pid());
  api_track_value.set_thread_id(grpc_api_track_float.tid());
  api_track_value.set_timestamp_ns(grpc_api_track_float.timestamp_ns());
  api_track_value.set_data_float(grpc_api_track_float.data());

  api_track_value.set_name(DecodeString(grpc_api_track_float));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackInt(
    const orbit_grpc_protos::ApiTrackInt& grpc_api_track_int) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(grpc_api_track_int.pid());
  api_track_value.set_thread_id(grpc_api_track_int.tid());
  api_track_value.set_timestamp_ns(grpc_api_track_int.timestamp_ns());
  api_track_value.set_data_int(grpc_api_track_int.data());

  api_track_value.set_name(DecodeString(grpc_api_track_int));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackInt64(
    const orbit_grpc_protos::ApiTrackInt64& grpc_api_track_int64) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(grpc_api_track_int64.pid());
  api_track_value.set_thread_id(grpc_api_track_int64.tid());
  api_track_value.set_timestamp_ns(grpc_api_track_int64.timestamp_ns());
  api_track_value.set_data_int64(grpc_api_track_int64.data());

  api_track_value.set_name(DecodeString(grpc_api_track_int64));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackUint(
    const orbit_grpc_protos::ApiTrackUint& grpc_api_track_uint) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(grpc_api_track_uint.pid());
  api_track_value.set_thread_id(grpc_api_track_uint.tid());
  api_track_value.set_timestamp_ns(grpc_api_track_uint.timestamp_ns());
  api_track_value.set_data_uint(grpc_api_track_uint.data());

  api_track_value.set_name(DecodeString(grpc_api_track_uint));

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackUint64(
    const orbit_grpc_protos::ApiTrackUint64& grpc_api_track_uint64) {
  ApiTrackValue api_track_value;
  api_track_value.set_process_id(grpc_api_track_uint64.pid());
  api_track_value.set_thread_id(grpc_api_track_uint64.tid());
  api_track_value.set_timestamp_ns(grpc_api_track_uint64.timestamp_ns());
  api_track_value.set_data_uint64(grpc_api_track_uint64.data());

  api_track_value.set_name(DecodeString(grpc_api_track_uint64));

  capture_listener_->OnApiTrackValue(api_track_value);
}

}  // namespace orbit_capture_client
