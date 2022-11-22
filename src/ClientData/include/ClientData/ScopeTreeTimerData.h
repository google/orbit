// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_SCOPE_TREE_TIMER_DATA_H_
#define CLIENT_DATA_SCOPE_TREE_TIMER_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <vector>

#include "ClientData/TimerChain.h"
#include "ClientProtos/capture_data.pb.h"
#include "Containers/ScopeTree.h"
#include "TimerData.h"
#include "TimerDataInterface.h"

namespace orbit_client_data {

// Stores all the timers from a particular ThreadId. Provides queries to get timers in a certain
// range as well as metadata from them.
class ScopeTreeTimerData final : public TimerDataInterface {
 public:
  enum class ScopeTreeUpdateType { kAlways, kOnCaptureComplete, kNever };
  explicit ScopeTreeTimerData(int64_t thread_id = -1, ScopeTreeUpdateType scope_tree_update_type =
                                                          ScopeTreeUpdateType::kAlways)
      : thread_id_(thread_id), scope_tree_update_type_(scope_tree_update_type){};

  // We are using a ScopeTree to automatically manage timers and their depth, no need to set it
  // here.
  const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info,
                                                 uint32_t /*unused_depth*/ = 0) override;
  // Timers queries
  [[nodiscard]] std::vector<const TimerChain*> GetChains() const override {
    return timer_data_.GetChains();
  }

  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      uint64_t start_ns = std::numeric_limits<uint64_t>::min(),
      uint64_t end_ns = std::numeric_limits<uint64_t>::max()) const override;
  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimersAtDepth(
      uint32_t depth, uint64_t start_ns = std::numeric_limits<uint64_t>::min(),
      uint64_t end_ns = std::numeric_limits<uint64_t>::max()) const;
  [[nodiscard]] std::vector<const orbit_client_protos::TimerInfo*> GetTimersAtDepthDiscretized(
      uint32_t depth, uint32_t resolution, uint64_t start_ns, uint64_t end_ns) const override;

  // Metadata queries
  [[nodiscard]] bool IsEmpty() const override { return GetNumberOfTimers() == 0; };
  // Special case. ScopeTree has a root node in depth 0 which shouldn't be considered.
  [[nodiscard]] size_t GetNumberOfTimers() const override {
    absl::MutexLock lock(&scope_tree_mutex_);
    return scope_tree_.Size() - 1;
  }
  [[nodiscard]] uint64_t GetMinTime() const override { return timer_data_.GetMinTime(); }
  [[nodiscard]] uint64_t GetMaxTime() const override { return timer_data_.GetMaxTime(); }
  [[nodiscard]] uint32_t GetDepth() const override {
    absl::MutexLock lock(&scope_tree_mutex_);
    return scope_tree_.Depth();
  }
  [[nodiscard]] uint32_t GetProcessId() const override { return timer_data_.GetProcessId(); }
  [[nodiscard]] int64_t GetThreadId() const override { return thread_id_; }

  // Relative timers queries
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer) const override;

  void OnCaptureComplete() override;

 private:
  const int64_t thread_id_;
  mutable absl::Mutex scope_tree_mutex_;
  orbit_containers::ScopeTree<const orbit_client_protos::TimerInfo> scope_tree_
      ABSL_GUARDED_BY(scope_tree_mutex_);
  ScopeTreeUpdateType scope_tree_update_type_;

  TimerData timer_data_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_SCOPE_TREE_TIMER_DATA_H_
