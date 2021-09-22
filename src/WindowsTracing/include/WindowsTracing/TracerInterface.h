// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_INTERFACE_H_
#define WINDOWS_TRACING_TRACER_INTERFACE_H_

#include "WindowsTracing/TracerListener.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

class TracerInterface {
 public:
  TracerInterface(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener)
      : capture_options_(capture_options), listener_(listener) {}
  virtual void Start() = 0;
  virtual void Stop() = 0;

 protected:
  orbit_grpc_protos::CaptureOptions capture_options_;
  TracerListener* listener_ = nullptr;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_INTERFACE_H_
