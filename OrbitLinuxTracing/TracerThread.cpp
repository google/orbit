#include "TracerThread.h"

#include <OrbitBase/Logging.h>

#include <thread>

#include "UprobesUnwindingVisitor.h"
#include "absl/strings/str_format.h"

namespace LinuxTracing {

// TODO: Refactor this huge method.
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

  if (trace_context_switches_) {
    for (int32_t cpu : all_cpus) {
      int context_switch_fd = context_switch_event_open(-1, cpu);
      std::string buffer_name = absl::StrFormat("context_switch_%u", cpu);
      PerfEventRingBuffer context_switch_ring_buffer{
          context_switch_fd, SMALL_RING_BUFFER_SIZE_KB, buffer_name};
      if (context_switch_ring_buffer.IsOpen()) {
        tracing_fds_.push_back(context_switch_fd);
        ring_buffers_.push_back(std::move(context_switch_ring_buffer));
      }
    }
  }

  auto uprobes_unwinding_visitor =
      std::make_unique<UprobesUnwindingVisitor>(ReadMaps(pid_));
  uprobes_unwinding_visitor->SetListener(listener_);
  // Switch between PerfEventProcessor and PerfEventProcessor2 here.
  // PerfEventProcessor2 is supposedly faster but assumes that events from the
  // same perf_event_open ring buffer are already sorted.
  uprobes_event_processor_ = std::make_shared<PerfEventProcessor2>(
      std::move(uprobes_unwinding_visitor));

  if (trace_instrumented_functions_) {
    for (const auto& function : instrumented_functions_) {
      std::vector<int> function_uprobes_fds;
      std::vector<int> function_uretprobes_fds;
      std::vector<PerfEventRingBuffer> function_uprobes_ring_buffers;
      bool function_uprobes_open_error = false;

      for (int32_t cpu : cpuset_cpus) {
        int uprobes_fd = uprobes_stack_event_open(
            function.BinaryPath().c_str(), function.FileOffset(), -1, cpu);
        std::string buffer_name = absl::StrFormat(
            "uprobe_retprobe_%#016lx_%u", function.VirtualAddress(), cpu);
        PerfEventRingBuffer uprobes_ring_buffer{
            uprobes_fd, BIG_RING_BUFFER_SIZE_KB, buffer_name};
        if (uprobes_ring_buffer.IsOpen()) {
          function_uprobes_fds.push_back(uprobes_fd);
          function_uprobes_ring_buffers.push_back(std::move(uprobes_ring_buffer));
        } else {
          function_uprobes_open_error = true;
          break;
        }

        int uretprobes_fd = uretprobes_event_open(
            function.BinaryPath().c_str(), function.FileOffset(), -1, cpu);
        if (uretprobes_fd >= 0) {
          function_uretprobes_fds.push_back(uretprobes_fd);
        } else {
          function_uprobes_open_error = true;
          break;
        }

        // Redirect uretprobes to the uprobes ring buffer to reduce number of
        // ring buffers and to coalesce closely related events.
        perf_event_redirect(uretprobes_fd, uprobes_fd);
      }

      if (function_uprobes_open_error) {
        ERROR("Opening u(ret)probes for function at %#016lx",
              function.VirtualAddress());
        function_uprobes_ring_buffers.clear();
        for (int uprobes_fd : function_uprobes_fds) {
          close(uprobes_fd);
        }
        for (int uretprobes_fd : function_uretprobes_fds) {
          close(uretprobes_fd);
        }
      } else {
        // Add function_uretprobes_fds to tracing_fds_ before
        // function_uprobes_fds. As we support having uretprobes without
        // associated uprobes, but not the opposite, this way the uretprobe is
        // enabled before the uprobe.
        tracing_fds_.insert(tracing_fds_.end(), function_uretprobes_fds.begin(),
                            function_uretprobes_fds.end());
        tracing_fds_.insert(tracing_fds_.end(), function_uprobes_fds.begin(),
                            function_uprobes_fds.end());
        for (PerfEventRingBuffer& uprobes_ring_buffer :
             function_uprobes_ring_buffers) {
          ring_buffers_.emplace_back(std::move(uprobes_ring_buffer));
        }
        for (int uprobes_fd : function_uprobes_fds) {
          uprobes_fds_to_function_.emplace(uprobes_fd, &function);
        }
      }
    }
  }

  for (int32_t cpu : cpuset_cpus) {
    int mmap_task_fd = mmap_task_event_open(-1, cpu);
    std::string buffer_name = absl::StrFormat("mmap_task_%u", cpu);
    PerfEventRingBuffer mmap_task_ring_buffer{
        mmap_task_fd, BIG_RING_BUFFER_SIZE_KB, buffer_name};
    if (mmap_task_ring_buffer.IsOpen()) {
      tracing_fds_.push_back(mmap_task_fd);
      ring_buffers_.push_back(std::move(mmap_task_ring_buffer));
    }
  }

