// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DispatchTable.h"

#include "OrbitBase/Logging.h"

namespace orbit_vulkan_layer {

void DispatchTable::CreateInstanceDispatchTable(
    const VkInstance& instance,
    const PFN_vkGetInstanceProcAddr& next_get_instance_proc_addr_function) {
  VkLayerInstanceDispatchTable dispatch_table;
  dispatch_table.DestroyInstance = absl::bit_cast<PFN_vkDestroyInstance>(
      next_get_instance_proc_addr_function(instance, "vkDestroyInstance"));
  dispatch_table.GetInstanceProcAddr = absl::bit_cast<PFN_vkGetInstanceProcAddr>(
      next_get_instance_proc_addr_function(instance, "vkGetInstanceProcAddr"));
  dispatch_table.EnumerateDeviceExtensionProperties =
      absl::bit_cast<PFN_vkEnumerateDeviceExtensionProperties>(
          next_get_instance_proc_addr_function(instance, "vkEnumerateDeviceExtensionProperties"));
  dispatch_table.EnumeratePhysicalDevices = absl::bit_cast<PFN_vkEnumeratePhysicalDevices>(
      next_get_instance_proc_addr_function(instance, "vkEnumeratePhysicalDevices"));

  {
    absl::WriterMutexLock lock(&mutex_);
    instance_dispatch_table_[instance] = dispatch_table;
  }
}

void DispatchTable::RemoveInstanceDispatchTable(const VkInstance& instance) {
  absl::WriterMutexLock lock(&mutex_);
  instance_dispatch_table_.erase(instance);
}

void DispatchTable::CreateDeviceDispatchTable(
    const VkDevice& device, const PFN_vkGetDeviceProcAddr& next_get_device_proc_add_function) {
  VkLayerDispatchTable dispatch_table;

  dispatch_table.CreateCommandPool = absl::bit_cast<PFN_vkCreateCommandPool>(
      next_get_device_proc_add_function(device, "vkCreateCommandPool"));
  dispatch_table.DestroyCommandPool = absl::bit_cast<PFN_vkDestroyCommandPool>(
      next_get_device_proc_add_function(device, "vkDestroyCommandPool"));
  dispatch_table.ResetCommandPool = absl::bit_cast<PFN_vkResetCommandPool>(
      next_get_device_proc_add_function(device, "vkResetCommandPool"));

  dispatch_table.AllocateCommandBuffers = absl::bit_cast<PFN_vkAllocateCommandBuffers>(
      next_get_device_proc_add_function(device, "vkAllocateCommandBuffers"));
  dispatch_table.FreeCommandBuffers = absl::bit_cast<PFN_vkFreeCommandBuffers>(
      next_get_device_proc_add_function(device, "vkFreeCommandBuffers"));
  dispatch_table.BeginCommandBuffer = absl::bit_cast<PFN_vkBeginCommandBuffer>(
      next_get_device_proc_add_function(device, "vkBeginCommandBuffer"));
  dispatch_table.EndCommandBuffer = absl::bit_cast<PFN_vkEndCommandBuffer>(
      next_get_device_proc_add_function(device, "vkEndCommandBuffer"));
  dispatch_table.ResetCommandBuffer = absl::bit_cast<PFN_vkResetCommandBuffer>(
      next_get_device_proc_add_function(device, "vkResetCommandBuffer"));

  dispatch_table.QueueSubmit =
      absl::bit_cast<PFN_vkQueueSubmit>(next_get_device_proc_add_function(device, "vkQueueSubmit"));
  dispatch_table.QueuePresentKHR = absl::bit_cast<PFN_vkQueuePresentKHR>(
      next_get_device_proc_add_function(device, "vkQueuePresentKHR"));

  dispatch_table.GetDeviceQueue = absl::bit_cast<PFN_vkGetDeviceQueue>(
      next_get_device_proc_add_function(device, "vkGetDeviceQueue"));
  dispatch_table.GetDeviceQueue2 = absl::bit_cast<PFN_vkGetDeviceQueue2>(
      next_get_device_proc_add_function(device, "vkGetDeviceQueue2"));

  dispatch_table.CreateQueryPool = absl::bit_cast<PFN_vkCreateQueryPool>(
      next_get_device_proc_add_function(device, "vkCreateQueryPool"));
  dispatch_table.CmdResetQueryPool = absl::bit_cast<PFN_vkCmdResetQueryPool>(
      next_get_device_proc_add_function(device, "vkCmdResetQueryPool"));

  dispatch_table.CmdWriteTimestamp = absl::bit_cast<PFN_vkCmdWriteTimestamp>(
      next_get_device_proc_add_function(device, "vkCmdWriteTimestamp"));
  dispatch_table.CmdBeginQuery = absl::bit_cast<PFN_vkCmdBeginQuery>(
      next_get_device_proc_add_function(device, "vkCmdBeginQuery"));
  dispatch_table.CmdEndQuery =
      absl::bit_cast<PFN_vkCmdEndQuery>(next_get_device_proc_add_function(device, "vkCmdEndQuery"));
  dispatch_table.GetQueryPoolResults = absl::bit_cast<PFN_vkGetQueryPoolResults>(
      next_get_device_proc_add_function(device, "vkGetQueryPoolResults"));

  {
    absl::WriterMutexLock lock(&mutex_);
    device_dispatch_table_[device] = dispatch_table;
  }
}

void DispatchTable::RemoveDeviceDispatchTable(const VkDevice& device) {
  absl::WriterMutexLock lock(&mutex_);
  device_dispatch_table_.erase(device);
}

PFN_vkDestroyDevice DispatchTable::DestroyDevice(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).DestroyDevice;
}

