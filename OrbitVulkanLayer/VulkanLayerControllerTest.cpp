// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vulkan/vk_layer.h>

#include "VulkanLayerController.h"
#include "VulkanLayerProducer.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AllOf;
using ::testing::Field;
using ::testing::IsSubsetOf;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::UnorderedElementsAreArray;

namespace orbit_vulkan_layer {

namespace {

class MockDispatchTable {
 public:
  MOCK_METHOD(PFN_vkEnumerateDeviceExtensionProperties, EnumerateDeviceExtensionProperties,
              (VkPhysicalDevice));
  MOCK_METHOD((void), CreateInstanceDispatchTable, (VkInstance, PFN_vkGetInstanceProcAddr));
  MOCK_METHOD((void), CreateDeviceDispatchTable, (VkDevice, PFN_vkGetDeviceProcAddr));
  MOCK_METHOD((void), RemoveInstanceDispatchTable, (VkInstance));
  MOCK_METHOD((void), RemoveDeviceDispatchTable, (VkDevice));
  MOCK_METHOD(PFN_vkGetDeviceProcAddr, GetDeviceProcAddr, (VkDevice));
  MOCK_METHOD(PFN_vkGetInstanceProcAddr, GetInstanceProcAddr, (VkInstance));
  MOCK_METHOD(PFN_vkDestroyInstance, DestroyInstance, (VkInstance));
  MOCK_METHOD(PFN_vkDestroyDevice, DestroyDevice, (VkDevice));
  MOCK_METHOD(PFN_vkResetCommandPool, ResetCommandPool, (VkDevice));
  MOCK_METHOD(PFN_vkAllocateCommandBuffers, AllocateCommandBuffers, (VkDevice));
  MOCK_METHOD(PFN_vkFreeCommandBuffers, FreeCommandBuffers, (VkDevice));
  MOCK_METHOD(PFN_vkBeginCommandBuffer, BeginCommandBuffer, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkEndCommandBuffer, EndCommandBuffer, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkResetCommandBuffer, ResetCommandBuffer, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkGetDeviceQueue, GetDeviceQueue, (VkDevice));
  MOCK_METHOD(PFN_vkGetDeviceQueue2, GetDeviceQueue2, (VkDevice));
  MOCK_METHOD(PFN_vkQueueSubmit, QueueSubmit, (VkQueue));
  MOCK_METHOD(PFN_vkQueuePresentKHR, QueuePresentKHR, (VkQueue));
  MOCK_METHOD(PFN_vkCmdBeginDebugUtilsLabelEXT, CmdBeginDebugUtilsLabelEXT, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkCmdEndDebugUtilsLabelEXT, CmdEndDebugUtilsLabelEXT, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkCmdDebugMarkerBeginEXT, CmdDebugMarkerBeginEXT, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkCmdDebugMarkerEndEXT, CmdDebugMarkerEndEXT, (VkCommandBuffer));
  MOCK_METHOD(bool, IsDebugUtilsExtensionSupported, (VkCommandBuffer));
  MOCK_METHOD(bool, IsDebugMarkerExtensionSupported, (VkCommandBuffer));
};

class MockDeviceManager {
 public:
  explicit MockDeviceManager(MockDispatchTable* /*dispatch_table*/) {}
  MOCK_METHOD((void), TrackLogicalDevice, (VkPhysicalDevice, VkDevice));
  MOCK_METHOD((void), UntrackLogicalDevice, (VkDevice));
};

class MockQueueManager {
 public:
  MOCK_METHOD((void), TrackQueue, (VkQueue, VkDevice));
  MOCK_METHOD((VkDevice), GetDeviceOfQueue, (VkQueue));
};

class MockTimerQueryPool {
 public:
  explicit MockTimerQueryPool(MockDispatchTable* /*dispatch_table*/, uint32_t /*num_slots*/) {}
  MOCK_METHOD((void), InitializeTimerQueryPool, (VkDevice));
};

struct Color {
  float red;
  float green;
  float blue;
  float alpha;
};

class MockSubmissionTracker {
 public:
  explicit MockSubmissionTracker(MockDispatchTable* /*dispatch_table*/,
                                 MockTimerQueryPool* /*timer_query_pool*/,
                                 MockDeviceManager* /*device_manager*/, uint32_t /*max_depth*/) {}
  MOCK_METHOD((void), SetVulkanLayerProducer, (VulkanLayerProducer*));
  MOCK_METHOD((void), ResetCommandPool, (VkCommandPool));
  MOCK_METHOD((void), TrackCommandBuffers,
              (VkDevice, VkCommandPool, const VkCommandBuffer*, uint32_t));
  MOCK_METHOD((void), UntrackCommandBuffers,
              (VkDevice, VkCommandPool, const VkCommandBuffer*, uint32_t));
  MOCK_METHOD((void), MarkCommandBufferBegin, (VkCommandBuffer));
  MOCK_METHOD((void), MarkCommandBufferEnd, (VkCommandBuffer));
  MOCK_METHOD((void), ResetCommandBuffer, (VkCommandBuffer));
  // Note we simplify the return type, as it is not really part of this test.
  MOCK_METHOD(bool, PersistCommandBuffersOnSubmit, (uint32_t, const VkSubmitInfo* submits));
  MOCK_METHOD(bool, PersistDebugMarkersOnSubmit, (VkQueue, uint32_t, const VkSubmitInfo*, bool));
  MOCK_METHOD((void), CompleteSubmits, (VkDevice));
  MOCK_METHOD((void), MarkDebugMarkerBegin, (VkCommandBuffer, const char*, Color));
  MOCK_METHOD((void), MarkDebugMarkerEnd, (VkCommandBuffer));
};

using VulkanLayerControllerImpl =
    VulkanLayerController<MockDispatchTable, MockQueueManager, MockDeviceManager,
                          MockTimerQueryPool, MockSubmissionTracker>;

Matcher<VkExtensionProperties> VkExtensionPropertiesAreEqual(
    const VkExtensionProperties& expected) {
  return AllOf(
      Field("specVersion", &VkExtensionProperties::specVersion, expected.specVersion),
      Field("extensionName", &VkExtensionProperties::extensionName, StrEq(expected.extensionName)));
}
}  // namespace

class VulkanLayerControllerTest : public ::testing::Test {
 protected:
  VulkanLayerController<MockDispatchTable, MockQueueManager, MockDeviceManager, MockTimerQueryPool,
                        MockSubmissionTracker>
      controller_;
  static constexpr VkExtensionProperties kDebugMarkerExtension{
      .extensionName = VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
      .specVersion = VK_EXT_DEBUG_MARKER_SPEC_VERSION};
  static constexpr VkExtensionProperties kDebugUtilsExtension{
      .extensionName = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
      .specVersion = VK_EXT_DEBUG_UTILS_SPEC_VERSION};
  static constexpr VkExtensionProperties kHostQueryResetExtension{
      .extensionName = VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
      .specVersion = VK_EXT_HOST_QUERY_RESET_SPEC_VERSION};

