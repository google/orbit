// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_
#define ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_

#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "StringManager.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"

class ManualInstrumentationManager {
 public:
  ManualInstrumentationManager() = default;

  using AsyncTimerInfoListener = std::function<void(
      const std::string& name, const orbit_client_protos::TimerInfo& timer_info)>;

  void AddAsyncTimerListener(AsyncTimerInfoListener* listener);
  void RemoveAsyncTimerListener(AsyncTimerInfoListener* listener);
  void ProcessAsyncTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessStringEvent(const orbit_api::Event& event);
  [[nodiscard]] std::string GetString(uint32_t id) const {
    return string_manager_.Get(id).value_or("");
  }
  [[nodiscard]] static orbit_api::Event ApiEventFromTimerInfo(
      const orbit_client_protos::TimerInfo& timer_info);

 private:
  absl::flat_hash_set<AsyncTimerInfoListener*> async_timer_info_listeners_;
  absl::flat_hash_map<uint32_t, orbit_client_protos::TimerInfo> async_timer_info_start_by_id_;
  StringManager string_manager_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_
