// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_TRACER_H_
#define LINUX_TRACING_TRACER_H_

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

#include "LinuxTracing/TracerListener.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {

class Tracer {
 public:
  virtual void Start() = 0;
  virtual void Stop() = 0;

  virtual ~Tracer() = default;

  [[nodiscard]] static std::unique_ptr<Tracer> Create(
      const orbit_grpc_protos::CaptureOptions& capture_options, TracerListener* listener);
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_TRACER_H_
