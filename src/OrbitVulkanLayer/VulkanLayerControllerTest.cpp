// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"
#include "VulkanLayerController.h"
#include "VulkanLayerProducer.h"

using ::testing::A;
using ::testing::AllOf;
using ::testing::Field;
using ::testing::Invoke;
using ::testing::IsSubsetOf;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::StrEq;
using ::testing::UnorderedElementsAreArray;

namespace orbit_vulkan_layer {

namespace {

class MockDispatchTable {
 public:
  MOCK_METHOD(PFN_vkEnumerateDeviceExtensionProperties, EnumerateDeviceExtensionProperties,
              (VkPhysicalDevice));
  MOCK_METHOD(void, CreateInstanceDispatchTable, (VkInstance, PFN_vkGetInstanceProcAddr));
  MOCK_METHOD(void, CreateDeviceDispatchTable, (VkDevice, PFN_vkGetDeviceProcAddr));
  MOCK_METHOD(void, RemoveInstanceDispatchTable, (VkInstance));
  MOCK_METHOD(void, RemoveDeviceDispatchTable, (VkDevice));
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
  MOCK_METHOD(bool, IsDebugUtilsExtensionSupported, (VkCommandBuffer), (const));
  MOCK_METHOD(bool, IsDebugUtilsExtensionSupported, (VkInstance), (const));
  MOCK_METHOD(bool, IsDebugUtilsExtensionSupported, (VkQueue), (const));
  MOCK_METHOD(bool, IsDebugUtilsExtensionSupported, (VkDevice), (const));
  MOCK_METHOD(bool, IsDebugMarkerExtensionSupported, (VkCommandBuffer), (const));
  MOCK_METHOD(bool, IsDebugMarkerExtensionSupported, (VkDevice), (const));
  MOCK_METHOD(bool, IsDebugReportExtensionSupported, (VkInstance), (const));
  MOCK_METHOD(PFN_vkCmdInsertDebugUtilsLabelEXT, CmdInsertDebugUtilsLabelEXT, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkCreateDebugUtilsMessengerEXT, CreateDebugUtilsMessengerEXT, (VkInstance));
  MOCK_METHOD(PFN_vkDestroyDebugUtilsMessengerEXT, DestroyDebugUtilsMessengerEXT, (VkInstance));
  MOCK_METHOD(PFN_vkQueueBeginDebugUtilsLabelEXT, QueueBeginDebugUtilsLabelEXT, (VkQueue));
  MOCK_METHOD(PFN_vkQueueEndDebugUtilsLabelEXT, QueueEndDebugUtilsLabelEXT, (VkQueue));
  MOCK_METHOD(PFN_vkQueueInsertDebugUtilsLabelEXT, QueueInsertDebugUtilsLabelEXT, (VkQueue));
  MOCK_METHOD(PFN_vkSetDebugUtilsObjectNameEXT, SetDebugUtilsObjectNameEXT, (VkDevice));
  MOCK_METHOD(PFN_vkSetDebugUtilsObjectTagEXT, SetDebugUtilsObjectTagEXT, (VkDevice));
  MOCK_METHOD(PFN_vkSubmitDebugUtilsMessageEXT, SubmitDebugUtilsMessageEXT, (VkInstance));
  MOCK_METHOD(PFN_vkCmdDebugMarkerInsertEXT, CmdDebugMarkerInsertEXT, (VkCommandBuffer));
  MOCK_METHOD(PFN_vkDebugMarkerSetObjectNameEXT, DebugMarkerSetObjectNameEXT, (VkDevice));
  MOCK_METHOD(PFN_vkDebugMarkerSetObjectTagEXT, DebugMarkerSetObjectTagEXT, (VkDevice));
  MOCK_METHOD(PFN_vkCreateDebugReportCallbackEXT, CreateDebugReportCallbackEXT, (VkInstance));
  MOCK_METHOD(PFN_vkDebugReportMessageEXT, DebugReportMessageEXT, (VkInstance));
  MOCK_METHOD(PFN_vkDestroyDebugReportCallbackEXT, DestroyDebugReportCallbackEXT, (VkInstance));
  MOCK_METHOD(VkInstance, GetInstance, (VkPhysicalDevice), (const));
};

class MockDeviceManager {
 public:
  explicit MockDeviceManager(MockDispatchTable* /*dispatch_table*/) {}
  MOCK_METHOD(void, TrackLogicalDevice, (VkPhysicalDevice, VkDevice));
  MOCK_METHOD(void, UntrackLogicalDevice, (VkDevice));
};

class MockQueueManager {
 public:
  MOCK_METHOD(void, TrackQueue, (VkQueue, VkDevice));
  MOCK_METHOD((VkDevice), GetDeviceOfQueue, (VkQueue));
};

class MockTimerQueryPool {
 public:
  explicit MockTimerQueryPool(MockDispatchTable* /*dispatch_table*/, uint32_t /*num_slots*/) {}
  MOCK_METHOD(void, InitializeTimerQueryPool, (VkDevice));
  MOCK_METHOD(void, DestroyTimerQueryPool, (VkDevice));
};

struct Color {
  float red;
  float green;
  float blue;
  float alpha;
};

class MockSubmissionTracker {
 public:
  struct QueueSubmission {};

  explicit MockSubmissionTracker(MockDispatchTable* /*dispatch_table*/,
                                 MockTimerQueryPool* /*timer_query_pool*/,
                                 MockDeviceManager* /*device_manager*/, uint32_t /*max_depth*/) {}
  MOCK_METHOD(void, SetVulkanLayerProducer, (VulkanLayerProducer*));
  MOCK_METHOD(void, ResetCommandPool, (VkCommandPool));
  MOCK_METHOD(void, TrackCommandBuffers,
              (VkDevice, VkCommandPool, const VkCommandBuffer*, uint32_t));
  MOCK_METHOD(void, UntrackCommandBuffers,
              (VkDevice, VkCommandPool, const VkCommandBuffer*, uint32_t));
  MOCK_METHOD(void, MarkCommandBufferBegin, (VkCommandBuffer));
  MOCK_METHOD(void, MarkCommandBufferEnd, (VkCommandBuffer));
  MOCK_METHOD(void, ResetCommandBuffer, (VkCommandBuffer));
  MOCK_METHOD(std::optional<QueueSubmission>, PersistCommandBuffersOnSubmit,
              (VkQueue, uint32_t, const VkSubmitInfo* submits));
  MOCK_METHOD(void, PersistDebugMarkersOnSubmit,
              (VkQueue, uint32_t, const VkSubmitInfo*, std::optional<QueueSubmission>));
  MOCK_METHOD(void, CompleteSubmits, (VkDevice));
  MOCK_METHOD(void, MarkDebugMarkerBegin, (VkCommandBuffer, const char*, Color));
  MOCK_METHOD(void, MarkDebugMarkerEnd, (VkCommandBuffer));
};

class MockVulkanWrapper {
 public:
  MOCK_METHOD(VkResult, CallVkEnumerateInstanceExtensionProperties,
              (const char*, uint32_t*, VkExtensionProperties*));
};

using VulkanLayerControllerImpl =
    VulkanLayerController<MockDispatchTable, MockQueueManager, MockDeviceManager,
                          MockTimerQueryPool, MockSubmissionTracker, MockVulkanWrapper>;

Matcher<VkExtensionProperties> VkExtensionPropertiesAreEqual(
    const VkExtensionProperties& expected) {
  return AllOf(
      Field("specVersion", &VkExtensionProperties::specVersion, expected.specVersion),
      Field("extensionName", &VkExtensionProperties::extensionName, StrEq(expected.extensionName)));
}
}  // namespace

class VulkanLayerControllerTest : public ::testing::Test {
 protected:
  VulkanLayerControllerImpl controller_;
  static constexpr VkExtensionProperties kDebugMarkerExtension{VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
                                                               VK_EXT_DEBUG_MARKER_SPEC_VERSION};
  static constexpr VkExtensionProperties kDebugUtilsExtension{VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                                              VK_EXT_DEBUG_UTILS_SPEC_VERSION};
  static constexpr VkExtensionProperties kDebugReportExtension{VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                               VK_EXT_DEBUG_REPORT_SPEC_VERSION};

