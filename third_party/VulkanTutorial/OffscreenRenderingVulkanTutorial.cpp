#include "VulkanTutorial/OffscreenRenderingVulkanTutorial.h"

#include <absl/base/casts.h>
#include <absl/synchronization/mutex.h>
#include <volk.h>

#include <cstdint>
#include <vector>

#include "OrbitBase/Logging.h"
#include "VulkanTutorialFragmentShader.h"
#include "VulkanTutorialVertexShader.h"

#define CHECK_VK_SUCCESS(call) ORBIT_CHECK((call) == VK_SUCCESS)

namespace orbit_vulkan_tutorial {

const std::vector<const char*> OffscreenRenderingVulkanTutorial::kValidationLayerNames = {
    "VK_LAYER_KHRONOS_validation"};

void OffscreenRenderingVulkanTutorial::Run(uint64_t frame_count) {
  InitVulkan();
  MainLoop(frame_count);
  CleanUp();
}

void OffscreenRenderingVulkanTutorial::StopAsync() {
  absl::MutexLock stop_requested_lock{&stop_requested_mutex_};
  stop_requested_ = true;
}

void OffscreenRenderingVulkanTutorial::InitVulkan() {
  ORBIT_LOG("InitVulkan");
  // To simplify our dependencies, we don't link to Vulkan, but we use volk instead:
  // https://github.com/zeux/volk
  CHECK_VK_SUCCESS(volkInitialize());
  CreateInstance();
  volkLoadInstance(instance_);
  // Note that, unlike what is described at
  // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Window_surface,
  // we don't need to create a surface, as we are performing offscreen rendering.
  PickPhysicalDevice();
  CreateLogicalDevice();
  // Note that, unlike what is described at
  // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain,
  // we don't need to create a swap chain, as we are performing offscreen rendering.
  // Create an image in memory instead.
  CreateOffscreenImage();
  CreateImageView();
  CreateRenderPass();
  CreateGraphicsPipeline();
  CreateFramebuffer();
  CreateCommandPool();
  CreateCommandBuffer();
  CreateFence();
}

void OffscreenRenderingVulkanTutorial::MainLoop(uint64_t frame_count) {
  ORBIT_LOG("MainLoop");
  for (uint64_t frame = 0; frame < frame_count; ++frame) {
    absl::Time next_frame_time = absl::Now() + absl::Microseconds(16667);
    DrawFrame();

    {
      absl::MutexLock stop_requested_lock{&stop_requested_mutex_};
      if (stop_requested_mutex_.AwaitWithDeadline(absl::Condition(&stop_requested_),
                                                  next_frame_time)) {
        break;
      }
    }
  }
  vkDeviceWaitIdle(device_);
}

void OffscreenRenderingVulkanTutorial::CleanUp() {
  ORBIT_LOG("CleanUp");
  vkDestroyFence(device_, fence_, nullptr);
  // Command buffers will be automatically freed when their command pool is destroyed, so we don't
  // need an explicit cleanup.
  vkDestroyCommandPool(device_, command_pool_, nullptr);
  vkDestroyFramebuffer(device_, framebuffer_, nullptr);
  vkDestroyPipeline(device_, graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyRenderPass(device_, render_pass_, nullptr);
  vkDestroyImageView(device_, image_view_, nullptr);
  vkFreeMemory(device_, memory_, nullptr);
  vkDestroyImage(device_, image_, nullptr);
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

void OffscreenRenderingVulkanTutorial::CreateInstance() {
  VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "VulkanTutorial",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_1,
  };

  ORBIT_CHECK(AreValidationLayersSupported());
  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = static_cast<uint32_t>(kValidationLayerNames.size()),
      .ppEnabledLayerNames = kValidationLayerNames.data(),
      // Note that, unlike what is described at
      // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance, we don't need to enable
      // extensions for the window system, as we are performing offscreen rendering.
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr,
  };

  CHECK_VK_SUCCESS(vkCreateInstance(&create_info, nullptr, &instance_));
}

bool OffscreenRenderingVulkanTutorial::AreValidationLayersSupported() {
  uint32_t layer_count = 0;
  CHECK_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));

  std::vector<VkLayerProperties> available_layers(layer_count);
  CHECK_VK_SUCCESS(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()));

  for (const char* layer_name : kValidationLayerNames) {
    bool layer_found = false;

    for (const VkLayerProperties& layer_properties : available_layers) {
      if (strcmp(layer_properties.layerName, layer_name) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      return false;
    }
  }
  return true;
}

