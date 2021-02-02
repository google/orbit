// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "VulkanLayerProducerImpl.h"

namespace orbit_vulkan_layer {

uint64_t VulkanLayerProducerImpl::InternStringIfNecessaryAndGetKey(std::string str) {
  uint64_t key = ComputeStringKey(str);
  {
    absl::MutexLock lock{&string_keys_sent_mutex_};
    bool inserted = string_keys_sent_.emplace(key).second;
    if (!inserted) {
      return key;
    }

    orbit_grpc_protos::ProducerCaptureEvent event;
    event.mutable_interned_string()->set_key(key);
    event.mutable_interned_string()->set_intern(std::move(str));
    if (!EnqueueCaptureEvent(std::move(event))) {
      // If the interned string wasn't actually sent because we are no longer capturing,
      // remove it from string_keys_sent_.
      string_keys_sent_.erase(key);
    }
    return key;
  }
}

}  // namespace orbit_vulkan_layer