  if (trace_callstacks_) {
    for (int32_t cpu : cpuset_cpus) {
      int sampling_fd = sample_event_open(sampling_period_ns_, -1, cpu);
      std::string buffer_name = absl::StrFormat("sampling_%u", cpu);
      PerfEventRingBuffer sampling_ring_buffer{
          sampling_fd, BIG_RING_BUFFER_SIZE_KB, buffer_name};
      if (sampling_ring_buffer.IsOpen()) {
        tracing_fds_.push_back(sampling_fd);
        ring_buffers_.push_back(std::move(sampling_ring_buffer));
      }
    }
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
    // Sleep if there was no new event in the last iteration so that we are not
    // constantly polling. Don't sleep so long that ring buffers overflow.
    // TODO: Refine this sleeping pattern, possibly using exponential backoff.
    if (!last_iteration_saw_events) {
      usleep(IDLE_TIME_ON_EMPTY_RING_BUFFERS_US);
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
                "Unexpected PERF_RECORD_SWITCH (only "
                "PERF_RECORD_SWITCH_CPU_WIDE are expected)");
            ProcessContextSwitchEvent(header, &ring_buffer);
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
          default:
            ERROR("Unexpected perf_event_header::type: %u", header.type);
            ring_buffer.SkipRecord(header);
            break;
        }

        // Periodically print event statistics.
        PrintStatsIfTimerElapsed();
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

void TracerThread::ProcessContextSwitchEvent(const perf_event_header& header,
                                             PerfEventRingBuffer* ring_buffer) {
  ContextSwitchPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);
  pid_t tid = event.GetTid();
  uint16_t cpu = static_cast<uint16_t>(event.GetCpu());
  uint64_t time = event.GetTimestamp();

  if (event.IsSwitchOut()) {
    listener_->OnContextSwitchOut(ContextSwitchOut(tid, cpu, time));
  } else {
    listener_->OnContextSwitchIn(ContextSwitchIn(tid, cpu, time));
  }

  ++stats_.sched_switch_count;
}

void TracerThread::ProcessContextSwitchCpuWideEvent(
    const perf_event_header& header, PerfEventRingBuffer* ring_buffer) {
  SystemWideContextSwitchPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);
  uint16_t cpu = static_cast<uint16_t>(event.GetCpu());
  uint64_t time = event.GetTimestamp();

  if (event.GetPrevTid() != 0) {
    ContextSwitchOut context_switch_out{event.GetPrevTid(), cpu, time};
    listener_->OnContextSwitchOut(context_switch_out);
  }
  if (event.GetNextTid() != 0) {
    ContextSwitchIn context_switch_in{event.GetNextTid(), cpu, time};
    listener_->OnContextSwitchIn(context_switch_in);
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
  int fd = ring_buffer->GetFileDescriptor();
  bool is_probe = uprobes_fds_to_function_.count(fd) > 0;
  constexpr size_t size_of_uretprobe = sizeof(perf_event_empty_sample);
  bool is_uretprobe = is_probe && (header.size == size_of_uretprobe);
  bool is_uprobe = is_probe && !is_uretprobe;

  pid_t pid;
  if (is_uretprobe) {
    pid = ReadUretprobesRecordPid(ring_buffer);
  } else {
    pid = ReadSampleRecordPid(ring_buffer);
  }

  if (pid != pid_) {
    ring_buffer->SkipRecord(header);
    return;
  }

  if (is_uprobe) {
    auto event =
        ConsumeSamplePerfEvent<UprobesWithStackPerfEvent>(ring_buffer, header);
    event->SetFunction(uprobes_fds_to_function_.at(fd));
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.uprobes_count;

  } else if (is_uretprobe) {
    auto event = make_unique_for_overwrite<UretprobesPerfEvent>();
    ring_buffer->ConsumeRecord(header, &event->ring_buffer_record);
    event->SetFunction(uprobes_fds_to_function_.at(fd));
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.uprobes_count;

  } else {
    auto event =
        ConsumeSamplePerfEvent<StackSamplePerfEvent>(ring_buffer, header);
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.sample_count;
  }
}

void TracerThread::ProcessLostEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  LostPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);
  LOG("Lost %lu events in buffer %s", event.GetNumLost(),
      ring_buffer->GetName().c_str());
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

void TracerThread::Reset() {
  tracing_fds_.clear();
  ring_buffers_.clear();
  uprobes_fds_to_function_.clear();
  deferred_events_.clear();
  stop_deferred_thread_ = false;
}

void TracerThread::PrintStatsIfTimerElapsed() {
  constexpr uint64_t EVENT_COUNT_WINDOW_S = 5;

  if (stats_.event_count_begin_ns + EVENT_COUNT_WINDOW_S * 1'000'000'000 <
      MonotonicTimestampNs()) {
    LOG("Events per second (last %lu s): "
        "sched switches: %lu; "
        "samples: %lu; "
        "u(ret)probes: %lu",
        EVENT_COUNT_WINDOW_S, stats_.sched_switch_count / EVENT_COUNT_WINDOW_S,
        stats_.sample_count / EVENT_COUNT_WINDOW_S,
        stats_.uprobes_count / EVENT_COUNT_WINDOW_S);
    stats_.Reset();
  }
}

}  // namespace LinuxTracing
