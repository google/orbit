//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "LinuxEventTracer.h"

#include <vector>

// TODO: Remove all references to BpfTrace.
#include "BpfTraceVisitor.h"
#include "Capture.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "LinuxPerfEvent.h"
#include "LinuxPerfEventProcessor.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"
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

  LinuxPerfEventProcessor uprobe_event_processor(
      std::make_unique<BpfTraceVisitor>());

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

  LibunwindstackUnwinder unwinder{};
  unwinder.SetMaps(LinuxUtils::ReadMaps(pid_));

  // TODO(b/148209993): Sample based on CPU and filter by pid.
  for (pid_t tid : LinuxUtils::ListThreads(pid_)) {
    // Keep threads in sync.
    Capture::GTargetProcess->AddThreadId(tid);

    if (!GParams.m_SampleWithPerf) {
      fds.push_back(LinuxPerfUtils::stack_sample_event_open(tid, sampling_period_ns_));
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
              // TODO: Is this correct?
              context_switch.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(context_switch);
            } else {
              ++Capture::GNumContextSwitches;

              ContextSwitch CS(ContextSwitch::In);
              CS.m_ThreadId = event.TID();
              CS.m_Time = event.Timestamp();
              CS.m_ProcessorIndex = event.CPU();
              // TODO: Is this correct?
              CS.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(CS);
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
              // TODO: Is this correct?
              context_switch.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(context_switch);
            }

            // record start of excetion
            if (event.NextTID() != 0) {
              ++Capture::GNumContextSwitches;

              ContextSwitch CS(ContextSwitch::In);
              CS.m_ThreadId = event.NextTID();
              CS.m_Time = event.Timestamp();
              CS.m_ProcessorIndex = event.CPU();
              // TODO: Is this correct?
              CS.m_ProcessorNumber = event.CPU();
              GCoreApp->ProcessContextSwitch(CS);
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
            unwinder.SetMaps(LinuxUtils::ReadMaps(pid_));
          } break;

          case PERF_RECORD_SAMPLE: {
            if (is_uprobes) {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxUprobeEventWithStack>(header);

              std::vector<unwindstack::FrameData> frames = unwinder.Unwind(
                  sample.Registers(), sample.StackDump(), sample.StackSize());
              if (!frames.empty()) {
                HandleCallstack(sample.TID(), sample.Timestamp(), frames);
              }

              sample.SetFunction(
                  uprobe_fds_to_function.at(ring_buffer.FileDescriptor()));
              uprobe_event_processor.Push(
                  std::make_unique<LinuxUprobeEventWithStack>(
                      std::move(sample)));

            } else if (is_uretprobes) {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxUretprobeEventWithStack>(
                      header);

              std::vector<unwindstack::FrameData> frames = unwinder.Unwind(
                  sample.Registers(), sample.StackDump(), sample.StackSize());
              if (!frames.empty()) {
                HandleCallstack(sample.TID(), sample.Timestamp(), frames);
              }

              sample.SetFunction(
                  uretprobe_fds_to_function.at(ring_buffer.FileDescriptor()));
              uprobe_event_processor.Push(
                  std::make_unique<LinuxUretprobeEventWithStack>(
                      std::move(sample)));

            } else {
              auto sample =
                  ring_buffer.ConsumeRecord<LinuxStackSampleEvent>(header);

              std::vector<unwindstack::FrameData> frames = unwinder.Unwind(
                  sample.Registers(), sample.StackDump(), sample.StackSize());
              if (!frames.empty()) {
                HandleCallstack(sample.TID(), sample.Timestamp(), frames);
              }
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
      int32_t fd =
          LinuxPerfUtils::stack_sample_event_open(new_thread, sampling_period_ns_);
      fds.push_back(fd);
      ring_buffers.emplace_back(fd);
      LinuxPerfUtils::start_capturing(fd);
    }

    uprobe_event_processor.ProcessTillOffset();
  }

  uprobe_event_processor.ProcessAll();

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

void LinuxEventTracerThread::HandleCallstack(
    pid_t tid, uint64_t timestamp,
    const std::vector<unwindstack::FrameData>& frames) {
  CallStack cs;
  cs.m_ThreadId = tid;
  for (const auto& frame : frames) {
    // TODO: Avoid repeating symbol resolution by caching already seen PCs.
    std::wstring moduleName = ToLower(Path::GetFileName(s2ws(frame.map_name)));
    std::shared_ptr<Module> moduleFromName =
        Capture::GTargetProcess->GetModuleFromName(ws2s(moduleName));

    uint64_t address = frame.pc;
    if (moduleFromName) {
      uint64_t new_address = moduleFromName->ValidateAddress(address);
      address = new_address;
    }
    cs.m_Data.push_back(address);

    if (!frame.function_name.empty() &&
        !Capture::GTargetProcess->HasSymbol(address)) {
      std::stringstream ss;
      ss << LinuxUtils::Demangle(frame.function_name.c_str()) << "+0x"
         << std::hex << frame.function_offset;
      GCoreApp->AddSymbol(address, frame.map_name, ss.str());
    }
  }
  cs.m_Depth = cs.m_Data.size();

  LinuxCallstackEvent callstack_event{"", timestamp, 1, cs};
  GCoreApp->ProcessSamplingCallStack(callstack_event);
}
