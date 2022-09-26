
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ScopeStatsCollection.h"

#include "OrbitBase/Logging.h"

namespace orbit_client_data {

static const ScopeStats kDefaultScopeStats;

ScopeStatsCollection::ScopeStatsCollection(ScopeIdProvider& scope_id_provider,
                                           const std::vector<const TimerInfo*>& timers) {
  for (const TimerInfo* timer : timers) {
    std::optional<ScopeId> scope_id = scope_id_provider.ProvideId(*timer);
    if (scope_id.has_value()) {
      UpdateScopeStats(scope_id.value(), *timer);
    }
  }

  SortTimers();
}

void ScopeStatsCollection::UpdateScopeStats(ScopeId scope_id, const TimerInfo& timer) {
  ScopeStats& stats = scope_stats_[scope_id];
  const uint64_t elapsed_nanos = timer.end() - timer.start();
  stats.UpdateStats(elapsed_nanos);
  scope_id_to_timer_durations_[scope_id].push_back(elapsed_nanos);
  timer_durations_are_sorted_ = false;
}

// TODO(b/249046906): Remove this test-only function.
void ScopeStatsCollection::SetScopeStats(ScopeId scope_id, const ScopeStats stats) {
  scope_stats_.insert_or_assign(scope_id, stats);
}

std::vector<ScopeId> ScopeStatsCollection::GetAllProvidedScopeIds() const {
  std::vector<ScopeId> ids;
  absl::c_transform(scope_stats_, std::back_inserter(ids),
                    [](const auto& entry) { return entry.first; });
  return ids;
}

const ScopeStats& ScopeStatsCollection::GetScopeStatsOrDefault(ScopeId scope_id) const {
  if (auto scope_stats_it = scope_stats_.find(scope_id); scope_stats_it != scope_stats_.end()) {
    return scope_stats_it->second;
  }
  return kDefaultScopeStats;
}

const std::vector<uint64_t>* ScopeStatsCollection::GetSortedTimerDurationsForScopeId(
    ScopeId scope_id) {
  if (!timer_durations_are_sorted_) {
    ORBIT_ERROR(
        "Calling GetSortedTimerDurationsForScopeId on unsorted timers. Must call SortTimers() "
        "first.");
    return nullptr;
  }
  if (const auto durations_it = scope_id_to_timer_durations_.find(scope_id);
      durations_it != scope_id_to_timer_durations_.end()) {
    return &durations_it->second;
  }
  return nullptr;
}

void ScopeStatsCollection::SortTimers() {
  if (timer_durations_are_sorted_) return;

  for (auto& [_, timer_durations] : scope_id_to_timer_durations_) {
    absl::c_sort(timer_durations);
  }
  timer_durations_are_sorted_ = true;
}

}  // namespace orbit_client_data