// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracerImpl.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/meta/type_traits.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/synchronization/mutex.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "Function.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "Introspection/Introspection.h"
#include "KernelTracepoints.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracingUtils.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "PerfEventOpen.h"
#include "PerfEventReaders.h"
#include "PerfEventRecords.h"

namespace orbit_linux_tracing {

using orbit_base::GetAllPids;
using orbit_base::GetTidsOfProcess;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModulesSnapshot;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadNamesSnapshot;

static std::optional<uint64_t> ComputeSamplingPeriodNs(double sampling_frequency) {
  double period_ns_dbl = 1'000'000'000 / sampling_frequency;
  if (period_ns_dbl > 0 &&
      period_ns_dbl <= static_cast<double>(std::numeric_limits<uint64_t>::max())) {
    return period_ns_dbl;
  }
  return std::nullopt;
}

TracerImpl::TracerImpl(
    const CaptureOptions& capture_options,
    std::unique_ptr<UserSpaceInstrumentationAddresses> user_space_instrumentation_addresses,
    TracerListener* listener)
    : trace_context_switches_{capture_options.trace_context_switches()},
      introspection_enabled_{capture_options.enable_introspection()},
      target_pid_{orbit_base::ToNativeProcessId(capture_options.pid())},
      unwinding_method_{capture_options.unwinding_method()},
      trace_thread_state_{capture_options.trace_thread_state()},
      trace_gpu_driver_{capture_options.trace_gpu_driver()},
      user_space_instrumentation_addresses_{std::move(user_space_instrumentation_addresses)},
      listener_{listener} {
  ORBIT_CHECK(listener_ != nullptr);

  uint32_t stack_dump_size = capture_options.stack_dump_size();
  if (stack_dump_size == std::numeric_limits<uint16_t>::max()) {
    constexpr uint16_t kDefaultStackSampleUserSizeFramePointer = 512;
    stack_dump_size = (unwinding_method_ == CaptureOptions::kDwarf)
                          ? kMaxStackSampleUserSize
                          : kDefaultStackSampleUserSizeFramePointer;
    ORBIT_LOG("No sample stack dump size was set; assigning to default: %u", stack_dump_size);
  } else if (stack_dump_size > kMaxStackSampleUserSize || stack_dump_size == 0) {
    // TODO(b/210439638): Support a stack_dump_size of 0. It might be valid for frame pointer
    //  sampling without leaf function patching.
    ORBIT_ERROR("Invalid sample stack dump size: %u; reassigning to default: %u", stack_dump_size,
                kMaxStackSampleUserSize);
    stack_dump_size = kMaxStackSampleUserSize;
  }
  stack_dump_size_ = static_cast<uint16_t>(stack_dump_size);

  if (capture_options.samples_per_second() == 0) {
    sampling_period_ns_ = std::nullopt;
  } else {
    sampling_period_ns_ = ComputeSamplingPeriodNs(capture_options.samples_per_second());
  }

  instrumented_functions_.reserve(capture_options.instrumented_functions_size());

  for (const InstrumentedFunction& instrumented_function :
       capture_options.instrumented_functions()) {
    uint64_t function_id = instrumented_function.function_id();
    instrumented_functions_.emplace_back(
        function_id, instrumented_function.file_path(), instrumented_function.file_offset(),
        instrumented_function.record_arguments(), instrumented_function.record_return_value());
  }

  for (const orbit_grpc_protos::TracepointInfo& instrumented_tracepoint :
       capture_options.instrumented_tracepoint()) {
    orbit_grpc_protos::TracepointInfo info;
    info.set_name(instrumented_tracepoint.name());
    info.set_category(instrumented_tracepoint.category());
    instrumented_tracepoints_.emplace_back(info);
  }
}

void TracerImpl::Start() {
  stop_run_thread_ = false;
  run_thread_ = std::thread(&TracerImpl::Run, this);
}

void TracerImpl::Stop() {
  stop_run_thread_ = true;
  ORBIT_CHECK(run_thread_.joinable());
  run_thread_.join();
}

void TracerImpl::ProcessFunctionEntry(const orbit_grpc_protos::FunctionEntry& function_entry) {
  UserSpaceFunctionEntryPerfEvent event{
      .timestamp = function_entry.timestamp_ns(),
      .ordered_stream =
          PerfEventOrderedStream::ThreadId(orbit_base::ToNativeThreadId(function_entry.tid())),
      .data =
          UserSpaceFunctionEntryPerfEventData{
              .pid = orbit_base::ToNativeProcessId(function_entry.pid()),
              .tid = orbit_base::ToNativeThreadId(function_entry.tid()),
              .function_id = function_entry.function_id(),
              .sp = function_entry.stack_pointer(),
              .return_address = function_entry.return_address(),
          },
  };
  DeferEvent(event);
}

void TracerImpl::ProcessFunctionExit(const orbit_grpc_protos::FunctionExit& function_exit) {
  UserSpaceFunctionExitPerfEvent event{
      .timestamp = function_exit.timestamp_ns(),
      .ordered_stream =
          PerfEventOrderedStream::ThreadId(orbit_base::ToNativeThreadId(function_exit.tid())),
      .data =
          UserSpaceFunctionExitPerfEventData{
              .pid = orbit_base::ToNativeProcessId(function_exit.pid()),
              .tid = orbit_base::ToNativeThreadId(function_exit.tid()),
          },
  };
  DeferEvent(event);
}

static void CloseFileDescriptors(const std::vector<int>& fds) {
  for (int fd : fds) {
    close(fd);
  }
}

static void CloseFileDescriptors(const absl::flat_hash_map<int32_t, int>& fds_per_cpu) {
  for (const auto& pair : fds_per_cpu) {
    close(pair.second);
  }
}

void TracerImpl::InitUprobesEventVisitor() {
  ORBIT_SCOPE_FUNCTION;
  maps_ = LibunwindstackMaps::ParseMaps(ReadMaps(target_pid_));
  unwinder_ = LibunwindstackUnwinder::Create();
  return_address_manager_.emplace(user_space_instrumentation_addresses_.get());
  leaf_function_call_manager_ = std::make_unique<LeafFunctionCallManager>(stack_dump_size_);
  uprobes_unwinding_visitor_ = std::make_unique<UprobesUnwindingVisitor>(
      listener_, &function_call_manager_, &return_address_manager_.value(), maps_.get(),
      unwinder_.get(), leaf_function_call_manager_.get(),
      user_space_instrumentation_addresses_.get());
  uprobes_unwinding_visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(
      &stats_.unwind_error_count, &stats_.samples_in_uretprobes_count);
  event_processor_.AddVisitor(uprobes_unwinding_visitor_.get());
}

bool TracerImpl::OpenUprobes(const orbit_linux_tracing::Function& function,
                             const std::vector<int32_t>& cpus,
                             absl::flat_hash_map<int32_t, int>* fds_per_cpu) {
  ORBIT_SCOPE_FUNCTION;
  const char* module = function.file_path().c_str();
  const uint64_t offset = function.file_offset();
  for (int32_t cpu : cpus) {
    int fd;
    if (function.record_arguments()) {
      fd = uprobes_retaddr_args_event_open(module, offset, /*pid=*/-1, cpu);
    } else {
      fd = uprobes_retaddr_event_open(module, offset, /*pid=*/-1, cpu);
    }
    if (fd < 0) {
      ORBIT_ERROR("Opening uprobe %s+%#x on cpu %d", function.file_path(), function.file_offset(),
                  cpu);
      return false;
    }
    (*fds_per_cpu)[cpu] = fd;
  }
  return true;
}

bool TracerImpl::OpenUretprobes(const orbit_linux_tracing::Function& function,
                                const std::vector<int32_t>& cpus,
                                absl::flat_hash_map<int32_t, int>* fds_per_cpu) {
  ORBIT_SCOPE_FUNCTION;
  const char* module = function.file_path().c_str();
  const uint64_t offset = function.file_offset();
  for (int32_t cpu : cpus) {
    int fd;
    if (function.record_return_value()) {
      fd = uretprobes_retval_event_open(module, offset, /*pid=*/-1, cpu);
    } else {
      fd = uretprobes_event_open(module, offset, /*pid=*/-1, cpu);
    }
    if (fd < 0) {
      ORBIT_ERROR("Opening uretprobe %s+%#x on cpu %d", function.file_path(),
                  function.file_offset(), cpu);
      return false;
    }
    (*fds_per_cpu)[cpu] = fd;
  }
  return true;
}

void TracerImpl::AddUprobesFileDescriptors(
    const absl::flat_hash_map<int32_t, int>& uprobes_fds_per_cpu,
    const orbit_linux_tracing::Function& function) {
  ORBIT_SCOPE_FUNCTION;
  for (const auto [cpu, fd] : uprobes_fds_per_cpu) {
    uint64_t stream_id = perf_event_get_id(fd);
    uprobes_uretprobes_ids_to_function_id_.emplace(stream_id, function.function_id());
    if (function.record_arguments()) {
      uprobes_with_args_ids_.insert(stream_id);
    } else {
      uprobes_ids_.insert(stream_id);
    }
    tracing_fds_.push_back(fd);
  }
}

void TracerImpl::AddUretprobesFileDescriptors(
    const absl::flat_hash_map<int32_t, int>& uretprobes_fds_per_cpu,
    const orbit_linux_tracing::Function& function) {
  ORBIT_SCOPE_FUNCTION;
  for (const auto [cpu, fd] : uretprobes_fds_per_cpu) {
    uint64_t stream_id = perf_event_get_id(fd);
    uprobes_uretprobes_ids_to_function_id_.emplace(stream_id, function.function_id());
    if (function.record_return_value()) {
      uretprobes_with_retval_ids_.insert(stream_id);
    } else {
      uretprobes_ids_.insert(stream_id);
    }
    tracing_fds_.push_back(fd);
  }
}

bool TracerImpl::OpenUserSpaceProbes(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  bool uprobes_event_open_errors = false;

  absl::flat_hash_map<int32_t, std::vector<int>> uprobes_uretpobres_fds_per_cpu;
  for (const auto& function : instrumented_functions_) {
    absl::flat_hash_map<int32_t, int> uprobes_fds_per_cpu;
    absl::flat_hash_map<int32_t, int> uretprobes_fds_per_cpu;

    bool success = OpenUprobes(function, cpus, &uprobes_fds_per_cpu) &&
                   OpenUretprobes(function, cpus, &uretprobes_fds_per_cpu);
    if (!success) {
      CloseFileDescriptors(uprobes_fds_per_cpu);
      CloseFileDescriptors(uretprobes_fds_per_cpu);
      uprobes_event_open_errors = true;
      continue;
    }

    // Uretprobe need to be enabled before uprobes as we support temporarily
    // not having a uprobe associated with a uretprobe but not the opposite.
    AddUretprobesFileDescriptors(uretprobes_fds_per_cpu, function);
    AddUprobesFileDescriptors(uprobes_fds_per_cpu, function);

    for (const auto& [cpu, fd] : uretprobes_fds_per_cpu) {
      uprobes_uretpobres_fds_per_cpu[cpu].push_back(fd);
    }
    for (const auto& [cpu, fd] : uprobes_fds_per_cpu) {
      uprobes_uretpobres_fds_per_cpu[cpu].push_back(fd);
    }
  }

  OpenUserSpaceProbesRingBuffers(uprobes_uretpobres_fds_per_cpu);

  return !uprobes_event_open_errors;
}

void TracerImpl::OpenUserSpaceProbesRingBuffers(
    const absl::flat_hash_map<int32_t, std::vector<int>>& uprobes_uretpobres_fds_per_cpu) {
  ORBIT_SCOPE_FUNCTION;
  for (const auto& [/*int32_t*/ cpu, /*std::vector<int>*/ fds] : uprobes_uretpobres_fds_per_cpu) {
    if (fds.empty()) continue;

    // Create a single ring buffer per cpu.
    int ring_buffer_fd = fds[0];
    std::string buffer_name = absl::StrFormat("uprobes_uretprobes_%u", cpu);
    ring_buffers_.emplace_back(ring_buffer_fd, UPROBES_RING_BUFFER_SIZE_KB, buffer_name);

    // Redirect subsequent fds to the cpu specific ring buffer created above.
    for (size_t i = 1; i < fds.size(); ++i) {
      perf_event_redirect(fds[i], ring_buffer_fd);
    }
  }
}

bool TracerImpl::OpenMmapTask(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  std::vector<int> mmap_task_tracing_fds;
  std::vector<PerfEventRingBuffer> mmap_task_ring_buffers;
  for (int32_t cpu : cpus) {
    int mmap_task_fd = mmap_task_event_open(-1, cpu);
    std::string buffer_name = absl::StrFormat("mmap_task_%d", cpu);
    PerfEventRingBuffer mmap_task_ring_buffer{mmap_task_fd, MMAP_TASK_RING_BUFFER_SIZE_KB,
                                              buffer_name};
    if (mmap_task_ring_buffer.IsOpen()) {
      mmap_task_tracing_fds.push_back(mmap_task_fd);
      mmap_task_ring_buffers.push_back(std::move(mmap_task_ring_buffer));
    } else {
      ORBIT_ERROR("Opening mmap, fork, and exit events for cpu %d", cpu);
      CloseFileDescriptors(mmap_task_tracing_fds);
      return false;
    }
  }

  for (int fd : mmap_task_tracing_fds) {
    tracing_fds_.push_back(fd);
  }
  for (PerfEventRingBuffer& buffer : mmap_task_ring_buffers) {
    ring_buffers_.emplace_back(std::move(buffer));
  }
  return true;
}

bool TracerImpl::OpenSampling(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(sampling_period_ns_.has_value());
  ORBIT_CHECK(unwinding_method_ == CaptureOptions::kFramePointers ||
              unwinding_method_ == CaptureOptions::kDwarf);

  std::vector<int> sampling_tracing_fds;
  std::vector<PerfEventRingBuffer> sampling_ring_buffers;
  for (int32_t cpu : cpus) {
    int sampling_fd;
    switch (unwinding_method_) {
      case CaptureOptions::kFramePointers:
        sampling_fd =
            callchain_sample_event_open(sampling_period_ns_.value(), -1, cpu, stack_dump_size_);
        break;
      case CaptureOptions::kDwarf:
        sampling_fd =
            stack_sample_event_open(sampling_period_ns_.value(), -1, cpu, stack_dump_size_);
        break;
      case CaptureOptions::kUndefined:
      default:
        ORBIT_UNREACHABLE();
        CloseFileDescriptors(sampling_tracing_fds);
        return false;
    }

    std::string buffer_name = absl::StrFormat("sampling_%d", cpu);
    PerfEventRingBuffer sampling_ring_buffer{sampling_fd, SAMPLING_RING_BUFFER_SIZE_KB,
                                             buffer_name};
    if (sampling_ring_buffer.IsOpen()) {
      sampling_tracing_fds.push_back(sampling_fd);
      sampling_ring_buffers.push_back(std::move(sampling_ring_buffer));
    } else {
      ORBIT_ERROR("Opening sampling for cpu %d", cpu);
      CloseFileDescriptors(sampling_tracing_fds);
      return false;
    }
  }

  for (int fd : sampling_tracing_fds) {
    tracing_fds_.push_back(fd);
    uint64_t stream_id = perf_event_get_id(fd);
    if (unwinding_method_ == CaptureOptions::kDwarf) {
      stack_sampling_ids_.insert(stream_id);
    } else if (unwinding_method_ == CaptureOptions::kFramePointers) {
      callchain_sampling_ids_.insert(stream_id);
    }
  }
  for (PerfEventRingBuffer& buffer : sampling_ring_buffers) {
    ring_buffers_.emplace_back(std::move(buffer));
  }
  return true;
}

static void OpenRingBuffersOrRedirectOnExisting(
    const absl::flat_hash_map<int32_t, int>& fds_per_cpu,
    absl::flat_hash_map<int32_t, int>* ring_buffer_fds_per_cpu,
    std::vector<PerfEventRingBuffer>* ring_buffers, uint64_t ring_buffer_size_kb,
    std::string_view buffer_name_prefix) {
  ORBIT_SCOPE_FUNCTION;
  // Redirect all events on the same cpu to a single ring buffer.
  for (const auto& cpu_and_fd : fds_per_cpu) {
    int32_t cpu = cpu_and_fd.first;
    int fd = cpu_and_fd.second;
    if (ring_buffer_fds_per_cpu->contains(cpu)) {
      // Redirect to the already opened ring buffer.
      int ring_bugger_fd = ring_buffer_fds_per_cpu->at(cpu);
      perf_event_redirect(fd, ring_bugger_fd);
    } else {
      // Create a ring buffer for this cpu.
      int ring_buffer_fd = fd;
      std::string buffer_name = absl::StrFormat("%s_%d", buffer_name_prefix, cpu);
      ring_buffers->emplace_back(ring_buffer_fd, ring_buffer_size_kb, buffer_name);
      ring_buffer_fds_per_cpu->emplace(cpu, ring_buffer_fd);
    }
  }
}

namespace {

struct TracepointToOpen {
  TracepointToOpen(const char* tracepoint_category, const char* tracepoint_name,
                   absl::flat_hash_set<uint64_t>* tracepoint_stream_ids)
      : tracepoint_category{tracepoint_category},
        tracepoint_name{tracepoint_name},
        tracepoint_stream_ids{tracepoint_stream_ids} {}

