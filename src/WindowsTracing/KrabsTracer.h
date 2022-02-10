// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_KRABS_TRACER_H_
#define WINDOWS_TRACING_KRABS_TRACER_H_

#include <krabs/krabs.hpp>
#include <memory>
#include <thread>

#include "ContextSwitchManager.h"
#include "OrbitBase/ThreadConstants.h"
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
  void SetIsSystemProfilePrivilegeEnabled(bool value);
  void SetupStackTracing();
  void Run();
  void OnThreadEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnStackWalkEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OutputStats();
  static void OutputLogFileInfo(const EVENT_TRACE_LOGFILE& log_file);

  struct Stats {
    uint64_t num_thread_events = 0;
    uint64_t num_stack_events = 0;
    uint64_t num_stack_events_for_target_pid = 0;
  };

 private:
  orbit_grpc_protos::CaptureOptions capture_options_;
  TracerListener* listener_ = nullptr;
  uint32_t target_pid_ = orbit_base::kInvalidProcessId;

  std::unique_ptr<ContextSwitchManager> context_switch_manager_;
  std::unique_ptr<std::thread> trace_thread_;
  Stats stats_;

  krabs::kernel_trace trace_;
  krabs::kernel::thread_provider thread_provider_;
  krabs::kernel::context_switch_provider context_switch_provider_;
  krabs::kernel_provider stack_walk_provider_;
  EVENT_TRACE_LOGFILE log_file_ = {0};
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_KRABS_TRACER_H_
