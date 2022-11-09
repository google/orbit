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
#include <absl/functional/bind_front.h>

// clang-format off
#include <d3d9.h>
#include <dxgi.h>
// clang-format on

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

namespace orbit_windows_tracing {

using orbit_grpc_protos::PresentEvent;

GraphicsEtwProvider::GraphicsEtwProvider(uint32_t pid, krabs::user_trace* trace,
                                         TracerListener* listener)
    : target_pid_(pid), trace_(trace), listener_(listener) {
  EnableProvider("Dxgi", Microsoft_Windows_DXGI::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnDXGIEvent, this));
  EnableProvider("D3d9", Microsoft_Windows_D3D9::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnD3d9Event, this));
  EnableProvider("DwmCore", Microsoft_Windows_Dwm_Core::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnDwmCoreEvent, this));
  EnableProvider("DwmCoreWin7", Microsoft_Windows_Dwm_Core::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnDwmCoreWin7Event, this));
  EnableProvider("DxgKrnl", Microsoft_Windows_DxgKrnl::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnDxgKrnlEvent, this));
  EnableProvider("DxgKrnlWin7Pres", Microsoft_Windows_DxgKrnl::Win7::PRESENTHISTORY_GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnDxgKrnlWin7PresEvent, this));
  EnableProvider("NtProcess", NT_Process::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnNtProcessEvent, this));
  EnableProvider("WindowsEventMetadata", Microsoft_Windows_EventMetadata::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnWindowsEventMetadata, this));
  EnableProvider("Win32K", Microsoft_Windows_Win32k::GUID,
                 absl::bind_front(&GraphicsEtwProvider::OnWin32KEvent, this));
}

void GraphicsEtwProvider::EnableProvider(std::string_view name, GUID guid,
                                         krabs::provider_event_callback callback) {
  ORBIT_CHECK(!name_to_provider_.contains(name));
  name_to_provider_[name] = std::make_unique<Provider>(name, guid, target_pid_, trace_, callback);
}

void GraphicsEtwProvider::OnPresentStart(orbit_grpc_protos::PresentEvent::Source present_source,
                                         uint32_t present_flags, const EVENT_HEADER& header) {
  // PRESENT_TEST is used to check if the application is running in fullscreen, ignore.
  if (present_flags & DXGI_PRESENT_TEST) {
    return;
  }

  uint64_t timestamp_ns = orbit_base::PerformanceCounterToNs(header.TimeStamp.QuadPart);
  orbit_grpc_protos::PresentEvent present_event;
  present_event.set_pid(header.ProcessId);
  present_event.set_tid(header.ThreadId);
  present_event.set_begin_timestamp_ns(timestamp_ns);
  present_event.set_source(present_source);
  listener_->OnPresentEvent(std::move(present_event));
}

// The "On[]Event" methods below are based on Intel's PresentMon project, see:
// https://github.com/GameTechDev/PresentMon/blob/main/PresentData/PresentMonTraceConsumer.cpp.

void GraphicsEtwProvider::OnDXGIEvent(const EVENT_RECORD& record,
                                      const krabs::trace_context& context) {
  krabs::schema schema(record, context.schema_locator);
  krabs::parser parser(schema);

  switch (record.EventHeader.EventDescriptor.Id) {
    case Microsoft_Windows_DXGI::Present_Start::Id:
    case Microsoft_Windows_DXGI::PresentMultiplaneOverlay_Start::Id:
      OnPresentStart(PresentEvent::kDxgi, parser.parse<uint32_t>(L"Flags"), record.EventHeader);
      break;
    default:
      break;
  }
}

void GraphicsEtwProvider::OnD3d9Event(const EVENT_RECORD& record,
                                      const krabs::trace_context& context) {
  krabs::schema schema(record, context.schema_locator);
  krabs::parser parser(schema);

  switch (record.EventHeader.EventDescriptor.Id) {
    case Microsoft_Windows_D3D9::Present_Start::Id:
      OnPresentStart(PresentEvent::kD3d9, parser.parse<uint32_t>(L"Flags"), record.EventHeader);
      break;
    default:
      break;
  }
}

void GraphicsEtwProvider::OnDwmCoreEvent(const EVENT_RECORD& record,
                                         const krabs::trace_context& context) {}

void GraphicsEtwProvider::OnDwmCoreWin7Event(const EVENT_RECORD& record,
                                             const krabs::trace_context& context) {}

void GraphicsEtwProvider::OnDxgKrnlEvent(const EVENT_RECORD& record,
                                         const krabs::trace_context& context) {}

void GraphicsEtwProvider::OnDxgKrnlWin7PresEvent(const EVENT_RECORD& record,
                                                 const krabs::trace_context& context) {}

void GraphicsEtwProvider::OnNtProcessEvent(const EVENT_RECORD& record,
                                           const krabs::trace_context& context) {}

void GraphicsEtwProvider::OnWindowsEventMetadata(const EVENT_RECORD& record,
                                                 const krabs::trace_context& context) {}

void GraphicsEtwProvider::OnWin32KEvent(const EVENT_RECORD& record,
                                        const krabs::trace_context& context) {}

void GraphicsEtwProvider::OutputStats() {
  ORBIT_LOG("--- GraphicsEtwProvider stats ---");
  for (const auto& [unused_name, provider] : name_to_provider_) {
    provider->Log();
  }
}

GraphicsEtwProvider::Provider::Provider(std::string_view name, GUID guid, uint32_t target_pid,
                                        krabs::user_trace* trace,
                                        krabs::provider_event_callback callback)
    : name_(name), krabs_provider_(guid), target_pid_(target_pid), callback_(callback) {
  krabs_provider_.add_on_event_callback(absl::bind_front(&Provider::OnEvent, this));
  trace->enable(krabs_provider_);
}

void GraphicsEtwProvider::Provider::OnEvent(const EVENT_RECORD& record,
                                            const krabs::trace_context& context) {
  ++num_events_received_;
  if (record.EventHeader.ProcessId == target_pid_) {
    ++num_events_processed_;
    // Relay interesting event to provided callback.
    callback_(record, context);
  }
}

void GraphicsEtwProvider::Provider::Log() {
  ORBIT_LOG("%s: %u/%u", name_, num_events_processed_, num_events_received_);
}

}  // namespace orbit_windows_tracing
