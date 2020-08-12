// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracerThread.h"

#include <OrbitBase/Logging.h>
#include <OrbitBase/Tracing.h>

#include <thread>

#include "UprobesUnwindingVisitor.h"
#include "absl/strings/str_format.h"

namespace LinuxTracing {

TracerThread::TracerThread(const CaptureOptions& capture_options)
    : trace_context_switches_{capture_options.trace_context_switches()},
      pid_{capture_options.pid()},
      unwinding_method_{capture_options.unwinding_method()},
      trace_gpu_driver_{capture_options.trace_gpu_driver()} {
  if (unwinding_method_ != CaptureOptions::kUndefined) {
    std::optional<uint64_t> sampling_period_ns =
        ComputeSamplingPeriodNs(capture_options.sampling_rate());
    FAIL_IF(!sampling_period_ns.has_value(), "Invalid sampling rate: %.1f",
            capture_options.sampling_rate());
    sampling_period_ns_ = sampling_period_ns.value();
  } else {
    sampling_period_ns_ = 0;
  }

  instrumented_functions_.clear();
  instrumented_functions_.reserve(
      capture_options.instrumented_functions_size());

  for (const CaptureOptions::InstrumentedFunction& instrumented_function :
       capture_options.instrumented_functions()) {
    uint64_t absolute_address = instrumented_function.absolute_address();
    instrumented_functions_.emplace_back(instrumented_function.file_path(),
                                         instrumented_function.file_offset(),
                                         absolute_address);

    // Manual instrumentation.
    if (instrumented_function.function_type() ==
        CaptureOptions_InstrumentedFunction::kTimerStart) {
      manual_instrumentation_config_.AddTimerStartAddress(absolute_address);
    } else if (instrumented_function.function_type() ==
               CaptureOptions_InstrumentedFunction::kTimerStop) {
      manual_instrumentation_config_.AddTimerStopAddress(absolute_address);
    }
  }
}

namespace {
void CloseFileDescriptors(const std::vector<int>& fds) {
  for (int fd : fds) {
    close(fd);
  }
}

void CloseFileDescriptors(
    const absl::flat_hash_map<int32_t, int>& fds_per_cpu) {
  for (const auto& pair : fds_per_cpu) {
    close(pair.second);
  }
}
}  // namespace

bool TracerThread::OpenContextSwitches(const std::vector<int32_t>& cpus) {
  std::vector<int> context_switch_tracing_fds;
  std::vector<PerfEventRingBuffer> context_switch_ring_buffers;
  for (int32_t cpu : cpus) {
    int context_switch_fd = context_switch_event_open(-1, cpu);
    std::string buffer_name = absl::StrFormat("context_switch_%d", cpu);
    PerfEventRingBuffer context_switch_ring_buffer{
        context_switch_fd, CONTEXT_SWITCHES_RING_BUFFER_SIZE_KB, buffer_name};
    if (context_switch_ring_buffer.IsOpen()) {
      context_switch_tracing_fds.push_back(context_switch_fd);
      context_switch_ring_buffers.push_back(
          std::move(context_switch_ring_buffer));
    } else {
      ERROR("Opening context switch events for cpu %d", cpu);
      CloseFileDescriptors(context_switch_tracing_fds);
      return false;
    }
  }

  for (int fd : context_switch_tracing_fds) {
    tracing_fds_.push_back(fd);
  }
  for (PerfEventRingBuffer& buffer : context_switch_ring_buffers) {
    ring_buffers_.emplace_back(std::move(buffer));
  }
  return true;
}

void TracerThread::InitUprobesEventProcessor() {
  auto uprobes_unwinding_visitor =
      std::make_unique<UprobesUnwindingVisitor>(ReadMaps(pid_));
  uprobes_unwinding_visitor->SetListener(listener_);
  uprobes_unwinding_visitor->SetUnwindErrorsAndDiscardedSamplesCounters(
      stats_.unwind_error_count, stats_.discarded_samples_in_uretprobes_count);
  uprobes_event_processor_ = std::make_unique<PerfEventProcessor>(
      std::move(uprobes_unwinding_visitor));
}

bool TracerThread::OpenUprobes(const LinuxTracing::Function& function,
                               const std::vector<int32_t>& cpus,
                               absl::flat_hash_map<int32_t, int>* fds_per_cpu) {
  const char* module = function.BinaryPath().c_str();
  const uint64_t offset = function.FileOffset();
  for (int32_t cpu : cpus) {
    int fd = uprobes_retaddr_event_open(module, offset, -1, cpu);
    if (fd < 0) {
      ERROR("Opening uprobe 0x%lx on cpu %d", function.VirtualAddress(), cpu);
      return false;
    }
    (*fds_per_cpu)[cpu] = fd;
  }
  return true;
}

bool TracerThread::OpenUretprobes(
    const LinuxTracing::Function& function, const std::vector<int32_t>& cpus,
    absl::flat_hash_map<int32_t, int>* fds_per_cpu) {
  const char* module = function.BinaryPath().c_str();
  const uint64_t offset = function.FileOffset();
  for (int32_t cpu : cpus) {
    int fd = uretprobes_event_open(module, offset, -1, cpu);
    if (fd < 0) {
      ERROR("Opening uretprobe 0x%lx on cpu %d", function.VirtualAddress(),
            cpu);
      return false;
    }
    (*fds_per_cpu)[cpu] = fd;
  }
  return true;
}

void TracerThread::AddUprobesFileDescriptors(
    const absl::flat_hash_map<int32_t, int>& uprobes_fds_per_cpu,
    const LinuxTracing::Function& function) {
  for (const auto [cpu, fd] : uprobes_fds_per_cpu) {
    uint64_t stream_id = perf_event_get_id(fd);
    uprobes_uretprobes_ids_to_function_.emplace(stream_id, &function);
    uprobes_ids_.insert(stream_id);
    tracing_fds_.push_back(fd);
    fds_per_cpu_[cpu].push_back(fd);
  }
}

void TracerThread::AddUretprobesFileDescriptors(
    const absl::flat_hash_map<int32_t, int>& uretprobes_fds_per_cpu,
    const LinuxTracing::Function& function) {
  for (const auto [cpu, fd] : uretprobes_fds_per_cpu) {
    uint64_t stream_id = perf_event_get_id(fd);
    uprobes_uretprobes_ids_to_function_.emplace(stream_id, &function);
    uretprobes_ids_.insert(stream_id);
    tracing_fds_.push_back(fd);
    fds_per_cpu_[cpu].push_back(fd);
  }
}

bool TracerThread::OpenUserSpaceProbes(const std::vector<int32_t>& cpus) {
  bool uprobes_event_open_errors = false;

  for (const auto& function : instrumented_functions_) {
    absl::flat_hash_map<int32_t, int> uprobes_fds_per_cpu;
    absl::flat_hash_map<int32_t, int> uretprobes_fds_per_cpu;
    uint64_t address = function.VirtualAddress();

    if (manual_instrumentation_config_.IsTimerStartAddress(address)) {
      // Only open uprobes for a "timer start" manual instrumentation function.
      if (!OpenUprobes(function, cpus, &uprobes_fds_per_cpu)) {
        CloseFileDescriptors(uprobes_fds_per_cpu);
        uprobes_event_open_errors = true;
        continue;
      }
    } else if (manual_instrumentation_config_.IsTimerStopAddress(address)) {
      // Only open uretprobes for a "timer stop" manual instrumentation
      // function.
      if (!OpenUretprobes(function, cpus, &uretprobes_fds_per_cpu)) {
        CloseFileDescriptors(uretprobes_fds_per_cpu);
        uprobes_event_open_errors = true;
        continue;
      }
    } else {
      // Open both uprobes and uretprobes for regular functions.
      bool success = OpenUprobes(function, cpus, &uprobes_fds_per_cpu) &&
                     OpenUretprobes(function, cpus, &uretprobes_fds_per_cpu);
      if (!success) {
        CloseFileDescriptors(uprobes_fds_per_cpu);
        CloseFileDescriptors(uretprobes_fds_per_cpu);
        uprobes_event_open_errors = true;
        continue;
      }
    }

    // Uretprobe need to be enabled before uprobes as we support temporarily
    // not having a uprobe associated with a uretprobe but not the opposite.
    AddUretprobesFileDescriptors(uretprobes_fds_per_cpu, function);
    AddUprobesFileDescriptors(uprobes_fds_per_cpu, function);
  }

  OpenUserSpaceProbesRingBuffers();

  return !uprobes_event_open_errors;
}

void TracerThread::OpenUserSpaceProbesRingBuffers() {
  for (const auto& [/*int32_t*/ cpu, /*std::vector<int>*/ fds] : fds_per_cpu_) {
    if (fds.empty()) continue;

    // Create a single ring buffer per cpu.
    int ring_buffer_fd = fds[0];
    constexpr uint64_t buffer_size = UPROBES_RING_BUFFER_SIZE_KB;
    std::string buffer_name = absl::StrFormat("uprobes_uretprobes_%u", cpu);
    ring_buffers_.emplace_back(ring_buffer_fd, buffer_size, buffer_name);

    // Redirect subsequent fds to the cpu specific ring buffer created above.
    for (size_t i = 1; i < fds.size(); ++i) {
      perf_event_redirect(fds[i], ring_buffer_fd);
    }
  }
}

bool TracerThread::OpenMmapTask(const std::vector<int32_t>& cpus) {
  std::vector<int> mmap_task_tracing_fds;
  std::vector<PerfEventRingBuffer> mmap_task_ring_buffers;
  for (int32_t cpu : cpus) {
    int mmap_task_fd = mmap_task_event_open(-1, cpu);
    std::string buffer_name = absl::StrFormat("mmap_task_%d", cpu);
    PerfEventRingBuffer mmap_task_ring_buffer{
        mmap_task_fd, MMAP_TASK_RING_BUFFER_SIZE_KB, buffer_name};
    if (mmap_task_ring_buffer.IsOpen()) {
      mmap_task_tracing_fds.push_back(mmap_task_fd);
      mmap_task_ring_buffers.push_back(std::move(mmap_task_ring_buffer));
    } else {
      ERROR("Opening mmap, fork, and exit events for cpu %d", cpu);
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

bool TracerThread::OpenSampling(const std::vector<int32_t>& cpus) {
  std::vector<int> sampling_tracing_fds;
  std::vector<PerfEventRingBuffer> sampling_ring_buffers;
  for (int32_t cpu : cpus) {
    int sampling_fd;
    switch (unwinding_method_) {
      case CaptureOptions::kFramePointers:
        sampling_fd = callchain_sample_event_open(sampling_period_ns_, -1, cpu);
        break;
      case CaptureOptions::kDwarf:
        sampling_fd = stack_sample_event_open(sampling_period_ns_, -1, cpu);
        break;
      case CaptureOptions::kUndefined:
      default:
        UNREACHABLE();
        CloseFileDescriptors(sampling_tracing_fds);
        return false;
    }

    std::string buffer_name = absl::StrFormat("sampling_%d", cpu);
    PerfEventRingBuffer sampling_ring_buffer{
        sampling_fd, SAMPLING_RING_BUFFER_SIZE_KB, buffer_name};
    if (sampling_ring_buffer.IsOpen()) {
      sampling_tracing_fds.push_back(sampling_fd);
      sampling_ring_buffers.push_back(std::move(sampling_ring_buffer));
    } else {
      ERROR("Opening sampling for cpu %d", cpu);
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

void TracerThread::OpenRingBuffersOrRedirectOnExisting(
    const absl::flat_hash_map<int32_t, int>& fds_per_cpu,
    absl::flat_hash_map<int32_t, int>* ring_buffer_fds_per_cpu,
    std::vector<PerfEventRingBuffer>* ring_buffers,
    uint64_t ring_buffer_size_kb, std::string_view buffer_name_prefix) {
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
      std::string buffer_name =
          absl::StrFormat("%s_%d", buffer_name_prefix, cpu);
      ring_buffers->emplace_back(ring_buffer_fd, ring_buffer_size_kb,
                                 buffer_name);
      ring_buffer_fds_per_cpu->emplace(cpu, ring_buffer_fd);
    }
  }
}

bool TracerThread::OpenRingBuffersForTracepoint(
    const char* tracepoint_category, const char* tracepoint_name,
    const std::vector<int32_t>& cpus, std::vector<int>* tracing_fds,
    absl::flat_hash_set<uint64_t>* tracepoint_ids,
    absl::flat_hash_map<int32_t, int>* tracepoint_ring_buffer_fds_per_cpu,
    std::vector<PerfEventRingBuffer>* ring_buffers) {
  absl::flat_hash_map<int32_t, int> tracepoint_fds_per_cpu;
  for (int32_t cpu : cpus) {
    int fd =
        tracepoint_event_open(tracepoint_category, tracepoint_name, -1, cpu);
    if (fd < 0) {
      ERROR("Opening %s:%s tracepoint for cpu %d", tracepoint_category,
            tracepoint_name, cpu);
      for (const auto& open_fd : tracepoint_fds_per_cpu) {
        close(open_fd.second);
      }
      return false;
    }
    tracepoint_fds_per_cpu.emplace(cpu, fd);
  }

  for (const auto& fd : tracepoint_fds_per_cpu) {
    tracing_fds->push_back(fd.second);
    uint64_t stream_id = perf_event_get_id(fd.second);
    tracepoint_ids->insert(stream_id);
  }

  OpenRingBuffersOrRedirectOnExisting(
      tracepoint_fds_per_cpu, tracepoint_ring_buffer_fds_per_cpu, ring_buffers,
      TRACEPOINTS_RING_BUFFER_SIZE_KB, "tracepoints");
  return true;
}

bool TracerThread::OpenTracepoints(const std::vector<int32_t>& cpus) {
  bool tracepoint_event_open_errors = false;
  absl::flat_hash_map<int32_t, int> tracepoint_ring_buffer_fds_per_cpu;

  tracepoint_event_open_errors |= !OpenRingBuffersForTracepoint(
      "task", "task_newtask", cpus, &tracing_fds_, &task_newtask_ids_,
      &tracepoint_ring_buffer_fds_per_cpu, &ring_buffers_);

  tracepoint_event_open_errors |= !OpenRingBuffersForTracepoint(
      "task", "task_rename", cpus, &tracing_fds_, &task_rename_ids_,
      &tracepoint_ring_buffer_fds_per_cpu, &ring_buffers_);
  
  /*tracepoint_event_open_errors |= !OpenRingBuffersForTracepoint(
      "sched", "sched_switch", cpus, &tracing_fds_, &sched_switch_ids_,
      &tracepoint_ring_buffer_fds_per_cpu, &ring_buffers_);*/


  return !tracepoint_event_open_errors;
}

bool TracerThread::InitGpuTracepointEventProcessor() {
  gpu_event_processor_ = std::make_unique<GpuTracepointEventProcessor>();
  gpu_event_processor_->SetListener(listener_);
  return true;
}

// This method enables events for GPU event tracing. We trace three events that
// correspond to the following GPU driver events:
// - A GPU job (command buffer submission) is scheduled by the application. This
//   is tracked by the event "amdgpu_cs_ioctl".
// - A GPU job is scheduled to run on the hardware. This is tracked by the event
//   "amdgpu_sched_run_job".
// - A GPU job is finished by the hardware. This is tracked by the corresponding
//   DMA fence being signaled and is tracked by the event "dma_fence_signaled".
// A single job execution thus correponds to three events, one of each type
// above, that share the same timeline, context, and seqno.
// We have to record events system-wide (per CPU) to ensure we record all
// relevant events.
// This method returns true on success, otherwise false.
bool TracerThread::OpenGpuTracepoints(const std::vector<int32_t>& cpus) {
  absl::flat_hash_map<int32_t, int> amdgpu_cs_ioctl_fds_per_cpu;
  absl::flat_hash_map<int32_t, int> amdgpu_sched_run_job_fds_per_cpu;
  absl::flat_hash_map<int32_t, int> dma_fence_signaled_fds_per_cpu;
  bool tracepoint_event_open_errors = false;
  for (int32_t cpu : cpus) {
    int amdgpu_cs_ioctl_fd =
        tracepoint_event_open("amdgpu", "amdgpu_cs_ioctl", -1, cpu);
    if (amdgpu_cs_ioctl_fd == -1) {
      ERROR("Opening amdgpu:amdgpu_cs_ioctl tracepoint for cpu %d", cpu);
      tracepoint_event_open_errors = true;
      break;
    }
    amdgpu_cs_ioctl_fds_per_cpu.emplace(cpu, amdgpu_cs_ioctl_fd);

    int amdgpu_sched_run_job_fd =
        tracepoint_event_open("amdgpu", "amdgpu_sched_run_job", -1, cpu);
    if (amdgpu_sched_run_job_fd == -1) {
      ERROR("Opening amdgpu:amdgpu_sched_run_job tracepoint for cpu %d", cpu);
      tracepoint_event_open_errors = true;
      break;
    }
    amdgpu_sched_run_job_fds_per_cpu.emplace(cpu, amdgpu_sched_run_job_fd);

    int dma_fence_signaled_fd =
        tracepoint_event_open("dma_fence", "dma_fence_signaled", -1, cpu);
    if (dma_fence_signaled_fd == -1) {
      ERROR("Opening dma_fence:dma_fence_signaled tracepoint for cpu %d", cpu);
      tracepoint_event_open_errors = true;
      break;
    }
    dma_fence_signaled_fds_per_cpu.emplace(cpu, dma_fence_signaled_fd);
  }

  if (tracepoint_event_open_errors) {
    for (const auto& cpu_and_fd : amdgpu_cs_ioctl_fds_per_cpu) {
      close(cpu_and_fd.second);
    }
    for (const auto& cpu_and_fd : amdgpu_sched_run_job_fds_per_cpu) {
      close(cpu_and_fd.second);
    }
    for (const auto& cpu_and_fd : dma_fence_signaled_fds_per_cpu) {
      close(cpu_and_fd.second);
    }
    return false;
  }

  // Since all tracepoints could successfully be opened, we can now commit all
  // file descriptors and ring buffers to the TracerThread members.
  for (const auto& cpu_and_fd : amdgpu_cs_ioctl_fds_per_cpu) {
    tracing_fds_.push_back(cpu_and_fd.second);
    amdgpu_cs_ioctl_ids_.insert(perf_event_get_id(cpu_and_fd.second));
  }
  for (const auto& cpu_and_fd : amdgpu_sched_run_job_fds_per_cpu) {
    tracing_fds_.push_back(cpu_and_fd.second);
    amdgpu_sched_run_job_ids_.insert(perf_event_get_id(cpu_and_fd.second));
  }
  for (const auto& cpu_and_fd : dma_fence_signaled_fds_per_cpu) {
    tracing_fds_.push_back(cpu_and_fd.second);
    dma_fence_signaled_ids_.insert(perf_event_get_id(cpu_and_fd.second));
  }

  // Redirect on the same ring buffer all the three GPU events that are open on
  // each CPU.
  absl::flat_hash_map<int32_t, int> gpu_tracepoint_ring_buffer_fds_per_cpu;
  OpenRingBuffersOrRedirectOnExisting(
      amdgpu_cs_ioctl_fds_per_cpu, &gpu_tracepoint_ring_buffer_fds_per_cpu,
      &ring_buffers_, GPU_TRACING_RING_BUFFER_SIZE_KB,
      absl::StrFormat("%s:%s", "amdgpu", "amdgpu_cs_ioctl"));
  OpenRingBuffersOrRedirectOnExisting(
      amdgpu_sched_run_job_fds_per_cpu, &gpu_tracepoint_ring_buffer_fds_per_cpu,
      &ring_buffers_, GPU_TRACING_RING_BUFFER_SIZE_KB,
      absl::StrFormat("%s:%s", "amdgpu", "amdgpu_sched_run_job"));
  OpenRingBuffersOrRedirectOnExisting(
      dma_fence_signaled_fds_per_cpu, &gpu_tracepoint_ring_buffer_fds_per_cpu,
      &ring_buffers_, GPU_TRACING_RING_BUFFER_SIZE_KB,
      absl::StrFormat("%s:%s", "dma_fence", "dma_fence_signaled"));

  return true;
}

void TracerThread::Run(
    const std::shared_ptr<std::atomic<bool>>& exit_requested) {
  FAIL_IF(listener_ == nullptr, "No listener set");

  Reset();

  // perf_event_open refers to cores as "CPUs".

  // Record context switches from all cores for all processes.
  std::vector<int32_t> all_cpus;
  for (int32_t cpu = 0; cpu < GetNumCores(); ++cpu) {
    all_cpus.push_back(cpu);
  }

  // Record calls to dynamically instrumented functions and sample only on cores
  // in this process's cgroup's cpuset, as these are the only cores the process
  // will be scheduled on.
  std::vector<int32_t> cpuset_cpus = GetCpusetCpus(pid_);
  if (cpuset_cpus.empty()) {
    ERROR("Could not read cpuset");
    cpuset_cpus = all_cpus;
  }

  // As we open two perf_event_open file descriptors (uprobe and uretprobe) per
  // cpu per instrumented function, increase the maximum number of open files.
  SetMaxOpenFilesSoftLimit(GetMaxOpenFilesHardLimit());

  bool perf_event_open_errors = false;

  if (trace_context_switches_) {
    perf_event_open_errors |= !OpenContextSwitches(all_cpus);
  }

  context_switch_manager_.Clear();

  perf_event_open_errors |= !OpenMmapTask(cpuset_cpus);

  bool uprobes_event_open_errors = false;
  if (!instrumented_functions_.empty()) {
    uprobes_event_open_errors = !OpenUserSpaceProbes(cpuset_cpus);
    perf_event_open_errors |= uprobes_event_open_errors;
  }

  perf_event_open_errors |= !OpenTracepoints(cpuset_cpus);

  // This takes an initial snapshot of the maps. Call it after OpenUprobes, as
  // calling perf_event_open for uprobes (just calling it, it is not necessary
  // to enable the file descriptor) causes a new [uprobes] map entry, and we
  // want to catch it.
  InitUprobesEventProcessor();

  if (unwinding_method_ == CaptureOptions::kFramePointers ||
      unwinding_method_ == CaptureOptions::kDwarf) {
    perf_event_open_errors |= !OpenSampling(cpuset_cpus);
  }

  bool gpu_event_open_errors = false;
  if (trace_gpu_driver_) {
    if (InitGpuTracepointEventProcessor()) {
      // We want to trace all GPU activity, hence we pass 'all_cpus' here.
      gpu_event_open_errors = !OpenGpuTracepoints(all_cpus);
    } else {
      ERROR(
          "Failed to initialize GPU tracepoint event processor: "
          "skipping opening GPU tracepoint events");
    }
  }

  if (gpu_event_open_errors) {
    LOG("There were errors opening GPU tracepoint events");
  }

  if (uprobes_event_open_errors) {
    LOG("There were errors with perf_event_open, including for uprobes: did "
        "you forget to run as root?");
  } else if (perf_event_open_errors) {
    LOG("There were errors with perf_event_open: did you forget to run as root "
        "or to set /proc/sys/kernel/perf_event_paranoid to -1?");
  }

  // Start recording events.
  for (int fd : tracing_fds_) {
    perf_event_enable(fd);
  }

  // Get the initial thread names and notify the listener_.
  RetrieveThreadNames();

  stats_.Reset();

  bool last_iteration_saw_events = false;
  std::thread deferred_events_thread(&TracerThread::ProcessDeferredEvents,
                                     this);

  while (!(*exit_requested)) {
    ORBIT_SCOPE("Tracer Iteration");

    if (!last_iteration_saw_events) {
      // Periodically print event statistics.
      PrintStatsIfTimerElapsed();

      // Sleep if there was no new event in the last iteration so that we are
      // not constantly polling. Don't sleep so long that ring buffers overflow.
      // TODO: Refine this sleeping pattern, possibly using exponential backoff.
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
      if (*exit_requested) {
        break;
      }

      // Read up to ROUND_ROBIN_POLLING_BATCH_SIZE (5) new events.
      // TODO: Some event types (e.g., stack samples) have a much longer
      //  processing time but are less frequent than others (e.g., context
      //  switches). Take this into account in our scheduling algorithm.
      for (int32_t read_from_this_buffer = 0;
           read_from_this_buffer < ROUND_ROBIN_POLLING_BATCH_SIZE;
           ++read_from_this_buffer) {
        if (*exit_requested) {
          break;
        }
        if (!ring_buffer.HasNewData()) {
          break;
        }

        last_iteration_saw_events = true;
        perf_event_header header;
        ring_buffer.ReadHeader(&header);

        // perf_event_header::type contains the type of record, e.g.,
        // PERF_RECORD_SAMPLE, PERF_RECORD_MMAP, etc., defined in enum
        // perf_event_type in linux/perf_event.h.
        switch (header.type) {
          case PERF_RECORD_SWITCH:
            // Note: as we are recording context switches on CPUs and not on
            // threads, we don't expect this type of record.
            ERROR(
                "Unexpected PERF_RECORD_SWITCH in ring buffer '%s' (only "
                "PERF_RECORD_SWITCH_CPU_WIDE are expected)",
                ring_buffer.GetName().c_str());
            break;
          case PERF_RECORD_SWITCH_CPU_WIDE:
            ProcessContextSwitchCpuWideEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_FORK:
            ProcessForkEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_EXIT:
            ProcessExitEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_MMAP:
            ProcessMmapEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_SAMPLE:
            ProcessSampleEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_LOST:
            ProcessLostEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_THROTTLE:
            // We don't use throttle/unthrottle events, but log them separately
            // from the default 'Unexpected perf_event_header::type' case.
            LOG("PERF_RECORD_THROTTLE in ring buffer '%s'",
                ring_buffer.GetName().c_str());
            ring_buffer.SkipRecord(header);
            break;
          case PERF_RECORD_UNTHROTTLE:
            LOG("PERF_RECORD_UNTHROTTLE in ring buffer '%s'",
                ring_buffer.GetName().c_str());
            ring_buffer.SkipRecord(header);
            break;
          default:
            ERROR("Unexpected perf_event_header::type in ring buffer '%s': %u",
                  ring_buffer.GetName().c_str(), header.type);
            ring_buffer.SkipRecord(header);
            break;
        }
      }
    }
  }

  // Finish processing all deferred events.
  stop_deferred_thread_ = true;
  deferred_events_thread.join();
  uprobes_event_processor_->ProcessAllEvents();

  // Stop recording.
  for (int fd : tracing_fds_) {
    perf_event_disable(fd);
  }

  // Close the ring buffers.
  ring_buffers_.clear();

  // Close the file descriptors.
  for (int fd : tracing_fds_) {
    close(fd);
  }
}

void TracerThread::ProcessContextSwitchCpuWideEvent(
    const perf_event_header& header, PerfEventRingBuffer* ring_buffer) {
  SystemWideContextSwitchPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);
  pid_t pid = event.GetPid();
  pid_t tid = event.GetTid();
  uint16_t cpu = static_cast<uint16_t>(event.GetCpu());
  uint64_t time = event.GetTimestamp();

  // Switches with pid/tid 0 are associated with idle state, discard them.
  if (tid != 0) {
    // TODO: Consider deferring context switches.
    if (event.IsSwitchOut()) {
      // Careful: when a switch out is caused by the thread exiting, pid and tid
      // have value -1.
      std::optional<SchedulingSlice> scheduling_slice =
          context_switch_manager_.ProcessContextSwitchOut(pid, tid, cpu, time);
      if (scheduling_slice.has_value()) {
        listener_->OnSchedulingSlice(std::move(scheduling_slice.value()));
      }
    } else {
      context_switch_manager_.ProcessContextSwitchIn(pid, tid, cpu, time);
    }
  }

  ++stats_.sched_switch_count;
}

void TracerThread::ProcessForkEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  ForkPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);

  if (event.GetPid() != pid_) {
    return;
  }

  // A new thread of the sampled process was spawned.
  // Nothing to do for now.
}

void TracerThread::ProcessExitEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  ExitPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);