  const char* const tracepoint_category;
  const char* const tracepoint_name;
  absl::flat_hash_set<uint64_t>* const tracepoint_stream_ids;
};

}  // namespace

static bool OpenFileDescriptorsAndRingBuffersForAllTracepoints(
    const std::vector<TracepointToOpen>& tracepoints_to_open, const std::vector<int32_t>& cpus,
    std::vector<int>* tracing_fds, uint64_t ring_buffer_size_kb,
    absl::flat_hash_map<int32_t, int>* tracepoint_ring_buffer_fds_per_cpu_for_redirection,
    std::vector<PerfEventRingBuffer>* ring_buffers) {
  ORBIT_SCOPE_FUNCTION;
  absl::flat_hash_map<size_t, absl::flat_hash_map<int32_t, int>> index_to_tracepoint_fds_per_cpu;
  bool tracepoint_event_open_errors = false;
  for (size_t tracepoint_index = 0;
       tracepoint_index < tracepoints_to_open.size() && !tracepoint_event_open_errors;
       ++tracepoint_index) {
    const char* tracepoint_category = tracepoints_to_open[tracepoint_index].tracepoint_category;
    const char* tracepoint_name = tracepoints_to_open[tracepoint_index].tracepoint_name;
    for (int32_t cpu : cpus) {
      int tracepoint_fd = tracepoint_event_open(tracepoint_category, tracepoint_name, -1, cpu);
      if (tracepoint_fd == -1) {
        ORBIT_ERROR("Opening %s:%s tracepoint for cpu %d", tracepoint_category, tracepoint_name,
                    cpu);
        tracepoint_event_open_errors = true;
        break;
      }
      index_to_tracepoint_fds_per_cpu[tracepoint_index].emplace(cpu, tracepoint_fd);
    }
  }

  if (tracepoint_event_open_errors) {
    for (const auto& index_and_tracepoint_fds_per_cpu : index_to_tracepoint_fds_per_cpu) {
      for (const auto& cpu_and_fd : index_and_tracepoint_fds_per_cpu.second) {
        close(cpu_and_fd.second);
      }
    }
    return false;
  }

  // Since all tracepoints could successfully be opened, we can now commit all file descriptors and
  // ring buffers to TracerThread's members.
  for (const auto& index_and_tracepoint_fds_per_cpu : index_to_tracepoint_fds_per_cpu) {
    const size_t tracepoint_index = index_and_tracepoint_fds_per_cpu.first;
    absl::flat_hash_set<uint64_t>* tracepoint_stream_ids =
        tracepoints_to_open[tracepoint_index].tracepoint_stream_ids;

    for (const auto& cpu_and_fd : index_and_tracepoint_fds_per_cpu.second) {
      tracing_fds->push_back(cpu_and_fd.second);
      tracepoint_stream_ids->insert(perf_event_get_id(cpu_and_fd.second));
    }
  }

  // Redirect on the same ring buffer all the tracepoint events that are open on each CPU.
  for (const auto& index_and_tracepoint_fds_per_cpu : index_to_tracepoint_fds_per_cpu) {
    const size_t tracepoint_index = index_and_tracepoint_fds_per_cpu.first;
    const char* tracepoint_category = tracepoints_to_open[tracepoint_index].tracepoint_category;
    const char* tracepoint_name = tracepoints_to_open[tracepoint_index].tracepoint_name;
    const absl::flat_hash_map<int32_t, int>& tracepoint_fds_per_cpu =
        index_and_tracepoint_fds_per_cpu.second;

    OpenRingBuffersOrRedirectOnExisting(
        tracepoint_fds_per_cpu, tracepoint_ring_buffer_fds_per_cpu_for_redirection, ring_buffers,
        ring_buffer_size_kb, absl::StrFormat("%s:%s", tracepoint_category, tracepoint_name));
  }
  return true;
}

bool TracerImpl::OpenThreadNameTracepoints(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  absl::flat_hash_map<int32_t, int> thread_name_tracepoint_ring_buffer_fds_per_cpu;
  return OpenFileDescriptorsAndRingBuffersForAllTracepoints(
      {{"task", "task_newtask", &task_newtask_ids_}, {"task", "task_rename", &task_rename_ids_}},
      cpus, &tracing_fds_, THREAD_NAMES_RING_BUFFER_SIZE_KB,
      &thread_name_tracepoint_ring_buffer_fds_per_cpu, &ring_buffers_);
}

void TracerImpl::InitSwitchesStatesNamesVisitor() {
  ORBIT_SCOPE_FUNCTION;
  switches_states_names_visitor_ = std::make_unique<SwitchesStatesNamesVisitor>(listener_);
  switches_states_names_visitor_->SetProduceSchedulingSlices(trace_context_switches_);
  if (trace_thread_state_) {
    // Filter thread states using target process id. We also send OrbitService's thread states when
    // introspection is enabled for more context on what our own threads are doing when capturing.
    std::set<pid_t> pids = {target_pid_};
    if (introspection_enabled_) {
      pids.insert(orbit_base::GetCurrentProcessIdNative());
    }
    switches_states_names_visitor_->SetThreadStatePidFilters(pids);
  }
  switches_states_names_visitor_->SetThreadStateCounter(&stats_.thread_state_count);
  event_processor_.AddVisitor(switches_states_names_visitor_.get());
}

bool TracerImpl::OpenContextSwitchAndThreadStateTracepoints(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  std::vector<TracepointToOpen> tracepoints_to_open;
  if (trace_thread_state_ || trace_context_switches_) {
    tracepoints_to_open.emplace_back("sched", "sched_switch", &sched_switch_ids_);
  }
  if (trace_thread_state_) {
    // We also need task:task_newtask, but this is already opened by OpenThreadNameTracepoints.
    tracepoints_to_open.emplace_back("sched", "sched_wakeup", &sched_wakeup_ids_);
  }
  if (tracepoints_to_open.empty()) {
    return true;
  }

  absl::flat_hash_map<int32_t, int> thread_state_tracepoint_ring_buffer_fds_per_cpu;
  return OpenFileDescriptorsAndRingBuffersForAllTracepoints(
      tracepoints_to_open, cpus, &tracing_fds_,
      CONTEXT_SWITCHES_AND_THREAD_STATE_RING_BUFFER_SIZE_KB,
      &thread_state_tracepoint_ring_buffer_fds_per_cpu, &ring_buffers_);
}

void TracerImpl::InitGpuTracepointEventVisitor() {
  ORBIT_SCOPE_FUNCTION;
  gpu_event_visitor_ = std::make_unique<GpuTracepointVisitor>(listener_);
  event_processor_.AddVisitor(gpu_event_visitor_.get());
}

// This method enables events for GPU event tracing. We trace three events that correspond to the
// following GPU driver events:
// - A GPU job (command buffer submission) is scheduled by the application. This is tracked by the
//   event "amdgpu_cs_ioctl".
// - A GPU job is scheduled to run on the hardware. This is tracked by the event
//   "amdgpu_sched_run_job".
// - A GPU job is finished by the hardware. This is tracked by the corresponding DMA fence being
//   signaled and is tracked by the event "dma_fence_signaled".
// A single job execution thus corresponds to three events, one of each type above, that share the
// same timeline, context, and seqno.
// We have to record events system-wide (per CPU) to ensure we record all relevant events.
// This method returns true on success, otherwise false.
bool TracerImpl::OpenGpuTracepoints(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  absl::flat_hash_map<int32_t, int> gpu_tracepoint_ring_buffer_fds_per_cpu;
  return OpenFileDescriptorsAndRingBuffersForAllTracepoints(
      {{"amdgpu", "amdgpu_cs_ioctl", &amdgpu_cs_ioctl_ids_},
       {"amdgpu", "amdgpu_sched_run_job", &amdgpu_sched_run_job_ids_},
       {"dma_fence", "dma_fence_signaled", &dma_fence_signaled_ids_}},
      cpus, &tracing_fds_, GPU_TRACING_RING_BUFFER_SIZE_KB, &gpu_tracepoint_ring_buffer_fds_per_cpu,
      &ring_buffers_);
}

bool TracerImpl::OpenInstrumentedTracepoints(const std::vector<int32_t>& cpus) {
  ORBIT_SCOPE_FUNCTION;
  bool tracepoint_event_open_errors = false;
  absl::flat_hash_map<int32_t, int> tracepoint_ring_buffer_fds_per_cpu;

  for (const auto& selected_tracepoint : instrumented_tracepoints_) {
    absl::flat_hash_set<uint64_t> stream_ids;
    tracepoint_event_open_errors |= !OpenFileDescriptorsAndRingBuffersForAllTracepoints(
        {{selected_tracepoint.category().c_str(), selected_tracepoint.name().c_str(), &stream_ids}},
        cpus, &tracing_fds_, INSTRUMENTED_TRACEPOINTS_RING_BUFFER_SIZE_KB,
        &tracepoint_ring_buffer_fds_per_cpu, &ring_buffers_);

    for (const auto& stream_id : stream_ids) {
      ids_to_tracepoint_info_.emplace(stream_id, selected_tracepoint);
    }
  }

  return !tracepoint_event_open_errors;
}

void TracerImpl::InitLostAndDiscardedEventVisitor() {
  ORBIT_SCOPE_FUNCTION;
  lost_and_discarded_event_visitor_ = std::make_unique<LostAndDiscardedEventVisitor>(listener_);
  event_processor_.AddVisitor(lost_and_discarded_event_visitor_.get());
}

static std::vector<ThreadName> RetrieveInitialThreadNamesSystemWide(uint64_t initial_timestamp_ns) {
  std::vector<ThreadName> thread_names;
  for (pid_t pid : GetAllPids()) {
    for (pid_t tid : GetTidsOfProcess(pid)) {
      std::string name = orbit_base::GetThreadNameNative(tid);
      if (name.empty()) {
        continue;
      }

      ThreadName thread_name;
      thread_name.set_pid(pid);
      thread_name.set_tid(tid);
      thread_name.set_name(std::move(name));
      thread_name.set_timestamp_ns(initial_timestamp_ns);
      thread_names.push_back(std::move(thread_name));
    }
  }

  return thread_names;
}

void TracerImpl::Startup() {
  ORBIT_SCOPE_FUNCTION;
  Reset();

  // perf_event_open refers to cores as "CPUs".

  // Record context switches from all cores for all processes.
  int32_t number_of_cores = GetNumCores();
  std::vector<int32_t> all_cpus;
  all_cpus.reserve(number_of_cores);
  for (int32_t cpu = 0; cpu < number_of_cores; ++cpu) {
    all_cpus.push_back(cpu);
  }

  // Record calls to dynamically instrumented functions and sample only on cores
  // in this process's cgroup's cpuset, as these are the only cores the process
  // will be scheduled on.
  std::vector<int32_t> cpuset_cpus = GetCpusetCpus(target_pid_);
  if (cpuset_cpus.empty()) {
    ORBIT_ERROR("Could not read cpuset");
    cpuset_cpus = all_cpus;
  }

  // As we open two perf_event_open file descriptors (uprobe and uretprobe) per
  // cpu per instrumented function, increase the maximum number of open files.
  SetMaxOpenFilesSoftLimit(GetMaxOpenFilesHardLimit());

  event_processor_.SetDiscardedOutOfOrderCounter(&stats_.discarded_out_of_order_count);

  InitLostAndDiscardedEventVisitor();

  bool perf_event_open_errors = false;
  std::vector<std::string> perf_event_open_error_details;

  if (bool opened = OpenMmapTask(all_cpus); !opened) {
    perf_event_open_error_details.emplace_back("mmap events, fork and exit events");
    perf_event_open_errors = true;
  }

  if (!instrumented_functions_.empty()) {
    if (bool opened = OpenUserSpaceProbes(cpuset_cpus); !opened) {
      perf_event_open_error_details.emplace_back("u(ret)probes");
      perf_event_open_errors = true;
    }
  }

  // This takes an initial snapshot of the maps. Note that, if at least one
  // function is dynamically instrumented, the snapshot might or might not
  // already contain the [uprobes] map entry. This depends on whether at least
  // one of those functions has already been called after the corresponding
  // uprobes file descriptor has been opened by OpenUserSpaceProbes (opening is
  // enough, it doesn't need to have been enabled).
  InitUprobesEventVisitor();

  if (sampling_period_ns_.has_value()) {
    if (bool opened = OpenSampling(cpuset_cpus); !opened) {
      perf_event_open_error_details.emplace_back("sampling");
      perf_event_open_errors = true;
    }
  }

  InitSwitchesStatesNamesVisitor();
  if (bool opened = OpenThreadNameTracepoints(all_cpus); !opened) {
    perf_event_open_error_details.emplace_back(
        "task:task_newtask and task:task_rename tracepoints");
    perf_event_open_errors = true;
  }
  if (trace_context_switches_ || trace_thread_state_) {
    if (bool opened = OpenContextSwitchAndThreadStateTracepoints(all_cpus); !opened) {
      perf_event_open_error_details.emplace_back(
          "sched:sched_switch and sched:sched_wakeup tracepoints");
      perf_event_open_errors = true;
    }
  }

  if (trace_gpu_driver_) {
    // We want to trace all GPU activity, hence we pass 'all_cpus' here.
    if (OpenGpuTracepoints(all_cpus)) {
      InitGpuTracepointEventVisitor();
    } else {
      ORBIT_LOG("There were errors opening GPU tracepoint events");
    }
  }

  if (bool opened = OpenInstrumentedTracepoints(all_cpus); !opened) {
    perf_event_open_error_details.emplace_back("selected tracepoints");
    perf_event_open_errors = true;
  }

  if (perf_event_open_errors) {
    ORBIT_ERROR("With perf_event_open: did you forget to run as root?");
    ORBIT_LOG("In particular, there were errors with opening %s",
              absl::StrJoin(perf_event_open_error_details, ", "));
    orbit_grpc_protos::ErrorsWithPerfEventOpenEvent errors_with_perf_event_open_event;
    errors_with_perf_event_open_event.set_timestamp_ns(orbit_base::CaptureTimestampNs());
    for (std::string& detail : perf_event_open_error_details) {
      errors_with_perf_event_open_event.add_failed_to_open(std::move(detail));
    }
    listener_->OnErrorsWithPerfEventOpenEvent(std::move(errors_with_perf_event_open_event));
  }

  // Start recording events.
  for (int fd : tracing_fds_) {
    perf_event_enable(fd);
  }

  effective_capture_start_timestamp_ns_ = orbit_base::CaptureTimestampNs();

  ModulesSnapshot modules_snapshot;
  modules_snapshot.set_pid(target_pid_);
  modules_snapshot.set_timestamp_ns(effective_capture_start_timestamp_ns_);
  auto modules_or_error = orbit_object_utils::ReadModules(target_pid_);
  if (modules_or_error.has_value()) {
    const std::vector<ModuleInfo>& modules = modules_or_error.value();
    *modules_snapshot.mutable_modules() = {modules.begin(), modules.end()};
    listener_->OnModulesSnapshot(std::move(modules_snapshot));
  } else {
    ORBIT_ERROR("Unable to load modules for %d: %s", target_pid_,
                modules_or_error.error().message());
  }

  // Get the initial thread names to notify the listener_.
  // All ThreadName events generated by this call will have effective_capture_start_timestamp_ns_ as
  // timestamp. As these events will be the first events of the capture, this prevents later events
  // from having a lower timestamp. After all, the timestamp of the initial ThreadName events is
  // approximate.
  std::vector<ThreadName> thread_names =
      RetrieveInitialThreadNamesSystemWide(effective_capture_start_timestamp_ns_);

  ThreadNamesSnapshot thread_names_snapshot;
  thread_names_snapshot.set_timestamp_ns(effective_capture_start_timestamp_ns_);
  *thread_names_snapshot.mutable_thread_names() = {thread_names.begin(), thread_names.end()};

  listener_->OnThreadNamesSnapshot(std::move(thread_names_snapshot));

  // Get the initial association of tids to pids and pass it to switches_states_names_visitor_.
  RetrieveInitialTidToPidAssociationSystemWide();

  if (trace_thread_state_) {
    // Get the initial thread states and pass them to switches_states_names_visitor_.
    RetrieveInitialThreadStatesOfTarget();
  }

  stats_.Reset();
}

void TracerImpl::Shutdown() {
  ORBIT_SCOPE_FUNCTION;
  if (trace_thread_state_) {
    switches_states_names_visitor_->ProcessRemainingOpenStates(orbit_base::CaptureTimestampNs());
  }

  // Stop recording.
  for (int fd : tracing_fds_) {
    perf_event_disable(fd);
  }

  // Close the ring buffers.
  {
    ORBIT_SCOPE("ring_buffers_.clear()");
    ring_buffers_.clear();
  }

  // Close the file descriptors.
  {
    ORBIT_SCOPE_WITH_COLOR(
        absl::StrFormat("Closing %d file descriptors", tracing_fds_.size()).c_str(),
        kOrbitColorRed);
    ORBIT_SCOPED_TIMED_LOG("Closing %d file descriptors", tracing_fds_.size());
    for (int fd : tracing_fds_) {
      ORBIT_SCOPE("Closing fd");
      close(fd);
    }
  }
}

void TracerImpl::ProcessOneRecord(PerfEventRingBuffer* ring_buffer) {
  uint64_t event_timestamp_ns = 0;

  perf_event_header header;
  ring_buffer->ReadHeader(&header);

  // perf_event_header::type contains the type of record, e.g.,
  // PERF_RECORD_SAMPLE, PERF_RECORD_MMAP, etc., defined in enum
  // perf_event_type in linux/perf_event.h.
  switch (header.type) {
    case PERF_RECORD_SWITCH:
      ORBIT_ERROR("Unexpected PERF_RECORD_SWITCH in ring buffer '%s'", ring_buffer->GetName());
      break;
    case PERF_RECORD_SWITCH_CPU_WIDE:
      ORBIT_ERROR("Unexpected PERF_RECORD_SWITCH_CPU_WIDE in ring buffer '%s'",
                  ring_buffer->GetName());
      break;
    case PERF_RECORD_FORK:
      event_timestamp_ns = ProcessForkEventAndReturnTimestamp(header, ring_buffer);
      break;
    case PERF_RECORD_EXIT:
      event_timestamp_ns = ProcessExitEventAndReturnTimestamp(header, ring_buffer);
      break;
    case PERF_RECORD_MMAP:
      event_timestamp_ns = ProcessMmapEventAndReturnTimestamp(header, ring_buffer);
      break;
    case PERF_RECORD_SAMPLE:
      event_timestamp_ns = ProcessSampleEventAndReturnTimestamp(header, ring_buffer);
      break;
    case PERF_RECORD_LOST:
      event_timestamp_ns = ProcessLostEventAndReturnTimestamp(header, ring_buffer);
      break;
    case PERF_RECORD_THROTTLE:
    case PERF_RECORD_UNTHROTTLE:
      event_timestamp_ns = ProcessThrottleUnthrottleEventAndReturnTimestamp(header, ring_buffer);
      break;
    default:
      ORBIT_ERROR("Unexpected perf_event_header::type in ring buffer '%s': %u",
                  ring_buffer->GetName(), header.type);
      ring_buffer->SkipRecord(header);
      break;
  }

  if (event_timestamp_ns != 0) {
    fds_to_last_timestamp_ns_.insert_or_assign(ring_buffer->GetFileDescriptor(),
                                               event_timestamp_ns);
  }
}

void TracerImpl::Run() {
  orbit_base::SetCurrentThreadName("Tracer::Run");

  Startup();

  bool last_iteration_saw_events = false;
  std::thread deferred_events_thread(&TracerImpl::ProcessDeferredEvents, this);

  while (!stop_run_thread_) {
    ORBIT_SCOPE("TracerThread::Run iteration");

    if (!last_iteration_saw_events) {
      // Periodically print event statistics.
      PrintStatsIfTimerElapsed();

      // Sleep if there was no new event in the last iteration so that we are
      // not constantly polling. Don't sleep so long that ring buffers overflow.
      {
        ORBIT_SCOPE("Sleep");
        usleep(IDLE_TIME_ON_EMPTY_RING_BUFFERS_US);
      }
    }

    last_iteration_saw_events = false;

    // Read and process events from all ring buffers. In order to ensure that no
    // buffer is read constantly while others overflow, we schedule the reading
    // using round-robin like scheduling.
    for (PerfEventRingBuffer& ring_buffer : ring_buffers_) {
      if (stop_run_thread_) {
        break;
      }

      // Read up to ROUND_ROBIN_POLLING_BATCH_SIZE (5) new events.
      // TODO: Some event types (e.g., stack samples) have a much longer
      //  processing time but are less frequent than others (e.g., context
      //  switches). Take this into account in our scheduling algorithm.
      for (int32_t read_from_this_buffer = 0;
           read_from_this_buffer < ROUND_ROBIN_POLLING_BATCH_SIZE; ++read_from_this_buffer) {
        if (stop_run_thread_) {
          break;
        }
        if (!ring_buffer.HasNewData()) {
          break;
        }

        last_iteration_saw_events = true;
        ProcessOneRecord(&ring_buffer);
      }
    }
  }

  // Finish processing all deferred events.
  stop_deferred_thread_ = true;
  deferred_events_thread.join();
  event_processor_.ProcessAllEvents();

  Shutdown();
}

uint64_t TracerImpl::ProcessForkEventAndReturnTimestamp(const perf_event_header& header,
                                                        PerfEventRingBuffer* ring_buffer) {
  perf_event_fork_exit ring_buffer_record;
  ring_buffer->ConsumeRecord(header, &ring_buffer_record);
  ForkPerfEvent event{
      .timestamp = ring_buffer_record.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(ring_buffer_record.pid),
              .tid = static_cast<pid_t>(ring_buffer_record.tid),
          },
  };

