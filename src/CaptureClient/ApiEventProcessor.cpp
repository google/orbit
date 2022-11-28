// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/ApiEventProcessor.h"

#include <absl/hash/hash.h>

#include <algorithm>
#include <string>

#include "ApiInterface/Orbit.h"
#include "ApiUtils/EncodedString.h"
#include "ClientData/ApiStringEvent.h"
#include "ClientData/ApiTrackValue.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_capture_client {

using orbit_client_data::ApiStringEvent;
using orbit_client_data::ApiTrackValue;
using orbit_client_protos::TimerInfo;
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
  ApiTrackValue api_track_value{grpc_api_track_double.pid(), grpc_api_track_double.tid(),
                                grpc_api_track_double.timestamp_ns(),
                                DecodeString(grpc_api_track_double), grpc_api_track_double.data()};

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackFloat(
    const orbit_grpc_protos::ApiTrackFloat& grpc_api_track_float) {
  ApiTrackValue api_track_value{
      grpc_api_track_float.pid(), grpc_api_track_float.tid(), grpc_api_track_float.timestamp_ns(),
      DecodeString(grpc_api_track_float), static_cast<double>(grpc_api_track_float.data())};

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackInt(
    const orbit_grpc_protos::ApiTrackInt& grpc_api_track_int) {
  ApiTrackValue api_track_value{grpc_api_track_int.pid(), grpc_api_track_int.tid(),
                                grpc_api_track_int.timestamp_ns(), DecodeString(grpc_api_track_int),
                                static_cast<double>(grpc_api_track_int.data())};

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackInt64(
    const orbit_grpc_protos::ApiTrackInt64& grpc_api_track_int64) {
  ApiTrackValue api_track_value{
      grpc_api_track_int64.pid(), grpc_api_track_int64.tid(), grpc_api_track_int64.timestamp_ns(),
      DecodeString(grpc_api_track_int64), static_cast<double>(grpc_api_track_int64.data())};

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackUint(
    const orbit_grpc_protos::ApiTrackUint& grpc_api_track_uint) {
  ApiTrackValue api_track_value{
      grpc_api_track_uint.pid(), grpc_api_track_uint.tid(), grpc_api_track_uint.timestamp_ns(),
      DecodeString(grpc_api_track_uint), static_cast<double>(grpc_api_track_uint.data())};

  capture_listener_->OnApiTrackValue(api_track_value);
}

void ApiEventProcessor::ProcessApiTrackUint64(
    const orbit_grpc_protos::ApiTrackUint64& grpc_api_track_uint64) {
  ApiTrackValue api_track_value{grpc_api_track_uint64.pid(), grpc_api_track_uint64.tid(),
                                grpc_api_track_uint64.timestamp_ns(),
                                DecodeString(grpc_api_track_uint64),
                                static_cast<double>(grpc_api_track_uint64.data())};

  capture_listener_->OnApiTrackValue(api_track_value);
}

}  // namespace orbit_capture_client
