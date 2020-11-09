// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_LOGIC_H_
#define ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_LOGIC_H_

#include <string.h>

#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"

// Contains the logic of the OrbitVulkanLayerClientGgp, which keeps track of the calculates the FPS
// and run Orbit captures automatically when they drop below a certain threshold. It also
// instantiates the classes and variables needed for this so the layer itself is transparent to it.
class LayerLogic {
 public:
  void InitLayerData();
  void CleanLayerData();
  void ProcessQueuePresentKHR();

 private:
  bool data_initialised_ = false;
  std::unique_ptr<CaptureClientGgpClient> ggp_capture_client_;

  void StartOrbitCaptureService();
};

#endif  // ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_LOGIC_H_
