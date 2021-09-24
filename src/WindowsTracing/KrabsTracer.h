// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_KRABS_TRACER_H_
#define WINDOWS_TRACING_KRABS_TRACER_H_

#include <krabs/krabs.hpp>
#include <memory>
#include <thread>

#include "ContextSwitchManager.h"
#include "WindowsTracing/TracerListener.h"

namespace orbit_windows_tracing {

// KrabsTracer uses Microsoft's krabsetw, a wrapper around the Event Tracing for Windows API (ETW),
// to provide kernel event tracing on Windows. Traced events include scheduling information and
// stack traces.
class KrabsTracer {
 public:
  KrabsTracer(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener);
  KrabsTracer() = delete;
  void Start();
  void Stop();

  krabs::kernel_trace& GetTrace() { return trace_; }

 private:
  void SetTraceProperties();
  void EnableProviders();
  void Run();
  void OnThreadEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OutputStats();

 private:
  orbit_grpc_protos::CaptureOptions capture_options_;
  TracerListener* listener_ = nullptr;

  std::unique_ptr<ContextSwitchManager> context_switch_manager_;
  std::unique_ptr<std::thread> trace_thread_;

  krabs::kernel_trace trace_;
  krabs::kernel::thread_provider thread_provider_;
  krabs::kernel::context_switch_provider context_switch_provider_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_KRABS_TRACER_H_