  if (event.timestamp < effective_capture_start_timestamp_ns_) {
    return event.timestamp;
  }

  DeferEvent(event);
  return event.timestamp;
}

uint64_t TracerImpl::ProcessExitEventAndReturnTimestamp(const perf_event_header& header,
                                                        PerfEventRingBuffer* ring_buffer) {
  perf_event_fork_exit ring_buffer_record;
  ring_buffer->ConsumeRecord(header, &ring_buffer_record);
  ExitPerfEvent event{
      .timestamp = ring_buffer_record.time,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .pid = static_cast<pid_t>(ring_buffer_record.pid),
              .tid = static_cast<pid_t>(ring_buffer_record.tid),
          },
  };

  if (event.timestamp < effective_capture_start_timestamp_ns_) {
    return event.timestamp;
  }

  DeferEvent(event);
  return event.timestamp;
}

uint64_t TracerImpl::ProcessMmapEventAndReturnTimestamp(const perf_event_header& header,
                                                        PerfEventRingBuffer* ring_buffer) {
  MmapPerfEvent event = ConsumeMmapPerfEvent(ring_buffer, header);
  const uint64_t timestamp_ns = event.timestamp;

  if (event.data.pid != target_pid_) {
    return timestamp_ns;
  }

  if (event.timestamp < effective_capture_start_timestamp_ns_) {
    return timestamp_ns;
  }

  DeferEvent(std::move(event));

  return timestamp_ns;
}

