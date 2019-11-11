//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------

#include "ContextSwitch.h"
#include "OrbitProcess.h"
#include "PrintVar.h"
#include "LinuxPerfUtils.h"
#include "LinuxPerfEvent.h"
#include "LinuxEventTracer.h"
#include "LinuxUtils.h"
#include "Utils.h"
#include "Capture.h"
#include "TimerManager.h"

#include <assert.h>
#include <functional>
#include <queue>
#include <vector>


//-----------------------------------------------------------------------------
LinuxEventTracer::LinuxEventTracer()
{
    //m_Callback = a_Callback ? a_Callback : [this](const std::string& a_Buffer)
    //{
    //    CommandCallback(a_Buffer);
    //};
}

//-----------------------------------------------------------------------------
void LinuxEventTracer::Start()
{
    PRINT_FUNC;
#if __linux__
    m_ExitRequested = false;

    m_Thread = std::make_shared<std::thread>(&LinuxEventTracer::foo, &m_ExitRequested);

   /* m_Thread = std::make_shared<std::thread>
        ( &LinuxUtils::StreamCommandOutput
        , "perf record -e sched:sched_switch -o - | perf script -i -"
        , m_Callback
        , &m_ExitRequested );
    */
    m_Thread->detach();
#endif
}

//-----------------------------------------------------------------------------
void LinuxEventTracer::Stop()
{
    m_ExitRequested = true;
}


//-----------------------------------------------------------------------------
void LinuxEventTracer::CommandCallback(std::string a_Line)
{
    std::vector<std::string> tokens = Tokenize(a_Line);
    
    // TODO: log broken lines
    if (tokens[4] != "sched:sched_switch:" || tokens[8] != "==>")
    {
        return;
    }

    auto oldTID = atoi(tokens[1].c_str());
    auto cpu = atoi(trim(tokens[2], "[]").c_str());
    auto time = GetMicros(tokens[3])*1000;
    char state = tokens[7].at(0);

    auto newCommCPUTID = Tokenize(tokens[9], "/:");
    auto newTID = atoi(newCommCPUTID.back().c_str());

    // TODO: only process context switches that relate to the traced process.

    ContextSwitch::SwitchType type = ContextSwitch::Invalid;
    int tid = -1;
    switch (state)
    {
    // sleep:
    case 'S':
        //if (!Capture::GTargetProcess->HasThread(oldTID))
        //    return;

        type = ContextSwitch::Out;
        tid = oldTID;
        break;
    
    // run:
    case 'R':
        //if (!Capture::GTargetProcess->HasThread(newTID))
        //    return;

        type = ContextSwitch::In;
        tid = newTID;
        break;

    // TODO: handle the other cases

    default:
        return;
    }

    if (state != ContextSwitch::Invalid && tid != -1)
    {
        ++Capture::GNumContextSwitches;

        ContextSwitch CS ( type );
        CS.m_ThreadId = tid;
        CS.m_Time = time;
        CS.m_ProcessorIndex = cpu;
        //TODO: Is this correct?
        CS.m_ProcessorNumber = cpu;
        GTimerManager->Add( CS );
    }
}

