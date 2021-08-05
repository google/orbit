// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/Api/LockFreeApiEventProducer.h"

namespace orbit_api {
namespace {
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

template <typename CaptureEvent>
inline void SetMetaData(const ApiEventMetaData& meta_data, CaptureEvent* out) {
  out->set_pid(meta_data.pid);
  out->set_tid(meta_data.tid);
  out->set_timestamp_ns(meta_data.timestamp_ns);
}

template <typename CaptureEvent>
inline void SetEncodedName(const ApiEncodedString& encoded_name, CaptureEvent* out) {
  out->set_encoded_name_1(encoded_name.encoded_name_1);
  out->set_encoded_name_2(encoded_name.encoded_name_2);
  out->set_encoded_name_3(encoded_name.encoded_name_3);
  out->set_encoded_name_4(encoded_name.encoded_name_4);
  out->set_encoded_name_5(encoded_name.encoded_name_5);
  out->set_encoded_name_6(encoded_name.encoded_name_6);
  out->set_encoded_name_7(encoded_name.encoded_name_7);
  out->set_encoded_name_8(encoded_name.encoded_name_8);
  out->mutable_encoded_name_additional()->Add(encoded_name.encoded_name_additional.begin(),
                                              encoded_name.encoded_name_additional.end());
}
}  // namespace

orbit_grpc_protos::ProducerCaptureEvent* LockFreeApiEventProducer::TranslateIntermediateEvent(
    ApiEventVariant&& raw_api_event, google::protobuf::Arena* arena) {
  auto* capture_event =
      google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);

  std::visit(overloaded{[](const std::monostate& /*unused*/) {},
                        [capture_event](const ApiScopeStart& scope_start) {
                          auto* api_event = capture_event->mutable_api_scope_start();
                          SetMetaData(scope_start.meta_data, api_event);
                          SetEncodedName(scope_start.encoded_name, api_event);
                          api_event->set_color_rgba(scope_start.color_rgba);
                          api_event->set_group_id(scope_start.group_id);
                          api_event->set_address_in_function(scope_start.address_in_function);
                        },
                        [capture_event](const ApiScopeStop& scope_stop) {
                          auto* api_event = capture_event->mutable_api_scope_stop();
                          SetMetaData(scope_stop.meta_data, api_event);
                        },
                        [capture_event](const ApiScopeStartAsync& scope_start_async) {
                          auto* api_event = capture_event->mutable_api_scope_start_async();
                          SetMetaData(scope_start_async.meta_data, api_event);
                          SetEncodedName(scope_start_async.encoded_name, api_event);
                          api_event->set_color_rgba(scope_start_async.color_rgba);
                          api_event->set_id(scope_start_async.id);
                          api_event->set_address_in_function(scope_start_async.address_in_function);
                        },
                        [capture_event](const ApiScopeStopAsync& scope_stop_async) {
                          auto* api_event = capture_event->mutable_api_scope_stop_async();
                          SetMetaData(scope_stop_async.meta_data, api_event);
                          api_event->set_id(scope_stop_async.id);
                        },
                        [capture_event](const ApiStringEvent& string_event) {
                          auto* api_event = capture_event->mutable_api_string_event();
                          SetMetaData(string_event.meta_data, api_event);
                          SetEncodedName(string_event.encoded_name, api_event);
                          api_event->set_id(string_event.id);
                          api_event->set_color_rgba(string_event.color_rgba);
                        },
                        [capture_event](const ApiTrackDouble& track_double) {
                          auto* api_event = capture_event->mutable_api_track_double();
                          SetMetaData(track_double.meta_data, api_event);
                          SetEncodedName(track_double.encoded_name, api_event);
                          api_event->set_data(track_double.data);
                          api_event->set_color_rgba(track_double.color_rgba);
                        },
                        [capture_event](const ApiTrackFloat& track_float) {
                          auto* api_event = capture_event->mutable_api_track_float();
                          SetMetaData(track_float.meta_data, api_event);
                          SetEncodedName(track_float.encoded_name, api_event);
                          api_event->set_data(track_float.data);
                          api_event->set_color_rgba(track_float.color_rgba);
                        },
                        [capture_event](const ApiTrackInt& track_int) {
                          auto* api_event = capture_event->mutable_api_track_int();
                          SetMetaData(track_int.meta_data, api_event);
                          SetEncodedName(track_int.encoded_name, api_event);
                          api_event->set_data(track_int.data);
                          api_event->set_color_rgba(track_int.color_rgba);
                        },
                        [capture_event](const ApiTrackInt64& track_int64) {
                          auto* api_event = capture_event->mutable_api_track_int64();
                          SetMetaData(track_int64.meta_data, api_event);
                          SetEncodedName(track_int64.encoded_name, api_event);
                          api_event->set_data(track_int64.data);
                          api_event->set_color_rgba(track_int64.color_rgba);
                        },
                        [capture_event](const ApiTrackUint& track_uint) {
                          auto* api_event = capture_event->mutable_api_track_uint();
                          SetMetaData(track_uint.meta_data, api_event);
                          SetEncodedName(track_uint.encoded_name, api_event);
                          api_event->set_data(track_uint.data);
                          api_event->set_color_rgba(track_uint.color_rgba);
                        },
                        [capture_event](const ApiTrackUint64& track_uint64) {
                          auto* api_event = capture_event->mutable_api_track_uint64();
                          SetMetaData(track_uint64.meta_data, api_event);
                          SetEncodedName(track_uint64.encoded_name, api_event);
                          api_event->set_data(track_uint64.data);
                          api_event->set_color_rgba(track_uint64.color_rgba);
                        }},
             raw_api_event);

  return capture_event;
}
}  // namespace orbit_api