  static constexpr VkExtensionProperties kFakeExtension1{.extensionName = "Other Extension 1",
                                                         .specVersion = 3};
  static constexpr VkExtensionProperties kFakeExtension2{.extensionName = "Other Extension 2",
                                                         .specVersion = 2};

  static constexpr PFN_vkEnumerateDeviceExtensionProperties
      kMockEnumerateDeviceExtensionPropertiesFunction =
          +[](VkPhysicalDevice /*physical_device*/, const char* /*layer_name*/,
              uint32_t* property_count, VkExtensionProperties* properties) {
            if (property_count != nullptr) {
              *property_count = 3;
            }

            std::array<VkExtensionProperties, 3> fake_extensions{kFakeExtension1, kFakeExtension2,
                                                                 kDebugMarkerExtension};
            if (properties != nullptr) {
              memcpy(properties, fake_extensions.data(), 3 * sizeof(VkExtensionProperties));
            }

            return VK_SUCCESS;
          };
};

// ----------------------------------------------------------------------------
// Layer enumeration functions
// ----------------------------------------------------------------------------
TEST_F(VulkanLayerControllerTest, CanEnumerateTheLayersInstanceLayerProperties) {
  uint32_t actual_property_count;
  VkResult result = controller_.OnEnumerateInstanceLayerProperties(&actual_property_count, nullptr);
  ASSERT_EQ(result, VK_SUCCESS);
  ASSERT_EQ(actual_property_count, 1);

  VkLayerProperties actual_properties;
  result =
      controller_.OnEnumerateInstanceLayerProperties(&actual_property_count, &actual_properties);
  ASSERT_EQ(result, VK_SUCCESS);
  EXPECT_STREQ(actual_properties.layerName, VulkanLayerControllerImpl::kLayerName);
  EXPECT_STREQ(actual_properties.description, VulkanLayerControllerImpl::kLayerDescription);
  EXPECT_EQ(actual_properties.specVersion, VulkanLayerControllerImpl::kLayerSpecVersion);
  EXPECT_EQ(actual_properties.implementationVersion, VulkanLayerControllerImpl::kLayerImplVersion);
}

TEST_F(VulkanLayerControllerTest, TheLayerHasNoInstanceExtensionProperties) {
  uint32_t actual_property_count = 123;
  VkResult result = controller_.OnEnumerateInstanceExtensionProperties(
      VulkanLayerControllerImpl::kLayerName, &actual_property_count, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_EQ(actual_property_count, 0);
}

TEST_F(VulkanLayerControllerTest, ErrorOnEnumerateInstanceExtensionPropertiesForDifferentLayer) {
  uint32_t actual_property_count;
  VkResult result = controller_.OnEnumerateInstanceExtensionProperties(
      "some layer name", &actual_property_count, nullptr);
  EXPECT_EQ(result, VK_ERROR_LAYER_NOT_PRESENT);
}

TEST_F(VulkanLayerControllerTest, ErrorOnEnumerateInstanceExtensionPropertiesOnNullString) {
  uint32_t actual_property_count;
  VkResult result =
      controller_.OnEnumerateInstanceExtensionProperties(nullptr, &actual_property_count, nullptr);
  EXPECT_EQ(result, VK_ERROR_LAYER_NOT_PRESENT);
}

TEST_F(VulkanLayerControllerTest, CanEnumerateTheLayersExclusiveDeviceExtensionProperties) {
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count;
  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, VulkanLayerControllerImpl::kLayerName, &actual_property_count, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
  ASSERT_EQ(actual_property_count, 3);
  std::array<VkExtensionProperties, 3> actual_properties = {};
  result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, VulkanLayerControllerImpl::kLayerName, &actual_property_count,
      actual_properties.data());
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_THAT(actual_properties,
              UnorderedElementsAreArray({VkExtensionPropertiesAreEqual(kDebugMarkerExtension),
                                         VkExtensionPropertiesAreEqual(kDebugUtilsExtension),
                                         VkExtensionPropertiesAreEqual(kHostQueryResetExtension)}));
}

TEST_F(VulkanLayerControllerTest,
       CanEnumerateASubsetOfTheLayersExclusiveDeviceExtensionProperties) {
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count = 2;
  std::array<VkExtensionProperties, 2> actual_properties = {};
  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, VulkanLayerControllerImpl::kLayerName, &actual_property_count,
      actual_properties.data());
  EXPECT_EQ(result, VK_INCOMPLETE);
  ASSERT_EQ(actual_property_count, 2);
  EXPECT_THAT(actual_properties,
              IsSubsetOf({VkExtensionPropertiesAreEqual(kDebugMarkerExtension),
                          VkExtensionPropertiesAreEqual(kDebugUtilsExtension),
                          VkExtensionPropertiesAreEqual(kHostQueryResetExtension)}));
}

