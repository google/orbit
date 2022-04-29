// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_SOURCE_H_
#define ORBIT_BASE_STOP_SOURCE_H_

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitBase/StopState.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

// StopSource together with StopToken is designed to enable thread safe task cancellation. Create a
// StopSource in the thread that starts a new thread and give that new thread a StopToken (from
// StopSource::GetStopToken). Then the a stop can be requested from the outer thread by calling
// StopSource::RequestStop
class StopSource {
 public:
  explicit StopSource() : shared_stop_state_(std::make_shared<orbit_base_internal::StopState>()) {}

  // Returns true if StopSource has a stop-state, otherwise false. If StopSource has a stop-state
  // and a stop has already been requested, this function still returns true.
  [[nodiscard]] bool IsStopPossible() const { return shared_stop_state_.use_count() > 0; }
  void RequestStop() {
    ORBIT_CHECK(IsStopPossible());
    shared_stop_state_->Stop();
  }
  [[nodiscard]] StopToken GetStopToken() const {
    ORBIT_CHECK(IsStopPossible());
    return StopToken(shared_stop_state_);
  }

 private:
  std::shared_ptr<orbit_base_internal::StopState> shared_stop_state_;
};

}  // namespace orbit_base

#endif