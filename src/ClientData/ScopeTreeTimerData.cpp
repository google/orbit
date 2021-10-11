// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ClientData/ScopeTreeTimerData.h>

namespace orbit_client_data {

size_t ScopeTreeTimerData::GetNumberOfTimers() const {
  absl::MutexLock lock(&scope_tree_mutex_);
  // Special case. ScopeTree has a root node in depth 0 which shouldn't be considered.
  return scope_tree_.Size() - 1;
}

uint32_t ScopeTreeTimerData::GetDepth() const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return scope_tree_.Depth();
}

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::AddTimer(
    orbit_client_protos::TimerInfo timer_info) {
  // Thread tracks use a ScopeTree so we don't need to create one TimerChain per depth.
  const auto& timer_info_ref = timer_data_.AddTimer(/*depth=*/0, std::move(timer_info));

  if (scope_tree_update_type_ == ScopeTreeUpdateType::kAlways) {
    absl::MutexLock lock(&scope_tree_mutex_);
    scope_tree_.Insert(&timer_info_ref);
  }
  return timer_info_ref;
}

void ScopeTreeTimerData::BuildScopeTreeFromTimerData() {
  CHECK(scope_tree_update_type_ == ScopeTreeUpdateType::kOnCaptureComplete);

  // Build ScopeTree from timer chains.
  std::vector<const TimerChain*> timer_chains = timer_data_.GetChains();
  for (const TimerChain* timer_chain : timer_chains) {
    CHECK(timer_chain != nullptr);
    absl::MutexLock lock(&scope_tree_mutex_);
    for (const auto& block : *timer_chain) {
      for (size_t k = 0; k < block.size(); ++k) {
        scope_tree_.Insert(&block[k]);
      }
    }
  }
}

[[nodiscard]] static inline uint64_t GetNextPixelBoundaryTimeNs(uint64_t current_timestamp_ns,
                                                                uint64_t start_ns, uint64_t end_ns,
                                                                uint64_t resolution) {
  uint64_t current_ns_from_start = current_timestamp_ns - start_ns;
  uint64_t total_ns = end_ns - start_ns;

  // Given a resolution of 4000 pixels, we can capture for 53 days without overflowing.
  uint64_t current_pixel = (current_ns_from_start * resolution) / total_ns;
  uint64_t next_pixel = current_pixel + 1;

  // To calculate the timestamp of a pixel boundary, we round to the left similar to how it works in
  // other parts of Orbit.
  uint64_t next_pixel_ns_from_min = total_ns * next_pixel / resolution;

  // Border case when we have a lot of pixels who have the same timestamp (because the number of
  // pixels is less than the nanoseconds in screen). In this case, as we've already drawn in the
  // current_timestamp, the next pixel to draw should have the next timestamp.
  if (next_pixel_ns_from_min == current_ns_from_start) {
    next_pixel_ns_from_min = current_ns_from_start + 1;
  }

  return start_ns + next_pixel_ns_from_min;
}

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetAllTimers(
    uint64_t min_tick, uint64_t max_tick, uint64_t resolution) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  std::vector<const orbit_client_protos::TimerInfo*> all_timers;
  for (const auto& [depth, _] : scope_tree_.GetOrderedNodesByDepth()) {
    auto timers_at_depth = GetTimersAtDepth(depth, min_tick, max_tick, resolution);
    all_timers.insert(all_timers.end(), timers_at_depth.begin(), timers_at_depth.end());
  }
  return all_timers;
}

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetTimersAtDepth(
    uint32_t depth, uint64_t min_tick, uint64_t max_tick, uint64_t resolution) const {
  std::vector<const orbit_client_protos::TimerInfo*> all_timers_at_depth;
  absl::MutexLock lock(&scope_tree_mutex_);

  // Special case. ScopeTree has a root node in depth 0 which shouldn't be considered.
  if (depth >= scope_tree_.Depth()) return all_timers_at_depth;
  // TODO(why we need this update)!! timer_data_->UpdateMaxDepth(depth);

  const orbit_client_protos::TimerInfo* timer_info =
      scope_tree_.FindFirstScopeAtOrAfterTime(depth, min_tick);

  while (timer_info != nullptr && timer_info->start() < max_tick) {
    all_timers_at_depth.push_back(timer_info);

    // Use the time at boundary of the next pixel as a threshold to avoid returning several timers
    // who will after overlap in the same pixel.
    uint64_t next_pixel_start_time_ns =
        GetNextPixelBoundaryTimeNs(timer_info->end(), min_tick, max_tick, resolution);
    timer_info = scope_tree_.FindFirstScopeAtOrAfterTime(depth, next_pixel_start_time_ns);
  }

  return all_timers_at_depth;
}

const orbit_client_protos::TimerInfo* ScopeTreeTimerData::GetLeft(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return scope_tree_.FindPreviousScopeAtDepth(timer);
}

const orbit_client_protos::TimerInfo* ScopeTreeTimerData::GetRight(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return scope_tree_.FindNextScopeAtDepth(timer);
}

const orbit_client_protos::TimerInfo* ScopeTreeTimerData::GetUp(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return scope_tree_.FindParent(timer);
}

const orbit_client_protos::TimerInfo* ScopeTreeTimerData::GetDown(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return scope_tree_.FindFirstChild(timer);
}

}  // namespace orbit_client_data