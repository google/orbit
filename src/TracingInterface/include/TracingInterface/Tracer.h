// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TRACING_INTERFACE_TRACER_H_
#define TRACING_INTERFACE_TRACER_H_

#include "TracerListener.h"
#include "capture.pb.h"

namespace orbit_tracing_interface {

// Interface for starting/stopping a trace and relaying information to the provided TracerListener.
// Platform specific implementations exist in "<Platform>Tracing" projects.

class Tracer {
 public:
  explicit Tracer(orbit_grpc_protos::CaptureOptions capture_options)
      : capture_options_{std::move(capture_options)} {}

  virtual ~Tracer() = default;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  virtual void Start() = 0;
  virtual void Stop() = 0;

 protected:
  orbit_grpc_protos::CaptureOptions capture_options_;

  TracerListener* listener_ = nullptr;
};

}  // namespace orbit_tracing_interface

#endif  // TRACING_INTERFACE_TRACER_H_