PFN_vkDestroyInstance DispatchTable::DestroyInstance(const VkInstance& instance) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(instance));
  return instance_dispatch_table_.at(instance).DestroyInstance;
}

PFN_vkEnumerateDeviceExtensionProperties DispatchTable::EnumerateDeviceExtensionProperties(
    const VkInstance& instance) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(instance));
  return instance_dispatch_table_.at(instance).EnumerateDeviceExtensionProperties;
}

PFN_vkEnumeratePhysicalDevices DispatchTable::EnumeratePhysicalDevices(const VkInstance& instance) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(instance));
  return instance_dispatch_table_.at(instance).EnumeratePhysicalDevices;
}

PFN_vkGetInstanceProcAddr DispatchTable::GetInstanceProcAddr(const VkInstance& instance) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(instance));
  return instance_dispatch_table_.at(instance).GetInstanceProcAddr;
}

PFN_vkGetDeviceProcAddr DispatchTable::GetDeviceProcAddr(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).GetDeviceProcAddr;
}

PFN_vkCreateCommandPool DispatchTable::CreateCommandPool(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).CreateCommandPool;
}

PFN_vkDestroyCommandPool DispatchTable::DestroyCommandPool(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).DestroyCommandPool;
}

PFN_vkResetCommandPool DispatchTable::ResetCommandPool(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).ResetCommandPool;
}

PFN_vkAllocateCommandBuffers DispatchTable::AllocateCommandBuffers(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).AllocateCommandBuffers;
}

PFN_vkFreeCommandBuffers DispatchTable::FreeCommandBuffers(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).FreeCommandBuffers;
}

PFN_vkBeginCommandBuffer DispatchTable::BeginCommandBuffer(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).BeginCommandBuffer;
}

PFN_vkEndCommandBuffer DispatchTable::EndCommandBuffer(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).EndCommandBuffer;
}

PFN_vkResetCommandBuffer DispatchTable::ResetCommandBuffer(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).ResetCommandBuffer;
}

PFN_vkGetDeviceQueue DispatchTable::GetDeviceQueue(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).GetDeviceQueue;
}

PFN_vkGetDeviceQueue2 DispatchTable::GetDeviceQueue2(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).GetDeviceQueue2;
}

PFN_vkQueueSubmit DispatchTable::QueueSubmit(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).QueueSubmit;
}

PFN_vkQueuePresentKHR DispatchTable::QueuePresentKHR(const VkDevice& device) {
  absl::ReaderMutexLock lock(&mutex_);
  CHECK(device_dispatch_table_.contains(device));
  return device_dispatch_table_.at(device).QueuePresentKHR;
}

}  // namespace orbit_vulkan_layer