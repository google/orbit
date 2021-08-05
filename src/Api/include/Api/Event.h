// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_API_EVENT_H_
#define ORBIT_API_EVENT_H_

#include <cstdint>
#include <variant>
#include <vector>

#include "EncodedString.h"
#include "Orbit.h"
#include "OrbitBase/Logging.h"
#include "absl/base/casts.h"

// We don't want to store protos in the LockFreeApiEventProducer's buffer, as they introduce
// expensive and unnessesary indirections and allocations. Therefore, we use the a std::variant of
// the following structs. The structs must be kept up-to-date with the protos in capture.proto.
namespace orbit_api {

struct ApiEventMetaData {
  ApiEventMetaData(int32_t pid, int32_t tid, uint64_t timestamp_ns)
      : pid(pid), tid(tid), timestamp_ns(timestamp_ns) {}
  int32_t pid{};
  int32_t tid{};
  uint64_t timestamp_ns{};
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
  uint64_t encoded_name_1{};
  uint64_t encoded_name_2{};
  uint64_t encoded_name_3{};
  uint64_t encoded_name_4{};
  uint64_t encoded_name_5{};
  uint64_t encoded_name_6{};
  uint64_t encoded_name_7{};
  uint64_t encoded_name_8{};
  std::vector<uint64_t> encoded_name_additional{};
};

struct ApiScopeStart {
  ApiScopeStart(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name,
                orbit_api_color color_rgba = kOrbitColorAuto, uint64_t group_id = 0,
                uint64_t address_in_function = 0)
      : meta_data(pid, tid, timestamp_ns),
        group_id(group_id),
        address_in_function(address_in_function),
        encoded_name(name),
        color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;
  uint64_t group_id{};
  uint64_t address_in_function{};

  ApiEncodedString encoded_name;

  uint32_t color_rgba{};
};

struct ApiScopeStop {
  ApiScopeStop(int32_t pid, int32_t tid, uint64_t timestamp_ns)
      : meta_data(pid, tid, timestamp_ns) {}

  ApiEventMetaData meta_data;
};

struct ApiScopeStartAsync {
  ApiScopeStartAsync(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, uint64_t id,
                     orbit_api_color color_rgba = kOrbitColorAuto, uint64_t address_in_function = 0)
      : meta_data(pid, tid, timestamp_ns),
        id(id),
        address_in_function(address_in_function),
        encoded_name(name),
        color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;
  uint64_t id{};
  uint64_t address_in_function{};

  ApiEncodedString encoded_name;

  uint32_t color_rgba{};
};

struct ApiScopeStopAsync {
  ApiScopeStopAsync(int32_t pid, int32_t tid, uint64_t timestamp_ns, uint64_t id)
      : meta_data(pid, tid, timestamp_ns), id(id) {}

  ApiEventMetaData meta_data;
  uint64_t id{};
};

struct ApiStringEvent {
  ApiStringEvent(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, uint64_t id,
                 orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), id(id), encoded_name(name), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;
  uint64_t id{};

  ApiEncodedString encoded_name;
  uint32_t color_rgba{};
};

struct ApiTrackInt {
  ApiTrackInt(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, int32_t data,
              orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;

  ApiEncodedString encoded_name;

  int32_t data{};

  uint32_t color_rgba{};
};

struct ApiTrackInt64 {
  ApiTrackInt64(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, int64_t data,
                orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;

  ApiEncodedString encoded_name;

  int64_t data{};

  uint32_t color_rgba{};
};

struct ApiTrackUint {
  ApiTrackUint(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, uint32_t data,
               orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;

  ApiEncodedString encoded_name;

  uint32_t data{};

  uint32_t color_rgba{};
};

struct ApiTrackUint64 {
  ApiTrackUint64(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, uint64_t data,
                 orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;

  ApiEncodedString encoded_name;

  uint64_t data{};

  uint32_t color_rgba{};
};

struct ApiTrackDouble {
  ApiTrackDouble(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, double data,
                 orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;

  ApiEncodedString encoded_name;

  double data{};

  uint32_t color_rgba{};
};

struct ApiTrackFloat {
  ApiTrackFloat(int32_t pid, int32_t tid, uint64_t timestamp_ns, const char* name, float data,
                orbit_api_color color_rgba = kOrbitColorAuto)
      : meta_data(pid, tid, timestamp_ns), encoded_name(name), data(data), color_rgba(color_rgba) {}

  ApiEventMetaData meta_data;

  ApiEncodedString encoded_name;

  float data{};

  uint32_t color_rgba{};
};

using ApiEventVariant =
    std::variant<std::monostate, ApiScopeStart, ApiScopeStop, ApiScopeStartAsync, ApiScopeStopAsync,
                 ApiStringEvent, ApiTrackDouble, ApiTrackFloat, ApiTrackInt, ApiTrackInt64,
                 ApiTrackUint, ApiTrackUint64>;

}  // namespace orbit_api

#endif  // ORBIT_API_EVENT_H_
