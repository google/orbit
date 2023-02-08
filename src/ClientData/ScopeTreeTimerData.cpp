// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ClientData/ScopeTreeTimerData.h>
#include <absl/container/btree_map.h>

#include <algorithm>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "ClientData/FastRenderingUtils.h"
#include "ClientData/TimerData.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::AddTimer(
    orbit_client_protos::TimerInfo timer_info, uint32_t /*depth*/) {
  // We don't need to have one TimerChain per depth because it's managed by ScopeTree.
  const auto& timer_info_ref = timer_data_.AddTimer(std::move(timer_info), /*unused_depth=*/0);

  if (scope_tree_update_type_ == ScopeTreeUpdateType::kAlways) {
    absl::MutexLock lock(&scope_tree_mutex_);
    scope_tree_.Insert(&timer_info_ref);
  }
  return timer_info_ref;
}

void ScopeTreeTimerData::OnCaptureComplete() {
  // Build ScopeTree from timer chains, when we are loading a capture.
  if (scope_tree_update_type_ != ScopeTreeUpdateType::kOnCaptureComplete) return;

  std::vector<const TimerChain*> timer_chains = timer_data_.GetChains();
  for (const TimerChain* timer_chain : timer_chains) {
    ORBIT_CHECK(timer_chain != nullptr);
    absl::MutexLock lock(&scope_tree_mutex_);
    for (const auto& block : *timer_chain) {
      for (size_t k = 0; k < block.size(); ++k) {
        scope_tree_.Insert(&block[k]);
      }
    }
  }
}

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetTimers(
    uint64_t start_ns, uint64_t end_ns, bool exclusive) const {
  ORBIT_SCOPE_WITH_COLOR("GetTimers", kOrbitColorAmber);
  // The query is for the interval [start_ns, end_ns], but it's easier to work with the close-open
  // interval [start_ns, end_ns+1). We have to be careful with overflowing.
  end_ns = std::max(end_ns, end_ns + 1);
  std::vector<const orbit_client_protos::TimerInfo*> all_timers;

  for (uint32_t depth = 0; depth < GetDepth(); ++depth) {
    std::vector<const orbit_client_protos::TimerInfo*> timers_at_depth =
        exclusive ? GetTimersAtDepthExclusive(depth, start_ns, end_ns)
                  : GetTimersAtDepth(depth, start_ns, end_ns);
    all_timers.insert(all_timers.end(), timers_at_depth.begin(), timers_at_depth.end());
  }

  return all_timers;
}

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetTimersAtDepthExclusive(
    uint32_t depth, uint64_t start_ns, uint64_t end_ns) const {
  ORBIT_SCOPE_WITH_COLOR("GetTimersAtDepthExclusive", kOrbitColorGreen);
  std::vector<const orbit_client_protos::TimerInfo*> all_timers_at_depth;
  absl::MutexLock lock(&scope_tree_mutex_);

  const auto& ordered_nodes = scope_tree_.GetOrderedNodesAtDepth(depth);
  if (ordered_nodes.empty()) return all_timers_at_depth;

  auto node_it = ordered_nodes.upper_bound(start_ns);
  // Include node if node.start() == start_ns.
  if (node_it != ordered_nodes.begin()) --node_it;
  if (node_it->second->GetScope()->start() < start_ns) ++node_it;

  for (auto it = node_it; it != ordered_nodes.end() && it->second->End() < end_ns; ++it) {
    all_timers_at_depth.push_back(it->second->GetScope());
  }

  return all_timers_at_depth;
}

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetTimersAtDepth(
    uint32_t depth, uint64_t start_ns, uint64_t end_ns) const {
  std::vector<const orbit_client_protos::TimerInfo*> all_timers_at_depth;
  absl::MutexLock lock(&scope_tree_mutex_);

  const auto& ordered_nodes = scope_tree_.GetOrderedNodesAtDepth(depth);
  if (ordered_nodes.empty()) return all_timers_at_depth;

  auto first_node_to_draw = ordered_nodes.upper_bound(start_ns);
  if (first_node_to_draw != ordered_nodes.begin()) --first_node_to_draw;

  // If this node is strictly before the range, we shouldn't include it.
  if (first_node_to_draw->second->GetScope()->end() < start_ns) ++first_node_to_draw;

  for (auto it = first_node_to_draw; it != ordered_nodes.end() && it->first < end_ns; ++it) {
    all_timers_at_depth.push_back(it->second->GetScope());
  }

  return all_timers_at_depth;
}

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetTimersAtDepthDiscretized(
    uint32_t depth, uint32_t resolution, uint64_t start_ns, uint64_t end_ns) const {
  ORBIT_SCOPE_WITH_COLOR("GetTimersAtDepthDiscretized", kOrbitColorAmber);
  if (resolution == 0) return {};
  absl::MutexLock lock(&scope_tree_mutex_);
  // The query is for the interval [start_ns, end_ns], but it's easier to work with the close-open
  // interval [start_ns, end_ns+1). We have to be careful with overflowing.
  end_ns = std::max(end_ns, end_ns + 1);

  std::vector<const orbit_client_protos::TimerInfo*> discretized_timers;
  const orbit_client_protos::TimerInfo* timer_info =
      scope_tree_.FindFirstScopeAtOrAfterTime(depth, start_ns);

  while (timer_info != nullptr && timer_info->start() < end_ns) {
    discretized_timers.push_back(timer_info);

    // Use the time of next pixel boundary as a threshold to avoid returning several timers
    // for the same pixel that will overlap after.
    uint64_t next_pixel_start_time_ns =
        GetNextPixelBoundaryTimeNs(timer_info->end(), resolution, start_ns, end_ns);
    timer_info = scope_tree_.FindFirstScopeAtOrAfterTime(depth, next_pixel_start_time_ns);
  }
  return discretized_timers;
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