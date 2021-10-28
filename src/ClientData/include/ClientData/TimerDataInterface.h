// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_INTERFACE_H_
#define CLIENT_DATA_TIMER_DATA_INTERFACE_H_

#include "TimerChain.h"
#include "capture_data.pb.h"

namespace orbit_client_data {

struct TimerMetadata {
  bool is_empty;
  size_t number_of_timers;
  uint64_t min_time;
  uint64_t max_time;
  uint32_t depth;
  uint32_t process_id;
};

// Interface to be use by TimerDataProvider to access data from TimerTracks.
class TimerDataInterface {
 public:
  virtual ~TimerDataInterface() = default;

  virtual const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info,
                                                         uint32_t depth) = 0;
  [[nodiscard]] virtual TimerMetadata GetTimerMetadata() const = 0;
  [[nodiscard]] virtual std::vector<const TimerChain*> GetChains() const = 0;
  [[nodiscard]] virtual std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      uint64_t min_tick, uint64_t max_tick) const = 0;
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer) const = 0;
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer) const = 0;
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer) const = 0;
  [[nodiscard]] virtual const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer) const = 0;

  // Only used in ScopeTreeTimerData
  [[nodiscard]] virtual int64_t GetThreadId() const = 0;
  virtual void OnCaptureComplete() = 0;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_TIMER_DATA_INTERFACE_H_
