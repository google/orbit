// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRIGGER_CAPTURE_VULKAN_LAYER_LAYER_OPTIONS_H_
#define ORBIT_TRIGGER_CAPTURE_VULKAN_LAYER_LAYER_OPTIONS_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "OrbitTriggerCaptureVulkanLayer/layer_config.pb.h"

// Reads the config file into a proto to be used in the layer.
class LayerOptions {
 public:
  void Init();
  double GetFrameTimeThresholdMilliseconds();
  uint32_t GetCaptureLengthSeconds();
  std::vector<std::string> BuildOrbitCaptureServiceArgv(std::string_view);

 private:
  orbit_vulkan_capture_protos::LayerConfig layer_config_;
};

#endif  // ORBIT_TRIGGER_CAPTURE_VULKAN_LAYER_LAYER_OPTIONS_H_