  static constexpr VkExtensionProperties kFakeExtension1{"Other Extension 1", 3};
  static constexpr VkExtensionProperties kFakeExtension2{"Other Extension 2", 2};

  static constexpr PFN_vkEnumerateDeviceExtensionProperties
      kFakeEnumerateDeviceExtensionPropertiesFunction =
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

TEST_F(VulkanLayerControllerTest, CanEnumerateInstanceExtensionPropertiesForThisLayer) {
  uint32_t actual_property_count = 123;
  VkResult result = controller_.OnEnumerateInstanceExtensionProperties(
      VulkanLayerControllerImpl::kLayerName, &actual_property_count, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
  ASSERT_EQ(actual_property_count, 2);
  std::array<VkExtensionProperties, 2> actual_properties = {};
  result = controller_.OnEnumerateInstanceExtensionProperties(
      VulkanLayerControllerImpl::kLayerName, &actual_property_count, actual_properties.data());
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_THAT(actual_properties,
              UnorderedElementsAreArray({VkExtensionPropertiesAreEqual(kDebugUtilsExtension),
                                         VkExtensionPropertiesAreEqual(kDebugReportExtension)}));
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
  ASSERT_EQ(actual_property_count, 1);
  std::array<VkExtensionProperties, 1> actual_properties = {};
  result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, VulkanLayerControllerImpl::kLayerName, &actual_property_count,
      actual_properties.data());
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_THAT(actual_properties,
              UnorderedElementsAreArray({VkExtensionPropertiesAreEqual(kDebugMarkerExtension)}));
}

TEST_F(VulkanLayerControllerTest,
       CanEnumerateASubsetOfTheLayersExclusiveDeviceExtensionProperties) {
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count = 0;
  std::array<VkExtensionProperties, 1> actual_properties = {};
  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, VulkanLayerControllerImpl::kLayerName, &actual_property_count,
      actual_properties.data());
  EXPECT_EQ(result, VK_INCOMPLETE);
  ASSERT_EQ(actual_property_count, 0);
}

TEST_F(VulkanLayerControllerTest, WillForwardCallOnEnumerateOtherLayersDeviceExtensionProperties) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .Times(2)
      .WillRepeatedly(Return(kFakeEnumerateDeviceExtensionPropertiesFunction));
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
  PFN_vkEnumerateDeviceExtensionProperties fake_enumerate_device_extension_properties_function =
      +[](VkPhysicalDevice /*physical_device*/, const char* /*layer_name*/,
          uint32_t* /*property_count*/,
          VkExtensionProperties* /*properties*/) { return VK_INCOMPLETE; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .Times(1)
      .WillRepeatedly(Return(fake_enumerate_device_extension_properties_function));
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
      .WillRepeatedly(Return(kFakeEnumerateDeviceExtensionPropertiesFunction));
  VkPhysicalDevice physical_device = {};
  uint32_t actual_property_count;

  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &actual_property_count, nullptr);

  EXPECT_EQ(result, VK_SUCCESS);
  ASSERT_EQ(actual_property_count, 3);

  std::array<VkExtensionProperties, 3> actual_properties = {};
  result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &actual_property_count, actual_properties.data());
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_THAT(actual_properties, UnorderedElementsAreArray({
                                     VkExtensionPropertiesAreEqual(kFakeExtension1),
                                     VkExtensionPropertiesAreEqual(kFakeExtension2),
                                     VkExtensionPropertiesAreEqual(kDebugMarkerExtension),
                                 }));
}

TEST_F(VulkanLayerControllerTest,
       CanMergePropertiesAndEnumerateASubsetForAllLayersDeviceExtensionProperties) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .WillRepeatedly(Return(kFakeEnumerateDeviceExtensionPropertiesFunction));
  VkPhysicalDevice physical_device = {};

  std::array<VkExtensionProperties, 2> actual_properties = {};
  uint32_t stripped_property_count = 2;
  VkResult result = controller_.OnEnumerateDeviceExtensionProperties(
      physical_device, nullptr, &stripped_property_count, actual_properties.data());
  EXPECT_EQ(result, VK_INCOMPLETE);
  EXPECT_THAT(actual_properties,
              IsSubsetOf({VkExtensionPropertiesAreEqual(kFakeExtension1),
                          VkExtensionPropertiesAreEqual(kFakeExtension2),
                          VkExtensionPropertiesAreEqual(kDebugMarkerExtension)}));
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

