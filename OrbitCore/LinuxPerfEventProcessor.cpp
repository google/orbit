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
    if (m_LastProcessTimestamp > 0 && timestamp < m_LastProcessTimestamp - (1000000 * DELAY_IN_MS))
        PRINT("Error: processed an event out of order.\n");
    #endif

    if (timestamp > m_MaxTimestamp)
        m_MaxTimestamp = timestamp;

    m_EventQueue.push(std::move(a_Event));
}

void LinuxPerfEventProcessor::ProcessAll()
{   
    while(true)
    {
        std::unique_ptr<LinuxPerfEvent> event = TryDequeue();
        if (event == nullptr)
            break;
        event->accept(m_Visitor.get());
        #ifndef NDEBUG
        m_LastProcessTimestamp = event->Timestamp();
        #endif
    }
}

void LinuxPerfEventProcessor::ProcessTillOffset()
{
    // Process the events in the event_queue
    // We do not lock this read, as otherwise, 
    //  it would lock the complete exceution of 
    //  that function.
    
    while(true)
    {
        std::unique_ptr<LinuxPerfEvent> event = TryDequeue();
        if (event == nullptr)
            break;

        // We should not read all events, otherwise we could miss events
        // close to the max timestamp in the queue.
        if (event->Timestamp() + (1000000 * DELAY_IN_MS) > m_MaxTimestamp) 
            break;
        event->accept(m_Visitor.get());
        #ifndef NDEBUG
        m_LastProcessTimestamp = event->Timestamp();
        #endif
    }
}

std::unique_ptr<LinuxPerfEvent> LinuxPerfEventProcessor::TryDequeue()
{

    ScopeLock lock(m_Mutex);
    if (m_EventQueue.empty())
    {
        return nullptr;
    }
    // needed to do the const_cast here, as pop() is void, 
    // so we need ot move out here
    std::unique_ptr<LinuxPerfEvent> element = std::move(
        const_cast<std::unique_ptr<LinuxPerfEvent>&>(m_EventQueue.top())
    );
    
    m_EventQueue.pop();

    return element;
}