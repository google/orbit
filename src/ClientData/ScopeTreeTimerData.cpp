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

uint32_t ScopeTreeTimerData::GetMaxDepth() const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return scope_tree_.Height() - 1;
}

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::AddTimer(
    orbit_client_protos::TimerInfo timer_info) {
  const auto& timer_info_ref = timer_data_.AddTimer(/*depth=*/0, timer_info);

  if (scope_tree_update_type_ == ScopeTreeUpdateType::kAlways) {
    absl::MutexLock lock(&scope_tree_mutex_);
    scope_tree_.Insert(&timer_info_ref);
  }
  return timer_info_ref;
}

void ScopeTreeTimerData::BuildScopeTreeFromTimerData() {
  CHECK(scope_tree_update_type_ == ScopeTreeUpdateType::kOnCaptureComplete);
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

std::vector<const orbit_client_protos::TimerInfo*> ScopeTreeTimerData::GetTimers(
    uint64_t min_tick, uint64_t max_tick) const {
  std::vector<const orbit_client_protos::TimerInfo*> all_timers;
  absl::MutexLock lock(&scope_tree_mutex_);

  for (const auto& [depth, ordered_nodes] : scope_tree_.GetOrderedNodesByDepth()) {
    // Special case. ScopeTree has a root node in depth 0 which shouldn't be considered.
    if (depth == 0) continue;

    auto first_node_to_draw = ordered_nodes.upper_bound(min_tick);
    if (first_node_to_draw != ordered_nodes.begin()) --first_node_to_draw;

    // If this node is strictly before the range, we shouldn't include it.
    if (first_node_to_draw->second->GetScope()->end() < min_tick) ++first_node_to_draw;

    for (auto it = first_node_to_draw; it != ordered_nodes.end() && it->first < max_tick; ++it) {
      all_timers.push_back(it->second->GetScope());
    }
  }
  return all_timers;
}

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::GetLeft(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return *scope_tree_.FindPreviousScopeAtDepth(timer);
}

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::GetRight(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return *scope_tree_.FindNextScopeAtDepth(timer);
}

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::GetUp(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return *scope_tree_.FindParent(timer);
}

const orbit_client_protos::TimerInfo& ScopeTreeTimerData::GetDown(
    const orbit_client_protos::TimerInfo& timer) const {
  absl::MutexLock lock(&scope_tree_mutex_);
  return *scope_tree_.FindFirstChild(timer);
}

}  // namespace orbit_client_data