// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DispatchTable.h"
#include "gtest/gtest.h"

namespace orbit_vulkan_layer {

TEST(DispatchTable, CanInitializeInstance) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);
  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);
}

TEST(DispatchTable, CannotInitializeInstanceTwice) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);
  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);
  EXPECT_DEATH(
      {
        dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);
      },
      "");
}

TEST(DispatchTable, CanRemoveInstance) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);
  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);
  dispatch_table.RemoveInstanceDispatchTable(instance);
}

TEST(DispatchTable, CanInitializeInstanceTwiceAfterRemove) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);
  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);
  dispatch_table.RemoveInstanceDispatchTable(instance);
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);
}

TEST(DispatchTable, CanInitializeDevice) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
}

TEST(DispatchTable, CannotInitializeDeviceTwice) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
  EXPECT_DEATH(
      { dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function); },
      "");
}

TEST(DispatchTable, CanRemoveDevice) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
  dispatch_table.RemoveDeviceDispatchTable(device);
}

TEST(DispatchTable, CanInitializeDeviceTwiceAfterRemove) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
  dispatch_table.RemoveDeviceDispatchTable(device);
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
}

TEST(DispatchTable, NoExtensionAvailable) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char * /*name*/) -> PFN_vkVoidFunction { return nullptr; };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
  EXPECT_FALSE(dispatch_table.IsDebugUtilsExtensionSupported(device));
  EXPECT_FALSE(dispatch_table.IsDebugMarkerExtensionSupported(device));
}

TEST(DispatchTable, CanSupportDebugUtilsExtension) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdBeginDebugUtilsLabelEXT") == 0) {
      PFN_vkCmdBeginDebugUtilsLabelEXT begin_function =
          +[](VkCommandBuffer /*command_buffer*/, const VkDebugUtilsLabelEXT* /*label_info*/) {};
      return absl::bit_cast<PFN_vkVoidFunction>(begin_function);
    }
    if (strcmp(name, "vkCmdEndDebugUtilsLabelEXT") == 0) {
      PFN_vkCmdEndDebugUtilsLabelEXT end_function = +[](VkCommandBuffer /*command_buffer*/) {};
      return absl::bit_cast<PFN_vkVoidFunction>(end_function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
  EXPECT_TRUE(dispatch_table.IsDebugUtilsExtensionSupported(device));
}

TEST(DispatchTable, CanSupportDebugMarkerExtension) {
  // We can not create an actual VkDevice, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);
  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdDebugMarkerBeginEXT") == 0) {
      PFN_vkCmdDebugMarkerBeginEXT begin_function =
          +[](VkCommandBuffer /*command_buffer*/,
              const VkDebugMarkerMarkerInfoEXT* /*label_info*/) {};
      return absl::bit_cast<PFN_vkVoidFunction>(begin_function);
    }
    if (strcmp(name, "vkCmdDebugMarkerEndEXT") == 0) {
      PFN_vkCmdDebugMarkerEndEXT end_function = +[](VkCommandBuffer /*command_buffer*/) {};
      return absl::bit_cast<PFN_vkVoidFunction>(end_function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);
  EXPECT_TRUE(dispatch_table.IsDebugMarkerExtensionSupported(device));
}

TEST(DispatchTable, CanCallEnumerateDeviceExtensionProperties) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);
  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkEnumerateDeviceExtensionProperties") == 0) {
      PFN_vkEnumerateDeviceExtensionProperties function =
          +[](VkPhysicalDevice /*physical_device*/, const char* /*layer_name*/,
              uint32_t* property_count, VkExtensionProperties *
              /*properties*/) -> VkResult {
        *property_count = 42;
        return VK_SUCCESS;
      };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);

  VkPhysicalDevice device = {};
  uint32_t property_count;

  VkResult result = dispatch_table.EnumerateDeviceExtensionProperties(instance)(
      device, nullptr, &property_count, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
  EXPECT_EQ(property_count, 42);
}

