
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ScopeCollection.h"

namespace orbit_client_data {

static const ScopeStats kDefaultScopeStats;

ScopeCollection::ScopeCollection(ScopeIdProvider& scope_id_provider,
                                 std::vector<const TimerInfo*>& timers) {
  for (const TimerInfo* timer : timers) {
    std::optional<ScopeId> scope_id = scope_id_provider.ProvideId(*timer);
    if (scope_id.has_value()) {
      UpdateScopeStats(scope_id.value(), *timer);
    }
  }

  SortTimers();
}

void ScopeCollection::UpdateScopeStats(ScopeId scope_id, const TimerInfo& timer) {
  ScopeStats& stats = scope_stats_[scope_id];
  const uint64_t elapsed_nanos = timer.end() - timer.start();
  stats.UpdateStats(elapsed_nanos);
  scope_id_to_timer_durations_[scope_id].push_back(elapsed_nanos);
  timers_are_sorted_ = false;
}

void ScopeCollection::SetScopeStats(ScopeId scope_id, const ScopeStats stats) {
  scope_stats_.insert_or_assign(scope_id, stats);
}

std::vector<ScopeId> ScopeCollection::GetAllProvidedScopeIds() const {
  std::vector<ScopeId> ids;
  std::transform(std::begin(scope_stats_), std::end(scope_stats_), std::back_inserter(ids),
                 [](const auto& entry) { return entry.first; });
  return ids;
}

const ScopeStats& ScopeCollection::GetScopeStatsOrDefault(ScopeId scope_id) const {
  auto scope_stats_it = scope_stats_.find(scope_id);
  if (scope_stats_it == scope_stats_.end()) {
    return kDefaultScopeStats;
  }
  return scope_stats_it->second;
}

const std::vector<uint64_t>* ScopeCollection::GetSortedTimerDurationsForScopeId(ScopeId scope_id) {
  if (!timers_are_sorted_) {
    SortTimers();
  }
  const auto durations_it = scope_id_to_timer_durations_.find(scope_id);
  if (durations_it == scope_id_to_timer_durations_.end()) return nullptr;
  return &durations_it->second;
}

void ScopeCollection::SortTimers() {
  for (auto& [_, timer_durations] : scope_id_to_timer_durations_) {
    std::sort(timer_durations.begin(), timer_durations.end());
  }
  timers_are_sorted_ = true;
}

}  // namespace orbit_client_data