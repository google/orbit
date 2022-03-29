// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_API_TRACK_VALUE_H_
#define CLIENT_DATA_API_TRACK_VALUE_H_

#include <cstdint>
#include <string>
#include <utility>

namespace orbit_client_data {

// Represents a value tracking event from our manual instrumentation API (Orbit.h). It contains a
// name of the track to which the data point should be added, and the data point itself.
// Note that we only display "double" values in the UI. Values of other types need to be converted
// to double.
// See `orbit_grpc_protos::ApiTrackValue*` protos.
class ApiTrackValue {
 private:
 public:
  ApiTrackValue(const uint32_t& process_id, const uint32_t& thread_id, const uint64_t& timestamp_ns,
                std::string name, double data)
      : process_id_(process_id),
        thread_id_(thread_id),
        timestamp_ns_(timestamp_ns),
        name_(std::move(name)),
        data_(data) {}

  [[nodiscard]] uint32_t process_id() const { return process_id_; }
  [[nodiscard]] uint32_t thread_id() const { return thread_id_; }
  [[nodiscard]] uint64_t timestamp_ns() const { return timestamp_ns_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] double data() const { return data_; }

 private:
  uint32_t process_id_;
  uint32_t thread_id_;
  uint64_t timestamp_ns_;
  std::string name_;
  double data_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_API_TRACK_VALUE_H_
