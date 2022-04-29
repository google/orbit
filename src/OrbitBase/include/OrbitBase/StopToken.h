// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_TOKEN_H_
#define ORBIT_BASE_STOP_TOKEN_H_

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitBase/StopState.h"

namespace orbit_base {

// StopToken together with StopSource is designed to enable thread safe task cancellation. Create a
// StopToken from StopSource::GetStopToken and it into a thread on creation. Then check periodically
// whether StopToken::IsStopRequested, to see if a stop has been requested by the corresponding
// StopSource.
class StopToken {
  friend class StopSource;

 public:
  // Returns true if StopToken has a stop-state, otherwise false. If StopToken has a stop-state and
  // a stop has already been requested, this function still returns true.
  [[nodiscard]] bool IsStopPossible() const { return shared_stop_state_ != nullptr; }
  [[nodiscard]] bool IsStopRequested() const {
    ORBIT_CHECK(IsStopPossible());
    return shared_stop_state_->IsStopped();
  }

 private:
  explicit StopToken(std::shared_ptr<orbit_base_internal::StopState> shared_stop_state)
      : shared_stop_state_(std::move(shared_stop_state)) {}

  std::shared_ptr<orbit_base_internal::StopState> shared_stop_state_;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_STOP_TOKEN_H_