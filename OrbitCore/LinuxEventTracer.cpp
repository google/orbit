//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------


#include "OrbitProcess.h"
#include "PrintVar.h"
#include "LinuxPerfEvent.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"
#include "LinuxEventTracer.h"
#include "LinuxPerfEventProcessor.h"
#include "LinuxUtils.h"
#include "Capture.h"
#include "Params.h"
#include "TimerManager.h"
#include "ContextSwitch.h"
#include "CoreApp.h"

#include <vector>


//-----------------------------------------------------------------------------
void LinuxEventTracer::Start()
{
    PRINT_FUNC;
#if __linux__
    m_ExitRequested = false;

    m_Thread = std::make_shared<std::thread>(&LinuxEventTracer::Run, &m_ExitRequested);
    m_Thread->detach();

#endif
}

//-----------------------------------------------------------------------------
void LinuxEventTracer::Stop()
{
    m_ExitRequested = true;
}

void LinuxEventTracer::Run(bool* a_ExitRequested)
{
    // get the number of cpus
    int cpus = std::thread::hardware_concurrency();
    // some compilers does not support hardware_concurrency.
    if (cpus == 0)
        cpus = std::stoi(LinuxUtils::ExecuteCommand("nproc"));
    
    std::vector<uint32_t> fds;

    if ( GParams.m_SystemWideScheduling )
    {
        // open perf events for all cpus to keep track of process spawning    
        for (int cpu = 0; cpu < cpus; cpu++)
        {
            fds.push_back(LinuxPerfUtils::context_switch_open(-1, cpu));
        }
    }
    else 
    {
        // open_perf_events for all cpus and the PID to keep track of process spawning
        fds.push_back(LinuxPerfUtils::context_switch_open(Capture::GTargetProcess->GetID(), -1));
    }

    std::vector<LinuxPerfRingBuffer> ring_buffers;
    for (int32_t fd : fds)
    {
        ring_buffers.emplace_back(fd);
    
        // start recording via ioctl
        LinuxPerfUtils::start_capturing(fd);
    }

    // keep threads in synch.
    auto threads = LinuxUtils::ListThreads(Capture::GTargetProcess->GetID());
    for (uint64_t tid : threads) {
        Capture::GTargetProcess->AddThreadId(tid);
    }

    bool new_events = false;

    while (!(*a_ExitRequested))
    {
        // If there was nothing new in the last iteration:
        // Lets sleep a bit, such that we are not constantly reading from the buffers
        // and thus wasting cpu time. 10 ms are still way small enought to 
        // not have our buffers overflown and therefore loosing events.
        if (!new_events)
        {
            OrbitSleepMs(10);
        }

        new_events = false;

        // Read from all ring buffers, create events and store them in the event_queue.
        // In order to ensure, no buffer is read constantly while others overflow,
        // we schedule the reading using round robbin like scheduling.
        for (LinuxPerfRingBuffer& ring_buffer : ring_buffers)
        {
            uint32_t i = 0;
            // Read up to ROUND_ROBIN_BATCH_SIZE (5) new events
            while (ring_buffer.HasNewData() && i < ROUND_ROBIN_BATCH_SIZE) {
                i++;
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
                            //TODO: Is this correct?
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
                            //TODO: Is this correct?
                            CS.m_ProcessorNumber = context_switch.CPU();
                            GCoreApp->ProcessContextSwitch( CS );
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
}