// This will test the good case of a call to CreateInstance.
// It ensures that the dispatch table and the producer are created. Further, it checks that the
// linkage in the VkLayerInstanceCreateInfo chain gets advanced, such that the next layer does not
// need to process all the previous layers.
// It tests that the required extensions are enabled in the actual call to the next
// vkCreateInstance. In this case, the game has not already requested the extensions.
TEST_F(
    VulkanLayerControllerTest,
    WillCreateDispatchTableAndVulkanLayerProducerAndAdvanceLinkageOnCreateInstanceWithGameNotEnablingRequiredExtensions) {
  std::unique_ptr<VulkanLayerControllerImpl> controller =
      std::make_unique<VulkanLayerControllerImpl>();
  const MockDispatchTable* dispatch_table = controller->dispatch_table();
  const MockSubmissionTracker* submission_tracker = controller->submission_tracker();
  const MockVulkanWrapper* vulkan_wrapper = controller->vulkan_wrapper();
  EXPECT_CALL(*dispatch_table, CreateInstanceDispatchTable).Times(1);
  EXPECT_CALL(*submission_tracker, SetVulkanLayerProducer).Times(1);

  EXPECT_CALL(*vulkan_wrapper, CallVkEnumerateInstanceExtensionProperties)
      .Times(2)
      .WillRepeatedly(Invoke([](const char* /*layer_name*/, uint32_t* property_count,
                                VkExtensionProperties* properties) -> VkResult {
        ORBIT_CHECK(property_count != nullptr);
        if (properties == nullptr) {
          *property_count = 1;
          return VK_SUCCESS;
        }
        VkExtensionProperties p{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION};
        *properties = p;
        return VK_SUCCESS;
      }));

  static constexpr PFN_vkCreateInstance kMockDriverCreateInstance =
      +[](const VkInstanceCreateInfo* create_info, const VkAllocationCallbacks* /*allocator*/,
          VkInstance* /*instance*/) {
        bool requested_get_physical_device_properties_extension = false;
        for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i) {
          if (strcmp(create_info->ppEnabledExtensionNames[i],
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
            requested_get_physical_device_properties_extension = true;
            break;
          }
        }
        EXPECT_TRUE(requested_get_physical_device_properties_extension);
        return VK_SUCCESS;
      };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateInstance") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateInstance);
    }
    return nullptr;
  };

  VkLayerInstanceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr};
  VkLayerInstanceLink layer_link_2 = {
      .pNext = &layer_link_1,
      .pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr,
  };
  VkLayerInstanceCreateInfo layer_create_info{
      .sType = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO, .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_2;
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = &layer_create_info,
                                   .enabledExtensionCount = 0,
                                   .ppEnabledExtensionNames = nullptr};

  VkInstance created_instance;
  VkResult result = controller->OnCreateInstance(&create_info, nullptr, &created_instance);
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_EQ(layer_create_info.u.pLayerInfo, &layer_link_1);

  ::testing::Mock::VerifyAndClearExpectations(absl::bit_cast<void*>(submission_tracker));
  // There will be a call in the destructor.
  auto* actual_producer = absl::bit_cast<VulkanLayerProducer*>(static_cast<uintptr_t>(0xdeadbeef));
  EXPECT_CALL(*submission_tracker, SetVulkanLayerProducer)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_producer));
  controller.reset();
  EXPECT_EQ(actual_producer, nullptr);
}  // namespace orbit_vulkan_layer

// This will test the good case of a call to CreateInstance. Similar to the test case above, but
// this time the game already requested the required extensions. We still ensure that those are
// requested in the next layer's vkCreateInstance.
TEST_F(
    VulkanLayerControllerTest,
    WillCreateDispatchTableAndVulkanLayerProducerOnCreateInstanceWithGameAlreadyEnablingRequiredExtensions) {
  std::unique_ptr<VulkanLayerControllerImpl> controller =
      std::make_unique<VulkanLayerControllerImpl>();
  const MockDispatchTable* dispatch_table = controller->dispatch_table();
  const MockSubmissionTracker* submission_tracker = controller->submission_tracker();
  const MockVulkanWrapper* vulkan_wrapper = controller->vulkan_wrapper();
  EXPECT_CALL(*dispatch_table, CreateInstanceDispatchTable).Times(1);
  EXPECT_CALL(*submission_tracker, SetVulkanLayerProducer).Times(1);

  EXPECT_CALL(*vulkan_wrapper, CallVkEnumerateInstanceExtensionProperties)
      .Times(2)
      .WillRepeatedly(Invoke([](const char* /*layer_name*/, uint32_t* property_count,
                                VkExtensionProperties* properties) -> VkResult {
        ORBIT_CHECK(property_count != nullptr);
        if (properties == nullptr) {
          *property_count = 1;
          return VK_SUCCESS;
        }
        VkExtensionProperties p{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION};
        *properties = p;
        return VK_SUCCESS;
      }));

  static constexpr PFN_vkCreateInstance kMockDriverCreateInstance =
      +[](const VkInstanceCreateInfo* create_info, const VkAllocationCallbacks* /*allocator*/,
          VkInstance* /*instance*/) {
        bool requested_get_physical_device_properties_extension = false;
        for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i) {
          if (strcmp(create_info->ppEnabledExtensionNames[i],
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
            requested_get_physical_device_properties_extension = true;
            break;
          }
        }
        EXPECT_TRUE(requested_get_physical_device_properties_extension);
        return VK_SUCCESS;
      };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateInstance") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateInstance);
    }
    return nullptr;
  };

  std::string requested_extension_name = VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME;
  std::vector<const char*> requested_extensions{};
  requested_extensions.push_back(requested_extension_name.c_str());

  VkLayerInstanceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr};
  VkLayerInstanceCreateInfo layer_create_info{
      .sType = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO, .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_1;
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = &layer_create_info,
                                   .enabledExtensionCount = 1,
                                   .ppEnabledExtensionNames = requested_extensions.data()};

  VkInstance created_instance;
  VkResult result = controller->OnCreateInstance(&create_info, nullptr, &created_instance);
  EXPECT_EQ(result, VK_SUCCESS);
  ::testing::Mock::VerifyAndClearExpectations(absl::bit_cast<void*>(submission_tracker));
  // There will be a call in the destructor.
  auto* actual_producer = absl::bit_cast<VulkanLayerProducer*>(static_cast<uintptr_t>(0xdeadbeef));
  EXPECT_CALL(*submission_tracker, SetVulkanLayerProducer)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_producer));
  controller.reset();
  EXPECT_EQ(actual_producer, nullptr);
}

TEST_F(VulkanLayerControllerTest, WillDumpPidOnCreateInstance) {
  std::unique_ptr<VulkanLayerControllerImpl> controller =
      std::make_unique<VulkanLayerControllerImpl>();
  const MockVulkanWrapper* vulkan_wrapper = controller->vulkan_wrapper();

  EXPECT_CALL(*vulkan_wrapper, CallVkEnumerateInstanceExtensionProperties)
      .WillRepeatedly(Invoke([](const char* /*layer_name*/, uint32_t* property_count,
                                VkExtensionProperties* properties) -> VkResult {
        ORBIT_CHECK(property_count != nullptr);
        if (properties == nullptr) {
          *property_count = 1;
          return VK_SUCCESS;
        }
        VkExtensionProperties p{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION};
        *properties = p;
        return VK_SUCCESS;
      }));

  static constexpr PFN_vkCreateInstance kMockDriverCreateInstance =
      +[](const VkInstanceCreateInfo* /*create_info*/, const VkAllocationCallbacks* /*allocator*/,
          VkInstance* /*instance*/) { return VK_SUCCESS; };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateInstance") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateInstance);
    }
    return nullptr;
  };

  VkLayerInstanceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr};
  VkLayerInstanceCreateInfo layer_create_info{
      .sType = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO, .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_1;
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = &layer_create_info,
                                   .enabledExtensionCount = 0,
                                   .ppEnabledExtensionNames = nullptr};

  VkInstance created_instance;
  constexpr const char* kFilename = "pid.txt";
  setenv("ORBIT_VULKAN_LAYER_PID_FILE", kFilename, /*overwrite*/ 1);
  VkResult result = controller->OnCreateInstance(&create_info, nullptr, &created_instance);
  uint32_t pid = orbit_base::GetCurrentProcessId();
  ErrorMessageOr<std::string> pid_or_error = orbit_base::ReadFileToString(kFilename);
  EXPECT_FALSE(pid_or_error.has_error());
  EXPECT_EQ(pid_or_error.value(), absl::StrFormat("%u", pid));
  EXPECT_EQ(result, VK_SUCCESS);
  ErrorMessageOr<bool> removed_or_error = orbit_base::RemoveFile(kFilename);
  EXPECT_FALSE(removed_or_error.has_error());
}

