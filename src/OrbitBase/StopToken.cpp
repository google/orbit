// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/StopToken.h"

#include "OrbitBase/Logging.h"

namespace orbit_base {

bool StopToken::IsStopRequested() const {
  ORBIT_CHECK(IsStopPossible());
  absl::MutexLock lock(&shared_stop_state_->mutex);
  return shared_stop_state_->IsFinished();
}

}  // namespace orbit_base