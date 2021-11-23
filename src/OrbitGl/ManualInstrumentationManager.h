// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_
#define ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "ClientProtos/capture_data.pb.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "StringManager/StringManager.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"

class ManualInstrumentationManager {
 public:
  ManualInstrumentationManager() = default;

  using AsyncTimerInfoListener = std::function<void(
      const std::string& name, const orbit_client_protos::TimerInfo& timer_info)>;

  void AddAsyncTimerListener(AsyncTimerInfoListener* listener);
  void RemoveAsyncTimerListener(AsyncTimerInfoListener* listener);
  void ProcessAsyncTimer(const orbit_client_protos::TimerInfo& timer_info);
  void ProcessStringEvent(const orbit_client_protos::ApiStringEvent& string_event);
  [[nodiscard]] std::string GetString(uint32_t id) const {
    return string_manager_.Get(id).value_or("");
  }

 private:
  absl::flat_hash_set<AsyncTimerInfoListener*> async_timer_info_listeners_;
  orbit_string_manager::StringManager string_manager_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_
