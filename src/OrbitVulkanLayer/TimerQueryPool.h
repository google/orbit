// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_TIMER_QUERY_POOL_H_
#define ORBIT_VULKAN_LAYER_TIMER_QUERY_POOL_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <absl/types/span.h>
#include <vulkan/vulkan.h>

#include <fstream>
#include <numeric>
#include <vector>

#include "OrbitBase/Logging.h"

namespace orbit_vulkan_layer {

// This class wraps Vulkan's VkQueryPool explicitly for timestamp queries, and provides utility
// methods to (1) initialize a pool, (2) retrieve an available slot index and (3) reset slot
// indices.
// In order to do so, it stores the internal `SlotState` for each index.
//
// Slots can be requested using `NextReadyQuerySlot, which will block them until being reset again.
// If the command buffer storing the slot index gets reset before it was even submitted,
// `RollbackPendingQuerySlots` can be called to mark the slot as being free (ready) again without
// the need of actually resetting the timer value underlying the slot.
// Once submitted, the slot is baked into the command buffer (until the command buffer is being
// reset again). In order to free that slot, two things need to happen:
// 1. The client (this Vulkan layer) needs to communicate that it will not do any attempts to read
//    that slot anymore (done by calling `MarkSlotDoneReading`).
// 2. The command buffer needs to reset, to ensure that the slot index is not baked into it anymore
//    (done by calling `MarkSlotForReset`).
//
// The following state machine represents the allowed calls.
//
// MarkQuerySlotForReset                  MarkQuerySlotDoneReading
//           ------------- kDoneReading <--------------
//          |                                          |
//          |                                          |
//          |           NextReadyQuerySlot             |
//          v          --------------------->          |
// kReadyForQueryIssue                        kQueryPendingOnGpu
//          ^          <---------------------          |
//          |         RollbackPendingQuerySlots        |
//          |                                          |
//          |                                          |
//           ------------ kResetRequested <------------
// MarkQuerySlotDoneReading                   MarkQuerySlotForReset
//
//
// Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
// accessed from different threads.
template <class DispatchTable>
class TimerQueryPool {
 public:
  explicit TimerQueryPool(DispatchTable* dispatch_table, uint32_t num_timer_query_slots)
      : dispatch_table_(dispatch_table), num_timer_query_slots_(num_timer_query_slots) {}

  // Creates and resets a vulkan `VkQueryPool`, ready to use for timestamp queries.
  void InitializeTimerQueryPool(VkDevice device) {
    VkQueryPool query_pool;

    VkQueryPoolCreateInfo create_info = {.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                         .pNext = nullptr,
                                         .flags = 0,
                                         .queryType = VK_QUERY_TYPE_TIMESTAMP,
                                         .queryCount = num_timer_query_slots_,
                                         .pipelineStatistics = 0};

    VkResult result =
        dispatch_table_->CreateQueryPool(device)(device, &create_info, nullptr, &query_pool);
    ORBIT_CHECK(result == VK_SUCCESS);

    dispatch_table_->ResetQueryPoolEXT(device)(device, query_pool, 0, num_timer_query_slots_);

    {
      absl::WriterMutexLock lock(&mutex_);
      ORBIT_CHECK(!device_to_query_pool_.contains(device));
      device_to_query_pool_[device] = query_pool;
      std::vector<SlotState> slots{num_timer_query_slots_};
      std::fill(slots.begin(), slots.end(), SlotState::kReadyForQueryIssue);
      device_to_query_slots_[device] = slots;
      std::vector<uint32_t> free_slots(num_timer_query_slots_);
      // At the beginning all slot indices in [0, num_timer_query_slots) are free.
      std::iota(free_slots.begin(), free_slots.end(), 0);
      device_to_free_slots_[device] = free_slots;
    }
  }

  // Destroys the VkQueryPool for the given device
  void DestroyTimerQueryPool(VkDevice device) {
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(device_to_query_pool_.contains(device));
    VkQueryPool query_pool = device_to_query_pool_.at(device);
    device_to_query_slots_.erase(device);
    device_to_free_slots_.erase(device);

    dispatch_table_->DestroyQueryPool(device)(device, query_pool, nullptr);

    device_to_query_pool_.erase(device);
  }

  // Retrieves the query pool for a given device. Note that the pool must be initialized using
  // `InitializeTimerQueryPool` before.
  [[nodiscard]] VkQueryPool GetQueryPool(VkDevice device) {
    absl::ReaderMutexLock lock(&mutex_);
    ORBIT_CHECK(device_to_query_pool_.contains(device));
    return device_to_query_pool_.at(device);
  }

  // Returns a free query slot from the device's pool if one still exists. It returns `false` if all
  // slots are occupied and true otherwise. If successful, the index will be written to the given
  // `allocated_index`.
  //
  // Note that the pool must be initialized using `InitializeTimerQueryPool` before.
  // See also `ResetQuerySlots` to make occupied slots available again.
  [[nodiscard]] bool NextReadyQuerySlot(VkDevice device, uint32_t* allocated_index) {
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(device_to_free_slots_.contains(device));
    ORBIT_CHECK(device_to_query_slots_.contains(device));
    std::vector<uint32_t>& free_slots = device_to_free_slots_.at(device);
    if (free_slots.empty()) {
      return false;
    }
    *allocated_index = free_slots.back();
    free_slots.pop_back();

    ORBIT_CHECK(device_to_query_slots_.at(device)[*allocated_index] ==
                SlotState::kReadyForQueryIssue);
    device_to_query_slots_.at(device)[*allocated_index] = SlotState::kQueryPendingOnGpu;
    return true;
  }