TEST_F(VulkanLayerControllerTest, WillForwardCallOnEnumerateOtherLayersDeviceExtensionProperties) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .Times(2)
      .WillRepeatedly(Return(kMockEnumerateDeviceExtensionPropertiesFunction));
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count;

  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, "other layer", &actual_property_count, nullptr);

  EXPECT_EQ(result, VK_SUCCESS);
  ASSERT_EQ(actual_property_count, 3);

  std::array<VkExtensionProperties, 3> actual_properties = {};
  result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, "other layer", &actual_property_count, actual_properties.data());
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_THAT(actual_properties,
              UnorderedElementsAreArray({VkExtensionPropertiesAreEqual(kFakeExtension1),
                                         VkExtensionPropertiesAreEqual(kFakeExtension2),
                                         VkExtensionPropertiesAreEqual(kDebugMarkerExtension)}));
}

TEST_F(VulkanLayerControllerTest,
       WillReturnErrorOnEnumerateAllLayersDeviceExtensionPropertiesError) {
  PFN_vkEnumerateDeviceExtensionProperties mock_enumerate_device_extension_properties_function =
      +[](VkPhysicalDevice /*physical_device*/, const char* /*layer_name*/,
          uint32_t* /*property_count*/,
          VkExtensionProperties* /*properties*/) { return VK_INCOMPLETE; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .Times(1)
      .WillRepeatedly(Return(mock_enumerate_device_extension_properties_function));
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count;
  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &actual_property_count, nullptr);
  EXPECT_EQ(result, VK_INCOMPLETE);
}

