#ifndef VULKAN_TUTORIAL_OFFSCREEN_RENDERING_VULKAN_TUTORIAL_H_
#define VULKAN_TUTORIAL_OFFSCREEN_RENDERING_VULKAN_TUTORIAL_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <volk.h>

#include <cstdint>
#include <optional>
#include <thread>
#include <vector>

namespace orbit_vulkan_tutorial {

// This class is a simplified version of the "triangle example" from the Vulkan Tutorial
// (https://vulkan-tutorial.com/Drawing_a_triangle).
// By rendering to a `VkImage` in memory instead of rendering to a surface, this saves us from
// having to deal with the swap chain and from having to create a window.
// Note, though, that this means that `vkQueuePresentKHR` is never called.
class OffscreenRenderingVulkanTutorial {
 public:
  // This method runs the entire example from setup to cleanup, rendering `frame_count` frames
  // offscreen in the main loop. If no parameter is passed, the main loop continues indefinitely
  // until `StopAsync` is called (from a different thread).
  void Run(uint64_t frame_count = std::numeric_limits<uint64_t>::max());

  // Call this method from a different thread than `Run` to stop the main rendering loop when this
  // is running indefinitely or before `frame_count` frames have been rendered.
  void StopAsync();

 private:
  void InitVulkan();
  void MainLoop(uint64_t frame_count);
  void CleanUp();

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
  };

  void CreateInstance();
  static bool AreValidationLayersSupported();
  void PickPhysicalDevice();
  static bool IsPhysicalDeviceSuitable(const VkPhysicalDevice& physical_device);
  static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device);
  void CreateLogicalDevice();
  void CreateOffscreenImage();
  void CreateImageView();
  void CreateRenderPass();
  VkShaderModule CreateShaderModule(const uint8_t* shader_code, size_t shader_code_size);
  void CreateGraphicsPipeline();
  void CreateFramebuffer();
  void CreateCommandPool();
  void CreateCommandBuffer();
  void CreateFence();
  void DrawFrame();

  static const std::vector<const char*> kValidationLayerNames;
  static constexpr uint32_t kImageWidth = 800;
  static constexpr uint32_t kImageHeight = 600;
  static constexpr VkFormat kImageFormat = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue graphics_queue_ = VK_NULL_HANDLE;
  VkExtent3D image_extent_{};
  VkImage image_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkImageView image_view_ = VK_NULL_HANDLE;
  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;
  VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
  VkFence fence_ = VK_NULL_HANDLE;

  absl::Mutex stop_requested_mutex_;
  bool stop_requested_ ABSL_GUARDED_BY(stop_requested_mutex_) = false;
};

}  // namespace orbit_vulkan_tutorial

#endif  // VULKAN_TUTORIAL_OFFSCREEN_RENDERING_VULKAN_TUTORIAL_H_