TEST_F(VulkanLayerControllerTest, DumpProcessIdFailsOnCreateInstanceByNonExistentPath) {
  std::unique_ptr<VulkanLayerControllerImpl> controller =
      std::make_unique<VulkanLayerControllerImpl>();
  const MockVulkanWrapper* vulkan_wrapper = controller->vulkan_wrapper();

  EXPECT_CALL(*vulkan_wrapper, CallVkEnumerateInstanceExtensionProperties)
      .WillRepeatedly(Invoke([](const char* /*layer_name*/, uint32_t* property_count,
                                VkExtensionProperties* properties) -> VkResult {
        ORBIT_CHECK(property_count != nullptr);
        if (properties == nullptr) {
          *property_count = 1;
          return VK_SUCCESS;
        }
        VkExtensionProperties p{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION};
        *properties = p;
        return VK_SUCCESS;
      }));

  static constexpr PFN_vkCreateInstance kMockDriverCreateInstance =
      +[](const VkInstanceCreateInfo* /*create_info*/, const VkAllocationCallbacks* /*allocator*/,
          VkInstance* /*instance*/) { return VK_SUCCESS; };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateInstance") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateInstance);
    }
    return nullptr;
  };

  VkLayerInstanceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr};
  VkLayerInstanceCreateInfo layer_create_info{
      .sType = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO, .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_1;
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = &layer_create_info,
                                   .enabledExtensionCount = 0,
                                   .ppEnabledExtensionNames = nullptr};

  VkInstance created_instance;
  constexpr const char* kFilename = "i_dont_exists_dir/pid.txt";
  setenv("ORBIT_VULKAN_LAYER_PID_FILE", kFilename, /*overwrite*/ 1);
  [[maybe_unused]] VkResult result = VK_SUCCESS;
  EXPECT_DEATH(result = controller->OnCreateInstance(&create_info, nullptr, &created_instance),
               "Opening \"i_dont_exists_dir/pid.txt\": Unable to open file "
               "\"i_dont_exists_dir/pid.txt\": No such file or directory");
}

TEST_F(VulkanLayerControllerTest, DumpProcessIdFailsOnCreateInstanceByInvalidFilename) {
  std::unique_ptr<VulkanLayerControllerImpl> controller =
      std::make_unique<VulkanLayerControllerImpl>();
  const MockVulkanWrapper* vulkan_wrapper = controller->vulkan_wrapper();

  EXPECT_CALL(*vulkan_wrapper, CallVkEnumerateInstanceExtensionProperties)
      .WillRepeatedly(Invoke([](const char* /*layer_name*/, uint32_t* property_count,
                                VkExtensionProperties* properties) -> VkResult {
        ORBIT_CHECK(property_count != nullptr);
        if (properties == nullptr) {
          *property_count = 1;
          return VK_SUCCESS;
        }
        VkExtensionProperties p{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION};
        *properties = p;
        return VK_SUCCESS;
      }));

  static constexpr PFN_vkCreateInstance kMockDriverCreateInstance =
      +[](const VkInstanceCreateInfo* /*create_info*/, const VkAllocationCallbacks* /*allocator*/,
          VkInstance* /*instance*/) { return VK_SUCCESS; };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateInstance") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateInstance);
    }
    return nullptr;
  };

  VkLayerInstanceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr};
  VkLayerInstanceCreateInfo layer_create_info{
      .sType = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO, .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_1;
  VkInstanceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                   .pNext = &layer_create_info,
                                   .enabledExtensionCount = 0,
                                   .ppEnabledExtensionNames = nullptr};

  VkInstance created_instance;
  constexpr const char* kFilename = "tmpdir/";
  setenv("ORBIT_VULKAN_LAYER_PID_FILE", kFilename, /*overwrite*/ 1);
  [[maybe_unused]] VkResult result = VK_SUCCESS;
  EXPECT_DEATH(result = controller->OnCreateInstance(&create_info, nullptr, &created_instance),
               "Opening \"tmpdir/\": Unable to open file \"tmpdir/\": Is a directory");
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
  PFN_vkGetDeviceProcAddr fake_get_device_proc_addr =
      +[](VkDevice /*device*/, const char* /*name*/) { return kExpectedFunction; };
  EXPECT_CALL(*dispatch_table, GetDeviceProcAddr)
      .Times(1)
      .WillOnce(Return(fake_get_device_proc_addr));
  VkDevice device = {};
  PFN_vkVoidFunction result = controller_.ForwardGetDeviceProcAddr(device, "some function");
  EXPECT_EQ(result, kExpectedFunction);
}

