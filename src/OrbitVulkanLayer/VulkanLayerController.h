// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_VULKAN_LAYER_CONTROLLER_H_
#define ORBIT_VULKAN_LAYER_VULKAN_LAYER_CONTROLLER_H_

#include <absl/base/casts.h>
#include <unistd.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan.h>

#include <cstdlib>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "VulkanLayerProducerImpl.h"

namespace orbit_vulkan_layer {

/**
 * This class controls the logic of this layer. For the intercepted Vulkan functions,
 * it provides an `On*` function (e.g. for `vkQueueSubmit` there is `OnQueueSubmit`) that delegates
 * to the driver/next layer (see `DispatchTable`) and calls the required functions for this layer to
 * function properly. So it ties together classes like `SubmissionTracker` or `TimerQueryPool`.
 * In particular it executes the bootstrapping code (OnCreateInstance/Device) and the enumerations
 * required by every Vulkan layer, to describe the layer as well as the extensions it uses.
 *
 * Usage: For an intercepted Vulkan function "X" in the layers entry (EntryPoints.cpp), `OnX`
 * needs to be called on this controller.
 *
 * Note the main reason not to expose the Vulkan functions directly in this class is that this
 * allows us to write tests. Those tests can check if we glue the code correctly together
 * and if we do the proper bootstrapping.
 */
template <class DispatchTable, class QueueManager, class DeviceManager, class TimerQueryPool,
          class SubmissionTracker, class VulkanWrapper>
class VulkanLayerController {
 public:
  // Layer metadata. This must be in sync with the json file in the resources.
  static constexpr const char* kLayerName = "ORBIT_VK_LAYER";
  static constexpr const char* kLayerDescription = "Provides GPU insights for the Orbit Profiler";
  static constexpr uint32_t kLayerImplVersion = 1;
  static constexpr uint32_t kLayerSpecVersion = VK_API_VERSION_1_1;

  static constexpr std::array<VkExtensionProperties, 1> kImplementedDeviceExtensions = {
      VkExtensionProperties{VK_EXT_DEBUG_MARKER_EXTENSION_NAME, VK_EXT_DEBUG_MARKER_SPEC_VERSION}};

  static constexpr std::array<VkExtensionProperties, 2> kImplementedInstanceExtensions = {
      VkExtensionProperties{VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION},
      VkExtensionProperties{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION}};

  VulkanLayerController()
      : device_manager_(&dispatch_table_),
        timer_query_pool_(&dispatch_table_, kNumTimerQuerySlots),
        submission_tracker_(&dispatch_table_, &timer_query_pool_, &device_manager_,
                            std::numeric_limits<uint32_t>::max()) {}

  ~VulkanLayerController() { CloseVulkanLayerProducerIfNecessary(); }

  // ----------------------------------------------------------------------------
  // Layer bootstrapping code
  // ----------------------------------------------------------------------------

