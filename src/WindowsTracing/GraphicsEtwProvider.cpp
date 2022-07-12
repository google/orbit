// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GraphicsEtwProvider.h"

// clang-format off
#include <guiddef.h>
#include <stdint.h>
// clang-format on

#include <PresentData/ETW/Microsoft_Windows_D3D9.h>
#include <PresentData/ETW/Microsoft_Windows_DXGI.h>
#include <PresentData/ETW/Microsoft_Windows_Dwm_Core.h>
#include <PresentData/ETW/Microsoft_Windows_DxgKrnl.h>
#include <PresentData/ETW/Microsoft_Windows_EventMetadata.h>
#include <PresentData/ETW/Microsoft_Windows_Win32k.h>
#include <PresentData/ETW/NT_Process.h>

// clang-format off
#include <d3d9.h>
#include <dxgi.h>
// clang-format on

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

namespace orbit_windows_tracing {

GraphicsEtwProvider::GraphicsEtwProvider(uint32_t pid, krabs::user_trace* trace,
                                         TracerListener* listener)
    : target_pid_(pid),
      listener_(listener),
      dxgi_provider_(Microsoft_Windows_DXGI::GUID),
      d3d9_provider_(Microsoft_Windows_D3D9::GUID),
      dwm_core_provider_(Microsoft_Windows_Dwm_Core::GUID),
      dwm_core_win7_provider_(Microsoft_Windows_Dwm_Core::Win7::GUID),
      dxg_krnl_provider_(Microsoft_Windows_DxgKrnl::GUID),
      dxg_krnl_win7_present_provider_(Microsoft_Windows_DxgKrnl::Win7::PRESENTHISTORY_GUID),
      nt_process_provider_(NT_Process::GUID),
      win_event_meta_data_provider_(Microsoft_Windows_EventMetadata::GUID),
      win_32_k_provider_(Microsoft_Windows_Win32k::GUID) {
  // Setup callbacks.
  dxgi_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnDXGIEvent(record, context); });
  d3d9_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnD3D9Event(record, context); });
  dwm_core_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnDwmCoreEvent(record, context); });
  dwm_core_win7_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnDwmCoreWin7Event(record, context); });
  dxg_krnl_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnDxgKrnlEvent(record, context); });
  dxg_krnl_win7_present_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnDxgKrnlWin7PresEvent(record, context); });
  nt_process_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnNtProcessEvent(record, context); });
  win_event_meta_data_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnWindowsEventMetadata(record, context); });
  win_32_k_provider_.add_on_event_callback(
      [this](const auto& record, const auto& context) { OnWin32KEvent(record, context); });

  // Enable providers.
  trace->enable(dxgi_provider_);
  trace->enable(d3d9_provider_);
  trace->enable(dwm_core_provider_);
  trace->enable(dwm_core_win7_provider_);
  trace->enable(dxg_krnl_provider_);
  trace->enable(dxg_krnl_win7_present_provider_);
  trace->enable(nt_process_provider_);
  trace->enable(win_event_meta_data_provider_);
  trace->enable(win_32_k_provider_);
}

static orbit_grpc_protos::PresentEvent::Source GrpcSourceFromPresentSource(
    const GraphicsEtwProvider::PresentSource& present_source) {
  switch (present_source) {
    case GraphicsEtwProvider::PresentSource::kD3d9:
      return orbit_grpc_protos::PresentEvent::kD3d9;
    case GraphicsEtwProvider::PresentSource::kDxgi:
      return orbit_grpc_protos::PresentEvent::kDxgi;
    case GraphicsEtwProvider::PresentSource::kUnknown:
    default:
      return orbit_grpc_protos::PresentEvent::kUnknown;
  }
}

void GraphicsEtwProvider::OnPresentStart(PresentSource present_source, const EVENT_HEADER& header,
                                         uint64_t swap_chain_address, uint32_t dxgi_present_flags,
                                         int32_t sync_interval) {
  // PRESENT_TEST is used to check if the application is running in fullscreen, ignore.
  if (dxgi_present_flags & DXGI_PRESENT_TEST) {
    return;
  }

  uint64_t timestamp_ns = orbit_base::PerformanceCounterToNs(header.TimeStamp.QuadPart);

  orbit_grpc_protos::PresentEvent present_event;
  present_event.set_pid(header.ProcessId);
  present_event.set_tid(header.ThreadId);
  present_event.set_begin_timestamp_ns(timestamp_ns);
  present_event.set_duration_ns(1'000'000);  // dummy 1 ms duration.
  present_event.set_source(GrpcSourceFromPresentSource(present_source));

  listener_->OnPresentEvent(std::move(present_event));
}

void GraphicsEtwProvider::OnPresentStop(PresentSource present_source, const EVENT_HEADER& hdr,
                                        uint32_t result) {}

