// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_SOURCE_H_
#define ORBIT_BASE_STOP_SOURCE_H_

#include <memory>

#include "OrbitBase/SharedState.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

class StopSource {
 public:
  explicit StopSource()
      : shared_state_(std::make_shared<orbit_base_internal::SharedState<void>>()) {}

  // Returns true if StopSource has a stop-state, otherwise false. If StopSource has a stop-state
  // and a stop has already been requested, this function still returns true.
  [[nodiscard]] bool IsStopPossible() { return shared_state_.use_count() > 0; }
  void RequestStop();
  [[nodiscard]] StopToken GetStopToken();

 private:
  std::shared_ptr<orbit_base_internal::SharedState<void>> shared_state_;
};

}  // namespace orbit_base

#endif