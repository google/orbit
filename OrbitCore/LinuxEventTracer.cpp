//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "LinuxEventTracer.h"

#include <vector>

#include "Capture.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "LinuxPerfEvent.h"
#include "LinuxPerfEventProcessor2.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"
#include "LinuxUprobesUnwindingVisitor.h"
#include "LinuxUtils.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Params.h"
#include "SamplingProfiler.h"
#include "TimerManager.h"

// TODO: Refactor this huge method.
void LinuxEventTracerThread::Run(
    const std::shared_ptr<std::atomic<bool>>& exit_requested) {
  if (!ComputeSamplingPeriodNs()) {
    return;
  }

  LoadNumCpus();

  std::unordered_map<int32_t, LinuxPerfRingBuffer> fds_to_ring_buffer;
  std::unordered_map<pid_t, int32_t> threads_to_fd;
  std::unordered_map<int32_t, Function*> uprobe_fds_to_function;
  std::unordered_map<int32_t, Function*> uretprobe_fds_to_function;

  if (GParams.m_TrackContextSwitches) {
    if (GParams.m_SystemWideScheduling) {
      // perf_event_open for all cpus to keep track of process spawning.
      for (int32_t cpu = 0; cpu < num_cpus_; cpu++) {
        int32_t context_switch_fd =
            LinuxPerfUtils::cpu_context_switch_open(cpu);
        fds_to_ring_buffer.emplace(context_switch_fd,
                                   LinuxPerfRingBuffer{context_switch_fd});
      }
    } else {
      // perf_event_open for all cpus and the PID to keep track of process
      // spawning.
      int32_t context_switch_fd = LinuxPerfUtils::pid_context_switch_open(pid_);
      fds_to_ring_buffer.emplace(context_switch_fd,
                                 LinuxPerfRingBuffer{context_switch_fd});
    }
  }

  LinuxPerfEventProcessor2 uprobe_event_processor{
      std::make_unique<LinuxUprobesUnwindingVisitor>(
          pid_, LinuxUtils::ReadMaps(pid_))};

  if (!GParams.m_UseBpftrace) {
    for (const auto& function : instrumented_functions_) {
      for (int32_t cpu = 0; cpu < num_cpus_; cpu++) {
        int uprobe_fd = LinuxPerfUtils::uprobe_stack_event_open(
            function->m_Pdb->GetFileName().c_str(), function->m_Address, -1,
            cpu);
        fds_to_ring_buffer.emplace(uprobe_fd, LinuxPerfRingBuffer{uprobe_fd});
        uprobe_fds_to_function.emplace(uprobe_fd, function);

        int uretprobe_fd = LinuxPerfUtils::uretprobe_stack_event_open(
            function->m_Pdb->GetFileName().c_str(), function->m_Address, -1,
            cpu);
        fds_to_ring_buffer.emplace(uretprobe_fd,
                                   LinuxPerfRingBuffer{uretprobe_fd});
        uretprobe_fds_to_function.emplace(uretprobe_fd, function);
      }
    }
  }

  // TODO(b/148209993): Consider sampling based on CPU and filter by pid.
  for (pid_t tid : LinuxUtils::ListThreads(pid_)) {
    // Keep threads in sync.
    Capture::GTargetProcess->AddThreadId(tid);

    if (!GParams.m_SampleWithPerf) {
      int sampling_fd =
          LinuxPerfUtils::sample_mmap_task_event_open(tid, sampling_period_ns_);
      fds_to_ring_buffer.emplace(sampling_fd, LinuxPerfRingBuffer{sampling_fd});
      threads_to_fd.emplace(tid, sampling_fd);
    }
  }

  // TODO: New threads might spawn here before forks are started to be recorded.
  //  Consider also polling threads regularly.

  // Start recording events.
  for (const auto& fd_to_ring_buffer : fds_to_ring_buffer) {
    LinuxPerfUtils::start_capturing(fd_to_ring_buffer.first);
  }

  // Record and periodically print basic statistics on the number events.
  constexpr uint64_t EVENT_COUNT_WINDOW_S = 5;
  uint64_t event_count_window_begin_ns = 0;
  uint64_t sched_switch_count = 0;
  uint64_t sample_count = 0;
  uint64_t uprobes_count = 0;

  bool last_iteration_saw_events = false;

  while (!(*exit_requested)) {
    // If there was nothing new in the last iteration:
    // Lets sleep a bit, so that we are not constantly reading from the
    // buffers and thus wasting cpu time. 10 ms are still small enough to
    // not have our buffers overflow and therefore lose events.
    if (!last_iteration_saw_events) {
      OrbitSleepMs(10);
    }

    last_iteration_saw_events = false;

    std::vector<std::pair<int32_t, LinuxPerfRingBuffer>>
        fds_to_ring_buffer_to_add;
    std::vector<int32_t> fds_to_remove;

    // Read and process events from all ring buffers. In order to ensure that no
    // buffer is read constantly while others overflow, we schedule the reading
    // using round-robin like scheduling.
    for (auto& fd_to_ring_buffer : fds_to_ring_buffer) {
      if (*exit_requested) {
        break;
      }

      const int32_t& fd = fd_to_ring_buffer.first;
      LinuxPerfRingBuffer& ring_buffer = fd_to_ring_buffer.second;

      bool is_uprobes = uprobe_fds_to_function.count(fd) > 0;
      bool is_uretprobes = uretprobe_fds_to_function.count(fd) > 0;

      int32_t read_from_this_buffer = 0;
      // Read up to ROUND_ROBIN_BATCH_SIZE (5) new events.
      // TODO: Some event types (e.g., stack samples) have a much longer
      //  processing time but are less frequent than others (e.g., context
      //  switches). Take this into account in our scheduling algorithm.
      while (ring_buffer.HasNewData() &&
             read_from_this_buffer < ROUND_ROBIN_BATCH_SIZE) {
        if (*exit_requested) {
          break;
        }

        read_from_this_buffer++;
        last_iteration_saw_events = true;
        perf_event_header header{};
        ring_buffer.ReadHeader(&header);

        // perf_event_header::type contains the type of record, e.g.,
        // PERF_RECORD_SAMPLE, PERF_RECORD_MMAP, etc., defined in enum
        // perf_event_type in perf_event.h.
        switch (header.type) {
          // non system-wide profiling:
          case PERF_RECORD_SWITCH: {
            auto event =
                ring_buffer.ConsumeRecord<LinuxContextSwitchEvent>(header);
            if (event.IsSwitchOut()) {
              ++Capture::GNumContextSwitches;

              ContextSwitch context_switch(ContextSwitch::Out);
              context_switch.m_ThreadId = event.TID();
              context_switch.m_Time = event.Timestamp();
              context_switch.m_ProcessorIndex = event.CPU();
              context_switch.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(context_switch);
            } else {
              ++Capture::GNumContextSwitches;

              ContextSwitch context_switch(ContextSwitch::In);
              context_switch.m_ThreadId = event.TID();
              context_switch.m_Time = event.Timestamp();
              context_switch.m_ProcessorIndex = event.CPU();
              context_switch.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(context_switch);
            }

            ++sched_switch_count;
          } break;

          // system-wide profiling
          case PERF_RECORD_SWITCH_CPU_WIDE: {
            auto event =
                ring_buffer.ConsumeRecord<LinuxSystemWideContextSwitchEvent>(
                    header);
            if (event.PrevTID() != 0) {
              ++Capture::GNumContextSwitches;

              ContextSwitch context_switch(ContextSwitch::Out);
              context_switch.m_ThreadId = event.PrevTID();
              context_switch.m_Time = event.Timestamp();
              context_switch.m_ProcessorIndex = event.CPU();
              context_switch.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(context_switch);
            }

            // record start of excetion
            if (event.NextTID() != 0) {
              ++Capture::GNumContextSwitches;

              ContextSwitch context_switch(ContextSwitch::In);
              context_switch.m_ThreadId = event.NextTID();
              context_switch.m_Time = event.Timestamp();
              context_switch.m_ProcessorIndex = event.CPU();
              context_switch.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(context_switch);
            }

            ++sched_switch_count;
          } break;

          case PERF_RECORD_FORK: {
            auto fork = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);

            if (fork.PID() == pid_) {
              // A new thread of the sampled process was spawned.
              int32_t sample_fd = LinuxPerfUtils::sample_mmap_task_event_open(
                  fork.TID(), sampling_period_ns_);
              LinuxPerfUtils::start_capturing(sample_fd);
              // Do not add a new ring buffer to fds_to_ring_buffer here as we
              // are already iterating over fds_to_ring_buffer.
              fds_to_ring_buffer_to_add.emplace_back(
                  sample_fd, LinuxPerfRingBuffer{sample_fd});
              threads_to_fd.emplace(fork.TID(), sample_fd);
            }
          } break;

          case PERF_RECORD_EXIT: {
            auto exit = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);
            if (exit.PID() == pid_) {
              if (threads_to_fd.count(exit.TID()) > 0) {
                int32_t sample_fd = threads_to_fd.at(exit.TID());
                LinuxPerfUtils::stop_capturing(sample_fd);
                close(sample_fd);
                // Do not remove the ring buffer from fds_to_ring_buffer here as
                // we are already iterating over fds_to_ring_buffer.
                fds_to_remove.push_back(sample_fd);
                threads_to_fd.erase(sample_fd);
              }
            }
          } break;

          case PERF_RECORD_MMAP: {
            // There was a call to mmap with PROT_EXEC, hence refresh the maps.
            // This should happen rarely.
            ring_buffer.SkipRecord(header);
            uprobe_event_processor.AddEvent(
                fd,
                std::make_unique<LinuxMapsEvent>(OrbitTicks(CLOCK_MONOTONIC),
                                                 LinuxUtils::ReadMaps(pid_)));
          } break;

          case PERF_RECORD_SAMPLE: {
            if (is_uprobes) {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxUprobeEventWithStack>(header);
              sample.SetFunction(uprobe_fds_to_function.at(fd));
              uprobe_event_processor.AddEvent(
                  fd, std::make_unique<LinuxUprobeEventWithStack>(
                          std::move(sample)));

              ++uprobes_count;

            } else if (is_uretprobes) {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxUretprobeEventWithStack>(
                      header);
              sample.SetFunction(uretprobe_fds_to_function.at(fd));
              uprobe_event_processor.AddEvent(
                  fd, std::make_unique<LinuxUretprobeEventWithStack>(
                          std::move(sample)));

              ++uprobes_count;

            } else {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxStackSampleEvent>(header);
              uprobe_event_processor.AddEvent(
                  fd,
                  std::make_unique<LinuxStackSampleEvent>(std::move(sample)));

              ++sample_count;
            }
          } break;

          case PERF_RECORD_LOST: {
            auto lost = ring_buffer.ConsumeRecord<LinuxPerfLostEvent>(header);
            PRINT("Lost %u events\n", lost.Lost());
          } break;

          default: {
            PRINT("Unexpected perf_event_header::type: %u\n", header.type);
            ring_buffer.SkipRecord(header);
          } break;
        }

        if (event_count_window_begin_ns == 0) {
          event_count_window_begin_ns = OrbitTicks(CLOCK_MONOTONIC);
        } else if (event_count_window_begin_ns +
                       EVENT_COUNT_WINDOW_S * 1'000'000'000 <
                   OrbitTicks(CLOCK_MONOTONIC)) {
          PRINT(
              "Events per second (last %lu s): "
              "sched switches: %lu; "
              "samples: %lu; "
              "u(ret)probes: %lu\n",
              EVENT_COUNT_WINDOW_S, sched_switch_count / EVENT_COUNT_WINDOW_S,
              sample_count / EVENT_COUNT_WINDOW_S,
              uprobes_count / EVENT_COUNT_WINDOW_S);
          sched_switch_count = 0;
          sample_count = 0;
          uprobes_count = 0;
          event_count_window_begin_ns = OrbitTicks(CLOCK_MONOTONIC);
        }
      }
    }

    uprobe_event_processor.ProcessOldEvents();

    for (auto& fd_to_ring_buffer_to_add : fds_to_ring_buffer_to_add) {
      fds_to_ring_buffer.emplace(std::move(fd_to_ring_buffer_to_add));
    }
    for (int32_t fd_to_remove : fds_to_remove) {
      fds_to_ring_buffer.erase(fd_to_remove);
    }
  }

  uprobe_event_processor.ProcessAllEvents();

  // Stop recording and close the file descriptors.
  for (auto& fd_to_ring_buffer : fds_to_ring_buffer) {
    const int32_t& fd = fd_to_ring_buffer.first;
    LinuxPerfUtils::stop_capturing(fd);
    close(fd);
  }
  fds_to_ring_buffer.clear();
}

bool LinuxEventTracerThread::ComputeSamplingPeriodNs() {
  double period_ns_dbl = 1'000'000'000 / sampling_frequency_;
  if (period_ns_dbl > 0 &&
      period_ns_dbl <
          static_cast<double>(std::numeric_limits<uint64_t>::max())) {
    sampling_period_ns_ = static_cast<uint64_t>(period_ns_dbl);
    return true;
  } else {
    PRINT("Invalid frequency: %.3f\n", sampling_frequency_);
    return false;
  }
}

void LinuxEventTracerThread::LoadNumCpus() {
  num_cpus_ = static_cast<int>(std::thread::hardware_concurrency());
  // Some compilers does not support hardware_concurrency.
  if (num_cpus_ == 0) {
    num_cpus_ = std::stoi(LinuxUtils::ExecuteCommand("nproc"));
  }
}