void OffscreenRenderingVulkanTutorial::PickPhysicalDevice() {
  uint32_t physical_device_count = 0;
  CHECK_VK_SUCCESS(vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr));
  ORBIT_CHECK(physical_device_count > 0);

  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());

  for (const VkPhysicalDevice& physical_device : physical_devices) {
    if (IsPhysicalDeviceSuitable(physical_device)) {
      physical_device_ = physical_device;
      break;
    }
  }

  ORBIT_CHECK(physical_device_ != VK_NULL_HANDLE);
}

bool OffscreenRenderingVulkanTutorial::IsPhysicalDeviceSuitable(
    VkPhysicalDevice const& physical_device) {
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device);
  return queue_family_indices.graphics_family.has_value();
}

OffscreenRenderingVulkanTutorial::QueueFamilyIndices
OffscreenRenderingVulkanTutorial::FindQueueFamilies(VkPhysicalDevice physical_device) {
  QueueFamilyIndices queue_family_indices;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                           queue_families.data());

  int queue_family_index = 0;
  for (const VkQueueFamilyProperties& queue_family : queue_families) {
    if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
      queue_family_indices.graphics_family = queue_family_index;
    }
    queue_family_index++;
  }

  return queue_family_indices;
}

void OffscreenRenderingVulkanTutorial::CreateLogicalDevice() {
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device_);

  constexpr float kQueuePriority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queue_family_indices.graphics_family.value(),
      .queueCount = 1,
      .pQueuePriorities = &kQueuePriority,
  };

  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create_info,
      .enabledLayerCount = static_cast<uint32_t>(kValidationLayerNames.size()),
      .ppEnabledLayerNames = kValidationLayerNames.data(),
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr,
      .pEnabledFeatures = &device_features,
  };

  CHECK_VK_SUCCESS(vkCreateDevice(physical_device_, &create_info, nullptr, &device_));

  vkGetDeviceQueue(device_, queue_family_indices.graphics_family.value(), 0, &graphics_queue_);
}

void OffscreenRenderingVulkanTutorial::CreateOffscreenImage() {
  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = kImageFormat,
      .extent = {.width = kImageWidth, .height = kImageHeight, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  image_extent_ = create_info.extent;

  CHECK_VK_SUCCESS(vkCreateImage(device_, &create_info, nullptr, &image_));

  // Good reference for the code that follows:
  // https://www.informit.com/articles/article.aspx?p=2756465&seqNum=3

  VkMemoryRequirements memory_requirements{};
  vkGetImageMemoryRequirements(device_, image_, &memory_requirements);
  ORBIT_CHECK(memory_requirements.memoryTypeBits != 0);

  // "memoryTypeBits is a bitmask and contains one bit set for every supported memory type for the
  // resource. Bit i is set if and only if the memory type i in the
  // VkPhysicalDeviceMemoryProperties structure for the physical device is supported for the
  // resource."
  // Since we don't have any requirement on the *properties* of the memory, simply choose the
  // lowest-indexed memory type.
  uint32_t memory_type_index = __builtin_ctz(memory_requirements.memoryTypeBits);
  ORBIT_LOG("memory_type_index=%d", memory_type_index);

  VkMemoryAllocateInfo allocate_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index,
  };
  CHECK_VK_SUCCESS(vkAllocateMemory(device_, &allocate_info, nullptr, &memory_));

  CHECK_VK_SUCCESS(vkBindImageMemory(device_, image_, memory_, 0));
}

void OffscreenRenderingVulkanTutorial::CreateImageView() {
  VkImageViewCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image_,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = kImageFormat,
      .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                     .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                     .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                     .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1},
  };
  CHECK_VK_SUCCESS(vkCreateImageView(device_, &create_info, nullptr, &image_view_));
}

void OffscreenRenderingVulkanTutorial::CreateRenderPass() {
  VkAttachmentDescription color_attachment{
      .format = kImageFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkAttachmentReference color_attachment_ref{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subpass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr,
  };

  // Note that, unlike what is described at
  // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation,
  // our simplified version doesn't require a subpass dependency.

  VkRenderPassCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &color_attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 0,
      .pDependencies = nullptr,
  };
  CHECK_VK_SUCCESS(vkCreateRenderPass(device_, &create_info, nullptr, &render_pass_));
}

VkShaderModule OffscreenRenderingVulkanTutorial::CreateShaderModule(const uint8_t* shader_code,
                                                                    size_t shader_code_size) {
  VkShaderModuleCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = shader_code_size,
      .pCode = absl::bit_cast<const uint32_t*>(shader_code),
  };

  VkShaderModule shader_module{};
  CHECK_VK_SUCCESS(vkCreateShaderModule(device_, &create_info, nullptr, &shader_module));
  return shader_module;
}

