// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_DATA_H_
#define ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_DATA_H_

#include <string>

#include "layer_config.pb.h"

// Reads the config file into a proto to be used in the layer.
class LayerData {
 public:
  void Init();
  double GetFrameTimeThresholdMilliseconds();
  uint32_t GetCaptureLengthSeconds();
  std::vector<char*> BuildOrbitCaptureServiceArgv(const std::string&);

 private:
  orbit_vulkan_capture_protos::LayerConfig layer_config_;
  std::string builder_functions_str_;
  std::string builder_sampling_rate_str_;
};

#endif  // ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_DATA_H_
