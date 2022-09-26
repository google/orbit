
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_SCOPE_COLLECTION_H_
#define CLIENT_DATA_SCOPE_COLLECTION_H_

#include <cmath>

#include "ClientData/ScopeIdProvider.h"
#include "ClientData/ScopeStats.h"
#include "ClientData/TimerTrackDataIdManager.h"

namespace orbit_client_data {

// ScopeCollection holds a subset of all Scopes in a capture keeping track of their stats and
// ordered durations.
class ScopeStatsCollection {
 public:
  explicit ScopeStatsCollection() = default;
  explicit ScopeStatsCollection(ScopeIdProvider& scope_id_provider,
                                const std::vector<const TimerInfo*>& timers);

  [[nodiscard]] std::vector<ScopeId> GetAllProvidedScopeIds() const;
  [[nodiscard]] const ScopeStats& GetScopeStatsOrDefault(ScopeId scope_id) const;
  [[nodiscard]] const std::vector<uint64_t>* GetSortedTimerDurationsForScopeId(ScopeId scope_id);

  // This causes the timers to no longer be sorted. Recognize that SortTImers() needs to be called
  // and will take time before GetSortedTimerDurationsForScopeId can be called/returned.
  void UpdateScopeStats(ScopeId scope_id, const TimerInfo& timer);
  void SetScopeStats(ScopeId scope_id, ScopeStats stats);

  void SortTimers();

 private:
  absl::flat_hash_map<ScopeId, ScopeStats> scope_stats_;
  absl::flat_hash_map<ScopeId, std::vector<uint64_t>> scope_id_to_timer_durations_;
  bool timer_durations_are_sorted_ = true;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_SCOPE_COLLECTION_H_
