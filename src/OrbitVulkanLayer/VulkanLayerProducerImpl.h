// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_IMPL_H_
#define ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_IMPL_H_

#include "OrbitProducer/LockFreeBufferCaptureEventProducer.h"
#include "VulkanLayerProducer.h"
#include "absl/container/flat_hash_set.h"

namespace orbit_vulkan_layer {

// This class provides the implementation of VulkanLayerProducer,
// delegating most methods to LockFreeBufferCaptureEventProducer
// while also handling interning of strings.
class VulkanLayerProducerImpl : public VulkanLayerProducer {
 public:
  void BringUp(const std::shared_ptr<grpc::Channel>& channel) override {
    return lock_free_producer_.BuildAndStart(channel);
  }

  void TakeDown() override { lock_free_producer_.ShutdownAndWait(); }

  [[nodiscard]] bool IsCapturing() override { return lock_free_producer_.IsCapturing(); }

  bool EnqueueCaptureEvent(orbit_grpc_protos::CaptureEvent&& capture_event) override {
    return lock_free_producer_.EnqueueIntermediateEventIfCapturing(
        [&capture_event] { return capture_event; });
  }

  [[nodiscard]] uint64_t InternStringIfNecessaryAndGetKey(std::string str) override;

  void SetCaptureStatusListener(CaptureStatusListener* listener) override { listener_ = listener; }

 private:
  class LockFreeBufferVulkanLayerProducer
      : public orbit_producer::LockFreeBufferCaptureEventProducer<orbit_grpc_protos::CaptureEvent> {
   public:
    explicit LockFreeBufferVulkanLayerProducer(VulkanLayerProducerImpl* outer) : outer_{outer} {}

   protected:
    void OnCaptureStart(orbit_grpc_protos::CaptureOptions capture_options) override {
      LockFreeBufferCaptureEventProducer::OnCaptureStart(capture_options);
      if (outer_->listener_ != nullptr) {
        outer_->listener_->OnCaptureStart(std::move(capture_options));
      }
    }

    void OnCaptureStop() override {
      LockFreeBufferCaptureEventProducer::OnCaptureStop();
      if (outer_->listener_ != nullptr) {
        outer_->listener_->OnCaptureStop();
      }
    }

    void OnCaptureFinished() override {
      LockFreeBufferCaptureEventProducer::OnCaptureFinished();
      if (outer_->listener_ != nullptr) {
        outer_->listener_->OnCaptureFinished();
      }

      outer_->ClearStringInternPool();
    }

    orbit_grpc_protos::CaptureEvent TranslateIntermediateEvent(
        orbit_grpc_protos::CaptureEvent&& intermediate_event) override {
      return std::move(intermediate_event);
    }

   private:
    VulkanLayerProducerImpl* outer_;
  };

 private:
  static uint64_t ComputeStringKey(const std::string& str) { return std::hash<std::string>{}(str); }

  void ClearStringInternPool() {
    absl::MutexLock lock{&string_keys_sent_mutex_};
    string_keys_sent_.clear();
  }

 private:
  LockFreeBufferVulkanLayerProducer lock_free_producer_{this};

  absl::flat_hash_set<uint64_t> string_keys_sent_;
  absl::Mutex string_keys_sent_mutex_;

  CaptureStatusListener* listener_ = nullptr;
};

}  // namespace orbit_vulkan_layer

#endif  // ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_IMPL_H_