TEST_F(VulkanLayerControllerTest,
       WillMergePropertiesOnEnumerateAllLayersDeviceExtensionProperties) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .WillRepeatedly(Return(kMockEnumerateDeviceExtensionPropertiesFunction));
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count;

  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &actual_property_count, nullptr);

  EXPECT_EQ(result, VK_SUCCESS);
  ASSERT_EQ(actual_property_count, 5);

  std::array<VkExtensionProperties, 5> actual_properties = {};
  result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &actual_property_count, actual_properties.data());
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_THAT(actual_properties,
              UnorderedElementsAreArray({VkExtensionPropertiesAreEqual(kFakeExtension1),
                                         VkExtensionPropertiesAreEqual(kFakeExtension2),
                                         VkExtensionPropertiesAreEqual(kDebugMarkerExtension),
                                         VkExtensionPropertiesAreEqual(kDebugUtilsExtension),
                                         VkExtensionPropertiesAreEqual(kHostQueryResetExtension)}));
}

TEST_F(VulkanLayerControllerTest,
       CanMergePropertiesAndEnumerateASubsetForAllLayersDeviceExtensionProperties) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .WillRepeatedly(Return(kMockEnumerateDeviceExtensionPropertiesFunction));
  VkPhysicalDevice physical_device = {};

  std::array<VkExtensionProperties, 3> actual_properties = {};
  uint32_t stripped_property_count = 3;
  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &stripped_property_count, actual_properties.data());
  EXPECT_EQ(result, VK_INCOMPLETE);
  EXPECT_THAT(actual_properties,
              IsSubsetOf({VkExtensionPropertiesAreEqual(kFakeExtension1),
                          VkExtensionPropertiesAreEqual(kFakeExtension2),
                          VkExtensionPropertiesAreEqual(kDebugMarkerExtension),
                          VkExtensionPropertiesAreEqual(kDebugUtilsExtension),
                          VkExtensionPropertiesAreEqual(kHostQueryResetExtension)}));
}

// ----------------------------------------------------------------------------
// Layer bootstrapping code
// ----------------------------------------------------------------------------

TEST_F(VulkanLayerControllerTest, InitializationFailsOnCreateInstanceWithNoInfo) {
  VkInstance created_instance;
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = nullptr};
  VkResult result = controller_.OnCreateInstance(&create_info, nullptr, &created_instance);
  EXPECT_EQ(result, VK_ERROR_INITIALIZATION_FAILED);
}

