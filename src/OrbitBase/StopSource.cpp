// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/StopSource.h"

#include "OrbitBase/Logging.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

void StopSource::RequestStop() {
  ORBIT_CHECK(IsStopPossible());
  absl::MutexLock lock{&shared_state_->mutex};
  shared_state_->finished = true;
}

[[nodiscard]] StopToken StopSource::GetStopToken() {
  ORBIT_CHECK(IsStopPossible());
  return StopToken(shared_state_);
}

}  // namespace orbit_base