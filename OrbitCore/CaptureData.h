// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CAPTURE_DATA_H_
#define ORBIT_CORE_CAPTURE_DATA_H_

#include <memory>
#include <vector>

#include "capture_data.pb.h"

class CaptureData {
 public:
  explicit CaptureData(
      int32_t process_id, std::string process_name,
      std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
          selected_functions)
      : process_id_{process_id},
        process_name_{std::move(process_name)},
        selected_functions_{std::move(selected_functions)} {}

  explicit CaptureData() = default;
  CaptureData(const CaptureData& other) = default;
  CaptureData& operator=(const CaptureData& other) = default;
  CaptureData(CaptureData&& other) = default;
  CaptureData& operator=(CaptureData&& other) = default;

  [[nodiscard]] const std::vector<
      std::shared_ptr<orbit_client_protos::FunctionInfo>>&
  selected_functions() {
    return selected_functions_;
  }

  [[nodiscard]] int32_t process_id() { return process_id_; }

  [[nodiscard]] const std::string& process_name() { return process_name_; }

  [[nodiscard]] const std::chrono::system_clock::time_point&
  capture_start_time() {
    return capture_start_time_;
  }

 private:
  int32_t process_id_ = -1;
  std::string process_name_;
  // TODO(kuebler): make them raw pointers at some point
  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
      selected_functions_;

  std::chrono::system_clock::time_point capture_start_time_ =
      std::chrono::system_clock::now();
};

#endif  // ORBIT_CORE_CAPTURE_DATA_H_