  // Marks for the given slots that the Vulkan layer will not do any attempts to read the underlying
  // slots anymore after this call.
  //
  // If for a given slot `MarkQuerySlotsForReset` was already called (i.e. `vkResetCommandBuffer`
  // was called on the command buffers using the slot), it resets an occupied slot to be ready for
  // queries again. In this case it will also call to Vulkan to reset the content of that slot
  // (in contrast to `RollbackPendingQuerySlots`).
  //
  // Note that the pool must be initialized using `InitializeTimerQueryPool` before.
  // Further, the given slots must be in the `kReadyForQueryIssue` state, i.e. must be a result
  // of `NextReadyQuerySlot` and must not have been reset yet.
  void MarkQuerySlotsDoneReading(VkDevice device, absl::Span<const uint32_t> slot_indices) {
    if (slot_indices.empty()) {
      return;
    }
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(device_to_query_slots_.contains(device));
    std::vector<SlotState>& slot_states = device_to_query_slots_.at(device);
    ORBIT_CHECK(device_to_free_slots_.contains(device));
    std::vector<uint32_t>& free_slots = device_to_free_slots_.at(device);
    for (uint32_t slot_index : slot_indices) {
      ORBIT_CHECK(slot_index < num_timer_query_slots_);
      const SlotState& current_state = slot_states[slot_index];
      if (current_state == SlotState::kQueryPendingOnGpu) {
        slot_states[slot_index] = SlotState::kDoneReading;
        continue;
      }
      ORBIT_CHECK(current_state == SlotState::kResetRequested);
      slot_states[slot_index] = SlotState::kReadyForQueryIssue;
      free_slots.push_back(slot_index);
      VkQueryPool query_pool = device_to_query_pool_.at(device);
      dispatch_table_->ResetQueryPoolEXT(device)(device, query_pool, slot_index, 1);
    }
  }

  // Marks that the underlying slots are not used by any command buffer anymore
  // (i.e. `vkResetCommandBuffer` was called on the command buffers using the slots).
  //
  // If for a given slot `MarkQuerySlotsDoneReading` was already called (i.e. the layer will not do
  // any attempt to read the slot), it resets an occupied slot to be ready for queries again.
  // In this case it will also call to Vulkan to reset the content of that slot
  // (in contrast to `RollbackPendingQuerySlots`).
  //
  // Note that the pool must be initialized using `InitializeTimerQueryPool` before.
  // Further, the given slots must be in the `kReadyForQueryIssue` state, i.e. must be a result
  // of `NextReadyQuerySlot` and must not have been reset yet.
  void MarkQuerySlotsForReset(VkDevice device, absl::Span<const uint32_t> slot_indices) {
    if (slot_indices.empty()) {
      return;
    }
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(device_to_query_slots_.contains(device));
    std::vector<SlotState>& slot_states = device_to_query_slots_.at(device);
    ORBIT_CHECK(device_to_free_slots_.contains(device));
    std::vector<uint32_t>& free_slots = device_to_free_slots_.at(device);
    for (uint32_t slot_index : slot_indices) {
      ORBIT_CHECK(slot_index < num_timer_query_slots_);
      const SlotState& current_state = slot_states[slot_index];
      if (current_state == SlotState::kQueryPendingOnGpu) {
        slot_states[slot_index] = SlotState::kResetRequested;
        continue;
      }
      ORBIT_CHECK(current_state == SlotState::kDoneReading);
      slot_states[slot_index] = SlotState::kReadyForQueryIssue;
      free_slots.push_back(slot_index);
      VkQueryPool query_pool = device_to_query_pool_.at(device);
      dispatch_table_->ResetQueryPoolEXT(device)(device, query_pool, slot_index, 1);
    }
  }

  // Resets an occupied slot to be ready for queries again. It will *not* call to Vulkan to reset
  // the content of that slot (in contrast to `MarkQuerySlotsForReset` or
  // `MarkQuerySlotsDoneReading`). This is useful, if the slot was retrieved, but the actual query
  // was not yet submitted to Vulkan (e.g. if on resetting the command buffer).
  //
  // Note that the pool must be initialized using `InitializeTimerQueryPool` before.
  // Further, the given slots must be in the `kReadyForQueryIssue` state, i.e. must be a result
  // of `NextReadyQuerySlot` and must not have been reset yet.
  void RollbackPendingQuerySlots(VkDevice device, absl::Span<const uint32_t> slot_indices) {
    if (slot_indices.empty()) {
      return;
    }
    absl::WriterMutexLock lock(&mutex_);
    ORBIT_CHECK(device_to_query_slots_.contains(device));
    std::vector<SlotState>& slot_states = device_to_query_slots_.at(device);
    ORBIT_CHECK(device_to_free_slots_.contains(device));
    std::vector<uint32_t>& free_slots = device_to_free_slots_.at(device);
    for (uint32_t slot_index : slot_indices) {
      ORBIT_CHECK(slot_index < num_timer_query_slots_);
      const SlotState& current_state = slot_states[slot_index];
      ORBIT_CHECK(current_state == SlotState::kQueryPendingOnGpu);
      slot_states[slot_index] = SlotState::kReadyForQueryIssue;
      free_slots.push_back(slot_index);
    }
  }

 private:
  enum class SlotState {
    kReadyForQueryIssue = 0,
    kQueryPendingOnGpu = 1,
    kDoneReading = 2,
    kResetRequested = 3
  };

  DispatchTable* dispatch_table_;
  const uint32_t num_timer_query_slots_;

  absl::Mutex mutex_;

  absl::flat_hash_map<VkDevice, VkQueryPool> device_to_query_pool_;
  absl::flat_hash_map<VkDevice, std::vector<SlotState>> device_to_query_slots_;
  absl::flat_hash_map<VkDevice, std::vector<uint32_t>> device_to_free_slots_;
};
}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_TIMER_QUERY_POOL_H_