uint64_t TracerImpl::ProcessSampleEventAndReturnTimestamp(const perf_event_header& header,
                                                          PerfEventRingBuffer* ring_buffer) {
  uint64_t timestamp_ns = ReadSampleRecordTime(ring_buffer);

  if (timestamp_ns < effective_capture_start_timestamp_ns_) {
    // Don't consider events that came before all file descriptors had been enabled.
    ring_buffer->SkipRecord(header);
    return timestamp_ns;
  }

  uint64_t stream_id = ReadSampleRecordStreamId(ring_buffer);
  bool is_uprobe = uprobes_ids_.contains(stream_id);
  bool is_uprobe_with_args = uprobes_with_args_ids_.contains(stream_id);
  bool is_uretprobe = uretprobes_ids_.contains(stream_id);
  bool is_uretprobe_with_retval = uretprobes_with_retval_ids_.contains(stream_id);
  bool is_stack_sample = stack_sampling_ids_.contains(stream_id);
  bool is_callchain_sample = callchain_sampling_ids_.contains(stream_id);
  bool is_task_newtask = task_newtask_ids_.contains(stream_id);
  bool is_task_rename = task_rename_ids_.contains(stream_id);
  bool is_sched_switch = sched_switch_ids_.contains(stream_id);
  bool is_sched_wakeup = sched_wakeup_ids_.contains(stream_id);
  bool is_amdgpu_cs_ioctl_event = amdgpu_cs_ioctl_ids_.contains(stream_id);
  bool is_amdgpu_sched_run_job_event = amdgpu_sched_run_job_ids_.contains(stream_id);
  bool is_dma_fence_signaled_event = dma_fence_signaled_ids_.contains(stream_id);
  bool is_user_instrumented_tracepoint = ids_to_tracepoint_info_.contains(stream_id);

  ORBIT_CHECK(is_uprobe + is_uretprobe + is_stack_sample + is_callchain_sample + is_task_newtask +
                  is_task_rename + is_sched_switch + is_sched_wakeup + is_amdgpu_cs_ioctl_event +
                  is_amdgpu_sched_run_job_event + is_dma_fence_signaled_event +
                  is_user_instrumented_tracepoint <=
              1);

  int fd = ring_buffer->GetFileDescriptor();

  if (is_uprobe) {
    ORBIT_CHECK(header.size == sizeof(perf_event_sp_ip_8bytes_sample));
    perf_event_sp_ip_8bytes_sample ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);

    if (static_cast<pid_t>(ring_buffer_record.sample_id.pid) != target_pid_) {
      return timestamp_ns;
    }

    UprobesPerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                .pid = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
                .tid = static_cast<pid_t>(ring_buffer_record.sample_id.tid),
                .cpu = ring_buffer_record.sample_id.cpu,
                .function_id = uprobes_uretprobes_ids_to_function_id_.at(
                    ring_buffer_record.sample_id.stream_id),
                .sp = ring_buffer_record.regs.sp,
                .ip = ring_buffer_record.regs.ip,
                .return_address = ring_buffer_record.stack.top8bytes,
            },
    };

    DeferEvent(event);
    ++stats_.uprobes_count;

  } else if (is_uprobe_with_args) {
    ORBIT_CHECK(header.size == sizeof(perf_event_sp_ip_arguments_8bytes_sample));
    perf_event_sp_ip_arguments_8bytes_sample ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);

    if (static_cast<pid_t>(ring_buffer_record.sample_id.pid) != target_pid_) {
      return timestamp_ns;
    }

    UprobesWithArgumentsPerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                .pid = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
                .tid = static_cast<pid_t>(ring_buffer_record.sample_id.tid),
                .cpu = ring_buffer_record.sample_id.cpu,
                .function_id = uprobes_uretprobes_ids_to_function_id_.at(
                    ring_buffer_record.sample_id.stream_id),
                .return_address = ring_buffer_record.stack.top8bytes,
                .regs = ring_buffer_record.regs,
            },
    };

    DeferEvent(event);
    ++stats_.uprobes_count;

  } else if (is_uretprobe) {
    ORBIT_CHECK(header.size == sizeof(perf_event_empty_sample));
    perf_event_empty_sample ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);

    if (static_cast<pid_t>(ring_buffer_record.sample_id.pid) != target_pid_) {
      return timestamp_ns;
    }

    UretprobesPerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                .pid = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
                .tid = static_cast<pid_t>(ring_buffer_record.sample_id.tid),
            },
    };

    DeferEvent(event);
    ++stats_.uprobes_count;

  } else if (is_uretprobe_with_retval) {
    ORBIT_CHECK(header.size == sizeof(perf_event_ax_sample));
    perf_event_ax_sample ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);

    if (static_cast<pid_t>(ring_buffer_record.sample_id.pid) != target_pid_) {
      return timestamp_ns;
    }

    UretprobesWithReturnValuePerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                .pid = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
                .tid = static_cast<pid_t>(ring_buffer_record.sample_id.tid),
                .rax = ring_buffer_record.regs.ax,
            },
    };
    DeferEvent(event);
    ++stats_.uprobes_count;

  } else if (is_stack_sample) {
    pid_t pid = ReadSampleRecordPid(ring_buffer);

    const size_t size_of_stack_sample = sizeof(perf_event_stack_sample_fixed) +
                                        2 * sizeof(uint64_t) /*size and dyn_size*/ +
                                        stack_dump_size_ /*data*/;

    if (header.size != size_of_stack_sample) {
      // Skip stack samples that have an unexpected size. These normally have
      // abi == PERF_SAMPLE_REGS_ABI_NONE and no registers, and size == 0 and
      // no stack. Usually, these samples have pid == tid == 0, but that's not
      // always the case: for example, when a process exits while tracing, we
      // might get a stack sample with pid and tid != 0 but still with
      // abi == PERF_SAMPLE_REGS_ABI_NONE and size == 0.
      ring_buffer->SkipRecord(header);
      return timestamp_ns;
    }
    if (pid != target_pid_) {
      ring_buffer->SkipRecord(header);
      return timestamp_ns;
    }
    // Do *not* filter out samples based on header.misc,
    // e.g., with header.misc == PERF_RECORD_MISC_KERNEL,
    // in general they seem to produce valid callstacks.

    StackSamplePerfEvent event = ConsumeStackSamplePerfEvent(ring_buffer, header);
    DeferEvent(std::move(event));
    ++stats_.sample_count;

  } else if (is_callchain_sample) {
    pid_t pid = ReadSampleRecordPid(ring_buffer);

    if (pid != target_pid_) {
      ring_buffer->SkipRecord(header);
      return timestamp_ns;
    }

    PerfEvent event = ConsumeCallchainSamplePerfEvent(ring_buffer, header);
    DeferEvent(std::move(event));
    ++stats_.sample_count;

  } else if (is_task_newtask) {
    ORBIT_CHECK(header.size == sizeof(perf_event_raw_sample<task_newtask_tracepoint>));
    perf_event_raw_sample<task_newtask_tracepoint> ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);
    TaskNewtaskPerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                // The tracepoint format calls the new tid "data.pid" but it's effectively the
                // thread id.
                // Note that ring_buffer_record.sample_id.pid and ring_buffer_record.sample_id.tid
                // are NOT the pid and tid of the new process/thread, but the ones of the
                // process/thread that created this one.
                .new_tid = ring_buffer_record.data.pid,
            },
    };
    memcpy(event.data.comm, ring_buffer_record.data.comm, 16);
    DeferEvent(event);

  } else if (is_task_rename) {
    ORBIT_CHECK(header.size == sizeof(perf_event_raw_sample<task_rename_tracepoint>));
    perf_event_raw_sample<task_rename_tracepoint> ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);

    TaskRenamePerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                // The tracepoint format calls the renamed tid "data.pid" but it's effectively the
                // thread id. This should match ring_buffer_record.sample_id.tid.
                .renamed_tid = ring_buffer_record.data.pid,
            },
    };

    memcpy(event.data.newcomm, ring_buffer_record.data.newcomm, 16);
    DeferEvent(event);

  } else if (is_sched_switch) {
    ORBIT_CHECK(header.size == sizeof(perf_event_raw_sample<sched_switch_tracepoint>));
    perf_event_raw_sample<sched_switch_tracepoint> ring_buffer_record;
    ring_buffer->ConsumeRecord(header, &ring_buffer_record);

    SchedSwitchPerfEvent event{
        .timestamp = ring_buffer_record.sample_id.time,
        .ordered_stream = PerfEventOrderedStream::FileDescriptor(fd),
        .data =
            {
                .cpu = ring_buffer_record.sample_id.cpu,
                // As the tracepoint data does not include the pid of the process that the thread
                // being switched out belongs to, we use the pid set by perf_event_open in the
                // corresponding generic field of the PERF_RECORD_SAMPLE.
                // Note, though, that this value is -1 when the switch out is caused by the thread
                // exiting. This is not the case for data.prev_pid, whose value is always correct as
                // it comes directly from the tracepoint data.
                .prev_pid_or_minus_one = static_cast<pid_t>(ring_buffer_record.sample_id.pid),
                .prev_tid = ring_buffer_record.data.prev_pid,
                .prev_state = ring_buffer_record.data.prev_state,
                .next_tid = ring_buffer_record.data.next_pid,
            },
    };
    DeferEvent(event);
    ++stats_.sched_switch_count;

  } else if (is_sched_wakeup) {
    SchedWakeupPerfEvent event = ConsumeSchedWakeupPerfEvent(ring_buffer, header);
    DeferEvent(event);

  } else if (is_amdgpu_cs_ioctl_event) {
    AmdgpuCsIoctlPerfEvent event = ConsumeAmdgpuCsIoctlPerfEvent(ring_buffer, header);
    DeferEvent(std::move(event));
    ++stats_.gpu_events_count;

  } else if (is_amdgpu_sched_run_job_event) {
    AmdgpuSchedRunJobPerfEvent event = ConsumeAmdgpuSchedRunJobPerfEvent(ring_buffer, header);
    DeferEvent(std::move(event));
    ++stats_.gpu_events_count;

  } else if (is_dma_fence_signaled_event) {
    DmaFenceSignaledPerfEvent event = ConsumeDmaFenceSignaledPerfEvent(ring_buffer, header);
    DeferEvent(std::move(event));
    ++stats_.gpu_events_count;

  } else if (is_user_instrumented_tracepoint) {
    auto it = ids_to_tracepoint_info_.find(stream_id);
    if (it == ids_to_tracepoint_info_.end()) {
      return timestamp_ns;
    }

    GenericTracepointPerfEvent event = ConsumeGenericTracepointPerfEvent(ring_buffer, header);

    orbit_grpc_protos::FullTracepointEvent tracepoint_event;
    tracepoint_event.set_pid(event.data.pid);
    tracepoint_event.set_tid(event.data.tid);
    tracepoint_event.set_timestamp_ns(event.timestamp);
    tracepoint_event.set_cpu(event.data.cpu);
    orbit_grpc_protos::TracepointInfo* tracepoint_info = tracepoint_event.mutable_tracepoint_info();
    tracepoint_info->set_name(it->second.name());
    tracepoint_info->set_category(it->second.category());

    listener_->OnTracepointEvent(std::move(tracepoint_event));
  } else {
    ORBIT_ERROR("PERF_EVENT_SAMPLE with unexpected stream_id: %lu", stream_id);
    ring_buffer->SkipRecord(header);
  }

  return timestamp_ns;
}

