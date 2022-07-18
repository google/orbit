// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "KrabsTracer.h"

#include <absl/base/casts.h>
#include <evntrace.h>

#include <filesystem>
#include <optional>

#include "EtwEventTypes.h"
#include "ObjectUtils/CoffFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/StringConversion.h"
#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/AdjustTokenPrivilege.h"
#include "WindowsUtils/PathConverter.h"

// clang-format off
#include "fileapi.h"
// clang-format on

namespace orbit_windows_tracing {

using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::SchedulingSlice;
using orbit_windows_utils::PathConverter;

KrabsTracer::KrabsTracer(uint32_t pid, double sampling_frequency_hz, TracerListener* listener)
    : KrabsTracer(pid, sampling_frequency_hz, listener, ProviderFlags::kAll) {}

KrabsTracer::KrabsTracer(uint32_t pid, double sampling_frequency_hz, TracerListener* listener,
                         ProviderFlags providers)
    : target_pid_(pid),
      sampling_frequency_hz_(sampling_frequency_hz),
      listener_(listener),
      providers_(providers),
      kernel_trace_(KERNEL_LOGGER_NAME),
      stack_walk_provider_(EVENT_TRACE_FLAG_PROFILE, krabs::guids::stack_walk) {
  path_converter_ = orbit_windows_utils::PathConverter::Create();
  SetTraceProperties();
  EnableProviders();
}

void KrabsTracer::SetTraceProperties() {
  // https://docs.microsoft.com/en-us/windows/win32/api/evntrace/ns-evntrace-event_trace_properties
  EVENT_TRACE_PROPERTIES properties = {0};
  properties.BufferSize = 256;
  properties.MinimumBuffers = 12;
  properties.MaximumBuffers = 48;
  properties.FlushTimer = 1;
  properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
  kernel_trace_.set_trace_properties(&properties);
}

bool KrabsTracer::IsProviderEnabled(ProviderFlags provider) const {
  return (providers_ & provider) != 0;
}

void KrabsTracer::EnableProviders() {
  if (IsProviderEnabled(ProviderFlags::kThread)) {
    thread_provider_.add_on_event_callback(
        [this](const auto& record, const auto& context) { OnThreadEvent(record, context); });
    kernel_trace_.enable(thread_provider_);
  }

  if (IsProviderEnabled(ProviderFlags::kContextSwitch)) {
    context_switch_provider_.add_on_event_callback(
        [this](const auto& record, const auto& context) { OnThreadEvent(record, context); });
    kernel_trace_.enable(context_switch_provider_);
  }

  if (IsProviderEnabled(ProviderFlags::kStackWalk)) {
    stack_walk_provider_.add_on_event_callback(
        [this](const auto& record, const auto& context) { OnStackWalkEvent(record, context); });
    kernel_trace_.enable(stack_walk_provider_);
  }

  if (IsProviderEnabled(ProviderFlags::kImageLoad)) {
    image_load_provider_.add_on_event_callback(
        [this](const auto& record, const auto& context) { OnImageLoadEvent(record, context); });
    kernel_trace_.enable(image_load_provider_);
  }

  if (IsProviderEnabled(ProviderFlags::kGraphics)) {
    graphics_etw_provider_ =
        std::make_unique<GraphicsEtwProvider>(target_pid_, &user_trace_, listener_);
  }
}

void KrabsTracer::SetIsSystemProfilePrivilegeEnabled(bool value) {
  auto result = orbit_windows_utils::AdjustTokenPrivilege(SE_SYSTEM_PROFILE_NAME, value);
  if (result.has_error()) ORBIT_ERROR("%s", result.error().message());
}

void KrabsTracer::SetupStackTracing() {
  // Set sampling frequency for ETW trace. Note that the session handle must be 0.
  ORBIT_CHECK(sampling_frequency_hz_ >= 0);
  double period_ns = 1'000'000'000.0 / sampling_frequency_hz_;
  static uint64_t performance_counter_period_ns = orbit_base::GetPerformanceCounterPeriodNs();
  TRACE_PROFILE_INTERVAL interval = {0};
  interval.Interval = static_cast<ULONG>(period_ns / performance_counter_period_ns);
  ULONG status = TraceSetInformation(/*SessionHandle=*/0, TraceSampledProfileIntervalInfo,
                                     (void*)&interval, sizeof(TRACE_PROFILE_INTERVAL));
  ORBIT_CHECK(status == ERROR_SUCCESS);

  // Initialize ETW stack tracing. Note that this must be executed after kernel_trace_.open() as
  // set_trace_information needs a valid session handle.
  CLASSIC_EVENT_ID event_id = {0};
  event_id.EventGuid = krabs::guids::perf_info;
  event_id.Type = kSampledProfileEventSampleProfile;
  kernel_trace_.set_trace_information(TraceStackTracingInfo, &event_id, sizeof(CLASSIC_EVENT_ID));
}

void KrabsTracer::Start() {
  ORBIT_CHECK(kernel_trace_thread_ == nullptr);
  ORBIT_CHECK(user_trace_thread_ == nullptr);
  context_switch_manager_ = std::make_unique<ContextSwitchManager>(listener_);
  SetIsSystemProfilePrivilegeEnabled(true);
  log_file_ = kernel_trace_.open();
  if (IsProviderEnabled(ProviderFlags::kStackWalk)) {
    SetupStackTracing();
  }
  kernel_trace_thread_ = std::make_unique<std::thread>(&KrabsTracer::KernelTraceThread, this);
  user_trace_thread_ = std::make_unique<std::thread>(&KrabsTracer::UserTraceThread, this);
}

void KrabsTracer::Stop() {
  StopKernelTrace();
  StopUserTrace();

  OutputStats();
  SetIsSystemProfilePrivilegeEnabled(false);
  context_switch_manager_ = nullptr;
}

void KrabsTracer::KernelTraceThread() {
  orbit_base::SetCurrentThreadName("KrabsTracer::KernelTraceThread");
  kernel_trace_.process();
}

void KrabsTracer::UserTraceThread() {
  orbit_base::SetCurrentThreadName("KrabsTracer::UserTraceThread");
  user_trace_.start();
}

void KrabsTracer::StopKernelTrace() {
  ORBIT_CHECK(kernel_trace_thread_ != nullptr && kernel_trace_thread_->joinable());
  kernel_trace_.stop();
  kernel_trace_thread_->join();
  kernel_trace_thread_ = nullptr;
}

void KrabsTracer::StopUserTrace() {
  ORBIT_CHECK(user_trace_thread_ != nullptr && user_trace_thread_->joinable());
  user_trace_.stop();
  user_trace_thread_->join();
  user_trace_thread_ = nullptr;
}

void KrabsTracer::OnThreadEvent(const EVENT_RECORD& record, const krabs::trace_context& context) {
  ++stats_.num_thread_events;
  switch (record.EventHeader.EventDescriptor.Opcode) {
    case kEtwThreadGroup1EventStart:
    case kEtwThreadGroup1EventDcStart:
    case kEtwThreadGroup1EventDcEnd: {
      // The Start event type corresponds to a thread's creation. The DCStart and DCEnd event types
      // enumerate the threads that are currently running at the time the kernel session starts and
      // ends, respectively.
      krabs::schema schema(record, context.schema_locator);
      krabs::parser parser(schema);
      uint32_t tid = parser.parse<uint32_t>(L"TThreadId");
      uint32_t pid = parser.parse<uint32_t>(L"ProcessId");
      context_switch_manager_->ProcessTidToPidMapping(tid, pid);
      break;
    }
    case kEtwThreadV2EventCSwitch: {
      // https://docs.microsoft.com/en-us/windows/win32/etw/cswitch
      krabs::schema schema(record, context.schema_locator);
      krabs::parser parser(schema);
      uint32_t old_tid = parser.parse<uint32_t>(L"OldThreadId");
      uint32_t new_tid = parser.parse<uint32_t>(L"NewThreadId");
      uint64_t timestamp_ns =
          orbit_base::PerformanceCounterToNs(record.EventHeader.TimeStamp.QuadPart);
      uint16_t cpu = record.BufferContext.ProcessorIndex;
      context_switch_manager_->ProcessContextSwitch(cpu, old_tid, new_tid, timestamp_ns);
    } break;
    default:
      // Discard uninteresting thread events.
      break;
  }
}

void KrabsTracer::OnStackWalkEvent(const EVENT_RECORD& record,
                                   const krabs::trace_context& context) {
  // https://docs.microsoft.com/en-us/windows/win32/etw/stackwalk-event
  ++stats_.num_stack_events;
  krabs::schema schema(record, context.schema_locator);
  krabs::parser parser(schema);
  uint32_t pid = parser.parse<uint32_t>(L"StackProcess");

  // Filter events based on target pid, if one was set.
  if (target_pid_ != orbit_base::kInvalidProcessId) {
    if (pid != target_pid_) return;
    ++stats_.num_stack_events_for_target_pid;
  }

  // Get callstack addresses. The first address is at offset 16, see stackwalk-event doc above.
  constexpr uint32_t kStackDataOffset = 16;
  ORBIT_CHECK(record.UserDataLength >= kStackDataOffset);
  const uint8_t* buffer_start = absl::bit_cast<uint8_t*>(record.UserData);
  const uint8_t* buffer_end = buffer_start + record.UserDataLength;
  const uint8_t* stack_data = buffer_start + kStackDataOffset;
  uint32_t depth = (buffer_end - stack_data) / sizeof(void*);
  ORBIT_CHECK(stack_data + depth * sizeof(void*) == buffer_end);

  FullCallstackSample sample;
  sample.set_pid(pid);
  sample.set_tid(parser.parse<uint32_t>(L"StackThread"));
  sample.set_timestamp_ns(
      orbit_base::PerformanceCounterToNs(record.EventHeader.TimeStamp.QuadPart));

  Callstack* callstack = sample.mutable_callstack();
  callstack->set_type(Callstack::kComplete);
  uint64_t* address = absl::bit_cast<uint64_t*>(stack_data);
  for (size_t i = 0; i < depth; ++i, ++address) {
    callstack->add_pcs(*address);
  }

  listener_->OnCallstackSample(sample);
}

void KrabsTracer::OnImageLoadEvent(const EVENT_RECORD& record,
                                   const krabs::trace_context& context) {
  krabs::schema schema(record, context.schema_locator);
  krabs::parser parser(schema);

  if (record.EventHeader.EventDescriptor.Opcode == kImageLoadEventDcStart) {
    uint32_t pid = parser.parse<uint32_t>(L"ProcessId");
    if (pid != target_pid_) return;
    ++stats_.num_image_load_events_for_target_pid;

    orbit_windows_utils::Module module;
    module.full_path = orbit_base::ToStdString(parser.parse<std::wstring>(L"FileName"));
    module.name = std::filesystem::path(module.full_path).filename().string();
    module.file_size = parser.parse<uint64_t>(L"ImageSize");
    module.address_start = parser.parse<uint64_t>(L"ImageBase");
    module.address_end = module.address_start + module.file_size;

    // The full path at this point contains the device name and not the drive letter, try to
    // convert it so that it takes a more conventional form.
    ErrorMessageOr<std::string> result = path_converter_->DeviceToDrive(module.full_path);
    if (result.has_value()) {
      module.full_path = result.value();
    } else {
      ORBIT_ERROR("Calling \"DeviceToDrive\": %s %s", result.error().message(),
                  path_converter_->ToString());
    }

    auto coff_file_or_error = orbit_object_utils::CreateCoffFile(module.full_path);
    if (coff_file_or_error.has_value()) {
      module.build_id = coff_file_or_error.value()->GetBuildId();
    } else {
      ORBIT_ERROR("Could not create Coff file for module \"%s\", build-id will be empty",
                  module.full_path);
    }

    absl::MutexLock lock{&modules_mutex_};
    modules_.emplace_back(std::move(module));
  }
}

std::vector<orbit_windows_utils::Module> KrabsTracer::GetLoadedModules() const {
  std::vector<orbit_windows_utils::Module> modules;
  absl::MutexLock lock{&modules_mutex_};
  modules = modules_;
  return modules;
}

void KrabsTracer::OutputStats() {
  krabs::trace_stats trace_stats = kernel_trace_.query_stats();
  ORBIT_LOG("--- ETW stats ---");
  ORBIT_LOG("Number of buffers: %u", trace_stats.buffersCount);
  ORBIT_LOG("Free buffers: %u", trace_stats.buffersFree);
  ORBIT_LOG("Buffers written: %u", trace_stats.buffersWritten);
  ORBIT_LOG("Buffers lost: %u", trace_stats.buffersLost);
  ORBIT_LOG("Events total (handled+lost): %u", trace_stats.eventsTotal);
  ORBIT_LOG("Events handled: %u", trace_stats.eventsHandled);
  ORBIT_LOG("Events lost: %u", trace_stats.eventsLost);
  ORBIT_LOG("--- KrabsTracer stats ---");
  ORBIT_LOG("Number of stack events: %u", stats_.num_stack_events);
  ORBIT_LOG("Number of stack events for target pid: %u", stats_.num_stack_events_for_target_pid);
  context_switch_manager_->OutputStats();
  if (graphics_etw_provider_ != nullptr) {
    graphics_etw_provider_->OutputStats();
  }
}

}  // namespace orbit_windows_tracing
