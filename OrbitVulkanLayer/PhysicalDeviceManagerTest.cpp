// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PhysicalDeviceManager.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Return;

namespace orbit_vulkan_layer {

class MockDispatchTable {
 public:
  MOCK_METHOD(PFN_vkGetPhysicalDeviceProperties, GetPhysicalDeviceProperties,
              (VkPhysicalDevice dispatchable_object));
};

TEST(PhysicalDeviceManager, ANonTrackedDeviceCanNotBeQuerried) {
  MockDispatchTable dispatch_table;
  PhysicalDeviceManager manager(&dispatch_table);
  VkDevice device = {};
  EXPECT_DEATH({ (void)manager.GetPhysicalDeviceOfLogicalDevice(device); }, "");
}

TEST(PhysicalDeviceManager, DevicePropertiesCanNotBeQuerriedForNonTrackedDevices) {
  MockDispatchTable dispatch_table;
  PhysicalDeviceManager manager(&dispatch_table);
  VkPhysicalDevice device = {};
  EXPECT_DEATH({ (void)manager.GetPhysicalDeviceProperties(device); }, "");
}

VkPhysicalDeviceProperties physical_device_properties = {
    .apiVersion = 1, .driverVersion = 2, .limits = {.timestampPeriod = 3.14f}};
void mockGetPhysicalDeviceProperties(VkPhysicalDevice, VkPhysicalDeviceProperties* out_properties) {
  *out_properties = physical_device_properties;
}

TEST(PhysicalDeviceManager, ATrackedDeviceCanBeQuerried) {
  MockDispatchTable dispatch_table;
  PhysicalDeviceManager manager(&dispatch_table);
  VkDevice logical_device = {};
  VkPhysicalDevice physical_device = {};

  EXPECT_CALL(dispatch_table, GetPhysicalDeviceProperties)
      .WillOnce(Return(&mockGetPhysicalDeviceProperties));

  manager.TrackPhysicalDevice(physical_device, logical_device);

  ASSERT_EQ(physical_device, manager.GetPhysicalDeviceOfLogicalDevice(logical_device));
  VkPhysicalDeviceProperties actual_properties =
      manager.GetPhysicalDeviceProperties(physical_device);
  EXPECT_EQ(actual_properties.apiVersion, physical_device_properties.apiVersion);
  EXPECT_EQ(actual_properties.driverVersion, physical_device_properties.driverVersion);
  EXPECT_EQ(actual_properties.limits.timestampPeriod,
            physical_device_properties.limits.timestampPeriod);
}

TEST(PhysicalDeviceManager, UntrackingRemovesTrackedDevice) {
  MockDispatchTable dispatch_table;
  PhysicalDeviceManager manager(&dispatch_table);
  VkDevice logical_device = {};
  VkPhysicalDevice physical_device = {};

  EXPECT_CALL(dispatch_table, GetPhysicalDeviceProperties)
      .WillOnce(Return(&mockGetPhysicalDeviceProperties));

  manager.TrackPhysicalDevice(physical_device, logical_device);
  manager.UntrackLogicalDevice(logical_device);

  EXPECT_DEATH({ (void)manager.GetPhysicalDeviceOfLogicalDevice(logical_device); }, "");
}

TEST(PhysicalDeviceManager, UntrackingRemovesDeviceProperties) {
  MockDispatchTable dispatch_table;
  PhysicalDeviceManager manager(&dispatch_table);
  VkDevice logical_device = {};
  VkPhysicalDevice physical_device = {};

  EXPECT_CALL(dispatch_table, GetPhysicalDeviceProperties)
      .WillOnce(Return(&mockGetPhysicalDeviceProperties));

  manager.TrackPhysicalDevice(physical_device, logical_device);

  manager.UntrackLogicalDevice(logical_device);
  EXPECT_DEATH({ (void)manager.GetPhysicalDeviceProperties(physical_device); }, "");
}

}  // namespace orbit_vulkan_layer
