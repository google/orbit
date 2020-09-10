// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TRACING_H_
#define ORBIT_BASE_TRACING_H_

#include <functional>
#include <memory>

#define ORBIT_API_INTERNAL_IMPL
// NOTE: Orbit.h will be moved to its own
//       OrbitApi project in a subsequent PR.
#include "../../../Orbit.h"
#include "OrbitBase/ThreadPool.h"

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
  const char* name = nullptr;
  ScopeType type = kNone;
};

using TimerCallback = std::function<void(const Scope& scope)>;

class Listener {
 public:
  explicit Listener(std::unique_ptr<TimerCallback> callback);
  ~Listener();

  static void DeferScopeProcessing(const Scope& scope);
  [[nodiscard]] inline static bool IsActive() { return active_; }

 private:
  std::unique_ptr<TimerCallback> user_callback_ = {};
  std::unique_ptr<ThreadPool> thread_pool_ = {};
  inline static bool active_ = false;
};

}  // namespace orbit::tracing

#endif  // ORBIT_BASE_TRACING_H_