TEST(DispatchTable, CanCallGetPhysicalDeviceProperties) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkGetPhysicalDeviceProperties") == 0) {
      PFN_vkGetPhysicalDeviceProperties function =
          +[](VkPhysicalDevice /*physical_device*/, VkPhysicalDeviceProperties*
              /*properties*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);

  VkPhysicalDevice device = {};
  dispatch_table.GetPhysicalDeviceProperties(instance)(device, nullptr);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallGetInstanceProcAddr) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerInstanceDispatchTable some_dispatch_table = {};
  auto instance = absl::bit_cast<VkInstance>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
      +[](VkInstance /*instance*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkGetInstanceProcAddr") == 0) {
      PFN_vkGetInstanceProcAddr function = +[](VkInstance /*instance*/, const char *
                                               /*name*/) -> PFN_vkVoidFunction {
        was_called = true;
        return nullptr;
      };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateInstanceDispatchTable(instance, next_get_instance_proc_addr_function);

  dispatch_table.GetInstanceProcAddr(instance)(instance, "");
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallGetDeviceProcAddr) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkGetDeviceProcAddr") == 0) {
      PFN_vkGetDeviceProcAddr function = +[](VkDevice /*device*/, const char *
                                             /*name*/) -> PFN_vkVoidFunction {
        was_called = true;
        return nullptr;
      };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  dispatch_table.GetDeviceProcAddr(device)(device, "");
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallResetCommandPool) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkResetCommandPool") == 0) {
      PFN_vkResetCommandPool function =
          +[](VkDevice /*device*/, VkCommandPool /*command_pool*/,
              VkCommandPoolResetFlags /*flags*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandPool command_pool = {};
  VkResult result = dispatch_table.ResetCommandPool(device)(device, command_pool, 0);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallAllocateCommandBuffers) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkAllocateCommandBuffers") == 0) {
      PFN_vkAllocateCommandBuffers function =
          +[](VkDevice /*device*/, const VkCommandBufferAllocateInfo* /*allocate_info*/,
              VkCommandBuffer *
              /*command_buffers*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkResult result = dispatch_table.AllocateCommandBuffers(device)(device, nullptr, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallFreeCommandBuffers) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkFreeCommandBuffers") == 0) {
      PFN_vkFreeCommandBuffers function =
          +[](VkDevice /*device*/, VkCommandPool /*command_pool*/,
              uint32_t /*command_buffer_count*/, const VkCommandBuffer*
              /*command_buffers*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandPool command_pool = {};
  dispatch_table.FreeCommandBuffers(device)(device, command_pool, 0, nullptr);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallBeginCommandBuffer) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkBeginCommandBuffer") == 0) {
      PFN_vkBeginCommandBuffer function =
          +[](VkCommandBuffer /*command_buffer*/, const VkCommandBufferBeginInfo *
              /*begin_info*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  VkResult result = dispatch_table.BeginCommandBuffer(device)(command_buffer, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallEndCommandBuffer) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkEndCommandBuffer") == 0) {
      PFN_vkEndCommandBuffer function =
          +[](VkCommandBuffer /*command_buffer*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  VkResult result = dispatch_table.EndCommandBuffer(device)(command_buffer);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallResetCommandBuffer) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkResetCommandBuffer") == 0) {
      PFN_vkResetCommandBuffer function =
          +[](VkCommandBuffer /*command_buffer*/, VkCommandBufferResetFlags /*flags*/) -> VkResult {
        return VK_SUCCESS;
      };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  VkResult result = dispatch_table.ResetCommandBuffer(device)(command_buffer, 0);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallGetDeviceQueue) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkGetDeviceQueue") == 0) {
      PFN_vkGetDeviceQueue function = +[](VkDevice /*device*/, uint32_t /*queue_family_index*/,
                                          uint32_t /*queue_index*/, VkQueue*
                                          /*queue*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  dispatch_table.GetDeviceQueue(device)(device, 0, 0, nullptr);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallGetDeviceQueue2) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkGetDeviceQueue2") == 0) {
      PFN_vkGetDeviceQueue2 function =
          +[](VkDevice /*device*/, const VkDeviceQueueInfo2* /*queue_info*/, VkQueue*
              /*queue*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  dispatch_table.GetDeviceQueue2(device)(device, nullptr, nullptr);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallQueueSubmit) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkQueueSubmit") == 0) {
      PFN_vkQueueSubmit function =
          +[](VkQueue /*queue*/, uint32_t /*submit_count*/, const VkSubmitInfo* /*submit_info*/,
              VkFence /*fence*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkQueue queue = {};
  VkFence fence = {};
  VkResult result = dispatch_table.QueueSubmit(device)(queue, 0, nullptr, fence);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallQueuePresentKHR) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkQueuePresentKHR") == 0) {
      PFN_vkQueuePresentKHR function = +[](VkQueue /*queue*/, const VkPresentInfoKHR *
                                           /*present_info*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkQueue queue = {};
  VkResult result = dispatch_table.QueuePresentKHR(device)(queue, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallCreateQueryPool) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCreateQueryPool") == 0) {
      PFN_vkCreateQueryPool function =
          +[](VkDevice /*device*/, const VkQueryPoolCreateInfo* /*create_info*/,
              const VkAllocationCallbacks* /*allocator*/, VkQueryPool *
              /*query_pool*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkResult result = dispatch_table.CreateQueryPool(device)(device, nullptr, nullptr, nullptr);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallResetQueryPoolEXT) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkResetQueryPoolEXT") == 0) {
      PFN_vkResetQueryPoolEXT function = +[](VkDevice /*device*/, VkQueryPool /*query_pool*/,
                                             uint32_t
                                             /*first_query*/,
                                             uint32_t /*query_count*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkQueryPool query_pool = {};
  dispatch_table.ResetQueryPoolEXT(device)(device, query_pool, 0, 0);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallGetQueryPoolResults) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkGetQueryPoolResults") == 0) {
      PFN_vkGetQueryPoolResults function =
          +[](VkDevice /*device*/, VkQueryPool /*query_pool*/, uint32_t /*first_query*/,
              uint32_t /*query_count*/, size_t /*data_size*/, void* /*data*/,
              VkDeviceSize /*stride*/,
              VkQueryResultFlags /*flags*/) -> VkResult { return VK_SUCCESS; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkQueryPool query_pool = {};
  VkResult result =
      dispatch_table.GetQueryPoolResults(device)(device, query_pool, 0, 0, 0, nullptr, 0, 0);
  EXPECT_EQ(result, VK_SUCCESS);
}

