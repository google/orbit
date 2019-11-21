//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
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

    std::vector<std::shared_ptr<LinuxPerfRingBuffer>> ring_buffers;
    for (int32_t fd : fds)
    {
        auto ring_buffer = std::make_shared<LinuxPerfRingBuffer>(fd);
        ring_buffers.push_back(ring_buffer);
    
        // start recording via ioctl
        LinuxPerfUtils::start_capturing(fd);
    }

    // keep threads in synch.
    auto threads = LinuxUtils::ListThreads(Capture::GTargetProcess->GetID());
    for (uint64_t tid : threads) {
        Capture::GTargetProcess->AddThreadId(tid);
    }

    // we will only use that buffer on non-system wide profiling
    LinuxPerfEventProcessor event_buffer(std::make_unique<LinuxEventTracerVisitor>());

    while (!(*a_ExitRequested))
    {
        // Lets sleep a bit, such that we are not constantly reading from the buffers
        // and thus wasting cpu time. 5000 microseconds are still small enought to 
        // not have our buffers overflown and therefore loosing events.
        usleep(1000);

        // read from all ring buffers, create events and store them in the event_queue
        for (std::shared_ptr<LinuxPerfRingBuffer> ring_buffer : ring_buffers)
        {
            // read everything that is new
            while (ring_buffer->HasNewData()) {
                perf_event_header header{};
                ring_buffer->ReadHeader(&header);

                // perf_event_header::type contains the type of record, e.g.,
                //  PERF_RECORD_SAMPLE, PERF_RECORD_MMAP, etc., defined in enum
                //  perf_event_type in perf_event.h.
                switch (header.type)
                {
                case PERF_RECORD_SAMPLE:
                    {
                        auto sched_switch = ring_buffer->ConsumeRecord<perf_event_record<sched_switch_tp>>(header);
                        if ( GParams.m_SystemWideScheduling )
                        {
                            // record end of excetion
                            if (sched_switch.data.prev_pid != 0)
                            {
                                ++Capture::GNumContextSwitches;

                                ContextSwitch CS ( ContextSwitch::Out );
                                CS.m_ThreadId = sched_switch.data.prev_pid;
                                CS.m_Time = sched_switch.time;
                                CS.m_ProcessorIndex = sched_switch.cpu;
                                //TODO: Is this correct?
                                CS.m_ProcessorNumber = sched_switch.cpu;
                                GTimerManager->Add( CS );
                            }

                            // record start of excetion
                            if (sched_switch.data.next_pid != 0)
                            {
                                ++Capture::GNumContextSwitches;

                                ContextSwitch CS ( ContextSwitch::In );
                                CS.m_ThreadId = sched_switch.data.next_pid;
                                CS.m_Time = sched_switch.time;
                                CS.m_ProcessorIndex = sched_switch.cpu;
                                //TODO: Is this correct?
                                CS.m_ProcessorNumber = sched_switch.cpu;
                                GTimerManager->Add( CS );
                            }
                        }
                        else
                        {
                            // we need to know the type of the perf_record_sample at compile time,
                            //  i.e. for multiple perf_event_open calls, we need be able to
                            //  distinguish the different ring buffers.
                            auto e = std::make_shared<LinuxSchedSwitchEvent>(sched_switch);
                            event_buffer.Push(e);
                        }
                    }
                    break;
                case PERF_RECORD_LOST:
                    {
                        auto lost = ring_buffer->ConsumeRecord<perf_event_lost>(header);
                        PRINT("Lost %u Events\n", lost.lost);
                    }
                    break;
                // the next two events will not occur when collecting system wide information
                case PERF_RECORD_FORK:
                    {
                        auto fork = ring_buffer->ConsumeRecord<perf_event_fork_exit>(header);
                        auto e = std::make_shared<LinuxForkEvent>(fork);
                        event_buffer.Push(e);
                    }
                    break;
                case PERF_RECORD_EXIT:
                    // TODO: here we could easily remove the threads from the process again.
                    ring_buffer->SkipRecord(header);
                    break;
                default:
                    PRINT("Unexpected Perf Sample Type: %u", header.type);
                    ring_buffer->SkipRecord(header);
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