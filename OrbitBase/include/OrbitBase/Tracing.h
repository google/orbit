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

#define ORBIT_SCOPE_FUNCTION ORBIT_SCOPE(__FUNCTION__)

namespace orbit::tracing {

struct Scope {
  Scope(orbit_api::EventType type, const char* name = nullptr, uint64_t data = 0,
        orbit::Color color = orbit::Color::kAuto);
  uint64_t begin = 0;
  uint64_t end = 0;
  uint32_t depth = 0;
  uint32_t tid = 0;
  orbit_api::EncodedEvent encoded_event;
};

using TimerCallback = std::function<void(const Scope& scope)>;

class Listener {
 public:
  explicit Listener(TimerCallback callback);
  ~Listener();

  static void DeferScopeProcessing(const Scope& scope);
  [[nodiscard]] inline static bool IsActive() { return active_; }

 private:
  TimerCallback user_callback_ = nullptr;
  std::unique_ptr<ThreadPool> thread_pool_ = {};
  inline static bool active_ = false;
};

}  // namespace orbit::tracing

#endif  // ORBIT_BASE_TRACING_H_