TEST(DispatchTable, CanCallCmdWriteTimestamp) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdWriteTimestamp") == 0) {
      PFN_vkCmdWriteTimestamp function =
          +[](VkCommandBuffer /*command_buffer*/, VkPipelineStageFlagBits /*pipeline_stage*/,
              VkQueryPool /*query_pool*/, uint32_t /*query*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  VkQueryPool query_pool = {};
  dispatch_table.CmdWriteTimestamp(device)(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           query_pool, 0);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallCmdBeginDebugUtilsLabelEXT) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdBeginDebugUtilsLabelEXT") == 0) {
      PFN_vkCmdBeginDebugUtilsLabelEXT function =
          +[](VkCommandBuffer /*command_buffer*/, const VkDebugUtilsLabelEXT* /*label_info*/) {
            was_called = true;
          };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  dispatch_table.CmdBeginDebugUtilsLabelEXT(device)(command_buffer, nullptr);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallCmdEndDebugUtilsLabelEXT) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdEndDebugUtilsLabelEXT") == 0) {
      PFN_vkCmdEndDebugUtilsLabelEXT function =
          +[](VkCommandBuffer /*command_buffer*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  dispatch_table.CmdEndDebugUtilsLabelEXT(device)(command_buffer);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallCmdDebugMarkerBeginEXT) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdDebugMarkerBeginEXT") == 0) {
      PFN_vkCmdDebugMarkerBeginEXT function =
          +[](VkCommandBuffer /*command_buffer*/,
              const VkDebugMarkerMarkerInfoEXT* /*marker_info*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  dispatch_table.CmdDebugMarkerBeginEXT(device)(command_buffer, nullptr);
  EXPECT_TRUE(was_called);
}

TEST(DispatchTable, CanCallCmdDebugMarkerEndEXT) {
  // We can not create an actual VkInstance, but the first bytes of an any dispatchable type in
  // Vulkan will be a pointer to a dispatch table. This characteristic will be used by our dispatch
  // table wrapper, so we need to mimic it.
  VkLayerDispatchTable some_dispatch_table = {};
  auto device = absl::bit_cast<VkDevice>(&some_dispatch_table);

  static bool was_called = false;

  PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
      +[](VkDevice /*device*/, const char* name) -> PFN_vkVoidFunction {
    if (strcmp(name, "vkCmdDebugMarkerEndEXT") == 0) {
      PFN_vkCmdDebugMarkerEndEXT function =
          +[](VkCommandBuffer /*command_buffer*/) { was_called = true; };
      return absl::bit_cast<PFN_vkVoidFunction>(function);
    }
    return nullptr;
  };

  DispatchTable dispatch_table = {};
  dispatch_table.CreateDeviceDispatchTable(device, next_get_device_proc_addr_function);

  VkCommandBuffer command_buffer = {};
  dispatch_table.CmdDebugMarkerEndEXT(device)(command_buffer);
  EXPECT_TRUE(was_called);
}

}  // namespace orbit_vulkan_layer