void GraphicsEtwProvider::OnDXGIEvent(const EVENT_RECORD& record,
                                      const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.dxgi_event_count)) return;

  krabs::schema schema(record, context.schema_locator);
  krabs::parser parser(schema);

  switch (record.EventHeader.EventDescriptor.Id) {
    case Microsoft_Windows_DXGI::Present_Start::Id:
    case Microsoft_Windows_DXGI::PresentMultiplaneOverlay_Start::Id: {
      auto swap_chain = parser.parse<uint64_t>(L"pIDXGISwapChain");
      auto flags = parser.parse<uint32_t>(L"Flags");
      auto sync_interval = parser.parse<uint32_t>(L"SyncInterval");
      OnPresentStart(PresentSource::kDxgi, record.EventHeader, swap_chain, flags, sync_interval);
      break;
    }
    case Microsoft_Windows_DXGI::Present_Stop::Id:
    case Microsoft_Windows_DXGI::PresentMultiplaneOverlay_Stop::Id:
      OnPresentStop(PresentSource::kDxgi, record.EventHeader, parser.parse<uint32_t>(L"Result"));
      break;
    default:
      break;
  }
}

void GraphicsEtwProvider::OnD3D9Event(const EVENT_RECORD& record,
                                      const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.d3d9_event_count)) return;

  krabs::schema schema(record, context.schema_locator);
  krabs::parser parser(schema);

  switch (record.EventHeader.EventDescriptor.Id) {
    case Microsoft_Windows_D3D9::Present_Start::Id: {
      auto swap_chain = parser.parse<uint64_t>(L"pSwapchain");
      auto flags = parser.parse<uint32_t>(L"Flags");

      uint32_t dxgi_present_flags = 0;
      if (flags & D3DPRESENT_DONOTFLIP) dxgi_present_flags |= DXGI_PRESENT_DO_NOT_SEQUENCE;
      if (flags & D3DPRESENT_DONOTWAIT) dxgi_present_flags |= DXGI_PRESENT_DO_NOT_WAIT;
      if (flags & D3DPRESENT_FLIPRESTART) dxgi_present_flags |= DXGI_PRESENT_RESTART;

      int32_t sync_interval = -1;
      if (flags & D3DPRESENT_FORCEIMMEDIATE) {
        sync_interval = 0;
      }

      OnPresentStart(PresentSource::kD3d9, record.EventHeader, swap_chain, flags, sync_interval);
      break;
    }
    case Microsoft_Windows_D3D9::Present_Stop::Id:
      OnPresentStop(PresentSource::kD3d9, record.EventHeader, parser.parse<uint32_t>(L"Result"));
      break;
    default:
      break;
  }
}

void GraphicsEtwProvider::OnDwmCoreEvent(const EVENT_RECORD& record,
                                         const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.dwm_core_event_count)) return;
}

void GraphicsEtwProvider::OnDwmCoreWin7Event(const EVENT_RECORD& record,
                                             const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.dwm_core_win_7_event_count)) return;
}

void GraphicsEtwProvider::OnDxgKrnlEvent(const EVENT_RECORD& record,
                                         const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.dxg_kernel_event_count)) return;
}

void GraphicsEtwProvider::OnDxgKrnlWin7PresEvent(const EVENT_RECORD& record,
                                                 const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.dxg_kernel_win_7_pres_event_count)) return;
}

void GraphicsEtwProvider::OnNtProcessEvent(const EVENT_RECORD& record,
                                           const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.nt_process_event_count)) return;
}

void GraphicsEtwProvider::OnWindowsEventMetadata(const EVENT_RECORD& record,
                                                 const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.win_metadata_event_count)) return;
}

void GraphicsEtwProvider::OnWin32KEvent(const EVENT_RECORD& record,
                                        const krabs::trace_context& context) {
  if (!ShouldProcessEvent(record, stats_.win_32_k_event_count)) return;
}

bool GraphicsEtwProvider::ShouldProcessEvent(const EVENT_RECORD& record,
                                             GraphicsEtwProvider::EventCount& event_count) {
  ++stats_.total_event_count.num_events_total;
  ++event_count.num_events_total;
  if (record.EventHeader.ProcessId == target_pid_) {
    ++stats_.total_event_count.num_events_processed;
    ++event_count.num_events_processed;
    return true;
  }
  return false;
}

void GraphicsEtwProvider::EventCount::Log(const char* name) {
  ORBIT_LOG("%s %u/%u", name, num_events_processed, num_events_total);
}

#define LOG_COUNTER(x) x.Log(#x)

void GraphicsEtwProvider::Stats::Log() {
  LOG_COUNTER(total_event_count);
  LOG_COUNTER(dxgi_event_count);
  LOG_COUNTER(d3d9_event_count);
  LOG_COUNTER(dwm_core_event_count);
  LOG_COUNTER(dwm_core_win_7_event_count);
  LOG_COUNTER(dxg_kernel_event_count);
  LOG_COUNTER(dxg_kernel_win_7_pres_event_count);
  LOG_COUNTER(nt_process_event_count);
  LOG_COUNTER(win_metadata_event_count);
  LOG_COUNTER(win_32_k_event_count);
}

void GraphicsEtwProvider::OutputStats() {
  ORBIT_LOG("--- GraphicsEtwProvider stats ---");
  stats_.Log();
}

}  // namespace orbit_windows_tracing
