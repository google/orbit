// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdint>
#include <vector>

#include "TimerQueryPool.h"

using ::testing::Return;

namespace orbit_vulkan_layer {

namespace {
class MockDispatchTable {
 public:
  MOCK_METHOD(PFN_vkCreateQueryPool, CreateQueryPool, (VkDevice), ());
  MOCK_METHOD(PFN_vkResetQueryPoolEXT, ResetQueryPoolEXT, (VkDevice), ());
  MOCK_METHOD(PFN_vkDestroyQueryPool, DestroyQueryPool, (VkDevice), ());
};

PFN_vkCreateQueryPool dummy_create_query_pool_function =
    +[](VkDevice /*device*/, const VkQueryPoolCreateInfo* /*create_info*/,
        const VkAllocationCallbacks* /*allocator*/, VkQueryPool* query_pool_out) -> VkResult {
  *query_pool_out = {};
  return VK_SUCCESS;
};

PFN_vkResetQueryPoolEXT dummy_reset_query_pool_function =
    +[](VkDevice /*device*/, VkQueryPool /*query_pool*/, uint32_t /*first_query*/,
        uint32_t /*query_count*/) {};

}  // namespace

TEST(TimerQueryPool, ATimerQueryPoolMustGetInitialized) {
  MockDispatchTable dispatch_table;
  uint32_t num_slots = 4;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, num_slots);
  VkDevice device = {};

  uint32_t slot_index = 32;
  std::vector<uint32_t> reset_slots;
  EXPECT_DEATH({ (void)query_pool.GetQueryPool(device); }, "");
  EXPECT_DEATH({ (void)query_pool.NextReadyQuerySlot(device, &slot_index); }, "");
  reset_slots.push_back(slot_index);
  EXPECT_DEATH({ (void)query_pool.MarkQuerySlotsDoneReading(device, reset_slots); }, "");
  EXPECT_DEATH({ (void)query_pool.MarkQuerySlotsForReset(device, reset_slots); }, "");
  EXPECT_DEATH({ (void)query_pool.RollbackPendingQuerySlots(device, reset_slots); }, "");
}

