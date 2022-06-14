// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_GRAPHICS_ETW_PROVIDER_H_
#define WINDOWS_TRACING_GRAPHICS_ETW_PROVIDER_H_

#include <krabs/krabs.hpp>

#include "OrbitBase/ThreadConstants.h"
#include "WindowsTracing/TracerListener.h"

namespace orbit_windows_tracing {

class GraphicsEtwProvider {
 public:
  GraphicsEtwProvider(uint32_t pid, krabs::user_trace* trace, TracerListener* listener);

  void Start();
  void Stop();

  void OutputStats();

  enum class PresentSource { kUnknown, kDxgi, kD3d9 };

 private:
  // Microsoft_Windows_DXGI::GUID
  void OnDXGIEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_D3D9::GUID
  void OnD3D9Event(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_Dwm_Core::GUID
  void OnDwmCoreEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_Dwm_Core::Win7::GUID
  void OnDwmCoreWin7Event(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_DxgKrnl::GUID
  void OnDxgKrnlEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_DxgKrnl::Win7::PRESENTHISTORY_GUID
  void OnDxgKrnlWin7PresEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  // NT_Process::GUID
  void OnNtProcessEvent(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_EventMetadata::GUID
  void OnWindowsEventMetadata(const EVENT_RECORD& record, const krabs::trace_context& context);
  // Microsoft_Windows_Win32k::GUID
  void OnWin2kEvent(const EVENT_RECORD& record, const krabs::trace_context& context);

  void OnPresentStart(PresentSource present_source, const EVENT_HEADER& header, uint64_t swap_chain_address,
                      uint32_t dxgi_present_flags, int32_t sync_interval);
  void OnPresentStop(PresentSource present_source, const EVENT_HEADER& hdr, uint32_t result);

  struct EventCount {
    uint64_t num_events_total = 0;
    uint64_t num_events_processed = 0;
    void Log(const char* name);
  };

  struct Stats {
    EventCount total_event_count;
    EventCount dxgi_event_count;
    EventCount d3d9_event_count;
    EventCount dwm_core_event_count;
    EventCount dwm_core_win_7_event_count;
    EventCount dxg_kernel_event_count;
    EventCount dxg_kernel_win_7_pres_event_count;
    EventCount nt_process_event_count;
    EventCount win_metadata_event_count;
    EventCount win_2k_event_count;
    void Log();
  };

  bool ShouldProcessEvent(const EVENT_RECORD& record, EventCount& event_count);

  uint32_t target_pid_ = orbit_base::kInvalidProcessId;
  TracerListener* listener_ = nullptr;
  Stats stats_;

  krabs::provider<> dxgi_provider_;
  krabs::provider<> d3d9_provider_;
  krabs::provider<> dwm_core_provider_;
  krabs::provider<> dwm_core_win7_provider_;
  krabs::provider<> dxg_krnl_provider_;
  krabs::provider<> dxg_krnl_win7_present_provider_;
  krabs::provider<> nt_process_provider_;
  krabs::provider<> win_event_meta_data_provider_;
  krabs::provider<> win2k_provider_;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_GRAPHICS_ETW_PROVIDER_H_
