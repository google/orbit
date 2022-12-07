// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_TIMER_DATA_INTERFACE_H_
#define CLIENT_DATA_TIMER_DATA_INTERFACE_H_

#include "ClientProtos/capture_data.pb.h"
#include "FastRenderingUtils.h"
#include "TimerChain.h"

namespace orbit_client_data {

// Interface to be use by TimerDataProvider to access data from TimerTracks.
class TimerDataInterface {
 public:
  virtual ~TimerDataInterface() = default;

  virtual const orbit_client_protos::TimerInfo& AddTimer(orbit_client_protos::TimerInfo timer_info,
                                                         uint32_t depth) = 0;

  // Timers queries
  [[nodiscard]] virtual std::vector<const TimerChain*> GetChains() const = 0;
  [[nodiscard]] virtual std::vector<const orbit_client_protos::TimerInfo*> GetTimers(
      uint64_t min_tick, uint64_t max_tick) const = 0;
  // Returns timers in a particular depth avoiding completely overlapped timers that map to the
  // same pixels in the screen. It assures to return at least one timer in each occupied pixel. The
  // overall complexity is faster than GetTimers since it doesn't require going through all timers.
  [[nodiscard]] virtual std::vector<const orbit_client_protos::TimerInfo*>
  GetTimersAtDepthDiscretized(uint32_t depth, uint32_t resolution, uint64_t start_ns,
                              uint64_t end_ns) const = 0;

  // Metadata queries
  [[nodiscard]] virtual bool IsEmpty() const = 0;
  [[nodiscard]] virtual size_t GetNumberOfTimers() const = 0;
  [[nodiscard]] virtual uint64_t GetMinTime() const = 0;
  [[nodiscard]] virtual uint64_t GetMaxTime() const = 0;
  [[nodiscard]] virtual uint32_t GetDepth() const = 0;
  [[nodiscard]] virtual uint32_t GetProcessId() const = 0;

  // Relative timers queries
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