TEST(TimerQueryPool, InitializationWillCreateAndResetAPool) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 4;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};

  PFN_vkCreateQueryPool mock_create_query_pool_function =
      +[](VkDevice /*device*/, const VkQueryPoolCreateInfo* create_info,
          const VkAllocationCallbacks* /*allocator*/, VkQueryPool* query_pool_out) -> VkResult {
    EXPECT_EQ(create_info->queryType, VK_QUERY_TYPE_TIMESTAMP);
    EXPECT_EQ(create_info->queryCount, kNumSlots);

    *query_pool_out = {};
    return VK_SUCCESS;
  };

  PFN_vkResetQueryPoolEXT mock_reset_query_pool_function =
      +[](VkDevice /*device*/, VkQueryPool /*query_pool*/, uint32_t first_query,
          uint32_t query_count) {
        EXPECT_EQ(first_query, 0);
        EXPECT_EQ(query_count, kNumSlots);
      };

  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .Times(1)
      .WillOnce(Return(mock_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .Times(1)
      .WillOnce(Return(mock_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
}

TEST(TimerQueryPool, DestroyPoolForDeviceWillCallDestroyQueryPool) {
  MockDispatchTable dispatch_table;

  static constexpr uint32_t kNumSlots = 4;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};

  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  static bool was_called = false;
  PFN_vkDestroyQueryPool mock_destroy_query_pool_function =
      +[](VkDevice /*device*/, VkQueryPool /*query_pool*/,
          const VkAllocationCallbacks* /*allocator*/
       ) { was_called = true; };

  EXPECT_CALL(dispatch_table, DestroyQueryPool)
      .Times(1)
      .WillOnce(Return(mock_destroy_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  query_pool.DestroyTimerQueryPool(device);
  EXPECT_TRUE(was_called);
}

TEST(TimerQueryPool, QueryPoolCanBeRetrievedAfterInitialization) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 4;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  static VkQueryPool expected_vulkan_query_pool = {};

  PFN_vkCreateQueryPool mock_create_query_pool_function =
      +[](VkDevice /*device*/, const VkQueryPoolCreateInfo* create_info,
          const VkAllocationCallbacks* /*allocator*/, VkQueryPool* query_pool_out) -> VkResult {
    EXPECT_EQ(create_info->queryType, VK_QUERY_TYPE_TIMESTAMP);
    EXPECT_EQ(create_info->queryCount, kNumSlots);

    *query_pool_out = expected_vulkan_query_pool;
    return VK_SUCCESS;
  };

  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(mock_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  VkQueryPool vulkan_query_pool = query_pool.GetQueryPool(device);
  EXPECT_EQ(vulkan_query_pool, expected_vulkan_query_pool);
}

TEST(TimerQueryPool, CanRetrieveNumSlotsUniqueSlots) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 4;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};

  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  absl::flat_hash_set<uint32_t> slots;

  for (uint32_t i = 0; i < kNumSlots; ++i) {
    uint32_t slot;
    bool found_slot = query_pool.NextReadyQuerySlot(device, &slot);
    ASSERT_TRUE(found_slot);
    slots.insert(slot);
  }

  EXPECT_EQ(slots.size(), kNumSlots);
}

TEST(TimerQueryPool, CannotRetrieveMoreThanNumSlotsSlots) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 4;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};

  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  for (uint32_t i = 0; i < kNumSlots; ++i) {
    uint32_t slot;
    (void)query_pool.NextReadyQuerySlot(device, &slot);
  }
  uint32_t slot;
  bool found_slot = query_pool.NextReadyQuerySlot(device, &slot);
  ASSERT_FALSE(found_slot);
}

TEST(TimerQueryPool, RollingBackSlotsMakesSlotsReady) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  uint32_t slot;
  (void)query_pool.NextReadyQuerySlot(device, &slot);

  std::vector<uint32_t> reset_slots;
  reset_slots.push_back(slot);

  query_pool.RollbackPendingQuerySlots(device, reset_slots);

  bool found_slot = query_pool.NextReadyQuerySlot(device, &slot);
  ASSERT_TRUE(found_slot);
}

TEST(TimerQueryPool, MarkResetSlotsAloneDoesNotMakeSlotsReady) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  uint32_t slot;
  (void)query_pool.NextReadyQuerySlot(device, &slot);

  std::vector<uint32_t> reset_slots;
  reset_slots.push_back(slot);

  query_pool.MarkQuerySlotsForReset(device, reset_slots);

  bool found_slot = query_pool.NextReadyQuerySlot(device, &slot);
  ASSERT_FALSE(found_slot);
}

TEST(TimerQueryPool, MarkSlotsDoneReadingAloneDoesNotMakeSlotsReady) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  uint32_t slot;
  (void)query_pool.NextReadyQuerySlot(device, &slot);

  std::vector<uint32_t> reset_slots;
  reset_slots.push_back(slot);

  query_pool.MarkQuerySlotsDoneReading(device, reset_slots);

  bool found_slot = query_pool.NextReadyQuerySlot(device, &slot);
  ASSERT_FALSE(found_slot);
}

TEST(TimerQueryPool, MarkSlotsDoneReadingAndReadForResetMakesThemReady) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  uint32_t slot;
  (void)query_pool.NextReadyQuerySlot(device, &slot);

  std::vector<uint32_t> reset_slots;
  reset_slots.push_back(slot);

  query_pool.MarkQuerySlotsDoneReading(device, reset_slots);
  query_pool.MarkQuerySlotsForReset(device, reset_slots);

  bool found_slot = query_pool.NextReadyQuerySlot(device, &slot);
  ASSERT_TRUE(found_slot);
}

