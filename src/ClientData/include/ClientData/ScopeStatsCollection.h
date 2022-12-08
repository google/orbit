
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_SCOPE_STATS_COLLECTION_H_
#define CLIENT_DATA_SCOPE_STATS_COLLECTION_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>

#include <cstdint>
#include <vector>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeIdProvider.h"
#include "ClientData/ScopeStats.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

// ScopeStatsCollection holds a subset of all Scopes in a capture keeping track of their stats and
// ordered durations.
class ScopeStatsCollectionInterface {
 public:
  virtual ~ScopeStatsCollectionInterface() = default;

  [[nodiscard]] virtual std::vector<ScopeId> GetAllProvidedScopeIds() const = 0;
  [[nodiscard]] virtual const ScopeStats& GetScopeStatsOrDefault(ScopeId scope_id) const = 0;
  [[nodiscard]] virtual const std::vector<uint64_t>* GetSortedTimerDurationsForScopeId(
      ScopeId scope_id) const = 0;

  // Calling this function causes the timer durations to no longer be sorted. OnCaptureComplete()
  // *must* be called after UpdateScopeStats and before GetSortedTimerDurationsForScopeId().
  virtual void UpdateScopeStats(ScopeId scope_id, const TimerInfo& timer) = 0;
  // TODO(b/249046906): Remove this test-only function.
  virtual void SetScopeStats(ScopeId scope_id, ScopeStats stats) = 0;
  virtual void OnCaptureComplete() = 0;
};

class ScopeStatsCollection : public ScopeStatsCollectionInterface {
 public:
  explicit ScopeStatsCollection() = default;
  explicit ScopeStatsCollection(ScopeIdProvider& scope_id_provider,
                                absl::Span<const TimerInfo* const> timers);

  [[nodiscard]] std::vector<ScopeId> GetAllProvidedScopeIds() const override;
  [[nodiscard]] const ScopeStats& GetScopeStatsOrDefault(ScopeId scope_id) const override;
  [[nodiscard]] const std::vector<uint64_t>* GetSortedTimerDurationsForScopeId(
      ScopeId scope_id) const override;

  void UpdateScopeStats(ScopeId scope_id, const TimerInfo& timer) override;
  void SetScopeStats(ScopeId scope_id, ScopeStats stats) override;
  void OnCaptureComplete() override;

 private:
  absl::flat_hash_map<ScopeId, ScopeStats> scope_stats_;
  absl::flat_hash_map<ScopeId, std::vector<uint64_t>> scope_id_to_timer_durations_;
  bool timer_durations_are_sorted_ = true;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_SCOPE_STATS_COLLECTION_H_