// This will test the good case of a call to CreateDevice.
// It ensures that the dispatch table is created and checks that the
// linkage in the VkLayerDeviceCreateInfo chain gets advanced, such that the next layer does not
// need to process all the previous layers.
// It tests that the required extensions are enabled in the actual call to the next
// vkCreateDevice. In this case, the game has not already requested the extensions.
TEST_F(
    VulkanLayerControllerTest,
    WillCreateDispatchTableAndVulkanLayerProducerAndAdvanceLinkageOnCreateDeviceWithGameNotEnablingRequiredExtensions) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, CreateDeviceDispatchTable).Times(1);
  const MockDeviceManager* device_manager = controller_.device_manager();
  EXPECT_CALL(*device_manager, TrackLogicalDevice).Times(1);
  const MockTimerQueryPool* timer_query_pool = controller_.timer_query_pool();
  EXPECT_CALL(*timer_query_pool, InitializeTimerQueryPool).Times(1);

  static constexpr PFN_vkCreateDevice kMockDriverCreateDevice =
      +[](VkPhysicalDevice /*physical_device*/, const VkDeviceCreateInfo* create_info,
          const VkAllocationCallbacks* /*allocator*/, VkDevice* /*instance*/) {
        bool requested_host_extension = false;
        for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i) {
          if (strcmp(create_info->ppEnabledExtensionNames[i],
                     VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0) {
            requested_host_extension = true;
            break;
          }
        }
        EXPECT_TRUE(requested_host_extension);
        return VK_SUCCESS;
      };
  static constexpr PFN_vkEnumerateDeviceExtensionProperties
      kFakeEnumerateDeviceExtensionProperties =
          +[](VkPhysicalDevice /*physical_device*/, const char* /*layer_name*/,
              uint32_t* property_count, VkExtensionProperties* properties) -> VkResult {
    ORBIT_CHECK(property_count != nullptr);
    if (properties == nullptr) {
      *property_count = 1;
      return VK_SUCCESS;
    }
    VkExtensionProperties p{VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
                            VK_EXT_HOST_QUERY_RESET_SPEC_VERSION};
    *properties = p;
    return VK_SUCCESS;
  };

  EXPECT_CALL(*dispatch_table, EnumerateDeviceExtensionProperties)
      .Times(1)
      .WillOnce(Invoke([](VkPhysicalDevice /*device*/) -> PFN_vkEnumerateDeviceExtensionProperties {
        return kFakeEnumerateDeviceExtensionProperties;
      }));

  PFN_vkGetDeviceProcAddr fake_get_device_proc_addr =
      +[](VkDevice /*device*/, const char* /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateDevice") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateDevice);
    }
    return nullptr;
  };

  VkLayerDeviceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr,
                                    .pfnNextGetDeviceProcAddr = fake_get_device_proc_addr};
  VkLayerDeviceLink layer_link_2 = {
      .pNext = &layer_link_1,
      .pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr,
      .pfnNextGetDeviceProcAddr = fake_get_device_proc_addr,
  };
  VkLayerDeviceCreateInfo layer_create_info{.sType = VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO,
                                            .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_2;
  VkDeviceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                 .pNext = &layer_create_info,
                                 .enabledExtensionCount = 0,
                                 .ppEnabledExtensionNames = nullptr};
  VkDevice created_device;
  VkPhysicalDevice physical_device = {};
  VkResult result =
      controller_.OnCreateDevice(physical_device, &create_info, nullptr, &created_device);
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_EQ(layer_create_info.u.pLayerInfo, &layer_link_1);
}

// This will test the good case of a call to CreateDevice. Similar to the test case above, but
// this time the game already requested the required extensions. We still ensure that those are
// requested in the next layer's vkCreateDevice.
TEST_F(VulkanLayerControllerTest,
       WillCreateDispatchTableOnCreateDeviceWithGameAlreadyEnablingRequiredExtensions) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, CreateDeviceDispatchTable).Times(1);
  const MockDeviceManager* device_manager = controller_.device_manager();
  EXPECT_CALL(*device_manager, TrackLogicalDevice).Times(1);
  const MockTimerQueryPool* timer_query_pool = controller_.timer_query_pool();
  EXPECT_CALL(*timer_query_pool, InitializeTimerQueryPool).Times(1);

  static constexpr PFN_vkCreateDevice kMockDriverCreateDevice =
      +[](VkPhysicalDevice /*physical_device*/, const VkDeviceCreateInfo* create_info,
          const VkAllocationCallbacks* /*allocator*/, VkDevice* /*instance*/) {
        bool requested_host_extension = false;
        for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i) {
          if (strcmp(create_info->ppEnabledExtensionNames[i],
                     VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0) {
            requested_host_extension = true;
            break;
          }
        }
        EXPECT_TRUE(requested_host_extension);
        return VK_SUCCESS;
      };
  static constexpr PFN_vkEnumerateDeviceExtensionProperties
      kFakeEnumerateDeviceExtensionProperties =
          +[](VkPhysicalDevice /*physical_device*/, const char* /*layer_name*/,
              uint32_t* property_count, VkExtensionProperties* properties) -> VkResult {
    ORBIT_CHECK(property_count != nullptr);
    if (properties == nullptr) {
      *property_count = 1;
      return VK_SUCCESS;
    }
    VkExtensionProperties p{VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
                            VK_EXT_HOST_QUERY_RESET_SPEC_VERSION};
    *properties = p;
    return VK_SUCCESS;
  };

  PFN_vkGetDeviceProcAddr fake_get_device_proc_addr =
      +[](VkDevice /*device*/, const char* /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateDevice") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kMockDriverCreateDevice);
    }
    if (strcmp(name, "vkEnumerateDeviceExtensionProperties") == 0) {
      return absl::bit_cast<PFN_vkVoidFunction>(kFakeEnumerateDeviceExtensionProperties);
    }
    return nullptr;
  };

  VkLayerDeviceLink layer_link_1 = {.pfnNextGetInstanceProcAddr = fake_get_instance_proc_addr,
                                    .pfnNextGetDeviceProcAddr = fake_get_device_proc_addr};
  VkLayerDeviceCreateInfo layer_create_info{.sType = VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO,
                                            .function = VK_LAYER_LINK_INFO};
  layer_create_info.u.pLayerInfo = &layer_link_1;
  std::string requested_extension_name = VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME;
  std::vector<const char*> requested_extensions{};
  requested_extensions.push_back(requested_extension_name.c_str());
  VkDeviceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                 .pNext = &layer_create_info,
                                 .enabledExtensionCount = 1,
                                 .ppEnabledExtensionNames = requested_extensions.data()};
  VkDevice created_device;
  VkPhysicalDevice physical_device = {};
  VkResult result =
      controller_.OnCreateDevice(physical_device, &create_info, nullptr, &created_device);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, CallInDispatchTableOnGetInstanceProcAddr) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  static constexpr PFN_vkVoidFunction kExpectedFunction = +[]() {};
  PFN_vkGetInstanceProcAddr fake_get_instance_proc_addr =
      +[](VkInstance /*instance*/, const char* /*name*/) { return kExpectedFunction; };
  EXPECT_CALL(*dispatch_table, GetInstanceProcAddr)
      .Times(1)
      .WillOnce(Return(fake_get_instance_proc_addr));
  VkInstance instance = {};
  PFN_vkVoidFunction result = controller_.ForwardGetInstanceProcAddr(instance, "some function");
  EXPECT_EQ(result, kExpectedFunction);
}

