// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_H_
#define CLIENT_DATA_TIMER_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "OrbitBase/ThreadConstants.h"
#include "TimerChain.h"
#include "capture_data.pb.h"

namespace orbit_client_data {

class TimerData final {
 public:
  [[nodiscard]] bool IsEmpty() const { return num_timers_ == 0; }
  [[nodiscard]] size_t GetNumberOfTimers() const { return num_timers_; }
  [[nodiscard]] uint64_t GetMinTime() const { return min_time_; }
  [[nodiscard]] uint64_t GetMaxTime() const { return max_time_; }
  [[nodiscard]] uint32_t GetMaxDepth() const { return max_depth_; }
  [[nodiscard]] uint32_t GetProcessId() const { return process_id_; }

  const orbit_client_protos::TimerInfo& AddTimer(uint64_t depth,
                                                 orbit_client_protos::TimerInfo timer_info) {
    if (process_id_ == orbit_base::kInvalidProcessId) {
      process_id_ = timer_info.process_id();
    }

    TimerChain* timer_chain = GetOrCreateTimerChain(depth);
    UpdateMinTime(timer_info.start());
    UpdateMaxTime(timer_info.end());
    ++num_timers_;
    UpdateMaxDepth(timer_info.depth() + 1);

    return timer_chain->emplace_back(std::move(timer_info));
  }

  [[nodiscard]] std::vector<const TimerChain*> GetChains() const {
    std::vector<const TimerChain*> chains;
    absl::MutexLock lock(&mutex_);
    for (const auto& it : timers_) {
      chains.push_back(it.second.get());
    }

    return chains;
  }

  [[nodiscard]] const TimerChain* GetChain(uint64_t depth) const {
    absl::MutexLock lock(&mutex_);
    auto it = timers_.find(depth);
    if (it != timers_.end()) {
      return it->second.get();
    }

    return nullptr;
  }

  void UpdateMinTime(uint64_t min_time) {
    uint64_t current_min = min_time_.load();
    while ((min_time < current_min) && !min_time_.compare_exchange_weak(current_min, min_time)) {
    }
  }

  void UpdateMaxTime(uint64_t max_time) {
    uint64_t current_max = max_time_.load();
    while ((max_time > current_max) && !max_time_.compare_exchange_weak(current_max, max_time)) {
    }
  }

  void UpdateMaxDepth(uint32_t depth) { max_depth_ = std::max(max_depth_, depth); }

 private:
  [[nodiscard]] TimerChain* GetOrCreateTimerChain(uint64_t depth) {
    absl::MutexLock lock(&mutex_);
    auto it = timers_.find(depth);
    if (it != timers_.end()) {
      return it->second.get();
    }

    auto [inserted_it, inserted] = timers_.insert_or_assign(depth, std::make_unique<TimerChain>());
    CHECK(inserted);
    return inserted_it->second.get();
  }

  uint32_t max_depth_ = 0;
  mutable absl::Mutex mutex_;
  std::map<uint64_t, std::unique_ptr<TimerChain>> timers_ ABSL_GUARDED_BY(mutex_);
  std::atomic<size_t> num_timers_{0};
  std::atomic<uint64_t> min_time_{std::numeric_limits<uint64_t>::max()};
  std::atomic<uint64_t> max_time_{std::numeric_limits<uint64_t>::min()};

  uint32_t process_id_ = orbit_base::kInvalidProcessId;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_DATA_H_
