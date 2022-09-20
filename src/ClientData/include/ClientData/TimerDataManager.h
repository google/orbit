// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_MANAGER_H_
#define CLIENT_DATA_TIMER_DATA_MANAGER_H_

#include <absl/synchronization/mutex.h>

#include <vector>

#include "ClientData/TimerData.h"
#include "ClientProtos/capture_data.pb.h"

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

  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      orbit_client_protos::TimerInfo_Type type,
      uint64_t min_tick = std::numeric_limits<uint64_t>::min(),
      uint64_t max_tick = std::numeric_limits<uint64_t>::max()) const {
    std::vector<const orbit_client_protos::TimerInfo*> timers;
    absl::MutexLock lock(&mutex_);
    for (const std::unique_ptr<TimerData>& timer_datum : timer_data_) {
      for (const orbit_client_protos::TimerInfo* timer :
           timer_datum->GetTimers(min_tick, max_tick)) {
        if (timer->type() == type) timers.push_back(timer);
      }
    }
    return timers;
  }

 private:
  mutable absl::Mutex mutex_;
  std::vector<std::unique_ptr<TimerData>> timer_data_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_DATA_MANAGER_H_
