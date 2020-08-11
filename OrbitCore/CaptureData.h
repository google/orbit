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
      std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
          selected_functions)
      : selected_functions_{std::move(selected_functions)} {}

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

 private:
  // TODO(kuebler): make them raw pointers at some point
  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
      selected_functions_;
};

#endif  // ORBIT_CORE_CAPTURE_DATA_H_