uint64_t TracerImpl::ProcessLostEventAndReturnTimestamp(const perf_event_header& header,
                                                        PerfEventRingBuffer* ring_buffer) {
  perf_event_lost ring_buffer_record;
  ring_buffer->ConsumeRecord(header, &ring_buffer_record);
  uint64_t timestamp = ring_buffer_record.sample_id.time;

  stats_.lost_count += ring_buffer_record.lost;
  stats_.lost_count_per_buffer[ring_buffer] += ring_buffer_record.lost;

  // Fetch the timestamp of the last event that preceded this PERF_RECORD_LOST in this same ring
  // buffer.
  uint64_t fd_previous_timestamp_ns = 0;
  if (auto it = fds_to_last_timestamp_ns_.find(ring_buffer->GetFileDescriptor());
      it != fds_to_last_timestamp_ns_.end()) {
    fd_previous_timestamp_ns = it->second;
  }
  if (fd_previous_timestamp_ns == 0) {
    // This shouldn't happen because PERF_RECORD_LOST is reported when a ring buffer is full, which
    // means that there were other events in the same ring buffers, and they have already been read.
    ORBIT_ERROR("Unknown previous timestamp for ring buffer '%s'", ring_buffer->GetName());
    return timestamp;
  }

  LostPerfEvent event{
      .timestamp = timestamp,
      .ordered_stream = PerfEventOrderedStream::FileDescriptor(ring_buffer->GetFileDescriptor()),
      .data =
          {
              .previous_timestamp = fd_previous_timestamp_ns,
          },
  };
  DeferEvent(event);

  return timestamp;
}

