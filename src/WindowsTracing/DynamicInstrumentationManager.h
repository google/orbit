// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_DYNAMIC_INSTRUMENTATION_MANAGER_H_
#define WINDOWS_TRACING_DYNAMIC_INSTRUMENTATION_MANAGER_H_

#include <OrbitLib/OrbitLib.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "WindowsTracing/TracerListener.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

// Controls a dynamic instrumentation session and relays FunctionCall objects to a listener.
class DynamicInstrumentationManager {
 public:
  DynamicInstrumentationManager() = default;

  void Start(const orbit_grpc_protos::CaptureOptions& capture_options, TracerListener* listener);
  void Stop();

 private:
  std::unique_ptr<orbit_lib::CaptureListener> capture_listener_ = nullptr;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_DYNAMIC_INSTRUMENTATION_MANAGER_H_
