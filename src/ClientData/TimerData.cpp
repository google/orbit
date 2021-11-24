// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ClientData/TimerData.h>

#include "ClientProtos/capture_data.pb.h"

using orbit_client_protos::TimerInfo;

namespace orbit_client_data {

const TimerInfo& TimerData::AddTimer(TimerInfo timer_info, uint32_t depth) {
  if (process_id_ == orbit_base::kInvalidProcessId) {
    process_id_ = timer_info.process_id();
  }

  TimerChain* timer_chain = GetOrCreateTimerChain(depth);
  UpdateMinTime(timer_info.start());
  UpdateMaxTime(timer_info.end());
  ++num_timers_;
  UpdateDepth(timer_info.depth() + 1);

  return timer_chain->emplace_back(std::move(timer_info));
}

std::vector<const TimerChain*> TimerData::GetChains() const {
  std::vector<const TimerChain*> chains;
  absl::MutexLock lock(&mutex_);
  for (const auto& it : timers_) {
    chains.push_back(it.second.get());
  }

  return chains;
}

const TimerChain* TimerData::GetChain(uint64_t depth) const {
  absl::MutexLock lock(&mutex_);
  auto it = timers_.find(depth);
  if (it != timers_.end()) {
    return it->second.get();
  }

  return nullptr;
}

const TimerInfo* TimerData::GetFirstAfterStartTime(uint64_t time, uint32_t depth) const {
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

const TimerInfo* TimerData::GetFirstBeforeStartTime(uint64_t time, uint32_t depth) const {
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

void TimerData::UpdateMinTime(uint64_t min_time) {
  uint64_t current_min = min_time_.load();
  while ((min_time < current_min) && !min_time_.compare_exchange_weak(current_min, min_time)) {
  }
}

void TimerData::UpdateMaxTime(uint64_t max_time) {
  uint64_t current_max = max_time_.load();
  while ((max_time > current_max) && !max_time_.compare_exchange_weak(current_max, max_time)) {
  }
}

TimerChain* TimerData::GetOrCreateTimerChain(uint64_t depth) {
  absl::MutexLock lock(&mutex_);
  auto it = timers_.find(depth);
  if (it != timers_.end()) {
    return it->second.get();
  }

  auto [inserted_it, inserted] = timers_.insert_or_assign(depth, std::make_unique<TimerChain>());
  CHECK(inserted);
  return inserted_it->second.get();
}

}  // namespace orbit_client_data