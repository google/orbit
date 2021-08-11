// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LockFreeApiEventProducer.h"

namespace orbit_api {
namespace {

inline void CreateCaptureEvent(const ApiScopeStart& scope_start,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_start();
  scope_start.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiScopeStop& scope_stop,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_stop();
  scope_stop.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiScopeStartAsync& scope_start_async,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_start_async();
  scope_start_async.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiScopeStopAsync& scope_stop_async,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_scope_stop_async();
  scope_stop_async.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiStringEvent& string_event,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_string_event();
  string_event.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiTrackDouble& track_double,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_double();
  track_double.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiTrackFloat& track_float,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_float();
  track_float.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiTrackInt& track_int,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_int();
  track_int.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiTrackInt64& track_int64,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_int64();
  track_int64.CopyToGrpcProto(api_event);
}

inline void CreateCaptureEvent(const ApiTrackUint& track_uint,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_uint();
  track_uint.CopyToGrpcProto(api_event);
}
inline void CreateCaptureEvent(const ApiTrackUint64& track_uint64,
                               orbit_grpc_protos::ProducerCaptureEvent* capture_event) {
  auto* api_event = capture_event->mutable_api_track_uint64();
  track_uint64.CopyToGrpcProto(api_event);
}

// The variant type `ApiEventVariant` requires to contain `std::monostate` in order to be default-
// constructable. However, that state is never expected to be called in the visitor.
inline void CreateCaptureEvent(const std::monostate& /*unused*/,
                               orbit_grpc_protos::ProducerCaptureEvent* /*unused*/) {
  UNREACHABLE();
}

}  // namespace

orbit_grpc_protos::ProducerCaptureEvent* LockFreeApiEventProducer::TranslateIntermediateEvent(
    ApiEventVariant&& raw_api_event, google::protobuf::Arena* arena) {
  auto* capture_event =
      google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);

  std::visit([capture_event](const auto& event) { CreateCaptureEvent(event, capture_event); },
             raw_api_event);

  return capture_event;
}
}  // namespace orbit_api