TEST(TimerQueryPool, RollingBackSlotsDoesNotResetOnVulkan) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));

  // Explicitly expect this call only once, as rolling back should not reset on vulkan side.
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .Times(1)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  uint32_t slot;
  (void)query_pool.NextReadyQuerySlot(device, &slot);

  std::vector<uint32_t> reset_slots;
  reset_slots.push_back(slot);

  query_pool.RollbackPendingQuerySlots(device, reset_slots);
}

TEST(TimerQueryPool, ResettingSlotsDoesResetOnVulkan) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));

  static uint32_t expected_reset_slot;
  PFN_vkResetQueryPoolEXT mock_reset_query_pool_function =
      +[](VkDevice /*device*/, VkQueryPool /*query_pool*/, uint32_t first_query,
          uint32_t query_count) {
        EXPECT_EQ(expected_reset_slot, first_query);
        EXPECT_EQ(1, query_count);
      };

  // Explicitly expect this call only once, as rolling back should not reset on vulkan side.
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .Times(2)
      .WillOnce(Return(dummy_reset_query_pool_function))
      .WillOnce(Return(mock_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);
  (void)query_pool.NextReadyQuerySlot(device, &expected_reset_slot);

  std::vector<uint32_t> reset_slots;
  reset_slots.push_back(expected_reset_slot);

  query_pool.MarkQuerySlotsDoneReading(device, reset_slots);
  query_pool.MarkQuerySlotsForReset(device, reset_slots);
}

TEST(TimerQueryPool, CannotRollbackReadySlots) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  uint32_t first_slot_index = 0;
  std::vector<uint32_t> rollback_slots;
  rollback_slots.push_back(first_slot_index);

  EXPECT_DEATH({ query_pool.RollbackPendingQuerySlots(device, rollback_slots); }, "");
}

TEST(TimerQueryPool, CannotMarkReadySlotsForReset) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  uint32_t first_slot_index = 0;
  std::vector<uint32_t> rollback_slots;
  rollback_slots.push_back(first_slot_index);

  EXPECT_DEATH({ query_pool.MarkQuerySlotsForReset(device, rollback_slots); }, "");
}

TEST(TimerQueryPool, CannotMarkReadySlotsDoneReading) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 1;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  uint32_t first_slot_index = 0;
  std::vector<uint32_t> rollback_slots;
  rollback_slots.push_back(first_slot_index);

  EXPECT_DEATH({ query_pool.MarkQuerySlotsDoneReading(device, rollback_slots); }, "");
}

TEST(TimerQueryPool, CanRepeatadlyRetrieveAndResetSlots) {
  MockDispatchTable dispatch_table;
  static constexpr uint32_t kNumSlots = 16;
  TimerQueryPool<MockDispatchTable> query_pool(&dispatch_table, kNumSlots);
  VkDevice device = {};
  EXPECT_CALL(dispatch_table, CreateQueryPool)
      .WillRepeatedly(Return(dummy_create_query_pool_function));
  EXPECT_CALL(dispatch_table, ResetQueryPoolEXT)
      .WillRepeatedly(Return(dummy_reset_query_pool_function));

  query_pool.InitializeTimerQueryPool(device);

  for (int i = 0; i < 2000; ++i) {
    uint32_t slot_index;
    bool found_slot = query_pool.NextReadyQuerySlot(device, &slot_index);
    EXPECT_TRUE(found_slot);

    std::vector<uint32_t> reset_slots;
    reset_slots.push_back(slot_index);

    // Check for both, rollback and actual resets.
    if (i % 2 == 0) {
      query_pool.MarkQuerySlotsDoneReading(device, reset_slots);
      query_pool.MarkQuerySlotsForReset(device, reset_slots);
    } else {
      query_pool.RollbackPendingQuerySlots(device, reset_slots);
    }
  }
}

}  // namespace orbit_vulkan_layer
