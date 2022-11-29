// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ClientData/TimerData.h>

#include <utility>

#include "ApiInterface/Orbit.h"
#include "ClientData/FastRenderingUtils.h"
#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitBase/Logging.h"

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

std::vector<const orbit_client_protos::TimerInfo*> TimerData::GetTimers(uint64_t min_tick,
                                                                        uint64_t max_tick) const {
  ORBIT_SCOPE_WITH_COLOR("GetTimersAtDepthDiscretized", kOrbitColorBlueGrey);
  // TODO(b/204173236): use it in TimerTracks.
  absl::MutexLock lock(&mutex_);
  std::vector<const orbit_client_protos::TimerInfo*> timers;
  for (const auto& [depth, chain] : timers_) {
    ORBIT_CHECK(chain != nullptr);
    for (const auto& block : *chain) {
      if (!block.Intersects(min_tick, max_tick)) continue;
      for (uint64_t i = 0; i < block.size(); i++) {
        const orbit_client_protos::TimerInfo* timer = &block[i];
        if (timer->start() <= max_tick && timer->end() >= min_tick) timers.push_back(timer);
      }
    }
  }

  return timers;
}

std::vector<const orbit_client_protos::TimerInfo*> TimerData::GetTimersAtDepthDiscretized(
    uint32_t depth, uint32_t resolution, uint64_t start_ns, uint64_t end_ns) const {
  ORBIT_SCOPE_WITH_COLOR("GetTimersAtDepthDiscretized", kOrbitColorBlueGrey);
  absl::MutexLock lock(&mutex_);
  // The query is for the interval [start_ns, end_ns], but it's easier to work with the close-open
  // interval [start_ns, end_ns+1). We have to be careful with overflowing if end_ns is the maximum
  // unsigned value. In that case, we will just ignore this max_timestamp for simplicity.
  end_ns = std::max(end_ns, end_ns + 1);

  if (timers_.find(depth) == timers_.end()) return {};

  std::vector<const orbit_client_protos::TimerInfo*> discretized_timers;
  uint64_t next_pixel_start_ns = start_ns;

  // We are iterating through all blocks until we are after end_ns.
  for (const auto& block : *timers_.at(depth)) {
    if (block.MinTimestamp() >= end_ns) break;

    // Several candidate timers might be in the same block.
    while (block.Intersects(next_pixel_start_ns, end_ns) && next_pixel_start_ns < end_ns) {
      // First timer for which the end timestamp isn't smaller than the start of the next pixel.
      const orbit_client_protos::TimerInfo* timer = block.LowerBound(next_pixel_start_ns);
      if (timer == nullptr || timer->start() >= end_ns) break;
      discretized_timers.push_back(timer);

      // Use the time of next pixel boundary as a threshold to avoid returning several timers
      // for the same pixel that will overlap after.
      next_pixel_start_ns = GetNextPixelBoundaryTimeNs(timer->end(), resolution, start_ns, end_ns);
    }
  }
  return discretized_timers;
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
  ORBIT_CHECK(inserted);
  return inserted_it->second.get();
}

}  // namespace orbit_client_data