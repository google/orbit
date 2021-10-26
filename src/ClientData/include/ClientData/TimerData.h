// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_H_
#define CLIENT_DATA_TIMER_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "OrbitBase/ThreadConstants.h"
#include "TimerChain.h"
#include "TimerDataInterface.h"
#include "capture_data.pb.h"

namespace orbit_client_data {

class TimerData final : public TimerDataInterface {
 public:
  const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info,
                                                 uint32_t depth) override {
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

  [[nodiscard]] virtual std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      uint64_t /*min_tick*/ = std::numeric_limits<uint64_t>::min(),
      uint64_t /*max_tick*/ = std::numeric_limits<uint64_t>::max()) const override {
    // TODO(b/204173236): Implement GetTimers and use it in TimerTracks.
    return {};
  }

  // TODO(b/204173036): Test depth and process_id.
  const TimerMetadata GetTimerMetadata() const override {
    TimerMetadata timer_metadata;
    timer_metadata.number_of_timers = num_timers_;
    timer_metadata.is_empty = num_timers_ == 0;
    timer_metadata.min_time = min_time_;
    timer_metadata.max_time = max_time_;
    timer_metadata.depth = max_depth_;
    timer_metadata.process_id = process_id_;
    return timer_metadata;
  }

  [[nodiscard]] std::vector<const TimerChain*> GetChains() const override {
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

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetFirstAfterStartTime(uint64_t time,
                                                                             uint32_t depth) const {
    const orbit_client_data::TimerChain* chain = GetChain(depth);
    if (chain == nullptr) return nullptr;

    // TODO(b/201044462): do better than linear search...
    for (const auto& it : *chain) {
      for (size_t k = 0; k < it.size(); ++k) {
        const orbit_client_protos::TimerInfo& timer_info = it[k];
        if (timer_info.start() > time) {
          return &timer_info;
        }
      }
    }
    return nullptr;
  }

  const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer_info) const override {
    return GetFirstBeforeStartTime(timer_info.start(), timer_info.depth());
  }

  const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer_info) const override {
    return GetFirstAfterStartTime(timer_info.start(), timer_info.depth());
  }

  const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer_info) const override {
    return GetFirstBeforeStartTime(timer_info.start(), timer_info.depth() - 1);
  }

  const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer_info) const override {
    return GetFirstAfterStartTime(timer_info.start(), timer_info.depth() + 1);
  }

  // Unused methods needed in TimerDataInterface
  [[nodiscard]] int64_t GetThreadId() const override { return -1; }
  virtual void OnCaptureComplete() override {}

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetFirstBeforeStartTime(
      uint64_t time, uint32_t depth) const {
    const orbit_client_data::TimerChain* chain = GetChain(depth);
    if (chain == nullptr) return nullptr;

    const orbit_client_protos::TimerInfo* first_timer_before_time = nullptr;

    // TODO(b/201044462): do better than linear search...
    for (const auto& it : *chain) {
      for (size_t k = 0; k < it.size(); ++k) {
        const orbit_client_protos::TimerInfo* timer_info = &it[k];
        if (timer_info->start() >= time) {
          return first_timer_before_time;
        }
        first_timer_before_time = timer_info;
      }
    }

    return first_timer_before_time;
  }

 private:
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
  std::map<uint32_t, std::unique_ptr<TimerChain>> timers_ ABSL_GUARDED_BY(mutex_);
  std::atomic<size_t> num_timers_{0};
  std::atomic<uint64_t> min_time_{std::numeric_limits<uint64_t>::max()};
  std::atomic<uint64_t> max_time_{std::numeric_limits<uint64_t>::min()};

  uint32_t process_id_ = orbit_base::kInvalidProcessId;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_DATA_H_