TEST_F(VulkanLayerControllerTest,
       WillCreateDispatchTableAndVulkanLayerProducerAndAdvanceLinkageOnCreateInstance) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*dispatch_table, CreateInstanceDispatchTable).Times(1);
  EXPECT_CALL(*submission_tracker, SetVulkanLayerProducer).Times(1);

  static constexpr PFN_vkCreateInstance kMockDriverCreateInstance =
      +[](const VkInstanceCreateInfo* /*create_info*/, const VkAllocationCallbacks* /*allocator*/,
          VkInstance* /*instance*/) { return VK_SUCCESS; };

  PFN_vkGetInstanceProcAddr mock_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateInstance") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateInstance);
    }
    return nullptr;
  };

  VkLayerInstanceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = mock_get_instance_proc_addr};
  VkLayerInstanceLink layer_link_2 = {.pfnNextGetInstanceProcAddr = mock_get_instance_proc_addr,
                                      .pNext = &layer_link_1};
  VkLayerInstanceCreateInfo layer_create_info{
      .sType = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO,
      .function = VK_LAYER_LINK_INFO,
      .u.pLayerInfo = &layer_link_2};
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = &layer_create_info};
  VkInstance created_instance;
  VkResult result = controller_.OnCreateInstance(&create_info, nullptr, &created_instance);
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_EQ(layer_create_info.u.pLayerInfo, &layer_link_1);

  ::testing::Mock::VerifyAndClearExpectations(absl::bit_cast<void*>(submission_tracker));
  // There will be a call at the destructor.
  EXPECT_CALL(*submission_tracker, SetVulkanLayerProducer).Times(1);
}

TEST_F(VulkanLayerControllerTest, InitializationFailsOnCreateDeviceWithNoInfo) {
  VkDevice created_device;
  VkPhysicalDevice physical_device = {};
  VkDeviceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = nullptr};
  VkResult result =
      controller_.OnCreateDevice(physical_device, &create_info, nullptr, &created_device);
  EXPECT_EQ(result, VK_ERROR_INITIALIZATION_FAILED);
}

TEST_F(VulkanLayerControllerTest, CallInDispatchTableOnGetDeviceProcAddr) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  static constexpr PFN_vkVoidFunction kExpectedFunction = +[]() {};
  PFN_vkGetDeviceProcAddr mock_get_device_proc_addr =
      +[](VkDevice /*device*/, const char* /*name*/) { return kExpectedFunction; };
  EXPECT_CALL(*dispatch_table, GetDeviceProcAddr)
      .Times(1)
      .WillOnce(Return(mock_get_device_proc_addr));
  VkDevice device = {};
  PFN_vkVoidFunction result = controller_.OnGetDeviceProcAddr(device, "some function");
  EXPECT_EQ(result, kExpectedFunction);
}

TEST_F(VulkanLayerControllerTest,
       WillCreateDispatchTableAndVulkanLayerProducerAndAdvanceLinkageOnCreateDevice) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, CreateDeviceDispatchTable).Times(1);
  const MockDeviceManager* device_manager = controller_.device_manager();
  EXPECT_CALL(*device_manager, TrackLogicalDevice).Times(1);
  const MockTimerQueryPool* timer_query_pool = controller_.timer_query_pool();
  EXPECT_CALL(*timer_query_pool, InitializeTimerQueryPool).Times(1);

  static constexpr PFN_vkCreateDevice kMockDriverCreateDevice =
      +[](VkPhysicalDevice /*physical_device*/, const VkDeviceCreateInfo* /*create_info*/,
          const VkAllocationCallbacks* /*allocator*/,
          VkDevice* /*instance*/) { return VK_SUCCESS; };

  PFN_vkGetDeviceProcAddr mock_get_device_proc_addr =
      +[](VkDevice /*device*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  PFN_vkGetInstanceProcAddr mock_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateDevice") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateDevice);
    }
    return nullptr;
  };

  VkLayerDeviceLink layer_link_1 = {.pfnNextGetDeviceProcAddr = mock_get_device_proc_addr,
                                    .pfnNextGetInstanceProcAddr = mock_get_instance_proc_addr};
  VkLayerDeviceLink layer_link_2 = {.pfnNextGetDeviceProcAddr = mock_get_device_proc_addr,
                                    .pfnNextGetInstanceProcAddr = mock_get_instance_proc_addr,
                                    .pNext = &layer_link_1};
  VkLayerDeviceCreateInfo layer_create_info{.sType = VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO,
                                            .function = VK_LAYER_LINK_INFO,
                                            .u.pLayerInfo = &layer_link_2};
  VkDeviceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                 .pNext = &layer_create_info};
  VkDevice created_device;
  VkPhysicalDevice physical_device = {};
  VkResult result =
      controller_.OnCreateDevice(physical_device, &create_info, nullptr, &created_device);
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_EQ(layer_create_info.u.pLayerInfo, &layer_link_1);
}

TEST_F(VulkanLayerControllerTest, CallInDispatchTableOnGetInstanceProcAddr) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  static constexpr PFN_vkVoidFunction kExpectedFunction = +[]() {};
  PFN_vkGetInstanceProcAddr mock_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* /*name*/) { return kExpectedFunction; };
  EXPECT_CALL(*dispatch_table, GetInstanceProcAddr)
      .Times(1)
      .WillOnce(Return(mock_get_instance_proc_addr));
  VkInstance instance = {};
  PFN_vkVoidFunction result = controller_.OnGetInstanceProcAddr(instance, "some function");
  EXPECT_EQ(result, kExpectedFunction);
}

