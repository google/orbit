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
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"
#include "LinuxPerfEventProcessor.h"
#include "LinuxUtils.h"
#include "OrbitProcess.h"
#include "OrbitModule.h"
#include "Params.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "TimerManager.h"

//-----------------------------------------------------------------------------
void LinuxEventTracer::Start()
{
    PRINT_FUNC;
#if __linux__
    *m_ExitRequested = false;

    m_Thread = std::make_shared<std::thread>(&LinuxEventTracer::Run,
            m_Pid, m_Frequency, m_ExitRequested);
    m_Thread->detach();
#endif
}

//-----------------------------------------------------------------------------
void LinuxEventTracer::Stop()
{
    *m_ExitRequested = true;
}

//-----------------------------------------------------------------------------
void LinuxEventTracer::Run(pid_t a_Pid, int32_t a_Frequency,
    const std::shared_ptr<std::atomic<bool>>& a_ExitRequested)
{
    int cpus = static_cast<int>(std::thread::hardware_concurrency());
    // Some compilers does not support hardware_concurrency.
    if (cpus == 0) {
        cpus = std::stoi(LinuxUtils::ExecuteCommand("nproc"));
    }
    
    std::vector<int32_t> fds;

    if (GParams.m_TrackContextSwitches)
    {
        if ( GParams.m_SystemWideScheduling )
        {
            // perf_event_open for all cpus to keep track of process spawning.
            for (int cpu = 0; cpu < cpus; cpu++)
            {
              fds.push_back(LinuxPerfUtils::context_switch_open(-1, cpu));
            }
        }
        else
        {
            // perf_event_open for all cpus and the PID to keep track of process spawning.
            fds.push_back(LinuxPerfUtils::context_switch_open(Capture::GTargetProcess->GetID(), -1));
        }
    }

    // TODO(b/148209993): Consider switching to CPU-wide profiling.
    for (pid_t tid : LinuxUtils::ListThreads(Capture::GTargetProcess->GetID())) {
        // Keep threads in sync.
        Capture::GTargetProcess->AddThreadId(tid);

        if (!GParams.m_SampleWithPerf) {
            fds.push_back(LinuxPerfUtils::stack_sample_event_open(
                    tid, a_Frequency));
        }
    }

    LibunwindstackUnwinder unwinder{};
    unwinder.SetMaps(LinuxUtils::ReadMaps(a_Pid));

    std::vector<LinuxPerfRingBuffer> ring_buffers;
    for (int32_t fd : fds)
    {
        ring_buffers.emplace_back(fd);
    
        // Start recording via ioctl.
        LinuxPerfUtils::start_capturing(fd);
    }

    bool new_events = false;

    while (!(*a_ExitRequested))
    {
        // If there was nothing new in the last iteration:
        // Lets sleep a bit, so that we are not constantly reading from the
        // buffers and thus wasting cpu time. 10 ms are still small enough to
        // not have our buffers overflow and therefore lose events.
        if (!new_events)
        {
            OrbitSleepMs(10);
        }

        new_events = false;

        // Read from all ring buffers, create events and store them in the event_queue.
        // In order to ensure that no buffer is read constantly while others overflow,
        // we schedule the reading using round-robin like scheduling.
        for (LinuxPerfRingBuffer& ring_buffer : ring_buffers)
        {
            uint32_t read_from_this_buffer = 0;
            // Read up to ROUND_ROBIN_BATCH_SIZE (5) new events
            while (ring_buffer.HasNewData() && read_from_this_buffer < ROUND_ROBIN_BATCH_SIZE) {
                read_from_this_buffer++;
                new_events = true;
                perf_event_header header{};
                ring_buffer.ReadHeader(&header);

                // perf_event_header::type contains the type of record, e.g.,
                //  PERF_RECORD_SAMPLE, PERF_RECORD_MMAP, etc., defined in enum
                //  perf_event_type in perf_event.h.
                switch (header.type)
                {
                // non system-wide profiling:
                case PERF_RECORD_SWITCH:
                    {
                        auto context_switch = ring_buffer.ConsumeRecord<LinuxContextSwitchEvent>(header);
                        if ( context_switch.IsSwitchOut() )
                        {
                            ++Capture::GNumContextSwitches;

                            ContextSwitch CS ( ContextSwitch::Out );
                            CS.m_ThreadId = context_switch.TID();
                            CS.m_Time = context_switch.Timestamp();
                            CS.m_ProcessorIndex = context_switch.CPU();
                            //TODO: Is this correct?
                            CS.m_ProcessorNumber = context_switch.CPU();
                            GCoreApp->ProcessContextSwitch( CS );
                        }
                        else
                        {
                            ++Capture::GNumContextSwitches;

                            ContextSwitch CS ( ContextSwitch::In );
                            CS.m_ThreadId = context_switch.TID();
                            CS.m_Time = context_switch.Timestamp();
                            CS.m_ProcessorIndex = context_switch.CPU();
                            //TODO: Is this correct?
                            CS.m_ProcessorNumber = context_switch.CPU();
                            GCoreApp->ProcessContextSwitch( CS );
                        }
                    }
                    break;

                // system-wide profiling
                case PERF_RECORD_SWITCH_CPU_WIDE:
                    {
                        auto context_switch = ring_buffer.ConsumeRecord<LinuxSystemWideContextSwitchEvent>(header);
                        if (context_switch.PrevTID() != 0)
                        {
                            ++Capture::GNumContextSwitches;

                            ContextSwitch CS ( ContextSwitch::Out );
                            CS.m_ThreadId = context_switch.PrevTID();
                            CS.m_Time = context_switch.Timestamp();
                            CS.m_ProcessorIndex = context_switch.CPU();
                            // TODO: Is this correct?
                            CS.m_ProcessorNumber = context_switch.CPU();
                            GCoreApp->ProcessContextSwitch( CS );
                        }

                        // record start of excetion
                        if (context_switch.NextTID() != 0)
                        {
                            ++Capture::GNumContextSwitches;

                            ContextSwitch CS ( ContextSwitch::In );
                            CS.m_ThreadId = context_switch.NextTID();
                            CS.m_Time = context_switch.Timestamp();
                            CS.m_ProcessorIndex = context_switch.CPU();
                            // TODO: Is this correct?
                            CS.m_ProcessorNumber = context_switch.CPU();
                            GCoreApp->ProcessContextSwitch( CS );
                        }
                    }
                    break;

                case PERF_RECORD_FORK:
                    {
                        auto fork = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);

                        if (fork.PID() == a_Pid) {
                            // A new thread of the sampled process was spawned:
                            // start sampling it.
                            int32_t fd = LinuxPerfUtils::stack_sample_event_open(
                                    fork.TID(), a_Frequency);
                            fds.push_back(fd);
                            ring_buffers.emplace_back(fd);
                            LinuxPerfUtils::start_capturing(fd);
                        }
                    }
                    break;

                case PERF_RECORD_EXIT:
                    {
                        auto exit = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);
                        if (exit.PID() == a_Pid) {
                            // TODO: stop event recording, close ring buffer, close file descriptor.
                        }
                    }
                    break;

                case PERF_RECORD_MMAP:
                    {
                        // There was a call to mmap with PROT_EXEC, hence
                        // refresh the maps. This should happen rarely.
                        ring_buffer.SkipRecord(header);
                        unwinder.SetMaps(LinuxUtils::ReadMaps(a_Pid));
                    }
                    break;

                case PERF_RECORD_SAMPLE:
                    {
                        auto sample = ring_buffer.ConsumeRecord<LinuxStackSampleEvent>(header);

                        std::vector<unwindstack::FrameData> frames =
                            unwinder.Unwind(sample.Registers(), sample.StackDump(), sample.StackSize());
                        if (!frames.empty()) {
                            HandleCallstack(sample.TID(), sample.Timestamp(), frames);
                        }
                    }
                    break;

                case PERF_RECORD_LOST:
                    {
                        auto lost = ring_buffer.ConsumeRecord<LinuxPerfLostEvent>(header);
                        PRINT("Lost %u Events\n", lost.Lost());
                    }
                    break;

                default:
                    PRINT("Unexpected Perf Sample Type: %u\n", header.type);
                    ring_buffer.SkipRecord(header);
                    break;
                }
            }
        }
    }

    // stop capturing
    for (int32_t fd : fds)
    {
        LinuxPerfUtils::stop_capturing(fd);
    }
    ring_buffers.clear();
    for (int32_t fd : fds)
    {
        close(fd);
    }
}

