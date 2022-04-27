// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_TOKEN_H_
#define ORBIT_BASE_STOP_TOKEN_H_

#include <memory>

#include "OrbitBase/SharedState.h"

namespace orbit_base {

// Forward declaration for befriending
class StopSource;

class StopToken {
  friend StopSource;

 public:
  explicit StopToken() = default;

  // Returns true if StopToken has a stop-state, otherwise false. If StopToken has a stop-state and
  // a stop has already been requested, this function still returns true.
  [[nodiscard]] bool IsStopPossible() const { return shared_stop_state_.use_count() > 0; }
  [[nodiscard]] bool IsStopRequested() const;

 private:
  explicit StopToken(std::shared_ptr<orbit_base_internal::SharedState<void>> shared_state)
      : shared_stop_state_(std::move(shared_state)) {}

  std::shared_ptr<orbit_base_internal::SharedState<void>> shared_stop_state_;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_STOP_TOKEN_H_