  [[nodiscard]] VkResult OnCreateInstance(const VkInstanceCreateInfo* create_info,
                                          const VkAllocationCallbacks* allocator,
                                          VkInstance* instance) {
    // The specification ensures that the create_info pointer is not nullptr.
    ORBIT_CHECK(create_info != nullptr);

    auto* layer_create_info = absl::bit_cast<VkLayerInstanceCreateInfo*>(create_info->pNext);

    // Iterate over the create info chain to find the layer linkage information. This contains
    // the GetInstanceProcAddr function of the next layer (or the driver if this is the last layer).
    while (layer_create_info != nullptr &&
           (layer_create_info->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO ||
            layer_create_info->function != VK_LAYER_LINK_INFO)) {
      layer_create_info = absl::bit_cast<VkLayerInstanceCreateInfo*>(layer_create_info->pNext);
    }

    if (layer_create_info == nullptr) {
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    InitVulkanLayerProducerIfNecessary();
    DumpProcessIdIfNecessary();

    PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
        layer_create_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;

    // Advance linkage for next layer.
    layer_create_info->u.pLayerInfo = layer_create_info->u.pLayerInfo->pNext;

    // Ensure that the extensions that the layer uses are requested in the CreateInstance call.
    // As we can and should not modify the given create_info, we need to create a modified version
    // containing the required extension. So first create a copy of the given extension names.
    std::vector<std::string> all_extension_names{};
    all_extension_names.assign(
        create_info->ppEnabledExtensionNames,
        create_info->ppEnabledExtensionNames + create_info->enabledExtensionCount);

    // Add our required extension (if not already present), to the extensions requested by the game.
    AddRequiredInstanceExtensionNameIfMissing(
        create_info, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, &all_extension_names);

    // Expose the c-strings (but ensure, the std::strings stay in memory!).
    std::vector<const char*> all_extension_names_cstr{};
    all_extension_names_cstr.reserve(all_extension_names.size());
    for (const std::string& extension : all_extension_names) {
      all_extension_names_cstr.push_back(extension.c_str());
    }

    // Copy the given create_info and set the modified requested extensions.
    VkInstanceCreateInfo create_info_modified = *create_info;
    create_info_modified.enabledExtensionCount = all_extension_names_cstr.size();
    create_info_modified.ppEnabledExtensionNames = all_extension_names_cstr.data();

    // Need to call vkCreateInstance down the chain to actually create the
    // instance, as we need the instance to be alive in the create instance dispatch table.
    auto create_instance = absl::bit_cast<PFN_vkCreateInstance>(
        next_get_instance_proc_addr_function(VK_NULL_HANDLE, "vkCreateInstance"));
    VkResult result = create_instance(&create_info_modified, allocator, instance);

    // Only create our dispatch table, if the instance was successfully created.
    if (result == VK_SUCCESS) {
      dispatch_table_.CreateInstanceDispatchTable(*instance, next_get_instance_proc_addr_function);
    }

    return result;
  }

  [[nodiscard]] VkResult OnCreateDevice(VkPhysicalDevice /*physical_device*/ physical_device,
                                        const VkDeviceCreateInfo* create_info,
                                        const VkAllocationCallbacks* allocator /*allocator*/,
                                        VkDevice* device) {
    auto* layer_create_info = absl::bit_cast<VkLayerDeviceCreateInfo*>(create_info->pNext);

    while (layer_create_info != nullptr &&
           (layer_create_info->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO ||
            layer_create_info->function != VK_LAYER_LINK_INFO)) {
      layer_create_info = absl::bit_cast<VkLayerDeviceCreateInfo*>(layer_create_info->pNext);
    }

    if (layer_create_info == nullptr) {
      return VK_ERROR_INITIALIZATION_FAILED;
    }

    PFN_vkGetInstanceProcAddr next_get_instance_proc_addr_function =
        layer_create_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr next_get_device_proc_addr_function =
        layer_create_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;

    // Advance linkage for next layer
    layer_create_info->u.pLayerInfo = layer_create_info->u.pLayerInfo->pNext;

    // Ensure that the extensions that the layer uses are requested in the CreateDevice call.
    // As we can and should not modify the given create_info, we need to create a modified version
    // containing the required extension. So first create a copy of the given extension names.
    std::vector<std::string> all_extension_names{};
    all_extension_names.assign(
        create_info->ppEnabledExtensionNames,
        create_info->ppEnabledExtensionNames + create_info->enabledExtensionCount);

    // Add our required extension (if not already present), to the extensions requested by the game.
    AddRequiredDeviceExtensionNameIfMissing(
        create_info, physical_device, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME, &all_extension_names);

    // Expose the c-strings (but ensure, the std::strings stay in memory!).
    std::vector<const char*> all_extension_names_cstr{};
    all_extension_names_cstr.reserve(all_extension_names.size());
    for (const std::string& extension : all_extension_names) {
      all_extension_names_cstr.push_back(extension.c_str());
    }

    // Copy the given create_info and set the modified requested extensions.
    VkDeviceCreateInfo create_info_modified = *create_info;
    create_info_modified.enabledExtensionCount = all_extension_names_cstr.size();
    create_info_modified.ppEnabledExtensionNames = all_extension_names_cstr.data();

    // Need to call vkCreateInstance down the chain to actually create the
    // instance, as we need it to be alive in the create instance dispatch table.
    auto create_device_function =
        absl::bit_cast<PFN_vkCreateDevice>(next_get_instance_proc_addr_function(
            dispatch_table_.GetInstance(physical_device), "vkCreateDevice"));
    VkResult result =
        create_device_function(physical_device, &create_info_modified, allocator, device);

    // Only create our dispatch table and do the initialization of this device, if the it was
    // actually created.
    if (result == VK_SUCCESS) {
      dispatch_table_.CreateDeviceDispatchTable(*device, next_get_device_proc_addr_function);

      device_manager_.TrackLogicalDevice(physical_device, *device);
      timer_query_pool_.InitializeTimerQueryPool(*device);
    }

    return result;
  }

  [[nodiscard]] PFN_vkVoidFunction ForwardGetDeviceProcAddr(VkDevice device, const char* name) {
    return dispatch_table_.GetDeviceProcAddr(device)(device, name);
  }

  [[nodiscard]] PFN_vkVoidFunction ForwardGetInstanceProcAddr(VkInstance instance,
                                                              const char* name) {
    return dispatch_table_.GetInstanceProcAddr(instance)(instance, name);
  }

  void OnDestroyInstance(VkInstance instance, const VkAllocationCallbacks* allocator) {
    PFN_vkDestroyInstance destroy_instance_function = dispatch_table_.DestroyInstance(instance);
    ORBIT_CHECK(destroy_instance_function != nullptr);
    dispatch_table_.RemoveInstanceDispatchTable(instance);

    destroy_instance_function(instance, allocator);

    CloseVulkanLayerProducerIfNecessary();
  }

  void OnDestroyDevice(VkDevice device, const VkAllocationCallbacks* allocator) {
    PFN_vkDestroyDevice destroy_device_function = dispatch_table_.DestroyDevice(device);
    ORBIT_CHECK(destroy_device_function != nullptr);
    device_manager_.UntrackLogicalDevice(device);
    timer_query_pool_.DestroyTimerQueryPool(device);
    dispatch_table_.RemoveDeviceDispatchTable(device);

    destroy_device_function(device, allocator);
  }

  // ----------------------------------------------------------------------------
  // Core layer logic
  // ----------------------------------------------------------------------------

  [[nodiscard]] VkResult OnResetCommandPool(VkDevice device, VkCommandPool command_pool,
                                            VkCommandPoolResetFlags flags) {
    submission_tracker_.ResetCommandPool(command_pool);
    return dispatch_table_.ResetCommandPool(device)(device, command_pool, flags);
  }

  [[nodiscard]] VkResult OnAllocateCommandBuffers(VkDevice device,
                                                  const VkCommandBufferAllocateInfo* allocate_info,
                                                  VkCommandBuffer* command_buffers) {
    VkResult result =
        dispatch_table_.AllocateCommandBuffers(device)(device, allocate_info, command_buffers);

    // Only track the command buffers, if they were successfully allocated.
    if (result == VK_SUCCESS) {
      VkCommandPool pool = allocate_info->commandPool;
      const uint32_t command_buffer_count = allocate_info->commandBufferCount;
      submission_tracker_.TrackCommandBuffers(device, pool, command_buffers, command_buffer_count);
    }

    return result;
  }

  void OnFreeCommandBuffers(VkDevice device, VkCommandPool command_pool,
                            uint32_t command_buffer_count, const VkCommandBuffer* command_buffers) {
    submission_tracker_.UntrackCommandBuffers(device, command_pool, command_buffers,
                                              command_buffer_count);
    return dispatch_table_.FreeCommandBuffers(device)(device, command_pool, command_buffer_count,
                                                      command_buffers);
  }

  [[nodiscard]] VkResult OnBeginCommandBuffer(VkCommandBuffer command_buffer,
                                              const VkCommandBufferBeginInfo* begin_info) {
    VkResult result =
        dispatch_table_.BeginCommandBuffer(command_buffer)(command_buffer, begin_info);

    // Only mark the command buffer's begin, if the Vulkan call was successful.
    if (result == VK_SUCCESS) {
      submission_tracker_.MarkCommandBufferBegin(command_buffer);
    }
    return result;
  }

  [[nodiscard]] VkResult OnEndCommandBuffer(VkCommandBuffer command_buffer) {
    submission_tracker_.MarkCommandBufferEnd(command_buffer);
    return dispatch_table_.EndCommandBuffer(command_buffer)(command_buffer);
  }

  [[nodiscard]] VkResult OnResetCommandBuffer(VkCommandBuffer command_buffer,
                                              VkCommandBufferResetFlags flags) {
    submission_tracker_.ResetCommandBuffer(command_buffer);
    return dispatch_table_.ResetCommandBuffer(command_buffer)(command_buffer, flags);
  }

  void OnGetDeviceQueue(VkDevice device, uint32_t queue_family_index, uint32_t queue_index,
                        VkQueue* queue) {
    dispatch_table_.GetDeviceQueue(device)(device, queue_family_index, queue_index, queue);
    queue_manager_.TrackQueue(*queue, device);
  }

  void OnGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* queue_info, VkQueue* queue) {
    dispatch_table_.GetDeviceQueue2(device)(device, queue_info, queue);
    queue_manager_.TrackQueue(*queue, device);
  }

