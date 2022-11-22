// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_UTILS_EVENT_H_
#define ORBIT_API_UTILS_EVENT_H_

#include <algorithm>
#include <cstdint>
#include <variant>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "EncodedString.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "absl/base/casts.h"

// We don't want to store protos in the LockFreeApiEventProducer's buffer, as they introduce
// expensive and unnecessary indirections and allocations. Therefore, we use the a std::variant of
// the following structs. The structs must be kept up-to-date with the protos in capture.proto.
namespace orbit_api {

struct ApiEventMetaData {
  ApiEventMetaData(uint32_t pid, uint32_t tid, uint64_t timestamp_ns)
      : pid(pid), tid(tid), timestamp_ns(timestamp_ns) {}
  uint32_t pid = 0;
  uint32_t tid = 0;
  uint64_t timestamp_ns = 0;
};

struct ApiEncodedString {
  ApiEncodedString(const char* name) { EncodeString(name, this); }
  void set_encoded_name_1(uint64_t value) { encoded_name_1 = value; }
  void set_encoded_name_2(uint64_t value) { encoded_name_2 = value; }
  void set_encoded_name_3(uint64_t value) { encoded_name_3 = value; }
  void set_encoded_name_4(uint64_t value) { encoded_name_4 = value; }
  void set_encoded_name_5(uint64_t value) { encoded_name_5 = value; }
  void set_encoded_name_6(uint64_t value) { encoded_name_6 = value; }
  void set_encoded_name_7(uint64_t value) { encoded_name_7 = value; }
  void set_encoded_name_8(uint64_t value) { encoded_name_8 = value; }
  void add_encoded_name_additional(uint64_t value) { encoded_name_additional.push_back(value); }
  uint64_t encoded_name_1 = 0;
  uint64_t encoded_name_2 = 0;
  uint64_t encoded_name_3 = 0;
  uint64_t encoded_name_4 = 0;
  uint64_t encoded_name_5 = 0;
  uint64_t encoded_name_6 = 0;
  uint64_t encoded_name_7 = 0;
  uint64_t encoded_name_8 = 0;
  std::vector<uint64_t> encoded_name_additional{};
};

struct ApiScopeStart {
  ApiScopeStart(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name,
                orbit_api_color color_rgba = kOrbitColorAuto, uint64_t group_id = 0,
                uint64_t address_in_function = 0)
      : meta_data(pid, tid, timestamp_ns),
        encoded_name(name),
        group_id(group_id),
        address_in_function(address_in_function),
        color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiScopeStart* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  uint64_t group_id = 0;
  uint64_t address_in_function = 0;
  uint32_t color_rgba = 0;
};

struct ApiScopeStop {
  ApiScopeStop(uint32_t pid, uint32_t tid, uint64_t timestamp_ns)
      : meta_data(pid, tid, timestamp_ns) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiScopeStop* grpc_proto) const;

  ApiEventMetaData meta_data;
};

struct ApiScopeStartAsync {
  ApiScopeStartAsync(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name,
                     uint64_t id, orbit_api_color color_rgba = kOrbitColorAuto,
                     uint64_t address_in_function = 0)
      : meta_data(pid, tid, timestamp_ns),
        encoded_name(name),
        id(id),
        address_in_function(address_in_function),
        color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiScopeStartAsync* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  uint64_t id = 0;
  uint64_t address_in_function = 0;
  uint32_t color_rgba = 0;
};

struct ApiScopeStopAsync {
  ApiScopeStopAsync(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, uint64_t id)
      : meta_data(pid, tid, timestamp_ns), id(id) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiScopeStopAsync* grpc_proto) const;

  ApiEventMetaData meta_data;
  uint64_t id = 0;
};

struct ApiStringEvent {
  ApiStringEvent(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, uint64_t id,
                 orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), id(id), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiStringEvent* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  uint64_t id = 0;
  uint32_t color_rgba = 0;
};

struct ApiTrackInt {
  ApiTrackInt(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, int32_t data,
              orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiTrackInt* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  int32_t data = 0;
  uint32_t color_rgba = 0;
};

struct ApiTrackInt64 {
  ApiTrackInt64(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, int64_t data,
                orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiTrackInt64* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  int64_t data = 0;
  uint32_t color_rgba = 0;
};

struct ApiTrackUint {
  ApiTrackUint(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, uint32_t data,
               orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiTrackUint* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  uint32_t data = 0;
  uint32_t color_rgba = 0;
};

struct ApiTrackUint64 {
  ApiTrackUint64(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, uint64_t data,
                 orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiTrackUint64* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  uint64_t data = 0;
  uint32_t color_rgba = 0;
};

struct ApiTrackDouble {
  ApiTrackDouble(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, double data,
                 orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiTrackDouble* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  double data = 0.;
  uint32_t color_rgba = 0;
};

struct ApiTrackFloat {
  ApiTrackFloat(uint32_t pid, uint32_t tid, uint64_t timestamp_ns, const char* name, float data,
                orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  void CopyToGrpcProto(orbit_grpc_protos::ApiTrackFloat* grpc_proto) const;

  ApiEventMetaData meta_data;
  ApiEncodedString encoded_name;
  float data = 0.f;
  uint32_t color_rgba = 0;
};

// Used in `LockFreeApiEventProducer`. The `std::monostate` is required make this variant default
// constructable. However, real (fully instantiated) values will never be of type `std::monostate`.
using ApiEventVariant =
    std::variant<std::monostate, ApiScopeStart, ApiScopeStop, ApiScopeStartAsync, ApiScopeStopAsync,
                 ApiStringEvent, ApiTrackDouble, ApiTrackFloat, ApiTrackInt, ApiTrackInt64,
                 ApiTrackUint, ApiTrackUint64>;

void FillProducerCaptureEventFromApiEvent(const ApiScopeStart& scope_start,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiScopeStop& scope_stop,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiScopeStartAsync& scope_start_async,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiScopeStopAsync& scope_stop_async,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiStringEvent& string_event,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiTrackDouble& track_double,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiTrackFloat& track_float,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiTrackInt& track_int,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiTrackInt64& track_int64,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiTrackUint& track_uint,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

void FillProducerCaptureEventFromApiEvent(const ApiTrackUint64& track_uint64,
                                          orbit_grpc_protos::ProducerCaptureEvent* capture_event);

// The variant type `ApiEventVariant` requires to contain `std::monostate` in order to be default-
// constructable. However, that state is never expected to be called in the visitor.
void FillProducerCaptureEventFromApiEvent(
    const std::monostate& /*monostate*/,
    orbit_grpc_protos::ProducerCaptureEvent* /*capture_event*/);

}  // namespace orbit_api

#endif  // ORBIT_API_UTILS_EVENT_H_
