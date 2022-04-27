// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STOP_TOKEN_H_
#define ORBIT_BASE_STOP_TOKEN_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <functional>

namespace orbit_base {

class StopToken {
 public:
  explicit StopToken() = default;
  StopToken(const StopToken& other) = delete;
  StopToken& operator=(const StopToken& other) = delete;
  StopToken(StopToken&& other) = delete;
  StopToken& operator=(StopToken&& other) = delete;

  [[nodiscard]] bool IsStopRequested() const {
    absl::MutexLock lock(&mutex_);
    return stop_requested_;
  }
  void RequestStop() {
    absl::MutexLock lock(&mutex_);
    stop_requested_ = true;
  }

 private:
  mutable absl::Mutex mutex_;
  bool stop_requested_ ABSL_GUARDED_BY(mutex_) = false;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_STOP_TOKEN_H_