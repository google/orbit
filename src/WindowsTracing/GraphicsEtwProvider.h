// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_GRAPHICS_ETW_PROVIDER_H_
#define WINDOWS_TRACING_GRAPHICS_ETW_PROVIDER_H_

#include <absl/container/flat_hash_map.h>

#include <krabs/krabs.hpp>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "WindowsTracing/TracerListener.h"

namespace orbit_windows_tracing {

// This class sets up a krabs::user_trace so that it receives graphics related ETW events. Those
// events are converted into PresentEvent objects that are relayed to a TraceListener.
class GraphicsEtwProvider {
 public:
  GraphicsEtwProvider(uint32_t pid, krabs::user_trace* user_trace, TracerListener* listener);

  void OutputStats();

 private:
  void EnableProvider(std::string_view name, GUID guid, krabs::provider_event_callback callback);

  void OnDXGIEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnD3d9Event(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnDwmCoreEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnDwmCoreWin7Event(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnDxgKrnlEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnDxgKrnlWin7PresEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnNtProcessEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnWindowsEventMetadata(const EVENT_RECORD& record, const krabs::trace_context& context);
  void OnWin32KEvent(const EVENT_RECORD& record, const krabs::trace_context& context);

  void OnPresentStart(orbit_grpc_protos::PresentEvent::Source present_source,
                      uint32_t present_flags, const EVENT_HEADER& header);

  // Wrapper around a krabs::provider that provides event filtering and maintains stats.
  class Provider {
   public:
    Provider(std::string_view name, GUID guid, uint32_t target_pid, krabs::user_trace* trace,
             krabs::provider_event_callback callback);
    void OnEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
    void Log();

   private:
    std::string name_;
    krabs::provider<> krabs_provider_;
    uint32_t target_pid_ = orbit_base::kInvalidProcessId;
    krabs::provider_event_callback callback_ = nullptr;
    uint64_t num_events_received_ = 0;
    uint64_t num_events_processed_ = 0;
  };

  uint32_t target_pid_ = orbit_base::kInvalidProcessId;
  krabs::user_trace* trace_ = nullptr;
  TracerListener* listener_ = nullptr;
  absl::flat_hash_map<std::string_view, std::unique_ptr<Provider>> name_to_provider_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_GRAPHICS_ETW_PROVIDER_H_
