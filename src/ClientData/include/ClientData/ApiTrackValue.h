// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_API_TRACK_VALUE_H_
#define CLIENT_DATA_API_TRACK_VALUE_H_

#include <cstdint>
#include <string>
#include <utility>

namespace orbit_client_data {

// Represents a value tracking event from our manual instrumentation API (Orbit.h). It contains the
// name of the track to which the data point should be added, and the data point itself.
// Note that we only display "double" values in the UI. Values of other types need to be converted
// to double.
// See `orbit_grpc_protos::ApiTrackValue*` protos.
class ApiTrackValue {
 public:
  ApiTrackValue() = delete;
  ApiTrackValue(uint32_t process_id, uint32_t thread_id, const uint64_t& timestamp_ns,
                std::string track_name, double value)
      : process_id_(process_id),
        thread_id_(thread_id),
        timestamp_ns_(timestamp_ns),
        track_name_(std::move(track_name)),
        value_(value) {}

  [[nodiscard]] uint32_t process_id() const { return process_id_; }
  [[nodiscard]] uint32_t thread_id() const { return thread_id_; }
  [[nodiscard]] uint64_t timestamp_ns() const { return timestamp_ns_; }
  [[nodiscard]] const std::string& track_name() const { return track_name_; }
  [[nodiscard]] double value() const { return value_; }

 private:
  uint32_t process_id_;
  uint32_t thread_id_;
  uint64_t timestamp_ns_;
  std::string track_name_;
  double value_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_API_TRACK_VALUE_H_
