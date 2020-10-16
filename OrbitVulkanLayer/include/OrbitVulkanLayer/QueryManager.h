// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LAYER_QUERY_MANAGER_H_
#define ORBIT_LAYER_QUERY_MANAGER_H_

#include <array>
#include <cstdint>
#include <vector>

namespace orbit::layer {

/*
 * TODO: This class will manage query slots (indexes).
 */
class QuerySlotManager {
 public:
  // potential slot states
  // 0. freshly created, who cares
  // 1. Reset completed, ready for query  issue
  // 2. Query in flight, pending GPU completion
  // 3. Query completed, ready for readback
  // 4. Data readback, ready for reset
  // 5. Reset issued, pending GPU completion
  //    (loop back to 1)
  // As far as the future slot manager is concerned, 2/3 are combined.
  // The client is responsible for checking on completion because it has the
  // submit info. This could be re-arranged so the list of submits is given to
  // the slot manager to update the timestamp completion state.
  enum SlotState {
    ReadyForQueryIssue = 0,
    QueryPendingOnGPU,
    QueryReadbackReady,
    ReadyForResetIssue,
    ResetPendingOnGPU,
    Count,
  };

  QuerySlotManager() = default;
  ~QuerySlotManager() = default;

  bool NextReadyQuerySlot(uint32_t& allocatedIndex);
  void MarkSlots(std::vector<uint32_t>& slotsToMark, SlotState newState);
  void RollBackSlots(std::vector<uint32_t>& slotsToMark, SlotState rollbackState);

 private:
  static constexpr uint32_t kNumLogicalQuerySlots = 16384;
  static constexpr uint32_t kNumPhysicalTimerQuerySlots = kNumLogicalQuerySlots * 2;

  volatile std::array<SlotState, kNumLogicalQuerySlots> slot_state_;
  volatile uint32_t next_free_index_ = 0;
};

}  // namespace orbit::layer

#endif  // ORBIT_LAYER_QUERY_MANAGER_H_