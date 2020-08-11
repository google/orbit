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
  void SetSelectedFunctions(
      std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
          selected_functions) {
    selected_functions_ = std::move(selected_functions);
  }

  void AddSelectedFunction(
      const std::shared_ptr<orbit_client_protos::FunctionInfo>& function) {
    selected_functions_.push_back(function);
  }

  [[nodiscard]] const std::vector<
      std::shared_ptr<orbit_client_protos::FunctionInfo>>&
  GetSelectedFunctions() {
    return selected_functions_;
  }

 private:
  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
      selected_functions_;
};

#endif  // ORBIT_CORE_CAPTURE_DATA_H_
