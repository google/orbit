// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_IMPL_H_
#define WINDOWS_TRACING_TRACER_IMPL_H_

#include "DynamicInstrumentationManager.h"
#include "KrabsTracer.h"
#include "WindowsTracing/Tracer.h"
#include "WindowsTracing/TracerListener.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

// Tracer implementation that creates a new KrabsTracer on Start(), and releases it on Stop().
class TracerImpl : public Tracer {
 public:
  TracerImpl(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener);
  TracerImpl() = delete;
  virtual ~TracerImpl() {}

  virtual void Start();
  virtual void Stop();

 private:
  void SendThreadSnapshot() const;
  void SendModuleSnapshot() const;

  orbit_grpc_protos::CaptureOptions capture_options_;
  TracerListener* listener_ = nullptr;
  std::unique_ptr<KrabsTracer> krabs_tracer_;
  std::unique_ptr<DynamicInstrumentationManager> dynamic_instrumentation_manager_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_IMPL_H_
