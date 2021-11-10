// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_SHARED_STATE_H_
#define ORBIT_BASE_SHARED_STATE_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <optional>
#include <variant>
#include <vector>

#include "OrbitBase/AnyInvocable.h"

namespace orbit_base_internal {

// SharedState<T> is an implementation detail of the Future<T> / Promise<T> facility.
//
// Don't use this class outside of Promise<T> / Future<T>!
template <typename T>
struct SharedState {
  absl::Mutex mutex;
  std::optional<T> result ABSL_GUARDED_BY(mutex);
  std::vector<orbit_base::AnyInvocable<void(const T&)>> continuations ABSL_GUARDED_BY(mutex);

  [[nodiscard]] bool IsFinished() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex) {
    return result.has_value();
  }
};

template <>
struct SharedState<void> {
  absl::Mutex mutex;
  bool finished ABSL_GUARDED_BY(mutex) = false;
  std::vector<orbit_base::AnyInvocable<void()>> continuations ABSL_GUARDED_BY(mutex);

  [[nodiscard]] bool IsFinished() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex) { return finished; }
};

}  // namespace orbit_base_internal

#endif  // ORBIT_BASE_SHARED_STATE_H_