TEST_F(VulkanLayerControllerTest, WillCleanUpOnDestroyInstance) {
  PFN_vkDestroyInstance fake_destroy_instance =
      +[](VkInstance /*instance*/, const VkAllocationCallbacks* /*allocator*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, DestroyInstance).Times(1).WillOnce(Return(fake_destroy_instance));
  EXPECT_CALL(*dispatch_table, RemoveInstanceDispatchTable).Times(1);

  VkInstance instance = {};

  controller_.OnDestroyInstance(instance, nullptr);
}

TEST_F(VulkanLayerControllerTest, WillCleanUpOnDestroyDevice) {
  PFN_vkDestroyDevice fake_destroy_device =
      +[](VkDevice /*device*/, const VkAllocationCallbacks* /*allocator*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, DestroyDevice).Times(1).WillOnce(Return(fake_destroy_device));
  EXPECT_CALL(*dispatch_table, RemoveDeviceDispatchTable).Times(1);
  const MockDeviceManager* device_manager = controller_.device_manager();
  EXPECT_CALL(*device_manager, UntrackLogicalDevice).Times(1);
  const MockTimerQueryPool* query_pool = controller_.timer_query_pool();
  EXPECT_CALL(*query_pool, DestroyTimerQueryPool).Times(1);

  VkDevice device = {};

  controller_.OnDestroyDevice(device, nullptr);
}

// ----------------------------------------------------------------------------
// Core layer logic
// ----------------------------------------------------------------------------

TEST_F(VulkanLayerControllerTest, ForwardsOnResetCommandPoolToSubmissionTracker) {
  PFN_vkResetCommandPool fake_reset_command_pool =
      +[](VkDevice /*device*/, VkCommandPool /*command_pool*/,
          VkCommandPoolResetFlags /*flags*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, ResetCommandPool).Times(1).WillOnce(Return(fake_reset_command_pool));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, ResetCommandPool).Times(1);
  VkDevice device = {};
  VkCommandPool command_pool = {};
  VkCommandPoolResetFlags flags = {};
  VkResult result = controller_.OnResetCommandPool(device, command_pool, flags);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnAllocateCommandBuffersToSubmissionTracker) {
  PFN_vkAllocateCommandBuffers fake_allocate_command_buffers =
      +[](VkDevice /*device*/, const VkCommandBufferAllocateInfo* /*allocate_info*/,
          VkCommandBuffer*
          /*command_buffers*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, AllocateCommandBuffers)
      .Times(1)
      .WillOnce(Return(fake_allocate_command_buffers));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, TrackCommandBuffers).Times(1);
  VkDevice device = {};
  VkCommandPool command_pool = {};
  VkCommandBuffer command_buffer;

  VkCommandBufferAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                            .pNext = nullptr,
                                            .commandPool = command_pool,
                                            .commandBufferCount = 1};

  VkResult result = controller_.OnAllocateCommandBuffers(device, &allocate_info, &command_buffer);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnFreeCommandBuffersToSubmissionTracker) {
  PFN_vkFreeCommandBuffers fake_free_command_buffers =
      +[](VkDevice /*device*/, VkCommandPool /*command_pool*/, uint32_t /*command_buffer_count*/,
          const VkCommandBuffer* /*command_buffers*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, FreeCommandBuffers)
      .Times(1)
      .WillOnce(Return(fake_free_command_buffers));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, UntrackCommandBuffers).Times(1);
  VkDevice device = {};
  VkCommandPool command_pool = {};
  VkCommandBuffer command_buffer;

  controller_.OnFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnBeginCommandBufferToSubmissionTracker) {
  PFN_vkBeginCommandBuffer fake_begin_command_buffer =
      +[](VkCommandBuffer /*command_buffer*/, const VkCommandBufferBeginInfo*
          /*begin_info*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, BeginCommandBuffer)
      .Times(1)
      .WillOnce(Return(fake_begin_command_buffer));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkCommandBufferBegin).Times(1);
  VkCommandBuffer command_buffer = {};
  VkResult result = controller_.OnBeginCommandBuffer(command_buffer, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnEndCommandBufferToSubmissionTracker) {
  PFN_vkEndCommandBuffer fake_end_command_buffer =
      +[](VkCommandBuffer /*command_buffer*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, EndCommandBuffer).Times(1).WillOnce(Return(fake_end_command_buffer));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkCommandBufferEnd).Times(1);
  VkCommandBuffer command_buffer = {};
  VkResult result = controller_.OnEndCommandBuffer(command_buffer);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnResetCommandBufferToSubmissionTracker) {
  PFN_vkResetCommandBuffer fake_reset_command_buffer =
      +[](VkCommandBuffer /*command_buffer*/, VkCommandBufferResetFlags /*flags*/) -> VkResult {
    return VK_SUCCESS;
  };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, ResetCommandBuffer)
      .Times(1)
      .WillOnce(Return(fake_reset_command_buffer));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, ResetCommandBuffer).Times(1);
  VkCommandBuffer command_buffer = {};
  VkResult result = controller_.OnResetCommandBuffer(
      command_buffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnGetDeviceQueueToQueueManager) {
  PFN_vkGetDeviceQueue fake_get_device_queue =
      +[](VkDevice /*device*/, uint32_t /*queue_family_index*/, uint32_t /*queue_index*/,
          VkQueue* /*queue*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, GetDeviceQueue).Times(1).WillOnce(Return(fake_get_device_queue));

  const MockQueueManager* queue_manager = controller_.queue_manager();
  EXPECT_CALL(*queue_manager, TrackQueue).Times(1);
  VkDevice device = {};
  VkQueue queue;
  controller_.OnGetDeviceQueue(device, 1, 2, &queue);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnGetDeviceQueue2ToQueueManager) {
  PFN_vkGetDeviceQueue2 fake_get_device_queue2 =
      +[](VkDevice /*device*/, const VkDeviceQueueInfo2* /*queue_info*/, VkQueue* /*queue*/) {};
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, GetDeviceQueue2).Times(1).WillOnce(Return(fake_get_device_queue2));

  const MockQueueManager* queue_manager = controller_.queue_manager();
  EXPECT_CALL(*queue_manager, TrackQueue).Times(1);

  VkDevice device = {};
  VkQueue queue;
  VkDeviceQueueInfo2 queue_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2, .queueFamilyIndex = 1, .queueIndex = 2};
  controller_.OnGetDeviceQueue2(device, &queue_info, &queue);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnGetQueueSubmitToSubmissionTracker) {
  PFN_vkQueueSubmit fake_queue_submit =
      +[](VkQueue /*queue*/, uint32_t /*submit_count*/, const VkSubmitInfo* /*submits*/,
          VkFence /*fence*/) -> VkResult { return VK_SUCCESS; };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, QueueSubmit).Times(1).WillOnce(Return(fake_queue_submit));

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