  [[nodiscard]] VkResult OnQueueSubmit(VkQueue queue, uint32_t submit_count,
                                       const VkSubmitInfo* submits, VkFence fence) {
    std::optional<typename SubmissionTracker::QueueSubmission> queue_submission_optional =
        submission_tracker_.PersistCommandBuffersOnSubmit(queue, submit_count, submits);
    VkResult result = dispatch_table_.QueueSubmit(queue)(queue, submit_count, submits, fence);

    // Only persist the submission, if the submit was successful.
    if (result == VK_SUCCESS) {
      submission_tracker_.PersistDebugMarkersOnSubmit(queue, submit_count, submits,
                                                      queue_submission_optional);
    }
    return result;
  }

  [[nodiscard]] VkResult OnQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info) {
    // TODO(b/185454430): Consider calling  CompleteSubmits periodically on vkQueueSubmit instead of
    //  on vkQueuePresentKHR.
    submission_tracker_.CompleteSubmits(queue_manager_.GetDeviceOfQueue(queue));
    return dispatch_table_.QueuePresentKHR(queue)(queue, present_info);
  }

  void OnCmdBeginDebugUtilsLabelEXT(VkCommandBuffer command_buffer,
                                    const VkDebugUtilsLabelEXT* label_info) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(command_buffer)) {
      dispatch_table_.CmdBeginDebugUtilsLabelEXT(command_buffer)(command_buffer, label_info);
    }

    // Specified by the standard.
    ORBIT_CHECK(label_info != nullptr);
    submission_tracker_.MarkDebugMarkerBegin(command_buffer, label_info->pLabelName,
                                             {
                                                 .red = label_info->color[0],
                                                 .green = label_info->color[1],
                                                 .blue = label_info->color[2],
                                                 .alpha = label_info->color[3],
                                             });
  }

  void OnCmdEndDebugUtilsLabelEXT(VkCommandBuffer command_buffer) {
    submission_tracker_.MarkDebugMarkerEnd(command_buffer);
    if (dispatch_table_.IsDebugUtilsExtensionSupported(command_buffer)) {
      dispatch_table_.CmdEndDebugUtilsLabelEXT(command_buffer)(command_buffer);
    }
  }

  void OnCmdDebugMarkerBeginEXT(VkCommandBuffer command_buffer,
                                const VkDebugMarkerMarkerInfoEXT* marker_info) {
    if (dispatch_table_.IsDebugMarkerExtensionSupported(command_buffer)) {
      dispatch_table_.CmdDebugMarkerBeginEXT(command_buffer)(command_buffer, marker_info);
    }

    // Specified by the standard.
    ORBIT_CHECK(marker_info != nullptr);
    submission_tracker_.MarkDebugMarkerBegin(command_buffer, marker_info->pMarkerName,
                                             {
                                                 .red = marker_info->color[0],
                                                 .green = marker_info->color[1],
                                                 .blue = marker_info->color[2],
                                                 .alpha = marker_info->color[3],
                                             });
  }

  void OnCmdDebugMarkerEndEXT(VkCommandBuffer command_buffer) {
    submission_tracker_.MarkDebugMarkerEnd(command_buffer);
    if (dispatch_table_.IsDebugMarkerExtensionSupported(command_buffer)) {
      dispatch_table_.CmdDebugMarkerEndEXT(command_buffer)(command_buffer);
    }
  }

  // ----------------------------------------------------------------------------
  // Unused but implemented extension methods (need to implement all methods of
  // an extension)
  // ----------------------------------------------------------------------------
  void OnCmdInsertDebugUtilsLabelEXT(VkCommandBuffer command_buffer,
                                     const VkDebugUtilsLabelEXT* label_info) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(command_buffer)) {
      dispatch_table_.CmdInsertDebugUtilsLabelEXT(command_buffer)(command_buffer, label_info);
    }
  }

  VkResult OnCreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                          const VkAllocationCallbacks* allocator,
                                          VkDebugUtilsMessengerEXT* messenger) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(instance)) {
      return dispatch_table_.CreateDebugUtilsMessengerEXT(instance)(instance, create_info,
                                                                    allocator, messenger);
    }
    return VK_SUCCESS;
  }

  void OnDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                       const VkAllocationCallbacks* allocator) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(instance)) {
      dispatch_table_.DestroyDebugUtilsMessengerEXT(instance)(instance, messenger, allocator);
    }
  }

  void OnQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* label_info) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(queue)) {
      dispatch_table_.QueueBeginDebugUtilsLabelEXT(queue)(queue, label_info);
    }
  }

  void OnQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(queue)) {
      dispatch_table_.QueueEndDebugUtilsLabelEXT(queue)(queue);
    }
  }

  void OnQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* label_info) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(queue)) {
      dispatch_table_.QueueInsertDebugUtilsLabelEXT(queue)(queue, label_info);
    }
  }

  VkResult OnSetDebugUtilsObjectNameEXT(VkDevice device,
                                        const VkDebugUtilsObjectNameInfoEXT* name_info) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(device)) {
      return dispatch_table_.SetDebugUtilsObjectNameEXT(device)(device, name_info);
    }
    return VK_SUCCESS;
  }

  VkResult OnSetDebugUtilsObjectTagEXT(VkDevice device,
                                       const VkDebugUtilsObjectTagInfoEXT* tag_info) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(device)) {
      return dispatch_table_.SetDebugUtilsObjectTagEXT(device)(device, tag_info);
    }
    return VK_SUCCESS;
  }

  void OnSubmitDebugUtilsMessageEXT(VkInstance instance,
                                    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                    VkDebugUtilsMessageTypeFlagsEXT message_types,
                                    const VkDebugUtilsMessengerCallbackDataEXT* callback_data) {
    if (dispatch_table_.IsDebugUtilsExtensionSupported(instance)) {
      dispatch_table_.SubmitDebugUtilsMessageEXT(instance)(instance, message_severity,
                                                           message_types, callback_data);
    }
  }

  void OnCmdDebugMarkerInsertEXT(VkCommandBuffer command_buffer,
                                 const VkDebugMarkerMarkerInfoEXT* marker_info) {
    if (dispatch_table_.IsDebugMarkerExtensionSupported(command_buffer)) {
      dispatch_table_.CmdDebugMarkerInsertEXT(command_buffer)(command_buffer, marker_info);
    }
  }

  VkResult OnDebugMarkerSetObjectNameEXT(VkDevice device,
                                         const VkDebugMarkerObjectNameInfoEXT* name_info) {
    if (dispatch_table_.IsDebugMarkerExtensionSupported(device)) {
      return dispatch_table_.DebugMarkerSetObjectNameEXT(device)(device, name_info);
    }
    return VK_SUCCESS;
  }

  VkResult OnDebugMarkerSetObjectTagEXT(VkDevice device,
                                        const VkDebugMarkerObjectTagInfoEXT* tag_info) {
    if (dispatch_table_.IsDebugMarkerExtensionSupported(device)) {
      return dispatch_table_.DebugMarkerSetObjectTagEXT(device)(device, tag_info);
    }
    return VK_SUCCESS;
  }

  VkResult OnCreateDebugReportCallbackEXT(VkInstance instance,
                                          const VkDebugReportCallbackCreateInfoEXT* create_info,
                                          const VkAllocationCallbacks* allocator,
                                          VkDebugReportCallbackEXT* callback) {
    if (dispatch_table_.IsDebugReportExtensionSupported(instance)) {
      return dispatch_table_.CreateDebugReportCallbackEXT(instance)(instance, create_info,
                                                                    allocator, callback);
    }
    return VK_SUCCESS;
  }

  void OnDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                               VkDebugReportObjectTypeEXT object_type, uint64_t object,
                               size_t location, int32_t message_code, const char* layer_prefix,
                               const char* message) {
    if (dispatch_table_.IsDebugReportExtensionSupported(instance)) {
      dispatch_table_.DebugReportMessageEXT(instance)(
          instance, flags, object_type, object, location, message_code, layer_prefix, message);
    }
  }

  void OnDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                       const VkAllocationCallbacks* allocator) {
    if (dispatch_table_.IsDebugReportExtensionSupported(instance)) {
      dispatch_table_.DestroyDebugReportCallbackEXT(instance)(instance, callback, allocator);
    }
  }

  // ----------------------------------------------------------------------------
  // Layer enumeration functions
  // ----------------------------------------------------------------------------

  [[nodiscard]] VkResult OnEnumerateInstanceLayerProperties(uint32_t* property_count,
                                                            VkLayerProperties* properties) {
    // Vulkan spec dictates that we are only supposed to enumerate ourselves.
    if (property_count != nullptr) {
      *property_count = 1;
    }
    if (properties != nullptr) {
      snprintf(properties->layerName, sizeof(properties->layerName), kLayerName);
      snprintf(properties->description, sizeof(properties->description), kLayerDescription);
      properties->implementationVersion = kLayerImplVersion;
      properties->specVersion = kLayerSpecVersion;
    }

    return VK_SUCCESS;
  }

  [[nodiscard]] VkResult OnEnumerateInstanceExtensionProperties(const char* layer_name,
                                                                uint32_t* property_count,
                                                                VkExtensionProperties* properties) {
    if (layer_name == nullptr || strcmp(layer_name, kLayerName) != 0) {
      // Vulkan spec mandates returning this when this layer isn't being queried.
      return VK_ERROR_LAYER_NOT_PRESENT;
    }

    ORBIT_CHECK(property_count != nullptr);
    if (property_count != nullptr) {
      *property_count = kImplementedInstanceExtensions.size();
    }
    if (properties != nullptr) {
      memcpy(properties, kImplementedInstanceExtensions.data(),
             kImplementedInstanceExtensions.size() * sizeof(VkExtensionProperties));
    }
    return VK_SUCCESS;
  }

  [[nodiscard]] VkResult OnEnumerateDeviceExtensionProperties(VkPhysicalDevice physical_device,
                                                              const char* layer_name,
                                                              uint32_t* property_count,
                                                              VkExtensionProperties* properties) {
    // If our layer is queried exclusively, we just return our extensions. Note that queries with
    // layer_name == nullptr request all extensions.
    if (layer_name != nullptr && strcmp(layer_name, kLayerName) == 0) {
      // If properties == nullptr, only the number of extensions is queried.
      if (properties == nullptr) {
        *property_count = kImplementedDeviceExtensions.size();
        return VK_SUCCESS;
      }
      uint32_t num_extensions_to_copy = kImplementedDeviceExtensions.size();
      // In the case that less extensions are queried then the layer uses, we copy on this number
      // and return VK_INCOMPLETE, according to the specification.
      if (*property_count < num_extensions_to_copy) {
        num_extensions_to_copy = *property_count;
      }
      memcpy(properties, kImplementedDeviceExtensions.data(),
             num_extensions_to_copy * sizeof(VkExtensionProperties));
      *property_count = num_extensions_to_copy;

      if (num_extensions_to_copy < kImplementedDeviceExtensions.size()) {
        return VK_INCOMPLETE;
      }
      return VK_SUCCESS;
    }

    // If a different layer is queried exclusively, we forward the call.
    if (layer_name != nullptr) {
      return dispatch_table_.EnumerateDeviceExtensionProperties(physical_device)(
          physical_device, layer_name, property_count, properties);
    }

    // This is a general query, so we need to append our extensions to the ones down in the
    // callchain.
    uint32_t num_other_extensions;
    VkResult result = dispatch_table_.EnumerateDeviceExtensionProperties(physical_device)(
        physical_device, nullptr, &num_other_extensions, nullptr);
    if (result != VK_SUCCESS) {
      return result;
    }

    std::vector<VkExtensionProperties> extensions(num_other_extensions);
    result = dispatch_table_.EnumerateDeviceExtensionProperties(physical_device)(
        physical_device, nullptr, &num_other_extensions, extensions.data());
    if (result != VK_SUCCESS) {
      return result;
    }

    // Append all of our extensions, that are not yet listed.
    // Note as this list of our extensions is very small, we are fine with O(N*M) runtime.
    for (const auto& extension : kImplementedDeviceExtensions) {
      bool already_present = false;
      for (const auto& other_extension : extensions) {
        if (strcmp(extension.extensionName, other_extension.extensionName) == 0) {
          already_present = true;
          break;
        }
      }
      if (!already_present) {
        extensions.push_back(extension);
      }
    }

    // As above, if properties is nullptr, only the number if extensions is queried.
    if (properties == nullptr) {
      *property_count = extensions.size();
      return VK_SUCCESS;
    }

    uint32_t num_extensions_to_copy = extensions.size();
    // In the case that less extensions are queried than the layer uses, we copy this number and
    // return VK_INCOMPLETE, according to the specification.
    if (*property_count < num_extensions_to_copy) {
      num_extensions_to_copy = *property_count;
    }
    memcpy(properties, extensions.data(), num_extensions_to_copy * sizeof(VkExtensionProperties));
    *property_count = num_extensions_to_copy;

    if (num_extensions_to_copy < extensions.size()) {
      return VK_INCOMPLETE;
    }
    return VK_SUCCESS;
  }

  [[nodiscard]] const DispatchTable* dispatch_table() const { return &dispatch_table_; }

  [[nodiscard]] const SubmissionTracker* submission_tracker() const { return &submission_tracker_; }

  [[nodiscard]] const DeviceManager* device_manager() const { return &device_manager_; }

  [[nodiscard]] const TimerQueryPool* timer_query_pool() const { return &timer_query_pool_; }

  [[nodiscard]] const QueueManager* queue_manager() const { return &queue_manager_; }

  [[nodiscard]] const VulkanWrapper* vulkan_wrapper() const { return &vulkan_wrapper_; }

 private:
  void InitVulkanLayerProducerIfNecessary() {
    absl::MutexLock lock{&vulkan_layer_producer_mutex_};
    if (vulkan_layer_producer_ == nullptr) {
      vulkan_layer_producer_ = std::make_unique<VulkanLayerProducerImpl>();
      ORBIT_LOG("Bringing up VulkanLayerProducer");
      vulkan_layer_producer_->BringUp(orbit_producer_side_channel::CreateProducerSideChannel());
      submission_tracker_.SetVulkanLayerProducer(vulkan_layer_producer_.get());
    }
  }

  void DumpProcessIdIfNecessary() const {
    const char* pid_file = std::getenv("ORBIT_VULKAN_LAYER_PID_FILE");
    if (pid_file == nullptr) {
      return;
    }
    uint32_t pid = orbit_base::GetCurrentProcessId();
    ORBIT_LOG("Writing PID of %u to \"%s\"", pid, pid_file);
    ErrorMessageOr<orbit_base::unique_fd> error_or_file = orbit_base::OpenFileForWriting(pid_file);
    ORBIT_FAIL_IF(error_or_file.has_error(), "Opening \"%s\": %s", pid_file,
                  error_or_file.error().message());
    ErrorMessageOr<void> result =
        orbit_base::WriteFully(error_or_file.value(), absl::StrFormat("%d", pid));
    ORBIT_FAIL_IF(result.has_error(), "Writing PID to \"%s\": %s", pid_file,
                  result.error().message());
  }

  void CloseVulkanLayerProducerIfNecessary() {
    absl::MutexLock lock{&vulkan_layer_producer_mutex_};
    if (vulkan_layer_producer_ != nullptr) {
      // TODO: Only do this when DestroyInstance has been called the same number of times as
      //  CreateInstance.
      ORBIT_LOG("Taking down VulkanLayerProducer");
      vulkan_layer_producer_->TakeDown();
      submission_tracker_.SetVulkanLayerProducer(nullptr);
      vulkan_layer_producer_.reset();
    }
  }

  template <typename CreateInfoT>
  void AddRequiredExtensionNameIfMissing(
      const CreateInfoT* create_info,
      const std::function<VkResult(uint32_t*, VkExtensionProperties*)>&
          enumerate_extension_properties_function,
      const char* extension_name, std::vector<std::string>* output) {
    bool extension_already_enabled = false;

    for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i) {
      const char* enabled_extension = create_info->ppEnabledExtensionNames[i];
      if (strcmp(enabled_extension, extension_name) == 0) {
        extension_already_enabled = true;
        break;
      }
    }

    if (!extension_already_enabled) {
      bool extension_supported = false;
      uint32_t count = 0;
      VkResult result = enumerate_extension_properties_function(&count, nullptr);
      ORBIT_CHECK(result == VK_SUCCESS);

      std::vector<VkExtensionProperties> extension_properties(count);
      result = enumerate_extension_properties_function(&count, extension_properties.data());
      ORBIT_CHECK(result == VK_SUCCESS);

      for (const auto& properties : extension_properties) {
        if (strcmp(properties.extensionName, extension_name) == 0) {
          extension_supported = true;
          break;
        }
      }

      ORBIT_FAIL_IF(!extension_supported,
                    "Orbit's Vulkan layer requires the %s extension to be supported.",
                    extension_name);
      output->emplace_back(extension_name);
    }
  }

  void AddRequiredDeviceExtensionNameIfMissing(const VkDeviceCreateInfo* create_info,
                                               VkPhysicalDevice physical_device,
                                               const char* extension_name,
                                               std::vector<std::string>* output) {
    auto raw_enumerate_device_extension_properties_function =
        absl::bit_cast<PFN_vkEnumerateDeviceExtensionProperties>(
            dispatch_table_.EnumerateDeviceExtensionProperties(physical_device));
    auto enumerate_device_extension_properties =
        [&raw_enumerate_device_extension_properties_function, physical_device](
            uint32_t* count, VkExtensionProperties* properties) -> VkResult {
      return raw_enumerate_device_extension_properties_function(physical_device, nullptr, count,
                                                                properties);
    };
    AddRequiredExtensionNameIfMissing(create_info, enumerate_device_extension_properties,
                                      extension_name, output);
  }

  void AddRequiredInstanceExtensionNameIfMissing(const VkInstanceCreateInfo* create_info,

                                                 const char* extension_name,
                                                 std::vector<std::string>* output) {
    auto enumerate_instance_extension_properties =
        [this](uint32_t* count, VkExtensionProperties* properties) -> VkResult {
      return vulkan_wrapper_.CallVkEnumerateInstanceExtensionProperties(nullptr, count, properties);
    };
    AddRequiredExtensionNameIfMissing(create_info, enumerate_instance_extension_properties,
                                      extension_name, output);
  }

  std::unique_ptr<VulkanLayerProducer> vulkan_layer_producer_ = nullptr;
  absl::Mutex vulkan_layer_producer_mutex_;

  DispatchTable dispatch_table_;
  DeviceManager device_manager_;
  TimerQueryPool timer_query_pool_;
  SubmissionTracker submission_tracker_;
  QueueManager queue_manager_;
  VulkanWrapper vulkan_wrapper_;

  // The number of timer query slots is chosen arbitrary such that it is large enough.
  static constexpr uint32_t kNumTimerQuerySlots = 131072;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_VULKAN_LAYER_CONTROLLER_H_