void LinuxEventTracer::foo(bool* a_ExitRequested)
{
    int cpus = std::stoi(LinuxUtils::ExecuteCommand("nproc"));
    uint64_t tracepointID = LinuxUtils::GetTracePointID("sched", "sched_switch");
    std::vector<uint32_t> fds;

    // open perf events for all cpus to keep track of process spawning
    for (int cpu = 0; cpu < cpus; cpu++)
    {
        fds.push_back(LinuxPerfUtils::task_event_open(cpu)); 
        fds.push_back(LinuxPerfUtils::tracepoint_event_open(tracepointID, -1 /*pid*/, cpu)); 
    }


    // TODO: wrap the ringbuffers into a class
    std::vector<perf_event_mmap_page*> metadatas;
    std::vector<char*> ring_buffers;
    std::vector<uint64_t> ring_buffer_lengths;
    for (int32_t fd : fds)
    {
        void* mmap_ret = LinuxPerfUtils::mmap_mapping(fd);

        // the memory directly before the ring buffer contains the metadata
        auto* metadata = reinterpret_cast<perf_event_mmap_page*>(mmap_ret);
        metadatas.push_back(metadata);

        // beginning of the ring buffer
        auto* buffer = reinterpret_cast<char*>(mmap_ret) + metadata->data_offset; 
        ring_buffers.push_back(buffer);
        uint64_t buffer_length = metadata->data_size;
        ring_buffer_lengths.push_back(buffer_length);

    
        // start recording via ioctl
        LinuxPerfUtils::start_capturing(fd);
    }

    // keep threads in synch.
    auto targetProcess = Capture::GTargetProcess;
    auto threads = LinuxUtils::ListThreads(targetProcess->GetID());

    for (uint64_t tid : threads) {
        targetProcess->AddThreadId(tid);
    }

    // create a priority queue to buffer events an process them in sorted by their timestamp
    auto compare = [](std::shared_ptr<LinuxPerfEvent> lhs, 
        std::shared_ptr<LinuxPerfEvent> rhs)
        {
            return lhs->Timestamp() > rhs->Timestamp();
        };

    std::priority_queue<
        std::shared_ptr<LinuxPerfEvent>, 
        std::vector<std::shared_ptr<LinuxPerfEvent>>, 
        decltype(compare)
    > event_queue(compare);


    while (!(*a_ExitRequested))
    {
        while(!event_queue.empty())
        {
            auto event = event_queue.top();
            //TODO: call the visitor
            event_queue.pop();
        }

        // TODO: do we need this sleeping?
        usleep(1000);

        // read from all ring buffers, create events and store them in the event_queue
        for (size_t i = 0; i < ring_buffers.size(); i++)
        {
            perf_event_mmap_page* metadata = metadatas[i];
            char* buffer = ring_buffers[i];
            uint64_t buffer_length = ring_buffer_lengths[i];

            // read everything that is new
            while (metadata->data_tail + sizeof(perf_event_header) <= metadata->data_head) {
                perf_event_header header{};
                LinuxPerfUtils::read_from_ring_buffer(reinterpret_cast<char*>(&header), buffer,
                                        buffer_length, metadata->data_tail,
                                        sizeof(perf_event_header));
                if (header.type == 0) {
                    break;
                }
                if (metadata->data_tail + sizeof(perf_event_header) + header.size >
                    metadata->data_head) {
                    break;
                }

                // perf_event_header::size contains the size of the entire record.
                char* record = reinterpret_cast<char*>(malloc(header.size));
                LinuxPerfUtils::read_from_ring_buffer(record, buffer, buffer_length, metadata->data_tail,
                                        header.size);


                // perf_event_header::type contains the type of record, e.g.,
                // PERF_RECORD_SAMPLE, PERF_RECORD_MMAP, etc., defined in enum
                // perf_event_type in perf_event.h.
                if (header.type == PERF_RECORD_SAMPLE) {
                    // TODO: Check the concrete sample type
                    const perf_event_sched_switch_tp* tp 
                        = reinterpret_cast<perf_event_sched_switch_tp*>(record);
                    auto e = std::make_shared<LinuxSchedSwitchEvent>(tp);
                    event_queue.push(e);
                } else if (header.type == PERF_RECORD_LOST) {
                    perf_event_lost* lost = reinterpret_cast<perf_event_lost*>(record);
                    auto e = std::make_shared<LinuxPerfLostEvent>(lost);
                    PRINT("Lost Events");
                    event_queue.push(e);
                } else if (header.type == PERF_RECORD_FORK) {
                    perf_event_fork_exit* fork = reinterpret_cast<perf_event_fork_exit*>(record);
                    auto e = std::make_shared<LinuxForkEvent>(fork);
                    event_queue.push(e);
                    // TODO: move this to the visitor
                    // todo: check if this is correct:
                    // todo: it might be also correct to check check if ppid == process.id
                    if (targetProcess->HasThread(fork->ptid))
                    {
                        targetProcess->AddThreadId(fork->tid);
                    }
                } else if (header.type == PERF_RECORD_EXIT) {
                    // TODO: here we could easily remove the threads from the process again.
                } else {
                    PRINT("Unexpected Perf Sample Type: %u", header.type);
                }

                free(record);

                // write back how far we read.
                metadata->data_tail += header.size;
            }
        }
    }

    // stop capturing
    for (int32_t fd : fds)
    {
        LinuxPerfUtils::stop_capturing(fd);
    }
}