uint64_t TracerImpl::ProcessThrottleUnthrottleEventAndReturnTimestamp(
    const perf_event_header& header, PerfEventRingBuffer* ring_buffer) {
  // Throttle/unthrottle events are reported when sampling causes too much throttling on the CPU.
  // They are usually caused by/reproducible with a very high sampling frequency.
  uint64_t timestamp_ns = ReadThrottleUnthrottleRecordTime(ring_buffer);

  ring_buffer->SkipRecord(header);

  // Simply log throttle/unthrottle events. If they are generated, they are quite low frequency.
  switch (header.type) {
    case PERF_RECORD_THROTTLE:
      ORBIT_LOG("PERF_RECORD_THROTTLE in ring buffer '%s' at timestamp %u", ring_buffer->GetName(),
                timestamp_ns);
      break;
    case PERF_RECORD_UNTHROTTLE:
      ORBIT_LOG("PERF_RECORD_UNTHROTTLE in ring buffer '%s' at timestamp %u",
                ring_buffer->GetName(), timestamp_ns);
      break;
    default:
      ORBIT_UNREACHABLE();
  }

  return timestamp_ns;
}

void TracerImpl::DeferEvent(PerfEvent&& event) {
  absl::MutexLock lock{&deferred_events_being_buffered_mutex_};
  deferred_events_being_buffered_.emplace_back(std::move(event));
}