TEST_F(VulkanLayerControllerTest, ForwardsOnQueuePresentKHRToSubmissionTracker) {
  PFN_vkQueuePresentKHR fake_queue_present =
      +[](VkQueue /*queue*/, const VkPresentInfoKHR* /*present_info*/) -> VkResult {
    return VK_SUCCESS;
  };
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, QueuePresentKHR).Times(1).WillOnce(Return(fake_queue_present));

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
       MarksDebugMarkerBeginButNotForwardIfDriverDoesNotSupportDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugUtilsLabelEXT debug_marker = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                       .pLabelName = "Marker"};
  controller_.OnCmdBeginDebugUtilsLabelEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnBeginDebugLabelIfDriverSupportsDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCmdBeginDebugUtilsLabelEXT fake_cmd_begin_debug_utils_label_ext =
      +[](VkCommandBuffer /*command_buffer*/, const VkDebugUtilsLabelEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, CmdBeginDebugUtilsLabelEXT)
      .Times(1)
      .WillOnce(Return(fake_cmd_begin_debug_utils_label_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugUtilsLabelEXT debug_marker = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                       .pLabelName = "Marker"};
  controller_.OnCmdBeginDebugUtilsLabelEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest,
       MarksDebugMarkerEndButNotForwardIfDriverDoesNotSupportDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdEndDebugUtilsLabelEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest, ForwardsOnEndDebugLabelIfDriverSupportsDebugUtils) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCmdEndDebugUtilsLabelEXT fake_cmd_end_debug_utils_label_ext =
      +[](VkCommandBuffer /*command_buffer*/) {};
  EXPECT_CALL(*dispatch_table, CmdEndDebugUtilsLabelEXT)
      .Times(1)
      .WillOnce(Return(fake_cmd_end_debug_utils_label_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdEndDebugUtilsLabelEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest,
       MarksDebugMarkerBeginButNotForwardIfDriverDoesNotSupportDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugMarkerMarkerInfoEXT debug_marker = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT, .pMarkerName = "Marker"};
  controller_.OnCmdDebugMarkerBeginEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest, ForwardsBeginDebugLabelIfDriverSupportsDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCmdDebugMarkerBeginEXT fake_cmd_debug_marker_begin_ext =
      +[](VkCommandBuffer /*command_buffer*/, const VkDebugMarkerMarkerInfoEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, CmdDebugMarkerBeginEXT)
      .Times(1)
      .WillOnce(Return(fake_cmd_debug_marker_begin_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerBegin).Times(1);

  VkCommandBuffer command_buffer = {};
  VkDebugMarkerMarkerInfoEXT debug_marker = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT, .pMarkerName = "Marker"};
  controller_.OnCmdDebugMarkerBeginEXT(command_buffer, &debug_marker);
}

TEST_F(VulkanLayerControllerTest,
       MarksDebugMarkerEndButNotForwardIfDriverDoesNotSupportDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(false));
  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdDebugMarkerEndEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest, ForwardsEndDebugLabelIfDriverSupportsDebugMarkers) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCmdDebugMarkerEndEXT fake_cmd_debug_marker_end_ext =
      +[](VkCommandBuffer /*command_buffer*/) {};
  EXPECT_CALL(*dispatch_table, CmdDebugMarkerEndEXT)
      .Times(1)
      .WillOnce(Return(fake_cmd_debug_marker_end_ext));

  const MockSubmissionTracker* submission_tracker = controller_.submission_tracker();
  EXPECT_CALL(*submission_tracker, MarkDebugMarkerEnd).Times(1);

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdDebugMarkerEndEXT(command_buffer);
}

TEST_F(VulkanLayerControllerTest, ForwardsInsertDebugUtilsLabelIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(false));

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdInsertDebugUtilsLabelEXT(command_buffer, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCmdInsertDebugUtilsLabelEXT fake =
      +[](VkCommandBuffer /*command_buffer*/, const VkDebugUtilsLabelEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, CmdInsertDebugUtilsLabelEXT).Times(1).WillOnce(Return(fake));
  controller_.OnCmdInsertDebugUtilsLabelEXT(command_buffer, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsCreateDebugUtilsMessengerEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(false));

  VkInstance instance = {};
  controller_.OnCreateDebugUtilsMessengerEXT(instance, nullptr, nullptr, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCreateDebugUtilsMessengerEXT fake =
      +[](VkInstance /*instance*/, const VkDebugUtilsMessengerCreateInfoEXT* /*create_info*/,
          const VkAllocationCallbacks* /*allocator*/, VkDebugUtilsMessengerEXT*
          /*messenger*/) -> VkResult { return VK_SUCCESS; };
  EXPECT_CALL(*dispatch_table, CreateDebugUtilsMessengerEXT).Times(1).WillOnce(Return(fake));
  controller_.OnCreateDebugUtilsMessengerEXT(instance, nullptr, nullptr, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsDestroyDebugUtilsMessengerEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(false));

  VkInstance instance = {};
  VkDebugUtilsMessengerEXT messenger = {};
  controller_.OnDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkDestroyDebugUtilsMessengerEXT fake =
      +[](VkInstance /*instance*/, VkDebugUtilsMessengerEXT /*messenger*/,
          const VkAllocationCallbacks* /*allocator*/) {};
  EXPECT_CALL(*dispatch_table, DestroyDebugUtilsMessengerEXT).Times(1).WillOnce(Return(fake));
  controller_.OnDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsQueueBeginDebugUtilsLabelEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkQueue>()))
      .Times(1)
      .WillOnce(Return(false));

  VkQueue queue = {};
  controller_.OnQueueBeginDebugUtilsLabelEXT(queue, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkQueue>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkQueueBeginDebugUtilsLabelEXT fake =
      +[](VkQueue /*queue*/, const VkDebugUtilsLabelEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, QueueBeginDebugUtilsLabelEXT).Times(1).WillOnce(Return(fake));
  controller_.OnQueueBeginDebugUtilsLabelEXT(queue, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsQueueEndDebugUtilsLabelEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkQueue>()))
      .Times(1)
      .WillOnce(Return(false));

  VkQueue queue = {};
  controller_.OnQueueEndDebugUtilsLabelEXT(queue);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkQueue>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkQueueEndDebugUtilsLabelEXT fake = +[](VkQueue /*queue*/) {};
  EXPECT_CALL(*dispatch_table, QueueEndDebugUtilsLabelEXT).Times(1).WillOnce(Return(fake));
  controller_.OnQueueEndDebugUtilsLabelEXT(queue);
}

