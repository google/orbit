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
#include "LinuxEventTracerVisitor.h"
#include "LinuxPerfEventProcessor.h"
#include "LinuxUtils.h"
#include "Capture.h"
#include "Params.h"
#include "TimerManager.h"
#include "ContextSwitch.h"
#include "ConnectionManager.h"

#include <vector>


//-----------------------------------------------------------------------------
void LinuxEventTracer::Start()
{
    PRINT_FUNC;
#if __linux__
    m_ExitRequested = false;

    // TODO: DISABLE REMOTE SCHEDULER INFORMATION TRACING UNTIL WE ACTUALLY SEND THE EVENTS.
    if ( !ConnectionManager::Get().IsService() )
    {
        m_Thread = std::make_shared<std::thread>(&LinuxEventTracer::Run, &m_ExitRequested);
        m_Thread->detach();
    }
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
        int cpus = std::stoi(LinuxUtils::ExecuteCommand("nproc"));

    uint64_t tracepoint_ID = LinuxUtils::GetTracePointID("sched", "sched_switch");
    
    std::vector<uint32_t> fds;

    // open perf events for all cpus to keep track of process spawning
    for (int cpu = 0; cpu < cpus; cpu++)
    {
        if ( !GParams.m_SystemWideScheduling )
        {
            fds.push_back(LinuxPerfUtils::task_event_open(cpu)); 
        }
        fds.push_back(LinuxPerfUtils::tracepoint_event_open(tracepoint_ID, -1 /*pid*/, cpu)); 
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

    // We will only use that buffer on non-system wide profiling.
    // On non-system wide profiling, we want only process context-switches that
    //  of threads that correspond to that process. This requires us to keep track of
    //  all threads being created by that process (fork events). As these all events,
    //  come from different ring buffers, we must ensure that we will not process a 
    //  context-switch before processing the corresponding fork event. Otherwise, we would
    //  not add this context-switch to the UI.
    //  The event_buffer, ensures that we process all events in synch to their timestamp.
    // TODO: Instead of using this buffer, which allows use to process all events in the
    //  order of their timestamp, we could do the following:
    //    1) When receiving a context-switch:
    //     a) If the TID already belongs to the PID => process the context-switch
    //     b) Otherwise => add the context-switch to a queue for this TID.
    //    2) When receiving a fork creating a TID belonging to this PID 
    //     => process all events queued for this TID.
    LinuxPerfEventProcessor event_buffer(std::make_unique<LinuxEventTracerVisitor>());

    bool new_events = false;

    while (!(*a_ExitRequested))
    {
        // If there was nothing new in the last iteration:
        // Lets sleep a bit, such that we are not constantly reading from the buffers
        // and thus wasting cpu time. 10000 microseconds are still way small enought to 
        // not have our buffers overflown and therefore loosing events.
        if (!new_events)
        {
            usleep(10000);
        }

        new_events = false;

        // Read from all ring buffers, create events and store them in the event_queue.
        // In order to ensure, no buffer is read constantly while others overflow,
        // we schedule the reading using round robbin like scheduling.
        for (LinuxPerfRingBuffer& ring_buffer : ring_buffers)
        {
            int i = 0;
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
                case PERF_RECORD_SAMPLE:
                    {
                        auto sched_switch = ring_buffer.ConsumeRecord<LinuxSchedSwitchEvent>(header);
                        if ( GParams.m_SystemWideScheduling )
                        {
                            // record end of excetion
                            if (sched_switch.PrevTID() != 0)
                            {
                                ++Capture::GNumContextSwitches;

                                ContextSwitch CS ( ContextSwitch::Out );
                                CS.m_ThreadId = sched_switch.PrevTID();
                                CS.m_Time = sched_switch.Timestamp();
                                CS.m_ProcessorIndex = sched_switch.CPU();
                                //TODO: Is this correct?
                                CS.m_ProcessorNumber = sched_switch.CPU();
                                GTimerManager->Add( CS );
                            }

                            // record start of excetion
                            if (sched_switch.NextTID() != 0)
                            {
                                ++Capture::GNumContextSwitches;

                                ContextSwitch CS ( ContextSwitch::In );
                                CS.m_ThreadId = sched_switch.NextTID();
                                CS.m_Time = sched_switch.Timestamp();
                                CS.m_ProcessorIndex = sched_switch.CPU();
                                //TODO: Is this correct?
                                CS.m_ProcessorNumber = sched_switch.CPU();
                                GTimerManager->Add( CS );
                            }
                        }
                        else
                        {
                            // we need to know the type of the perf_record_sample at compile time,
                            //  i.e. for multiple perf_event_open calls, we need be able to
                            //  distinguish the different ring buffers.
                            event_buffer.Push(std::make_unique<LinuxSchedSwitchEvent>(std::move(sched_switch)));
                        }
                    }
                    break;
                case PERF_RECORD_LOST:
                    {
                        auto lost = ring_buffer.ConsumeRecord<LinuxPerfLostEvent>(header);
                        PRINT("Lost %u Events\n", lost.Lost());
                    }
                    break;
                // the next two events will not occur when collecting system wide information
                case PERF_RECORD_FORK:
                    {
                        auto fork = ring_buffer.ConsumeRecord<LinuxForkEvent>(header);
                        event_buffer.Push(std::make_unique<LinuxForkEvent>(std::move(fork)));
                    }
                    break;
                case PERF_RECORD_EXIT:
                    // TODO: here we could easily remove the threads from the process again.
                    ring_buffer.SkipRecord(header);
                    break;
                default:
                    PRINT("Unexpected Perf Sample Type: %u", header.type);
                    ring_buffer.SkipRecord(header);
                    break;
                }
            }
        }

        // will be empty when collecting system wide information
        event_buffer.ProcessTillOffset();
    }

    // stop capturing
    for (int32_t fd : fds)
    {
        LinuxPerfUtils::stop_capturing(fd);
    }


    // will be empty when collecting system wide information
    event_buffer.ProcessAll();
}