// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACING_TRACING_H_
#define ORBIT_TRACING_TRACING_H_

#include <functional>
#include <memory>

#include "absl/synchronization/mutex.h"

#define ORBIT_API_INTERNAL_IMPL
// NOTE: Orbit.h will be moved to its own
//       OrbitApi project in a subsequent PR.
#include "../../../Orbit.h"

namespace orbit::tracing {

enum ScopeType : int {
  kNone = 0,
  kScope = 1,
  kScopeAsync = 2,
  kTrackInt = 3,
  kTrackInt64 = 4,
  kTrackUint = 5,
  kTrackUint64 = 6,
  kTrackFloat = 7,
  kTrackDouble = 8,
  kTrackFloatAsInt = 9,
  kTrackDoubleAsInt64 = 10
};

struct Scope {
  uint64_t begin = 0;
  uint64_t end = 0;
  uint64_t tracked_value = 0;
  uint32_t depth = 0;
  uint32_t tid = 0;
  orbit::Color color = orbit::Color::kAuto;
  const char* name;
  ScopeType type = kNone;
};

using TimerCallback = std::function<void(Scope&& scope)>;

class Listener {
 public:
  explicit Listener(const TimerCallback& callback);
  ~Listener();

  static absl::Mutex* GetMutex() { return &mutex_; }

 private:
  TimerCallback callback_;
  static inline absl::Mutex mutex_;
};

void Start(const TimerCallback* callback);
void Stop();
}  // namespace orbit::tracing

#endif  // ORBIT_TRACING_TRACING_H_
