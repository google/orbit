// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_H_
#define WINDOWS_TRACING_TRACER_H_

#include <memory>

#include "GrpcProtos/capture.pb.h"
#include "WindowsTracing/TracerListener.h"

namespace orbit_windows_tracing {

// Interface for a tracer that forwards interesting Windows kernel events to a TracerListener.
class Tracer {
 public:
  virtual void Start() = 0;
  virtual void Stop() = 0;

  [[nodiscard]] static std::unique_ptr<Tracer> Create(
      orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener);
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_H_
