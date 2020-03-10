#include "TracerThread.h"

#include "Logging.h"
#include "UprobesUnwindingVisitor.h"

namespace LinuxTracing {

// TODO: Refactor this huge method.
void TracerThread::Run(
    const std::shared_ptr<std::atomic<bool>>& exit_requested) {
  if (listener_ == nullptr) {
    ERROR("No listener set to TracerThread, aborting trace");
    return;
  }

  Reset();

  // perf_event_open refers to cores as "CPUs".
  int32_t num_cpus = GetNumCores();

  if (trace_context_switches_) {
    // Record context switches from all cores for all processes.
    for (int32_t cpu = 0; cpu < num_cpus; cpu++) {
      int context_switch_fd = context_switch_event_open(-1, cpu);
      PerfEventRingBuffer context_switch_ring_buffer{context_switch_fd,
                                                     SMALL_RING_BUFFER_SIZE_KB};
      if (context_switch_ring_buffer.IsOpen()) {
        fds_to_ring_buffer_.emplace(context_switch_fd,
                                    std::move(context_switch_ring_buffer));
      }
    }
  }

  auto uprobes_unwinding_visitor =
      std::make_unique<UprobesUnwindingVisitor>(ReadMaps(pid_));
  uprobes_unwinding_visitor->SetListener(listener_);
  // Switch between PerfEventProcessor and PerfEventProcessor2 here.
  // PerfEventProcessor2 is supposedly faster but assumes that events from
  // the same perf_event_open ring buffer are already sorted.
  uprobes_event_processor_ = std::make_shared<PerfEventProcessor2>(
      std::move(uprobes_unwinding_visitor));

  if (trace_instrumented_functions_) {
    for (const auto& function : instrumented_functions_) {
      for (int32_t cpu = 0; cpu < num_cpus; cpu++) {
        int uprobe_fd = uprobes_stack_event_open(
            function.BinaryPath().c_str(), function.FileOffset(), -1, cpu);
        PerfEventRingBuffer uprobe_ring_buffer{uprobe_fd,
                                               BIG_RING_BUFFER_SIZE_KB};
        if (uprobe_ring_buffer.IsOpen()) {
          fds_to_ring_buffer_.emplace(uprobe_fd, std::move(uprobe_ring_buffer));
          uprobe_fds_to_function_.emplace(uprobe_fd, &function);
        }

        int uretprobe_fd = uretprobes_event_open(
            function.BinaryPath().c_str(), function.FileOffset(), -1, cpu);

        // Redirect uretprobes to uprobe ring buffer to reduce number of ring
        // buffers and to coalesce closely related events.
        perf_event_redirect(uretprobe_fd, uprobe_fd);
        perf_event_enable(uretprobe_fd);

        // Track uretprobe fds separately as they map to a uprobe ring buffer.
        uret_probes_fds_.push_back(uretprobe_fd);
      }
    }
  }

  // TODO(b/148209993): Consider sampling based on CPU and filter by pid.
  for (pid_t tid : ListThreads(pid_)) {
    // Keep threads in sync.
    listener_->OnTid(tid);

    if (trace_callstacks_) {
      int sampling_fd =
          sample_mmap_task_event_open(sampling_period_ns_, tid, -1);
      PerfEventRingBuffer sampling_ring_buffer{sampling_fd,
                                               BIG_RING_BUFFER_SIZE_KB};
      if (sampling_ring_buffer.IsOpen()) {
        fds_to_ring_buffer_.emplace(sampling_fd,
                                    std::move(sampling_ring_buffer));
        threads_to_fd_.emplace(tid, sampling_fd);
      }
    }
  }

  // TODO: New threads might spawn here before forks are started to be recorded.
  //  Consider also polling threads regularly.

  // Start recording events.
  for (const auto& fd_to_ring_buffer : fds_to_ring_buffer_) {
    perf_event_enable(fd_to_ring_buffer.first);
  }

  stats_.Reset();

  bool last_iteration_saw_events = false;
  std::thread deferred_events_thread(&TracerThread::ProcessDeferredEvents,
                                     this);

  while (!(*exit_requested)) {
    // Sleep if there was no new event in the last iteration so that we are not
    // constantly polling.  Don't sleep so long that ring buffers overflow.
    // TODO: Refine this sleeping pattern, possibly using exponential backoff.
    if (!last_iteration_saw_events) {
      usleep(IDLE_TIME_ON_EMPTY_RING_BUFFERS_US);
    }

    last_iteration_saw_events = false;
    fds_to_remove_.clear();

    // Read and process events from all ring buffers. In order to ensure that no
    // buffer is read constantly while others overflow, we schedule the reading
    // using round-robin like scheduling.
    for (auto& fd_to_ring_buffer : fds_to_ring_buffer_) {
      if (*exit_requested) {
        break;
      }

      const int& fd = fd_to_ring_buffer.first;
      PerfEventRingBuffer& ring_buffer = fd_to_ring_buffer.second;

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
        // perf_event_type in perf_event.h.
        switch (header.type) {
          case PERF_RECORD_SWITCH:  // non system-wide profiling:
            ProcessContextSwitchEvent(header, &ring_buffer);
            break;
          case PERF_RECORD_SWITCH_CPU_WIDE:  // system-wide profiling
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

    // Add new ring buffers.
    for (auto& fd_to_ring_buffer_to_add : fds_to_ring_buffer_to_add_) {
      fds_to_ring_buffer_.emplace(std::move(fd_to_ring_buffer_to_add));
    }

    // Erase old ring buffers.
    for (int fd_to_remove : fds_to_remove_) {
      fds_to_ring_buffer_.erase(fd_to_remove);
    }
  }

  // Finish processing all deferred events.
  stop_deferred_thread_ = true;
  deferred_events_thread.join();
  uprobes_event_processor_->ProcessAllEvents();

  // Stop recording and close the file descriptors.
  for (auto& fd_to_ring_buffer : fds_to_ring_buffer_) {
    const int& fd = fd_to_ring_buffer.first;
    perf_event_disable(fd);
    close(fd);
  }
  fds_to_ring_buffer_.clear();

  for (int uret_probe_fd : uret_probes_fds_) {
    perf_event_disable(uret_probe_fd);
    close(uret_probe_fd);
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

  if (event.GetPid() == pid_) {
    // A new thread of the sampled process was spawned.
    int sampling_fd =
        sample_mmap_task_event_open(sampling_period_ns_, event.GetTid(), -1);
    perf_event_enable(sampling_fd);
    PerfEventRingBuffer sampling_ring_buffer{sampling_fd,
                                             BIG_RING_BUFFER_SIZE_KB};
    if (sampling_ring_buffer.IsOpen()) {
      // Do not add a new ring buffer to fds_to_ring_buffer here as we
      // are already iterating over fds_to_ring_buffer.
      fds_to_ring_buffer_to_add_.emplace_back(sampling_fd,
                                              std::move(sampling_ring_buffer));
      threads_to_fd_.emplace(event.GetTid(), sampling_fd);
    }
  }
}

void TracerThread::ProcessExitEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  ExitPerfEvent event;
  ring_buffer->ConsumeRecord(header, &event.ring_buffer_record);

  if (event.GetPid() == pid_) {
    if (threads_to_fd_.count(event.GetTid()) > 0) {
      int sample_fd = threads_to_fd_.at(event.GetTid());
      perf_event_disable(sample_fd);
      close(sample_fd);
      fds_to_remove_.push_back(sample_fd);
      threads_to_fd_.erase(sample_fd);
    }
  }
}

void TracerThread::ProcessMmapEvent(const perf_event_header& header,
                                    PerfEventRingBuffer* ring_buffer) {
  // There was a call to mmap with PROT_EXEC, hence refresh the maps.
  // This should happen rarely.
  ring_buffer->SkipRecord(header);
  int fd = ring_buffer->GetFileDescriptor();
  auto event =
      std::make_unique<MapsPerfEvent>(MonotonicTimestampNs(), ReadMaps(pid_));
  event->SetOriginFileDescriptor(fd);
  DeferEvent(std::move(event));
}

void TracerThread::ProcessSampleEvent(const perf_event_header& header,
                                      PerfEventRingBuffer* ring_buffer) {
  int fd = ring_buffer->GetFileDescriptor();
  bool is_probe = uprobe_fds_to_function_.count(fd) > 0;
  constexpr size_t size_of_uretprobe = sizeof(perf_event_empty_sample);
  bool is_uretprobe = is_probe && (header.size == size_of_uretprobe);
  bool is_uprobe = is_probe && !is_uretprobe;

  if (is_uprobe) {
    auto event =
        ConsumeSamplePerfEvent<UprobesWithStackPerfEvent>(ring_buffer, header);
    event->SetFunction(uprobe_fds_to_function_.at(fd));
    event->SetOriginFileDescriptor(fd);
    DeferEvent(std::move(event));
    ++stats_.uprobes_count;
  } else if (is_uretprobe) {
    auto event = make_unique_for_overwrite<UretprobesPerfEvent>();
    ring_buffer->ConsumeRecord(header, &event->ring_buffer_record);
    event->SetFunction(uprobe_fds_to_function_.at(fd));
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
  LOG("Lost %lu events", event.GetNumLost());
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
    // When "should_exit" becomes true, we know that we have stopped
    // generating deferred events. The last iteration will consume all
    // remaining events.
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
  fds_to_ring_buffer_.clear();
  fds_to_ring_buffer_to_add_.clear();
  uret_probes_fds_.clear();
  threads_to_fd_.clear();
  uprobe_fds_to_function_.clear();
  fds_to_remove_.clear();
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