void TracerImpl::ProcessDeferredEvents() {
  orbit_base::SetCurrentThreadName("Proc.Def.Events");
  bool should_exit = false;
  while (!should_exit) {
    ORBIT_SCOPE("ProcessDeferredEvents iteration");
    // When "should_exit" becomes true, we know that we have stopped generating
    // deferred events. The last iteration will consume all remaining events.
    should_exit = stop_deferred_thread_;

    {
      absl::MutexLock lock{&deferred_events_being_buffered_mutex_};
      deferred_events_being_buffered_.swap(deferred_events_to_process_);
    }

    if (deferred_events_to_process_.empty()) {
      ORBIT_SCOPE("Sleep");
      usleep(IDLE_TIME_ON_EMPTY_DEFERRED_EVENTS_US);
      continue;
    }

    {
      ORBIT_SCOPE("AddEvents");
      for (auto& event : deferred_events_to_process_) {
        event_processor_.AddEvent(std::move(event));
      }
    }
    // Note (https://en.cppreference.com/w/cpp/container/vector/clear): std::vector::clear() "Leaves
    // the capacity() of the vector unchanged", which is desired as deferred_events_being_buffered_
    // won't have to be grown again after the swap.
    deferred_events_to_process_.clear();
    {
      ORBIT_SCOPE("ProcessOldEvents");
      event_processor_.ProcessOldEvents();
    }
  }
}

