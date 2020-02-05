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
#include "LinuxPerfEventProcessor.h"
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
    const std::shared_ptr<std::atomic<bool>>& a_ExitRequested) {
  if (!ComputeSamplingPeriodNs()) {
    return;
  }

  LoadNumCpus();

  std::vector<int32_t> fds;
  std::unordered_map<int32_t, Function*> uprobe_fds_to_function;
  std::unordered_map<int32_t, Function*> uretprobe_fds_to_function;

  if (GParams.m_TrackContextSwitches) {
    if (GParams.m_SystemWideScheduling) {
      // perf_event_open for all cpus to keep track of process spawning.
      for (int32_t cpu = 0; cpu < num_cpus_; cpu++) {
        fds.push_back(LinuxPerfUtils::context_switch_open(-1, cpu));
      }
    } else {
      // perf_event_open for all cpus and the PID to keep track of process
      // spawning.
      fds.push_back(LinuxPerfUtils::context_switch_open(pid_, -1));
    }
  }

  LinuxPerfEventProcessor uprobe_event_processor{
      std::make_unique<LinuxUprobesUnwindingVisitor>(
          pid_, LinuxUtils::ReadMaps(pid_))};

  if (!GParams.m_UseBpftrace) {
    for (const auto& function : instrumented_functions_) {
      for (int32_t cpu = 0; cpu < num_cpus_; cpu++) {
        int uprobe_fd = LinuxPerfUtils::uprobe_stack_event_open(
            ws2s(function->m_Pdb->GetFileName()).c_str(), function->m_Address,
            -1, cpu);
        fds.push_back(uprobe_fd);
        uprobe_fds_to_function[uprobe_fd] = function;

        int uretprobe_fd = LinuxPerfUtils::uretprobe_stack_event_open(
            ws2s(function->m_Pdb->GetFileName()).c_str(), function->m_Address,
            -1, cpu);
        fds.push_back(uretprobe_fd);
        uretprobe_fds_to_function[uretprobe_fd] = function;
      }
    }
  }

  // TODO(b/148209993): Sample based on CPU and filter by pid.
  for (pid_t tid : LinuxUtils::ListThreads(pid_)) {
    // Keep threads in sync.
    Capture::GTargetProcess->AddThreadId(tid);

    if (!GParams.m_SampleWithPerf) {
      fds.push_back(
          LinuxPerfUtils::stack_sample_event_open(tid, sampling_period_ns_));
    }
  }

  // TODO: New threads might spawn here before forks can be recorded.

  // Create the perf_event_open ring buffers.
  std::vector<LinuxPerfRingBuffer> ring_buffers;
  ring_buffers.reserve(fds.size());
  for (int32_t fd : fds) {
    ring_buffers.emplace_back(fd);
  }
  // Start capturing.
  for (int32_t fd : fds) {
    LinuxPerfUtils::start_capturing(fd);
  }

  bool new_events = false;

  while (!(*a_ExitRequested)) {
    // If there was nothing new in the last iteration:
    // Lets sleep a bit, so that we are not constantly reading from the
    // buffers and thus wasting cpu time. 10 ms are still small enough to
    // not have our buffers overflow and therefore lose events.
    if (!new_events) {
      OrbitSleepMs(10);
    }

    new_events = false;
    pid_t new_thread = -1;

    // Read from all ring buffers, create events and store them in the
    // event_queue. In order to ensure that no buffer is read constantly while
    // others overflow, we schedule the reading using round-robin like
    // scheduling.
    for (LinuxPerfRingBuffer& ring_buffer : ring_buffers) {
      if (*a_ExitRequested || new_thread >= 0) {
        break;
      }

      bool is_uprobes =
          uprobe_fds_to_function.count(ring_buffer.FileDescriptor()) > 0;
      bool is_uretprobes =
          uretprobe_fds_to_function.count(ring_buffer.FileDescriptor()) > 0;

      uint32_t read_from_this_buffer = 0;
      // Read up to ROUND_ROBIN_BATCH_SIZE (5) new events
      while (ring_buffer.HasNewData() &&
             read_from_this_buffer < ROUND_ROBIN_BATCH_SIZE) {
        if (*a_ExitRequested || new_thread >= 0) {
          break;
        }

        read_from_this_buffer++;
        new_events = true;
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
          } break;

          case PERF_RECORD_FORK: {
            auto fork = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);

            if (fork.PID() == pid_) {
              // A new thread of the sampled process was spawned.
              new_thread = fork.TID();
              // Do not add a new ring buffer to ring_buffers here as we are
              // already iterating over ring_buffers.
            }
          } break;

          case PERF_RECORD_EXIT: {
            auto exit = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);
            if (exit.PID() == pid_) {
              // TODO: stop event recording, close ring buffer, close file
              //  descriptor.
            }
          } break;

          case PERF_RECORD_MMAP: {
            // There was a call to mmap with PROT_EXEC, hence refresh the maps.
            // This should happen rarely.
            ring_buffer.SkipRecord(header);
            uprobe_event_processor.Push(std::make_unique<LinuxMapsEvent>(
                OrbitTicks(CLOCK_MONOTONIC), LinuxUtils::ReadMaps(pid_)));
          } break;

          case PERF_RECORD_SAMPLE: {
            if (is_uprobes) {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxUprobeEventWithStack>(header);
              sample.SetFunction(
                  uprobe_fds_to_function.at(ring_buffer.FileDescriptor()));
              uprobe_event_processor.Push(
                  std::make_unique<LinuxUprobeEventWithStack>(
                      std::move(sample)));

            } else if (is_uretprobes) {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxUretprobeEventWithStack>(
                      header);
              sample.SetFunction(
                  uretprobe_fds_to_function.at(ring_buffer.FileDescriptor()));
              uprobe_event_processor.Push(
                  std::make_unique<LinuxUretprobeEventWithStack>(
                      std::move(sample)));

            } else {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxStackSampleEvent>(header);
              uprobe_event_processor.Push(
                  std::make_unique<LinuxStackSampleEvent>(std::move(sample)));
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
      }
    }

    if (new_thread >= 0) {
      int32_t fd = LinuxPerfUtils::stack_sample_event_open(new_thread,
                                                           sampling_period_ns_);
      fds.push_back(fd);
      ring_buffers.emplace_back(fd);
      LinuxPerfUtils::start_capturing(fd);
    }

    uprobe_event_processor.ProcessOldEvents();
  }

  uprobe_event_processor.ProcessAllEvents();

  // stop capturing
  for (int32_t fd : fds) {
    LinuxPerfUtils::stop_capturing(fd);
  }

  ring_buffers.clear();
  for (int32_t fd : fds) {
    close(fd);
  }
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