  if (event.GetPid() != pid_) {
    return;
  }

  // Nothing to do for now.
}

void TracerThread::ProcessMmapEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  pid_t pid = ReadMmapRecordPid(ring_buffer);
  ring_buffer->SkipRecord(header);

  if (pid != pid_) {
    return;
  }

  // There was a call to mmap with PROT_EXEC, hence refresh the maps.
  // This should happen rarely.
  auto event =
      std::make_unique<MapsPerfEvent>(MonotonicTimestampNs(), ReadMaps(pid_));
  event->SetOriginFileDescriptor(ring_buffer->GetFileDescriptor());
  DeferEvent(std::move(event));
}

void TracerThread::ProcessSampleEvent(const perf_event_header& header,
                                      PerfEventRingBuffer* ring_buffer) {
  uint64_t stream_id = ReadSampleRecordStreamId(ring_buffer);
  bool is_uprobe = uprobes_ids_.contains(stream_id);
  bool is_uretprobe = uretprobes_ids_.contains(stream_id);
  bool is_stack_sample = stack_sampling_ids_.contains(stream_id);
  bool is_task_newtask = task_newtask_ids_.contains(stream_id);
  bool is_sched_switch = sched_switch_ids_.contains(stream_id);
  bool is_task_rename = task_rename_ids_.contains(stream_id);
  bool is_amdgpu_cs_ioctl_event = amdgpu_cs_ioctl_ids_.contains(stream_id);
  bool is_amdgpu_sched_run_job_event =
      amdgpu_sched_run_job_ids_.contains(stream_id);
  bool is_dma_fence_signaled_event =
      dma_fence_signaled_ids_.contains(stream_id);
  bool is_callchain_sample = callchain_sampling_ids_.contains(stream_id);
  CHECK(is_uprobe + is_uretprobe + is_stack_sample + is_task_newtask +
            is_task_rename + is_sched_switch + is_amdgpu_cs_ioctl_event +
            is_amdgpu_sched_run_job_event + is_dma_fence_signaled_event +
            is_callchain_sample <=
        1);

  int fd = ring_buffer->GetFileDescriptor();

  if (is_uprobe) {
    auto event = make_unique_for_overwrite<UprobesPerfEvent>();
    ring_buffer->ConsumeRecord(header, &event->ring_buffer_record);
    using perf_event_uprobe = perf_event_sp_ip_arguments_8bytes_sample;
    constexpr size_t size_of_uprobes = sizeof(perf_event_uprobe);
    CHECK(header.size == size_of_uprobes);
    if (event->GetPid() != pid_) {
      return;
    }

    event->SetFunction(
        uprobes_uretprobes_ids_to_function_.at(event->GetStreamId()));
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.uprobes_count;

  } else if (is_uretprobe) {
    auto event = make_unique_for_overwrite<UretprobesPerfEvent>();
    ring_buffer->ConsumeRecord(header, &event->ring_buffer_record);
    constexpr size_t size_of_uretprobes = sizeof(perf_event_ax_sample);
    CHECK(header.size == size_of_uretprobes);
    if (event->GetPid() != pid_) {
      return;
    }

    event->SetFunction(
        uprobes_uretprobes_ids_to_function_.at(event->GetStreamId()));
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.uprobes_count;

  } else if (is_stack_sample) {
    pid_t pid = ReadSampleRecordPid(ring_buffer);
    constexpr size_t size_of_stack_sample = sizeof(perf_event_stack_sample);
    if (header.size != size_of_stack_sample) {
      // Skip stack samples that have an unexpected size. These normally have
      // abi == PERF_SAMPLE_REGS_ABI_NONE and no registers, and size == 0 and
      // no stack. Usually, these samples have pid == tid == 0, but that's not
      // always the case: for example, when a process exits while tracing, we
      // might get a stack sample with pid and tid != 0 but still with
      // abi == PERF_SAMPLE_REGS_ABI_NONE and size == 0.
      ring_buffer->SkipRecord(header);
      return;
    }
    if (pid != pid_) {
      ring_buffer->SkipRecord(header);
      return;
    }
    // Do *not* filter out samples based on header.misc,
    // e.g., with header.misc == PERF_RECORD_MISC_KERNEL,
    // in general they seem to produce valid callstacks.

    auto event = ConsumeStackSamplePerfEvent(ring_buffer, header);
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.sample_count;

  } else if (is_task_newtask) {
    auto event =
        ConsumeTracepointPerfEvent<TaskNewtaskPerfEvent>(ring_buffer, header);
    ThreadName thread_name;
    thread_name.set_tid(event->GetTid());
    thread_name.set_name(event->GetComm());
    thread_name.set_timestamp_ns(event->GetTimestamp());
    listener_->OnThreadName(std::move(thread_name));

  } else if (is_task_rename) {
    auto event =
        ConsumeTracepointPerfEvent<TaskRenamePerfEvent>(ring_buffer, header);
    ThreadName thread_name;
    thread_name.set_tid(event->GetTid());
    thread_name.set_name(event->GetNewComm());
    thread_name.set_timestamp_ns(event->GetTimestamp());
    listener_->OnThreadName(std::move(thread_name));

  } else if (is_sched_switch) {
    auto event =
        ConsumeTracepointPerfEvent<SchedSwitchPerfEvent>(ring_buffer, header);

    LOG(" CPU: %d ** %s : %d -> %s : %d", event->GetCpu(), event->GetPrevComm(),
        event->GetPrevPid(), event->GetNextComm(), event->GetNextPid());

  } else if (is_amdgpu_cs_ioctl_event) {
    // TODO: Consider deferring GPU events.
    auto event =
        ConsumeTracepointPerfEvent<AmdgpuCsIoctlPerfEvent>(ring_buffer, header);
    // Do not filter GPU tracepoint events based on pid as we want to have
    // visibility into all GPU activity across the system.
    gpu_event_processor_->PushEvent(*event);
    ++stats_.gpu_events_count;

  } else if (is_amdgpu_sched_run_job_event) {
    auto event = ConsumeTracepointPerfEvent<AmdgpuSchedRunJobPerfEvent>(
        ring_buffer, header);
    gpu_event_processor_->PushEvent(*event);
    ++stats_.gpu_events_count;
  } else if (is_dma_fence_signaled_event) {
    auto event = ConsumeTracepointPerfEvent<DmaFenceSignaledPerfEvent>(
        ring_buffer, header);
    gpu_event_processor_->PushEvent(*event);
    ++stats_.gpu_events_count;

  } else if (is_callchain_sample) {
    pid_t pid = ReadSampleRecordPid(ring_buffer);
    if (pid != pid_) {
      ring_buffer->SkipRecord(header);
      return;
    }

    auto event = ConsumeCallchainSamplePerfEvent(ring_buffer, header);
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.sample_count;

  } else {
    ERROR("PERF_EVENT_SAMPLE with unexpected stream_id: %lu", stream_id);
    ring_buffer->SkipRecord(header);
  }
}