//-----------------------------------------------------------------------------
void LinuxEventTracer::HandleCallstack(ThreadID tid, uint64_t timestamp,
                                       const std::vector<unwindstack::FrameData>& frames) {
  CallStack cs;
  cs.m_ThreadId = tid;
  for (const auto& frame : frames) {
      // TODO: Avoid repeating symbol resolution by caching already seen PCs.
      std::wstring moduleName = ToLower(Path::GetFileName(s2ws(frame.map_name)));
      std::shared_ptr<Module> moduleFromName = Capture::GTargetProcess->GetModuleFromName( ws2s(moduleName) );

      uint64_t address = frame.pc;
      if (moduleFromName) {
          uint64_t new_address = moduleFromName->ValidateAddress(address);
          address = new_address;
      }
      cs.m_Data.push_back(address);

      if(!frame.function_name.empty() && !Capture::GTargetProcess->HasSymbol(address)) {
          std::stringstream ss;
          ss << LinuxUtils::Demangle(frame.function_name.c_str()) << "+0x" << std::hex << frame.function_offset;
          GCoreApp->AddSymbol(address, frame.map_name, ss.str());
      }
  }
  cs.m_Depth = cs.m_Data.size();

  LinuxCallstackEvent callstack_event{"", timestamp, 1, cs};
  GCoreApp->ProcessSamplingCallStack(callstack_event);
}