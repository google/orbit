//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------

#include "LinuxPerfEventProcessor.h"
#include <queue>

void LinuxPerfEventProcessor::Push(std::unique_ptr<LinuxPerfEvent> a_Event)
{
    ScopeLock lock(m_Mutex);
    const uint64_t timestamp = a_Event->Timestamp();
    #ifndef NDEBUG
    if (m_LastProcessTimestamp > 0 && timestamp < m_LastProcessTimestamp - DELAY_IN_NS)
        PRINT("Error: processed an event out of order.\n");
    #endif

    if (timestamp > m_MaxTimestamp)
        m_MaxTimestamp = timestamp;

    m_EventQueue.push(std::move(a_Event));
}

void LinuxPerfEventProcessor::ProcessAll()
{
    ScopeLock lock(m_Mutex);
    while(!m_EventQueue.empty())
    {
        LinuxPerfEvent* event = m_EventQueue.top().get();
        event->accept(m_Visitor.get());
        #ifndef NDEBUG
        m_LastProcessTimestamp = event->Timestamp();
        #endif
        m_EventQueue.pop();
    }
}

void LinuxPerfEventProcessor::ProcessTillOffset()
{
    // Process the events in the event_queue
    // We do not lock this read, as otherwise, 
    //  it would lock the complete exceution of 
    //  that function.
    while(!m_EventQueue.empty())
    {
        ScopeLock lock(m_Mutex);
        LinuxPerfEvent* event = m_EventQueue.top().get();

        // We should not read all events, otherwise we could miss events
        // close to the max timestamp in the queue.
        if (event->Timestamp() + DELAY_IN_NS > m_MaxTimestamp) 
            break;
        event->accept(m_Visitor.get());
        #ifndef NDEBUG
        m_LastProcessTimestamp = event->Timestamp();
        #endif
        m_EventQueue.pop();
    }
}