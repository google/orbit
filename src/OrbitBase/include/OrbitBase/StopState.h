// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_STATE_H_
#define ORBIT_BASE_STOP_STATE_H_

#include <absl/synchronization/mutex.h>

namespace orbit_base_internal {

// StopState is an implementation detail of StopSource and StopToken and is not intended to be used
// outside of StopSource and StopToken.
//
// StopState is a simple wrapper around a bool protected by a mutex. A StopState is not stopped on
// creation (IsStopped() returns false) and can be stopped once by calling Stop(). Afterwards
// IsStopped() returns true.
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
  bool stopped_ ABSL_GUARDED_BY(mutex_) = false;
};

}  // namespace orbit_base_internal

#endif  // ORBIT_BASE_STOP_STATE_H_