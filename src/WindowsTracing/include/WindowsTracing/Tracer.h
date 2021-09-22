// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_H_
#define WINDOWS_TRACING_TRACER_H_

#include <memory>

#include "WindowsTracing/TracerInterface.h"
#include "WindowsTracing/TracerListener.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

// Forwards interesting Windows kernel events to a TraceListener.
class Tracer : public TracerInterface {
 public:
  explicit Tracer(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener);
  virtual ~Tracer() = default;

  void Start() override;
  void Stop() override;

 private:
  std::unique_ptr<TracerInterface> tracer_impl_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_H_