TEST_F(VulkanLayerControllerTest, WillClearUpOnDestroyInstance) {
  PFN_vkDestroyInstance mock_destroy_instance =
      +[](VkInstance /*instance*/, const VkAllocationCallbacks* /*allocator*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, DestroyInstance).Times(1).WillOnce(Return(mock_destroy_instance));
  EXPECT_CALL(*dispatch_table, RemoveInstanceDispatchTable).Times(1);

  VkInstance instance = {};

  controller_.OnDestroyInstance(instance, nullptr);
}

TEST_F(VulkanLayerControllerTest, WillClearUpOnDestroyDevice) {
  PFN_vkDestroyDevice mock_destroy_device =
      +[](VkDevice /*device*/, const VkAllocationCallbacks* /*allocator*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, DestroyDevice).Times(1).WillOnce(Return(mock_destroy_device));
  EXPECT_CALL(*dispatch_table, RemoveDeviceDispatchTable).Times(1);
  const MockDeviceManager* device_manager = controller_.device_manager();
  EXPECT_CALL(*device_manager, UntrackLogicalDevice).Times(1);

  VkDevice device = {};

  controller_.OnDestroyDevice(device, nullptr);
}

// ----------------------------------------------------------------------------
// Core layer logic
// ----------------------------------------------------------------------------

TEST_F(VulkanLayerControllerTest, CanDelegateOnResetCommandPool) {
  PFN_vkResetCommandPool mock_reset_command_pool =
      +[](VkDevice /*device*/, VkCommandPool /*command_pool*/,
          VkCommandPoolResetFlags /*flags*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, ResetCommandPool).Times(1).WillOnce(Return(mock_reset_command_pool));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, ResetCommandPool).Times(1);
  VkDevice device = {};
  VkCommandPool command_pool = {};
  VkCommandPoolResetFlags flags = {};
  VkResult result = controller_.OnResetCommandPool(device, command_pool, flags);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnAllocateCommandBuffers) {
  PFN_vkAllocateCommandBuffers mock_allocate_command_buffers =
      +[](VkDevice /*device*/, const VkCommandBufferAllocateInfo* /*allocate_info*/,
          VkCommandBuffer *
          /*command_buffers*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, AllocateCommandBuffers)
      .Times(1)
      .WillOnce(Return(mock_allocate_command_buffers));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, TrackCommandBuffers).Times(1);
  VkDevice device = {};
  VkCommandPool command_pool = {};
  VkCommandBuffer command_buffer;

  VkCommandBufferAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                            .pNext = nullptr,
                                            .commandBufferCount = 1,
                                            .commandPool = command_pool};

  VkResult result = controller_.OnAllocateCommandBuffers(device, &allocate_info, &command_buffer);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnFreeCommandBuffers) {
  PFN_vkFreeCommandBuffers mock_free_command_buffers =
      +[](VkDevice /*device*/, VkCommandPool /*command_pool*/, uint32_t /*command_buffer_count*/,
          const VkCommandBuffer*
          /*command_buffers*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, FreeCommandBuffers)
      .Times(1)
      .WillOnce(Return(mock_free_command_buffers));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, UntrackCommandBuffers).Times(1);
  VkDevice device = {};
  VkCommandPool command_pool = {};
  VkCommandBuffer command_buffer;

  controller_.OnFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnBeginCommandBuffer) {
  PFN_vkBeginCommandBuffer mock_begin_command_buffer =
      +[](VkCommandBuffer /*command_buffer*/, const VkCommandBufferBeginInfo *
          /*begin_info*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, BeginCommandBuffer)
      .Times(1)
      .WillOnce(Return(mock_begin_command_buffer));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkCommandBufferBegin).Times(1);
  VkCommandBuffer command_buffer = {};
  VkResult result = controller_.OnBeginCommandBuffer(command_buffer, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnEndCommandBuffer) {
  PFN_vkEndCommandBuffer mock_end_command_buffer =
      +[](VkCommandBuffer /*command_buffer*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EndCommandBuffer).Times(1).WillOnce(Return(mock_end_command_buffer));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkCommandBufferEnd).Times(1);
  VkCommandBuffer command_buffer = {};
  VkResult result = controller_.OnEndCommandBuffer(command_buffer);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnResetCommandBuffer) {
  PFN_vkResetCommandBuffer mock_reset_command_buffer =
      +[](VkCommandBuffer /*command_buffer*/, VkCommandBufferResetFlags /*flags*/) -> VkResult {
    return VK_SUCCESS;
  };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, ResetCommandBuffer)
      .Times(1)
      .WillOnce(Return(mock_reset_command_buffer));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, ResetCommandBuffer).Times(1);
  VkCommandBuffer command_buffer = {};
  VkResult result = controller_.OnResetCommandBuffer(
      command_buffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnGetDeviceQueue) {
  PFN_vkGetDeviceQueue mock_get_device_queue =
      +[](VkDevice /*device*/, uint32_t /*queue_family_index*/, uint32_t /*queue_index*/,
          VkQueue* /*queue*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, GetDeviceQueue).Times(1).WillOnce(Return(mock_get_device_queue));

  const MockQueueManager* queue_manager = controller_.queue_manager();
  EXPECT_CALL(*queue_manager, TrackQueue).Times(1);
  VkDevice device = {};
  VkQueue queue;
  controller_.OnGetDeviceQueue(device, 1, 2, &queue);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnGetDeviceQueue2) {
  PFN_vkGetDeviceQueue2 mock_get_device_queue2 =
      +[](VkDevice /*device*/, const VkDeviceQueueInfo2* /*queue_info*/, VkQueue* /*queue*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, GetDeviceQueue2).Times(1).WillOnce(Return(mock_get_device_queue2));

  const MockQueueManager* queue_manager = controller_.queue_manager();
  EXPECT_CALL(*queue_manager, TrackQueue).Times(1);

  VkDevice device = {};
  VkQueue queue;
  VkDeviceQueueInfo2 queue_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2, .queueFamilyIndex = 1, .queueIndex = 2};
  controller_.OnGetDeviceQueue2(device, &queue_info, &queue);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnGetQueueSubmit) {
  PFN_vkQueueSubmit mock_queue_submit =
      +[](VkQueue /*queue*/, uint32_t /*submit_count*/, const VkSubmitInfo* /*submits*/,
          VkFence /*fence*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, QueueSubmit).Times(1).WillOnce(Return(mock_queue_submit));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, PersistCommandBuffersOnSubmit).Times(1);
  EXPECT_CALL(*submission_tracker, PersistDebugMarkersOnSubmit).Times(1);

  VkQueue queue = {};
  VkCommandBuffer command_buffer = {};
  VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           .commandBufferCount = 1,
                           .pCommandBuffers = &command_buffer};
  VkFence fence = {};
  VkResult result = controller_.OnQueueSubmit(queue, 1, &submit_info, fence);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CanDelegateOnQueuePresentKHR) {
  PFN_vkQueuePresentKHR mock_queue_present =
      +[](VkQueue /*queue*/, const VkPresentInfoKHR * /*present_info*/) -> VkResult {
    return VK_SUCCESS;
  };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, QueuePresentKHR).Times(1).WillOnce(Return(mock_queue_present));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, CompleteSubmits).Times(1);
  VkDevice device = {};
  const MockQueueManager* queue_manager = controller_.queue_manager();
  EXPECT_CALL(*queue_manager, GetDeviceOfQueue).Times(1).WillOnce(Return(device));

  VkQueue queue = {};
  VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  VkResult result = controller_.OnQueuePresentKHR(queue, &present_info);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest,
       WillMarkDebugMarkerBeginButNotDelegateIfDriverDoesNotSupportDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported).Times(1).WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugUtilsLabelEXT debug_marker = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                       .pLabelName = "Marker"};
  controller_.OnCmdBeginDebugUtilsLabelEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest, WillDelegateOnBeginDebugLabelIfDriverDoesSupportDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported).Times(1).WillOnce(Return(true));

  PFN_vkCmdBeginDebugUtilsLabelEXT mock_cmd_begin_debug_utils_label_ext =
      +[](VkCommandBuffer /*command_buffer*/, const VkDebugUtilsLabelEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, CmdBeginDebugUtilsLabelEXT)
      .Times(1)
      .WillOnce(Return(mock_cmd_begin_debug_utils_label_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugUtilsLabelEXT debug_marker = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                       .pLabelName = "Marker"};
  controller_.OnCmdBeginDebugUtilsLabelEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest,
       WillMarkDebugMarkerEndButNotDelegateIfDriverDoesNotSupportDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported).Times(1).WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdEndDebugUtilsLabelEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest, WillDelegateOnEndDebugLabelIfDriverDoesSupportDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported).Times(1).WillOnce(Return(true));

  PFN_vkCmdEndDebugUtilsLabelEXT mock_cmd_end_debug_utils_label_ext =
      +[](VkCommandBuffer /*command_buffer*/) {};
  EXPECT_CALL(*dispatch_table, CmdEndDebugUtilsLabelEXT)
      .Times(1)
      .WillOnce(Return(mock_cmd_end_debug_utils_label_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdEndDebugUtilsLabelEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest,
       WillMarkDebugMarkerBeginButNotDelegateIfDriverDoesNotSupportDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported).Times(1).WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugMarkerMarkerInfoEXT debug_marker = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT, .pMarkerName = "Marker"};
  controller_.OnCmdDebugMarkerBeginEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest, WillDelegateOnBeginDebugMarkerIfDriverDoesSupportDebugMrkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported).Times(1).WillOnce(Return(true));

  PFN_vkCmdDebugMarkerBeginEXT mock_cmd_debug_marker_begin_ext =
      +[](VkCommandBuffer /*command_buffer*/, const VkDebugMarkerMarkerInfoEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, CmdDebugMarkerBeginEXT)
      .Times(1)
      .WillOnce(Return(mock_cmd_debug_marker_begin_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugMarkerMarkerInfoEXT debug_marker = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT, .pMarkerName = "Marker"};
  controller_.OnCmdDebugMarkerBeginEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest,
       WillMarkDebugMarkerEndButNotDelegateIfDriverDoesNotSupportDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported).Times(1).WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdDebugMarkerEndEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest, WillDelegateOnEndDebugMarkerIfDriverDoesSupportDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported).Times(1).WillOnce(Return(true));

  PFN_vkCmdDebugMarkerEndEXT mock_cmd_debug_marker_end_ext =
      +[](VkCommandBuffer /*command_buffer*/) {};
  EXPECT_CALL(*dispatch_table, CmdDebugMarkerEndEXT)
      .Times(1)
      .WillOnce(Return(mock_cmd_debug_marker_end_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdDebugMarkerEndEXT(command_buffer);
}

}  // namespace orbit_vulkan_layer
