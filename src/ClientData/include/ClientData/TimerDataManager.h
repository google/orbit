// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_MANAGER_H_
#define CLIENT_DATA_TIMER_DATA_MANAGER_H_

#include <absl/synchronization/mutex.h>

#include <vector>

#include "ClientData/TimerData.h"

namespace orbit_client_data {

// Creates and stores TimerData in a thread-safe way.
// Note that this class does not provide thread-safe access to ThreadData itself.
class TimerDataManager final {
 public:
  TimerDataManager() = default;

  [[nodiscard]] std::pair<uint64_t, TimerData*> CreateTimerData() {
    absl::MutexLock lock(&mutex_);
    uint64_t id = timer_data_.size();
    timer_data_.emplace_back(std::make_unique<TimerData>());
    return std::make_pair(id, timer_data_.at(id).get());
  }

 private:
  mutable absl::Mutex mutex_;
  std::vector<std::unique_ptr<TimerData>> timer_data_ GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_DATA_MANAGER_H_