void OffscreenRenderingVulkanTutorial::CreateGraphicsPipeline() {
  VkShaderModule vertex_shader_module =
      CreateShaderModule(kVulkanTutorialVertexShaderSpv, kVulkanTutorialVertexShaderSpvLength);
  VkShaderModule fragment_shader_module =
      CreateShaderModule(kVulkanTutorialFragmentShaderSpv, kVulkanTutorialFragmentShaderSpvLength);

  VkPipelineShaderStageCreateInfo vertex_shader_stage_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertex_shader_module,
      .pName = "main",
      .pSpecializationInfo = nullptr,
  };

  VkPipelineShaderStageCreateInfo fragment_shader_stage_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragment_shader_module,
      .pName = "main",
      .pSpecializationInfo = nullptr,
  };

  std::array shader_stages = {vertex_shader_stage_info, fragment_shader_stage_info};

  VkPipelineVertexInputStateCreateInfo vertex_input_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .pVertexBindingDescriptions = nullptr,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions = nullptr,
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = kImageWidth,
      .height = kImageHeight,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  VkRect2D scissor{
      .offset = {0, 0},
      .extent = {image_extent_.width, image_extent_.height},
  };

  VkPipelineViewportStateCreateInfo viewport_state_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizer_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampling_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo color_blending_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
  };

  VkPipelineLayoutCreateInfo pipeline_layout_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };
  CHECK_VK_SUCCESS(
      vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &pipeline_layout_));

  VkGraphicsPipelineCreateInfo pipeline_create_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = shader_stages.size(),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_info,
      .pInputAssemblyState = &input_assembly_info,
      .pTessellationState = nullptr,
      .pViewportState = &viewport_state_info,
      .pRasterizationState = &rasterizer_info,
      .pMultisampleState = &multisampling_info,
      .pDepthStencilState = nullptr,
      .pColorBlendState = &color_blending_info,
      .pDynamicState = nullptr,
      .layout = pipeline_layout_,
      .renderPass = render_pass_,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };
  CHECK_VK_SUCCESS(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_create_info,
                                             nullptr, &graphics_pipeline_));

  vkDestroyShaderModule(device_, fragment_shader_module, nullptr);
  vkDestroyShaderModule(device_, vertex_shader_module, nullptr);
}

void OffscreenRenderingVulkanTutorial::CreateFramebuffer() {
  VkFramebufferCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = render_pass_,
      .attachmentCount = 1,
      .pAttachments = &image_view_,
      .width = image_extent_.width,
      .height = image_extent_.height,
      .layers = 1,
  };
  CHECK_VK_SUCCESS(vkCreateFramebuffer(device_, &create_info, nullptr, &framebuffer_));
}

void OffscreenRenderingVulkanTutorial::CreateCommandPool() {
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device_);

  VkCommandPoolCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = queue_family_indices.graphics_family.value(),
  };
  CHECK_VK_SUCCESS(vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_));
}

void OffscreenRenderingVulkanTutorial::CreateCommandBuffer() {
  VkCommandBufferAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  CHECK_VK_SUCCESS(vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer_));

  VkCommandBufferBeginInfo begin_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pInheritanceInfo = nullptr,
  };
  CHECK_VK_SUCCESS(vkBeginCommandBuffer(command_buffer_, &begin_info));

  VkClearValue clear_value{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}};

  VkRenderPassBeginInfo render_pass_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = render_pass_,
      .framebuffer = framebuffer_,
      .renderArea = {.offset = {0, 0},
                     .extent = {.width = image_extent_.width, .height = image_extent_.height}},
      .clearValueCount = 1,
      .pClearValues = &clear_value,
  };

  vkCmdBeginRenderPass(command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

  vkCmdDraw(command_buffer_, 3, 1, 0, 0);

  vkCmdEndRenderPass(command_buffer_);

  CHECK_VK_SUCCESS(vkEndCommandBuffer(command_buffer_));
}

void OffscreenRenderingVulkanTutorial::CreateFence() {
  VkFenceCreateInfo fence_create_info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  CHECK_VK_SUCCESS(vkCreateFence(device_, &fence_create_info, nullptr, &fence_));
}

void OffscreenRenderingVulkanTutorial::DrawFrame() {
  vkWaitForFences(device_, 1, &fence_, VK_TRUE, UINT64_MAX);
  vkResetFences(device_, 1, &fence_);

  VkSubmitInfo submit_info{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .pWaitDstStageMask = nullptr,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer_,
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = nullptr,
  };
  CHECK_VK_SUCCESS(vkQueueSubmit(graphics_queue_, 1, &submit_info, fence_));
}

}  // namespace orbit_vulkan_tutorial