void TracerThread::ProcessLostEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  LostPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);
  stats_.lost_count += event.GetNumLost();
  stats_.lost_count_per_buffer[ring_buffer] += event.GetNumLost();
}

void TracerThread::DeferEvent(std::unique_ptr<PerfEvent> event) {
  std::lock_guard<std::mutex> lock(deferred_events_mutex_);
  deferred_events_.emplace_back(std::move(event));
}

std::vector<std::unique_ptr<PerfEvent>> TracerThread::ConsumeDeferredEvents() {
  std::lock_guard<std::mutex> lock(deferred_events_mutex_);
  std::vector<std::unique_ptr<PerfEvent>> events(std::move(deferred_events_));
  deferred_events_.clear();
  return events;
}

void TracerThread::ProcessDeferredEvents() {
  pthread_setname_np(pthread_self(), "Proc.Def.Events");
  bool should_exit = false;
  while (!should_exit) {
    // When "should_exit" becomes true, we know that we have stopped generating
    // deferred events. The last iteration will consume all remaining events.
    should_exit = stop_deferred_thread_;
    std::vector<std::unique_ptr<PerfEvent>> events = ConsumeDeferredEvents();
    if (events.empty()) {
      // TODO: use a wait/notify mechanism instead of check/sleep.
      usleep(IDLE_TIME_ON_EMPTY_DEFERRED_EVENTS_US);
    } else {
      for (auto& event : events) {
        int fd = event->GetOriginFileDescriptor();
        uprobes_event_processor_->AddEvent(fd, std::move(event));
      }

      uprobes_event_processor_->ProcessOldEvents();
    }
  }
}

