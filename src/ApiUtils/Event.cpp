// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ApiUtils/Event.h"

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_api {

namespace {

template <typename CaptureEventT>
inline void SetMetaData(const ApiEventMetaData& meta_data, CaptureEventT* out) {
  out->set_pid(meta_data.pid);
  out->set_tid(meta_data.tid);
  out->set_timestamp_ns(meta_data.timestamp_ns);
}
}  // namespace

template <typename CaptureEventT>
inline void SetEncodedName(const ApiEncodedString& encoded_name, CaptureEventT* out) {
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

void ApiScopeStart::CopyToGrpcProto(orbit_grpc_protos::ApiScopeStart* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_color_rgba(color_rgba);
  grpc_proto->set_group_id(group_id);
  grpc_proto->set_address_in_function(address_in_function);
}

void ApiScopeStop::CopyToGrpcProto(orbit_grpc_protos::ApiScopeStop* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
}

void ApiScopeStartAsync::CopyToGrpcProto(orbit_grpc_protos::ApiScopeStartAsync* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_color_rgba(color_rgba);
  grpc_proto->set_id(id);
  grpc_proto->set_address_in_function(address_in_function);
}

void ApiScopeStopAsync::CopyToGrpcProto(orbit_grpc_protos::ApiScopeStopAsync* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  grpc_proto->set_id(id);
}

void ApiStringEvent::CopyToGrpcProto(orbit_grpc_protos::ApiStringEvent* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_id(id);
  grpc_proto->set_color_rgba(color_rgba);
}

void ApiTrackInt::CopyToGrpcProto(orbit_grpc_protos::ApiTrackInt* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_data(data);
  grpc_proto->set_color_rgba(color_rgba);
}

void ApiTrackInt64::CopyToGrpcProto(orbit_grpc_protos::ApiTrackInt64* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_data(data);
  grpc_proto->set_color_rgba(color_rgba);
}

void ApiTrackUint::CopyToGrpcProto(orbit_grpc_protos::ApiTrackUint* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_data(data);
  grpc_proto->set_color_rgba(color_rgba);
}

void ApiTrackUint64::CopyToGrpcProto(orbit_grpc_protos::ApiTrackUint64* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_data(data);
  grpc_proto->set_color_rgba(color_rgba);
}

void ApiTrackDouble::CopyToGrpcProto(orbit_grpc_protos::ApiTrackDouble* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_data(data);
  grpc_proto->set_color_rgba(color_rgba);
}

void ApiTrackFloat::CopyToGrpcProto(orbit_grpc_protos::ApiTrackFloat* grpc_proto) const {
  SetMetaData(meta_data, grpc_proto);
  SetEncodedName(encoded_name, grpc_proto);
  grpc_proto->set_data(data);
  grpc_proto->set_color_rgba(color_rgba);
}

void FillProducerCaptureEventFromApiEvent(const ApiScopeStart& scope_start,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_start();
  scope_start.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiScopeStop& scope_stop,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_stop();
  scope_stop.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiScopeStartAsync& scope_start_async,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_start_async();
  scope_start_async.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiScopeStopAsync& scope_stop_async,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_stop_async();
  scope_stop_async.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiStringEvent& string_event,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_string_event();
  string_event.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiTrackDouble& track_double,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_double();
  track_double.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiTrackFloat& track_float,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_float();
  track_float.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiTrackInt& track_int,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_int();
  track_int.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiTrackInt64& track_int64,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_int64();
  track_int64.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiTrackUint& track_uint,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_uint();
  track_uint.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(const ApiTrackUint64& track_uint64,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_uint64();
  track_uint64.CopyToGrpcProto(api_event);
}

void FillProducerCaptureEventFromApiEvent(
    const std::monostate& /*monostate*/,
    orbit_grpc_protos::ProducerCaptureEvent* /*capture_event*/) {
  ORBIT_UNREACHABLE();
}

}  // namespace orbit_api