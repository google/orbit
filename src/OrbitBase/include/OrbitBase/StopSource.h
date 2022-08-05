// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_SOURCE_H_
#define ORBIT_BASE_STOP_SOURCE_H_

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

// StopSource together with StopToken is designed to enable thread safe task cancellation. Create a
// StopSource when creating a new task and give that task a StopToken via StopSource::GetStopToken.
// Then the a stop can be requested from any thread by calling StopSource::RequestStop.
class StopSource {
 public:
  // Returns true if StopSource has a stop-state, otherwise false. If StopSource has a stop-state
  // and a stop has already been requested, this function still returns true.
  [[nodiscard]] bool IsStopPossible() const { return promise_.IsValid(); }
  void RequestStop() {
    ORBIT_CHECK(IsStopPossible());
    promise_.MarkFinished();
  }
  [[nodiscard]] StopToken GetStopToken() const {
    ORBIT_CHECK(IsStopPossible());
    return StopToken(promise_.GetFuture());
  }

 private:
  Promise<void> promise_;
};

}  // namespace orbit_base

#endif