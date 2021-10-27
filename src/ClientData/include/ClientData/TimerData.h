// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_H_
#define CLIENT_DATA_TIMER_DATA_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "OrbitBase/ThreadConstants.h"
#include "TimerChain.h"
#include "capture_data.pb.h"

namespace orbit_client_data {

class TimerData final {
 public:
  [[nodiscard]] bool IsEmpty() const { return num_timers_ == 0; }
  [[nodiscard]] size_t GetNumberOfTimers() const { return num_timers_; }
  [[nodiscard]] uint64_t GetMinTime() const { return min_time_; }
  [[nodiscard]] uint64_t GetMaxTime() const { return max_time_; }
  [[nodiscard]] uint32_t GetMaxDepth() const { return max_depth_; }
  [[nodiscard]] uint32_t GetProcessId() const { return process_id_; }

  const orbit_client_protos::TimerInfo& AddTimer(uint64_t depth,
                                                 orbit_client_protos::TimerInfo timer_info);

  [[nodiscard]] std::vector<const TimerChain*> GetChains() const;
  [[nodiscard]] const TimerChain* GetChain(uint64_t depth) const;

  void UpdateMinTime(uint64_t min_time);
  void UpdateMaxTime(uint64_t max_time);

  void UpdateMaxDepth(uint32_t depth) { max_depth_ = std::max(max_depth_, depth); }

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetFirstAfterStartTime(uint64_t time,
                                                                             uint32_t depth) const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetFirstBeforeStartTime(uint64_t time,
                                                                              uint32_t depth) const;

 private:
  [[nodiscard]] TimerChain* GetOrCreateTimerChain(uint64_t depth);

  uint32_t max_depth_ = 0;
  mutable absl::Mutex mutex_;
  std::map<uint64_t, std::unique_ptr<TimerChain>> timers_ ABSL_GUARDED_BY(mutex_);
  std::atomic<size_t> num_timers_{0};
  std::atomic<uint64_t> min_time_{std::numeric_limits<uint64_t>::max()};
  std::atomic<uint64_t> max_time_{std::numeric_limits<uint64_t>::min()};

  uint32_t process_id_ = orbit_base::kInvalidProcessId;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_DATA_H_
