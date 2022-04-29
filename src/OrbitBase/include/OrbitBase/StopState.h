// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_STATE_H_
#define ORBIT_BASE_STOP_STATE_H_

#include <absl/synchronization/mutex.h>

namespace orbit_base_internal {

// StopState is an implementation detail of StopSource and StopToken and is not intended to be used
// outside of StopSource and StopToken
class StopState {
 public:
  [[nodiscard]] bool IsStopped() const {
    absl::MutexLock lock{&mutex_};
    return stopped_;
  }
  void Stop() {
    absl::MutexLock lock{&mutex_};
    stopped_ = true;
  }

 private:
  mutable absl::Mutex mutex_;
  bool stopped_ = false;
};

}  // namespace orbit_base_internal

#endif  // ORBIT_BASE_STOP_STATE_H_