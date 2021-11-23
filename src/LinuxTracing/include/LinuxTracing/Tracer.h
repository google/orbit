// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_TRACER_H_
#define LINUX_TRACING_TRACER_H_

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"

namespace orbit_linux_tracing {

class Tracer {
 public:
  virtual void Start() = 0;
  virtual void Stop() = 0;

  // The `FunctionEntry` and `FunctionExit` events are received from user space instrumentation and
  // piped back into `LinuxTracing` using these two methods. This way they can be processed together
  // with stack samples, allowing us to correctly unwind the samples that fall inside one or more
  // dynamically instrumented functions, just like we do with u(ret)probes.
  virtual void ProcessFunctionEntry(const orbit_grpc_protos::FunctionEntry& function_entry) = 0;
  virtual void ProcessFunctionExit(const orbit_grpc_protos::FunctionExit& function_exit) = 0;

  virtual ~Tracer() = default;

  [[nodiscard]] static std::unique_ptr<Tracer> Create(
      const orbit_grpc_protos::CaptureOptions& capture_options,
      std::unique_ptr<UserSpaceInstrumentationAddresses> user_space_instrumentation_addresses,
      TracerListener* listener);
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_TRACER_H_
