// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef INTROSPECTION_INTROSPECTION_H_
#define INTROSPECTION_INTROSPECTION_H_

#include <stdint.h>

#include <functional>
#include <memory>

#include "Api/EncodedEvent.h"
#include "Api/Orbit.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitBase/ThreadUtils.h"

#define ORBIT_SCOPE_FUNCTION ORBIT_SCOPE(__FUNCTION__)

namespace orbit_introspection {

struct TracingScope {
  TracingScope(orbit_api::EventType type, const char* name = nullptr, uint64_t data = 0,
               orbit_api_color color = kOrbitColorAuto);
  uint64_t begin = 0;
  uint64_t end = 0;
  uint32_t depth = 0;
  uint32_t tid = 0;
  orbit_api::EncodedEvent encoded_event;
};

using TracingTimerCallback = std::function<void(const TracingScope& scope)>;

class TracingListener {
 public:
  explicit TracingListener(TracingTimerCallback callback);
  ~TracingListener();

  static void DeferScopeProcessing(const TracingScope& scope);
  [[nodiscard]] inline static bool IsActive() { return active_; }
  [[nodiscard]] inline static bool IsShutdownInitiated() { return shutdown_initiated_; }

 private:
  TracingTimerCallback user_callback_ = nullptr;
  std::shared_ptr<ThreadPool> thread_pool_ = {};
  inline static bool active_ = false;
  inline static bool shutdown_initiated_ = true;
};

}  // namespace orbit_introspection

#endif  // INTROSPECTION_INTROSPECTION_H_
