// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_IMPL_H_
#define ORBIT_VULKAN_LAYER_VULKAN_LAYER_PRODUCER_IMPL_H_

#include <google/protobuf/arena.h>

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

  bool EnqueueCaptureEvent(orbit_grpc_protos::ProducerCaptureEvent&& capture_event) override {
    return lock_free_producer_.EnqueueIntermediateEventIfCapturing(
        [&capture_event] { return capture_event; });
  }

  [[nodiscard]] uint64_t InternStringIfNecessaryAndGetKey(std::string str) override;

  void SetCaptureStatusListener(CaptureStatusListener* listener) override { listener_ = listener; }

 private:
  class LockFreeBufferVulkanLayerProducer
      : public orbit_producer::LockFreeBufferCaptureEventProducer<
            orbit_grpc_protos::ProducerCaptureEvent> {
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

    std::vector<orbit_grpc_protos::ProducerCaptureEvent*> TranslateIntermediateEvents(
        orbit_grpc_protos::ProducerCaptureEvent* moveable_intermediate_events, size_t num_events,
        google::protobuf::Arena* arena) override {
      std::vector<orbit_grpc_protos::ProducerCaptureEvent*> translated_events(num_events);
      for (size_t i = 0; i < num_events; ++i) {
        auto* capture_event =
            google::protobuf::Arena::CreateMessage<orbit_grpc_protos::ProducerCaptureEvent>(arena);
        // Note that, as capture_event is in the Arena and intermediate_event is on the heap, this
        // std::move will actually end up being a copy, as it will use CopyFrom internally.
        // For the amount of events that this Vulkan layer produces, this is fine performance-wise.
        // This is also in line with the principle of this method, which in general expects a
        // transformation from any type that intermediate_event could be to the ProducerCaptureEvent
        // protobuf.
        *capture_event = std::move(moveable_intermediate_events[i]);
        translated_events.push_back(capture_event);
      }
      return translated_events;
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
