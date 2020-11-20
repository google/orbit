// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_TIMER_QUERY_POOL_H_
#define ORBIT_VULKAN_LAYER_TIMER_QUERY_POOL_H_

#include "OrbitBase/Logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "vulkan/vulkan.h"

namespace orbit_vulkan_layer {

namespace internal {
enum SlotState {
  kReadyForQueryIssue = 0,
  kQueryPendingOnGPU,
};
}

/*
 * This class wraps Vulkan's VkQueryPool specific for timestamp queries, and provides utility
 * methods to (1) initialize a poo, (2) retrieve an available slot index and (3) reset slot indices.
 * In order to do so, it stores the internal `SlotState` for each index.
 *
 * Thread-Safety: This class is internally synchronized (using read/write locks) and can be safely
 * accessed from different threads.
 */
template <class DispatchTable>
class TimerQueryPool {
 public:
  explicit TimerQueryPool(DispatchTable* dispatch_table, uint32_t num_timer_query_slots)
      : dispatch_table_(dispatch_table), num_timer_query_slots_(num_timer_query_slots) {}

  /*
   * Creates and resets a vulkan `VkQueryPool`, ready to use for timestamp queries.
   */
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
    CHECK(result == VK_SUCCESS);

    dispatch_table_->ResetQueryPoolEXT(device)(device, query_pool, 0, num_timer_query_slots_);

    {
      absl::WriterMutexLock lock(&mutex_);
      device_to_query_pool_[device] = query_pool;
      std::vector<internal::SlotState> slots{num_timer_query_slots_};
      std::fill(slots.begin(), slots.end(), internal::SlotState::kReadyForQueryIssue);
      device_to_query_slots_[device] = slots;
      device_to_potential_next_free_index_[device] = 0;
    }
  }

  /*
   * Retrieves the query pool for a give device. Note, that the pool must be initialized using
   * `InitializeTimerQueryPool` before.
   */
  [[nodiscard]] VkQueryPool GetQueryPool(VkDevice device) {
    absl::ReaderMutexLock lock(&mutex_);
    CHECK(device_to_query_pool_.contains(device));
    return device_to_query_pool_.at(device);
  }

  /*
   * Tries to find a free query slot in the device`s pool. It returns `false` if no slot was found
   * (all slots are occupied) and true otherwise.
   * In case it successfully found a slot, the index will be written to the given `allocated_index`.
   *
   * Note, that the pool pool must be initialized using `InitializeTimerQueryPool` before.
   * See also `ResetQuerySlots` to make occupied slots available again.
   */
  [[nodiscard]] bool NextReadyQuerySlot(VkDevice device, uint32_t* allocated_index) {
    absl::WriterMutexLock lock(&mutex_);
    CHECK(device_to_potential_next_free_index_.contains(device));
    CHECK(device_to_query_slots_.contains(device));
    uint32_t potential_next_free_slot = device_to_potential_next_free_index_.at(device);
    std::vector<internal::SlotState>& slots = device_to_query_slots_.at(device);
    uint32_t current_slot = potential_next_free_slot;
    do {
      if (slots.at(current_slot) == internal::SlotState::kReadyForQueryIssue) {
        device_to_potential_next_free_index_[device] = (current_slot + 1) % num_timer_query_slots_;
        slots.at(current_slot) = internal::SlotState::kQueryPendingOnGPU;
        *allocated_index = current_slot;
        return true;
      }
      current_slot = (current_slot + 1) % num_timer_query_slots_;
    } while (current_slot != potential_next_free_slot);

    return false;
  }

  /*
   * Resets an occupied slot to be ready for queries again.
   * If `rollback_only` is set, it will not call to Vulkan to reset the content at that slot.
   * This is useful, if the slot was retrieved, but the actual query was not yet submitted to
   * Vulkan (e.g. if on resetting the command buffer).
   *
   * Note, that the pool pool must be initialized using `InitializeTimerQueryPool` before.
   * Further, the given slots must be in the `kReadyForQueryIssue` state, i.e. must be a result
   * of `NextReadyQuerySlot` and have not been reset yet.
   */
  void ResetQuerySlots(VkDevice device, const std::vector<uint32_t>& physical_slot_indices,
                       bool rollback_only) {
    if (physical_slot_indices.empty()) {
      return;
    }
    absl::WriterMutexLock lock(&mutex_);
    CHECK(device_to_query_slots_.contains(device));
    std::vector<internal::SlotState>& slot_states = device_to_query_slots_.at(device);
    for (uint32_t physical_slot_index : physical_slot_indices) {
      CHECK(physical_slot_index < num_timer_query_slots_);
      const internal::SlotState& current_state = slot_states.at(physical_slot_index);
      CHECK(current_state == internal::SlotState::kQueryPendingOnGPU);
      slot_states.at(physical_slot_index) = internal::SlotState::kReadyForQueryIssue;
      if (rollback_only) {
        continue;
      }
      VkQueryPool query_pool = device_to_query_pool_.at(device);
      dispatch_table_->ResetQueryPoolEXT(device)(device, query_pool, physical_slot_index, 1);
    }
  }

 private:
  DispatchTable* dispatch_table_;
  const uint32_t num_timer_query_slots_;

  absl::Mutex mutex_;

  absl::flat_hash_map<VkDevice, VkQueryPool> device_to_query_pool_;
  absl::flat_hash_map<VkDevice, std::vector<internal::SlotState>> device_to_query_slots_;
  absl::flat_hash_map<VkDevice, uint32_t> device_to_potential_next_free_index_;
};
}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_TIMER_QUERY_POOL_H_