TEST_F(VulkanLayerControllerTest, ForwardsQueueInsertDebugUtilsLabelEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkQueue>()))
      .Times(1)
      .WillOnce(Return(false));

  VkQueue queue = {};
  controller_.OnQueueInsertDebugUtilsLabelEXT(queue, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkQueue>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkQueueInsertDebugUtilsLabelEXT fake =
      +[](VkQueue /*queue*/, const VkDebugUtilsLabelEXT* /*label_info*/) {};
  EXPECT_CALL(*dispatch_table, QueueInsertDebugUtilsLabelEXT).Times(1).WillOnce(Return(fake));
  controller_.OnQueueInsertDebugUtilsLabelEXT(queue, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsSetDebugUtilsObjectNameEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(false));

  VkDevice device = {};
  controller_.OnSetDebugUtilsObjectNameEXT(device, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkSetDebugUtilsObjectNameEXT fake =
      +[](VkDevice /*device*/, const VkDebugUtilsObjectNameInfoEXT* /*name_info*/) -> VkResult {
    return VK_SUCCESS;
  };
  EXPECT_CALL(*dispatch_table, SetDebugUtilsObjectNameEXT).Times(1).WillOnce(Return(fake));
  controller_.OnSetDebugUtilsObjectNameEXT(device, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsSetDebugUtilsObjectTagEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(false));

  VkDevice device = {};
  controller_.OnSetDebugUtilsObjectTagEXT(device, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkSetDebugUtilsObjectTagEXT fake =
      +[](VkDevice /*device*/, const VkDebugUtilsObjectTagInfoEXT* /*tag_info*/) -> VkResult {
    return VK_SUCCESS;
  };
  EXPECT_CALL(*dispatch_table, SetDebugUtilsObjectTagEXT).Times(1).WillOnce(Return(fake));
  controller_.OnSetDebugUtilsObjectTagEXT(device, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsSubmitDebugUtilsMessageEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(false));

  VkInstance device = {};
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity = {};
  VkDebugUtilsMessageTypeFlagsEXT message_types = {};
  controller_.OnSubmitDebugUtilsMessageEXT(device, message_severity, message_types, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugUtilsExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkSubmitDebugUtilsMessageEXT fake =
      +[](VkInstance /*instance*/, VkDebugUtilsMessageSeverityFlagBitsEXT /*message_severity*/,
          VkDebugUtilsMessageTypeFlagsEXT /*message_types*/,
          const VkDebugUtilsMessengerCallbackDataEXT* /*callback_data*/) {};
  EXPECT_CALL(*dispatch_table, SubmitDebugUtilsMessageEXT).Times(1).WillOnce(Return(fake));
  controller_.OnSubmitDebugUtilsMessageEXT(device, message_severity, message_types, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsCmdDebugMarkerInsertEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(false));

  VkCommandBuffer command_buffer = {};
  controller_.OnCmdDebugMarkerInsertEXT(command_buffer, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkCommandBuffer>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCmdDebugMarkerInsertEXT fake =
      +[](VkCommandBuffer /*command_buffer*/, const VkDebugMarkerMarkerInfoEXT* /*marker_info*/) {};
  EXPECT_CALL(*dispatch_table, CmdDebugMarkerInsertEXT).Times(1).WillOnce(Return(fake));
  controller_.OnCmdDebugMarkerInsertEXT(command_buffer, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsDebugMarkerSetObjectNameEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(false));

  VkDevice device = {};
  controller_.OnDebugMarkerSetObjectNameEXT(device, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkDebugMarkerSetObjectNameEXT fake =
      +[](VkDevice /*device*/, const VkDebugMarkerObjectNameInfoEXT* /*name_info*/) -> VkResult {
    return VK_SUCCESS;
  };
  EXPECT_CALL(*dispatch_table, DebugMarkerSetObjectNameEXT).Times(1).WillOnce(Return(fake));
  controller_.OnDebugMarkerSetObjectNameEXT(device, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsDebugMarkerSetObjectTagEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(false));

  VkDevice device = {};
  controller_.OnDebugMarkerSetObjectTagEXT(device, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugMarkerExtensionSupported(A<VkDevice>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkDebugMarkerSetObjectTagEXT fake =
      +[](VkDevice /*device*/, const VkDebugMarkerObjectTagInfoEXT* /*tag*/) -> VkResult {
    return VK_SUCCESS;
  };
  EXPECT_CALL(*dispatch_table, DebugMarkerSetObjectTagEXT).Times(1).WillOnce(Return(fake));
  controller_.OnDebugMarkerSetObjectTagEXT(device, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsCreateDebugReportCallbackEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugReportExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(false));

  VkInstance instance = {};
  controller_.OnCreateDebugReportCallbackEXT(instance, nullptr, nullptr, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugReportExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkCreateDebugReportCallbackEXT fake =
      +[](VkInstance /*instance*/, const VkDebugReportCallbackCreateInfoEXT* /*create_info*/,
          const VkAllocationCallbacks* /*allocator*/, VkDebugReportCallbackEXT*
          /*callback*/) -> VkResult { return VK_SUCCESS; };
  EXPECT_CALL(*dispatch_table, CreateDebugReportCallbackEXT).Times(1).WillOnce(Return(fake));
  controller_.OnCreateDebugReportCallbackEXT(instance, nullptr, nullptr, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsCreateDebugReportMessageEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugReportExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(false));

  VkInstance instance = {};
  VkDebugReportFlagsEXT flags = {};
  VkDebugReportObjectTypeEXT object_type = {};
  controller_.OnDebugReportMessageEXT(instance, flags, object_type, 0, 0, 0, nullptr, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugReportExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkDebugReportMessageEXT fake =
      +[](VkInstance /*instance*/, VkDebugReportFlagsEXT /*flags*/,
          VkDebugReportObjectTypeEXT /*object_type*/, uint64_t /*object*/, size_t /*location*/,
          int32_t /*message_code*/, const char* /*layer_prefix*/, const char* /*message*/) {};
  EXPECT_CALL(*dispatch_table, DebugReportMessageEXT).Times(1).WillOnce(Return(fake));
  controller_.OnDebugReportMessageEXT(instance, flags, object_type, 0, 0, 0, nullptr, nullptr);
}

TEST_F(VulkanLayerControllerTest, ForwardsDestroyDebugReportCallbackEXTIfExtensionSupported) {
  const MockDispatchTable* dispatch_table = controller_.dispatch_table();
  EXPECT_CALL(*dispatch_table, IsDebugReportExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(false));

  VkInstance instance = {};
  VkDebugReportCallbackEXT callback = {};
  controller_.OnDestroyDebugReportCallbackEXT(instance, callback, nullptr);

  testing::Mock::VerifyAndClearExpectations(&dispatch_table);
  EXPECT_CALL(*dispatch_table, IsDebugReportExtensionSupported(A<VkInstance>()))
      .Times(1)
      .WillOnce(Return(true));

  PFN_vkDestroyDebugReportCallbackEXT fake =
      +[](VkInstance /*instance*/, VkDebugReportCallbackEXT /*callback*/,
          const VkAllocationCallbacks* /*allocator*/) {};
  EXPECT_CALL(*dispatch_table, DestroyDebugReportCallbackEXT).Times(1).WillOnce(Return(fake));
  controller_.OnDestroyDebugReportCallbackEXT(instance, callback, nullptr);
}
}  // namespace orbit_vulkan_layer
