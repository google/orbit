// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_TOKEN_H_
#define ORBIT_BASE_STOP_TOKEN_H_

#include <memory>

#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"

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
  [[nodiscard]] bool IsStopPossible() const { return future_.IsValid(); }
  [[nodiscard]] bool IsStopRequested() const {
    ORBIT_CHECK(IsStopPossible());
    return future_.IsFinished();
  }

  [[nodiscard]] Future<void> GetFuture() const { return future_; }

 private:
  explicit StopToken(Future<void> future) : future_{std::move(future)} {}

  orbit_base::Future<void> future_;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_STOP_TOKEN_H_