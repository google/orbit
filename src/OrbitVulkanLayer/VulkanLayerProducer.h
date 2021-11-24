// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_H_
#define ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_H_

#include <grpcpp/grpcpp.h>

#include "GrpcProtos/capture.pb.h"

namespace orbit_vulkan_layer {

// This interface exposes methods for the communication between the Vulkan layer and Orbit,
// while also allowing to be mocked for testing.
// In particular, it provides such methods to LayerLogic and CommandBufferManager.
class VulkanLayerProducer {
 public:
  virtual ~VulkanLayerProducer() = default;

  // This method tries to establish a gRPC connection with OrbitService over the specified gRPC
  // channel and gets the class ready to send CaptureEvents.
  virtual void BringUp(const std::shared_ptr<grpc::Channel>& channel) = 0;

  // This method causes the class to stop sending any remaining queued CaptureEvent
  // and closes the connection with OrbitService.
  virtual void TakeDown() = 0;

  // Use this method to query whether Orbit is currently capturing.
  [[nodiscard]] virtual bool IsCapturing() = 0;

  // Use this method to enqueue a CaptureEvent to be sent to OrbitService.
  // Returns true if the event was enqueued as the capture is in progress, false otherwise.
  // Callers can use the return value to check if the event was actually enqueued as the capture
  // is in progress.
  virtual bool EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent&& capture_event) = 0;

  // This method enqueues an InternedString to be sent to OrbitService the first time the string
  // passed as argument is seen. In all cases, it returns the key corresponding to the string.
  [[nodiscard]] virtual uint64_t InternStringIfNecessaryAndGetKey(std::string str) = 0;

  class CaptureStatusListener {
   public:
    virtual ~CaptureStatusListener() = default;
    virtual void OnCaptureStart(orbit_grpc_protos::CaptureOptions capture_options) = 0;
    virtual void OnCaptureStop() = 0;
    virtual void OnCaptureFinished() = 0;
  };

  // Use this method to set a listener and be notified on capture start, stopped, completed.
  virtual void SetCaptureStatusListener(CaptureStatusListener* listener) = 0;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_H_