void TracerThread::RetrieveThreadNames() {
  uint64_t timestamp_ns = MonotonicTimestampNs();
  for (pid_t tid : ListThreads(pid_)) {
    std::string name = GetThreadName(tid);
    if (name.empty()) {
      continue;
    }

    ThreadName thread_name;
    thread_name.set_tid(tid);
    thread_name.set_name(std::move(name));
    thread_name.set_timestamp_ns(timestamp_ns);
    listener_->OnThreadName(std::move(thread_name));
  }
}

void TracerThread::Reset() {
  tracing_fds_.clear();
  fds_per_cpu_.clear();
  ring_buffers_.clear();

  uprobes_uretprobes_ids_to_function_.clear();
  uprobes_ids_.clear();
  uretprobes_ids_.clear();
  stack_sampling_ids_.clear();
  task_newtask_ids_.clear();
  task_rename_ids_.clear();
  amdgpu_cs_ioctl_ids_.clear();
  amdgpu_sched_run_job_ids_.clear();
  dma_fence_signaled_ids_.clear();
  callchain_sampling_ids_.clear();

  deferred_events_.clear();
  stop_deferred_thread_ = false;
}

void TracerThread::PrintStatsIfTimerElapsed() {
  uint64_t timestamp_ns = MonotonicTimestampNs();
  if (stats_.event_count_begin_ns + EVENT_STATS_WINDOW_S * NS_PER_SECOND <
      timestamp_ns) {
    double actual_window_s =
        static_cast<double>(timestamp_ns - stats_.event_count_begin_ns) /
        NS_PER_SECOND;
    LOG("Events per second (last %.1f s):", actual_window_s);
    LOG("  sched switches: %.0f", stats_.sched_switch_count / actual_window_s);
    LOG("  samples: %.0f", stats_.sample_count / actual_window_s);
    LOG("  u(ret)probes: %.0f", stats_.uprobes_count / actual_window_s);
    LOG("  gpu events: %.0f", stats_.gpu_events_count / actual_window_s);

    if (stats_.lost_count_per_buffer.empty()) {
      LOG("  lost: %.0f", stats_.lost_count / actual_window_s);
    } else {
      LOG("  lost: %.0f, of which:", stats_.lost_count / actual_window_s);
      for (const auto& lost_from_buffer : stats_.lost_count_per_buffer) {
        LOG("    from %s: %.0f", lost_from_buffer.first->GetName().c_str(),
            lost_from_buffer.second / actual_window_s);
      }
    }

    uint64_t unwind_error_count = *stats_.unwind_error_count;
    LOG("  unwind errors: %.0f (%.1f%%)", unwind_error_count / actual_window_s,
        100.0 * unwind_error_count / stats_.sample_count);
    uint64_t discarded_samples_in_uretprobes_count =
        *stats_.discarded_samples_in_uretprobes_count;
    LOG("  discarded samples in u(ret)probes: %.0f (%.1f%%)",
        discarded_samples_in_uretprobes_count / actual_window_s,
        100.0 * discarded_samples_in_uretprobes_count / stats_.sample_count);
    stats_.Reset();
  }
}

}  // namespace LinuxTracing
