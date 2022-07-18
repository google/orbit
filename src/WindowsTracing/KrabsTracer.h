// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_KRABS_TRACER_H_
#define WINDOWS_TRACING_KRABS_TRACER_H_

#include <absl/synchronization/mutex.h>

#include <krabs/krabs.hpp>
#include <memory>
#include <thread>

#include "ContextSwitchManager.h"
#include "GraphicsEtwProvider.h"
#include "OrbitBase/ThreadConstants.h"
#include "WindowsTracing/TracerListener.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/PathConverter.h"

namespace orbit_windows_tracing {

// KrabsTracer uses Microsoft's krabsetw, a wrapper around the Event Tracing for Windows API (ETW),
// to provide kernel event tracing on Windows. Traced events include scheduling information and
// stack traces.
class KrabsTracer {
 public:
  enum ProviderFlags {
    kThread = 1 << 0,
    kContextSwitch = 1 << 2,
    kStackWalk = 1 << 3,
    kImageLoad = 1 << 4,
    kGraphics = 1 << 5,
    kAll = kThread | kContextSwitch | kStackWalk | kImageLoad | kGraphics
  };

  KrabsTracer(uint32_t pid, double sampling_frequency_hz, TracerListener* listener);
  KrabsTracer(uint32_t pid, double sampling_frequency_hz, TracerListener* listener,
              ProviderFlags providers);

  KrabsTracer() = delete;
  void Start();
  void Stop();

  [[nodiscard]] bool IsProviderEnabled(ProviderFlags provider) const;
  [[nodiscard]] std::vector<orbit_windows_utils::Module> GetLoadedModules() const;

 private:
  void SetTraceProperties();
  void EnableProviders();
  void SetIsSystemProfilePrivilegeEnabled(bool value);
  void SetupStackTracing();
  void KernelTraceThread();
  void UserTraceThread();
  void StopKernelTrace();
  void StopUserTrace();
  void OnThreadEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnStackWalkEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnImageLoadEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OutputStats();

  struct Stats {
    uint64_t num_thread_events = 0;
    uint64_t num_stack_events = 0;
    uint64_t num_stack_events_for_target_pid = 0;
    uint64_t num_image_load_events_for_target_pid = 0;
  };

 private:
  uint32_t target_pid_ = orbit_base::kInvalidProcessId;
  double sampling_frequency_hz_ = 0;
  TracerListener* listener_ = nullptr;
  ProviderFlags providers_ = ProviderFlags::kAll;

  std::unique_ptr<ContextSwitchManager> context_switch_manager_;
  std::unique_ptr<std::thread> kernel_trace_thread_;
  std::unique_ptr<std::thread> user_trace_thread_;
  Stats stats_;

  krabs::user_trace user_trace_;
  krabs::kernel_trace kernel_trace_;
  krabs::kernel::thread_provider thread_provider_;
  krabs::kernel::context_switch_provider context_switch_provider_;
  krabs::kernel_provider stack_walk_provider_;
  krabs::kernel::image_load_provider image_load_provider_;
  std::unique_ptr<GraphicsEtwProvider> graphics_etw_provider_;
  EVENT_TRACE_LOGFILE log_file_ = {0};

  mutable absl::Mutex modules_mutex_;
  std::vector<orbit_windows_utils::Module> modules_ ABSL_GUARDED_BY(modules_mutex_);
  std::unique_ptr<orbit_windows_utils::PathConverter> path_converter_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_KRABS_TRACER_H_
