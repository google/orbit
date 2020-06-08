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

namespace {
void CloseFileDescriptors(const std::vector<int>& fds) {
  for (int fd : fds) {
    close(fd);
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
  // Switch between PerfEventProcessor and PerfEventProcessor2 here.
  // PerfEventProcessor2 is supposedly faster but assumes that events from the
  // same perf_event_open ring buffer are already sorted.
  uprobes_event_processor_ = std::make_shared<PerfEventProcessor2>(
      std::move(uprobes_unwinding_visitor));
}

bool TracerThread::OpenUprobes(const std::vector<int32_t>& cpus) {
  bool uprobes_event_open_errors = false;
  absl::flat_hash_map<int32_t, int> uprobes_ring_buffer_fds_per_cpu;

  for (const auto& function : instrumented_functions_) {
    absl::flat_hash_map<int32_t, int> function_uprobes_fds_per_cpu;
    absl::flat_hash_map<int32_t, int> function_uretprobes_fds_per_cpu;
    bool function_uprobes_open_error = false;

    for (int32_t cpu : cpus) {
      int uprobes_fd = uprobes_retaddr_event_open(
          function.BinaryPath().c_str(), function.FileOffset(), -1, cpu);
      if (uprobes_fd < 0) {
        function_uprobes_open_error = true;
        break;
      }
      function_uprobes_fds_per_cpu.emplace(cpu, uprobes_fd);

      int uretprobes_fd = uretprobes_event_open(function.BinaryPath().c_str(),
                                                function.FileOffset(), -1, cpu);
      if (uretprobes_fd < 0) {
        function_uprobes_open_error = true;
        break;
      }
      function_uretprobes_fds_per_cpu.emplace(cpu, uretprobes_fd);
    }

    if (function_uprobes_open_error) {
      ERROR("Opening u(ret)probes for function at %#016lx",
            function.VirtualAddress());
      uprobes_event_open_errors = true;
      for (const auto& uprobes_fd : function_uprobes_fds_per_cpu) {
        close(uprobes_fd.second);
      }
      for (const auto& uretprobes_fd : function_uretprobes_fds_per_cpu) {
        close(uretprobes_fd.second);
      }
      continue;
    }

    // Add function_uretprobes_fds_per_cpu to tracing_fds_ before
    // function_uprobes_fds_per_cpu. As we support having uretprobes without
    // associated uprobes, but not the opposite, this way the uretprobe is
    // enabled before the uprobe.
    for (const auto& uretprobes_fd : function_uretprobes_fds_per_cpu) {
      tracing_fds_.push_back(uretprobes_fd.second);
    }
    for (const auto& uprobes_fd : function_uprobes_fds_per_cpu) {
      tracing_fds_.push_back(uprobes_fd.second);
    }

    // Record the association between the stream_id and the function
    // (as well as which stream_ids are uprobes and uretprobes).
    for (const auto& uprobes_fd : function_uprobes_fds_per_cpu) {
      uint64_t stream_id = perf_event_get_id(uprobes_fd.second);
      uprobes_uretprobes_ids_to_function_.emplace(stream_id, &function);
      uprobes_ids_.insert(stream_id);
    }
    for (const auto& uretprobes_fd : function_uretprobes_fds_per_cpu) {
      uint64_t stream_id = perf_event_get_id(uretprobes_fd.second);
      uprobes_uretprobes_ids_to_function_.emplace(stream_id, &function);
      uretprobes_ids_.insert(stream_id);
    }

    // Redirect all uprobes and uretprobes on the same cpu to a single ring
    // buffer to reduce the number of ring buffers.
    for (int32_t cpu : cpus) {
      int uprobes_fd = function_uprobes_fds_per_cpu.at(cpu);
      int uretprobes_fd = function_uretprobes_fds_per_cpu.at(cpu);
      if (uprobes_ring_buffer_fds_per_cpu.contains(cpu)) {
        // Redirect to the already opened ring buffer.
        int ring_buffer_fd = uprobes_ring_buffer_fds_per_cpu.at(cpu);
        perf_event_redirect(uprobes_fd, ring_buffer_fd);
        perf_event_redirect(uretprobes_fd, ring_buffer_fd);
      } else {
        // No ring buffer has yet been created for this cpu, as this is the
        // first uprobes to have been opened successfully. Hence, create a
        // ring buffer for this cpu associated to uprobes_fd and redirect the
        // uretprobes to it. The other uprobes and uretprobes for this cpu
        // will be redirected to this ring buffer.
        int ring_buffer_fd = uprobes_fd;
        std::string buffer_name = absl::StrFormat("uprobes_uretprobes_%u", cpu);
        ring_buffers_.emplace_back(ring_buffer_fd, UPROBES_RING_BUFFER_SIZE_KB,
                                   buffer_name);
        uprobes_ring_buffer_fds_per_cpu[cpu] = ring_buffer_fd;
        // Must be called after the ring buffer has been opened.
        perf_event_redirect(uretprobes_fd, ring_buffer_fd);
      }
    }
  }

  return !uprobes_event_open_errors;
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
    switch (tracing_options_.sampling_method) {
      case SamplingMethod::kFramePointers:
        sampling_fd = callchain_sample_event_open(sampling_period_ns_, -1, cpu);
        break;
      case SamplingMethod::kDwarf:
        sampling_fd = stack_sample_event_open(sampling_period_ns_, -1, cpu);
        break;
      case SamplingMethod::kOff:
        FATAL("Sampling is off. This statement is unreachable.");
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
    if (tracing_options_.sampling_method == SamplingMethod::kDwarf) {
      stack_sampling_ids_.insert(stream_id);
    } else if (tracing_options_.sampling_method ==
               SamplingMethod::kFramePointers) {
      callchain_sampling_ids_.insert(stream_id);
    }
  }
  for (PerfEventRingBuffer& buffer : sampling_ring_buffers) {
    ring_buffers_.emplace_back(std::move(buffer));
  }
  return true;
}

bool TracerThread::InitGpuTracepointEventProcessor() {
  int amdgpu_cs_ioctl_id = GetTracepointId("amdgpu", "amdgpu_cs_ioctl");
  if (amdgpu_cs_ioctl_id == -1) {
    return false;
  }
  int amdgpu_sched_run_job_id =
      GetTracepointId("amdgpu", "amdgpu_sched_run_job");
  if (amdgpu_sched_run_job_id == -1) {
    return false;
  }
  int dma_fence_signaled_id =
      GetTracepointId("dma_fence", "dma_fence_signaled");
  if (dma_fence_signaled_id == -1) {
    return false;
  }
  gpu_event_processor_ = std::make_shared<GpuTracepointEventProcessor>(
      amdgpu_cs_ioctl_id, amdgpu_sched_run_job_id, dma_fence_signaled_id);
  gpu_event_processor_->SetListener(listener_);
  return true;
}

bool TracerThread::OpenRingBufferForGpuTracepoint(
    const char* tracepoint_category, const char* tracepoint_name, int32_t cpu,
    std::vector<int>* gpu_tracing_fds,
    std::vector<PerfEventRingBuffer>* gpu_ring_buffers) {
  int fd = tracepoint_event_open(tracepoint_category, tracepoint_name, -1, cpu);
  if (fd == -1) {
    return false;
  }
  gpu_tracing_fds->push_back(fd);

  std::string buffer_name =
      absl::StrFormat("%s:%s_%i", tracepoint_category, tracepoint_name, cpu);
  PerfEventRingBuffer ring_buffer{fd, GPU_TRACING_RING_BUFFER_SIZE_KB,
                                  buffer_name};
  if (!ring_buffer.IsOpen()) {
    return false;
  }
  gpu_ring_buffers->push_back(std::move(ring_buffer));

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
  std::vector<int> gpu_tracing_fds;
  std::vector<PerfEventRingBuffer> gpu_ring_buffers;
  for (int32_t cpu : cpus) {
    if (!OpenRingBufferForGpuTracepoint("amdgpu", "amdgpu_cs_ioctl", cpu,
                                        &gpu_tracing_fds, &gpu_ring_buffers)) {
      CloseFileDescriptors(gpu_tracing_fds);
      return false;
    }
    if (!OpenRingBufferForGpuTracepoint("amdgpu", "amdgpu_sched_run_job", cpu,
                                        &gpu_tracing_fds, &gpu_ring_buffers)) {
      CloseFileDescriptors(gpu_tracing_fds);
      return false;
    }
    if (!OpenRingBufferForGpuTracepoint("dma_fence", "dma_fence_signaled", cpu,
                                        &gpu_tracing_fds, &gpu_ring_buffers)) {
      CloseFileDescriptors(gpu_tracing_fds);
      return false;
    }
  }

  // Since all tracepoints could successfully be opened, we can now commit all
  // file descriptors and ring buffers to the TracerThread members.
  for (int fd : gpu_tracing_fds) {
    tracing_fds_.push_back(fd);
    gpu_tracing_ids_.insert(perf_event_get_id(fd));
  }
  for (PerfEventRingBuffer& buffer : gpu_ring_buffers) {
    ring_buffers_.emplace_back(std::move(buffer));
  }

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

  if (tracing_options_.trace_context_switches) {
    perf_event_open_errors |= !OpenContextSwitches(all_cpus);
  }

  context_switch_manager_.Clear();

  perf_event_open_errors |= !OpenMmapTask(cpuset_cpus);

  bool uprobes_event_open_errors = false;
  if (tracing_options_.trace_instrumented_functions) {
    uprobes_event_open_errors = !OpenUprobes(cpuset_cpus);
    perf_event_open_errors |= uprobes_event_open_errors;
  }

  // This takes an initial snapshot of the maps. Call it after OpenUprobes, as
  // calling perf_event_open for uprobes (just calling it, it is not necessary
  // to enable the file descriptor) causes a new [uprobes] map entry, and we
  // want to catch it.
  InitUprobesEventProcessor();

  if (tracing_options_.sampling_method != SamplingMethod::kOff) {
    perf_event_open_errors |= !OpenSampling(cpuset_cpus);
  }

  bool gpu_event_open_errors = false;
  if (tracing_options_.trace_gpu_driver) {
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

  for (pid_t tid : ListThreads(pid_)) {
    // Keep threads in sync.
    listener_->OnTid(tid);
  }

  stats_.Reset();

  bool last_iteration_saw_events = false;
  std::thread deferred_events_thread(&TracerThread::ProcessDeferredEvents,
                                     this);

  while (!(*exit_requested)) {
    ORBIT_SCOPE("Tracer Iteration");

    if (!last_iteration_saw_events) {
      // Check for updates of thread names and in case notify the listener_.
      UpdateThreadNamesIfDelayElapsed();

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
        listener_->OnSchedulingSlice(scheduling_slice.value());
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
  listener_->OnTid(event.GetTid());
}

void TracerThread::ProcessExitEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  ExitPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);

  if (event.GetPid() != pid_) {
    return;
  }

  // Nothing to do.
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
  bool is_gpu_event = gpu_tracing_ids_.contains(stream_id);
  bool is_callchain_sample = callchain_sampling_ids_.contains(stream_id);
  CHECK(is_uprobe + is_uretprobe + is_stack_sample + is_gpu_event +
            is_callchain_sample <=
        1);

  int fd = ring_buffer->GetFileDescriptor();

  if (is_uprobe) {
    auto event = make_unique_for_overwrite<UprobesPerfEvent>();
    ring_buffer->ConsumeRecord(header, &event->ring_buffer_record);
    constexpr size_t size_of_uprobes = sizeof(perf_event_sp_ip_8bytes_sample);
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

  } else if (is_gpu_event) {
    // TODO: Consider deferring GPU events.
    auto event = ConsumeSampleRaw(ring_buffer, header);
    // Do not filter GPU tracepoint events based on pid as we want to have
    // visibility into all GPU activity across the system.
    gpu_event_processor_->PushEvent(event);
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

void TracerThread::UpdateThreadNamesIfDelayElapsed() {
  uint64_t timestamp_ns = MonotonicTimestampNs();
  if (last_thread_names_update +
          THREAD_NAMES_UPDATE_DELAY_MS * NS_PER_MILLISECOND <
      timestamp_ns) {
    for (pid_t tid : ListThreads(pid_)) {
      std::string name = GetThreadName(tid);
      if (name.empty()) {
        continue;
      }

      auto last_name_it = thread_names_.find(tid);
      if (last_name_it == thread_names_.end() || name != last_name_it->second) {
        thread_names_[tid] = name;
        listener_->OnThreadName(tid, name);
      }
    }
    last_thread_names_update = timestamp_ns;
  }
}

void TracerThread::Reset() {
  tracing_fds_.clear();
  ring_buffers_.clear();

  uprobes_uretprobes_ids_to_function_.clear();
  uprobes_ids_.clear();
  uretprobes_ids_.clear();
  stack_sampling_ids_.clear();
  gpu_tracing_ids_.clear();
  callchain_sampling_ids_.clear();

  deferred_events_.clear();
  stop_deferred_thread_ = false;

  thread_names_.clear();
  last_thread_names_update = 0;
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