void TracerImpl::RetrieveInitialTidToPidAssociationSystemWide() {
  for (pid_t pid : GetAllPids()) {
    for (pid_t tid : GetTidsOfProcess(pid)) {
      switches_states_names_visitor_->ProcessInitialTidToPidAssociation(tid, pid);
    }
  }
}

void TracerImpl::RetrieveInitialThreadStatesOfTarget() {
  for (pid_t tid : GetTidsOfProcess(target_pid_)) {
    uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
    std::optional<char> state = GetThreadState(tid);
    if (!state.has_value()) {
      continue;
    }
    switches_states_names_visitor_->ProcessInitialState(timestamp_ns, tid, state.value());
  }
}

void TracerImpl::Reset() {
  ORBIT_SCOPE_FUNCTION;
  tracing_fds_.clear();
  ring_buffers_.clear();
  fds_to_last_timestamp_ns_.clear();

  uprobes_uretprobes_ids_to_function_id_.clear();
  uprobes_ids_.clear();
  uprobes_with_args_ids_.clear();
  uretprobes_ids_.clear();
  uretprobes_with_retval_ids_.clear();
  stack_sampling_ids_.clear();
  callchain_sampling_ids_.clear();
  task_newtask_ids_.clear();
  task_rename_ids_.clear();
  sched_switch_ids_.clear();
  sched_wakeup_ids_.clear();
  amdgpu_cs_ioctl_ids_.clear();
  amdgpu_sched_run_job_ids_.clear();
  dma_fence_signaled_ids_.clear();
  ids_to_tracepoint_info_.clear();

  effective_capture_start_timestamp_ns_ = 0;

  stop_deferred_thread_ = false;
  {
    absl::MutexLock lock{&deferred_events_being_buffered_mutex_};
    deferred_events_being_buffered_.clear();
  }
  deferred_events_to_process_.clear();
  uprobes_unwinding_visitor_.reset();
  leaf_function_call_manager_.reset();
  return_address_manager_.reset();
  switches_states_names_visitor_.reset();
  gpu_event_visitor_.reset();
  event_processor_.ClearVisitors();
}

void TracerImpl::PrintStatsIfTimerElapsed() {
  ORBIT_SCOPE_FUNCTION;
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  if (stats_.event_count_begin_ns + EVENT_STATS_WINDOW_S * NS_PER_SECOND >= timestamp_ns) {
    return;
  }

  double actual_window_s =
      static_cast<double>(timestamp_ns - stats_.event_count_begin_ns) / NS_PER_SECOND;
  ORBIT_CHECK(actual_window_s > 0.0);

  ORBIT_LOG("Events per second (and total) last %.3f s:", actual_window_s);
  ORBIT_LOG("  sched switches: %.0f/s (%lu)", stats_.sched_switch_count / actual_window_s,
            stats_.sched_switch_count);
  ORBIT_LOG("  samples: %.0f/s (%lu)", stats_.sample_count / actual_window_s, stats_.sample_count);
  ORBIT_LOG("  u(ret)probes: %.0f/s (%lu)", stats_.uprobes_count / actual_window_s,
            stats_.uprobes_count);
  ORBIT_LOG("  gpu events: %.0f/s (%lu)", stats_.gpu_events_count / actual_window_s,
            stats_.gpu_events_count);

  if (stats_.lost_count_per_buffer.empty()) {
    ORBIT_LOG("  lost: %.0f/s (%lu)", stats_.lost_count / actual_window_s, stats_.lost_count);
  } else {
    ORBIT_LOG("  LOST: %.0f/s (%lu), of which:", stats_.lost_count / actual_window_s,
              stats_.lost_count);
    for (const auto& buffer_and_lost_count : stats_.lost_count_per_buffer) {
      ORBIT_LOG("    from %s: %.0f/s (%lu)", buffer_and_lost_count.first->GetName().c_str(),
                buffer_and_lost_count.second / actual_window_s, buffer_and_lost_count.second);
    }
  }

  uint64_t discarded_out_of_order_count = stats_.discarded_out_of_order_count;
  ORBIT_LOG(
      "  %s: %.0f/s (%lu)",
      discarded_out_of_order_count == 0 ? "discarded as out of order" : "DISCARDED AS OUT OF ORDER",
      discarded_out_of_order_count / actual_window_s, discarded_out_of_order_count);

  // Ensure we can divide by 0.0 safely in case stats_.sample_count is zero.
  static_assert(std::numeric_limits<double>::is_iec559);

  uint64_t unwind_error_count = stats_.unwind_error_count;
  ORBIT_LOG("  unwind errors: %.0f/s (%lu) [%.1f%%]", unwind_error_count / actual_window_s,
            unwind_error_count, 100.0 * unwind_error_count / stats_.sample_count);
  uint64_t discarded_samples_in_uretprobes_count = stats_.samples_in_uretprobes_count;
  ORBIT_LOG("  samples in u(ret)probes: %.0f/s (%lu) [%.1f%%]",
            discarded_samples_in_uretprobes_count / actual_window_s,
            discarded_samples_in_uretprobes_count,
            100.0 * discarded_samples_in_uretprobes_count / stats_.sample_count);

  uint64_t thread_state_count = stats_.thread_state_count;
  ORBIT_LOG("  target's thread states: %.0f/s (%lu)", thread_state_count / actual_window_s,
            thread_state_count);
  stats_.Reset();
}

}  // namespace orbit_linux_tracing
