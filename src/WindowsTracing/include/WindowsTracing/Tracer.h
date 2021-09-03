// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_H_
#define WINDOWS_TRACING_TRACER_H_

#include <memory>

#include "WindowsTracing/TracerListener.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

class Tracer {
 public:
  explicit Tracer(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener);
  virtual ~Tracer() = default;

  virtual void Start();
  virtual void Stop();

 protected:
  orbit_grpc_protos::CaptureOptions capture_options_;
  TracerListener* listener_ = nullptr;
  std::unique_ptr<Tracer> tracer_impl_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_H_
