//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include <queue>
#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"

// A comparator used for the priority queue, such that pop/top will
// always return the oldest event in the queue.
class TimestampCompare
{
public:
    bool operator() (std::shared_ptr<LinuxPerfEvent> a_LHS, std::shared_ptr<LinuxPerfEvent> a_RHS)
    {
        return a_LHS->Timestamp() > a_RHS->Timestamp();
    }
};

class LinuxPerfEventProcessor
{
public:
    // While processing, we do not touch the events with a timestamp less 
    // than 1/100 sec smaller than the most recent one in the queue.
    // This way we can ensure, that all events (from different sources)
    // are processed in the correct order.
    uint64_t OFFSET_IN_NS = 10000000 /*ns*/;

    LinuxPerfEventProcessor(std::unique_ptr<LinuxPerfEventVisitor> a_Visitor) :
        m_Visitor(std::move(a_Visitor))
    { 
    }

    void Push(std::shared_ptr<LinuxPerfEvent> a_Event)
    {
        uint64_t timestamp = a_Event->Timestamp();
        if (timestamp > m_MaxTimestamp)
            m_MaxTimestamp = timestamp;
        m_EventQueue.push(a_Event);
    }

    void ProcessAll()
    {
        while(!m_EventQueue.empty())
        {
            std::shared_ptr<LinuxPerfEvent> event = m_EventQueue.top();
            event->accept(m_Visitor.get());
            m_EventQueue.pop();
        }
    }

    void ProcessTillOffset()
    {
        // Process the events in the event_queue
        while(!m_EventQueue.empty())
        {
            std::shared_ptr<LinuxPerfEvent> event = m_EventQueue.top();

            // We should not read all events, otherwise we could miss events
            // close to the max timestamp in the queue.
            if (event->Timestamp() + OFFSET_IN_NS > m_MaxTimestamp) 
                break;
            event->accept(m_Visitor.get());
            m_EventQueue.pop();
        }
    }

private:
    std::priority_queue<
        std::shared_ptr<LinuxPerfEvent>, 
        std::vector<std::shared_ptr<LinuxPerfEvent>>, 
        TimestampCompare
    > m_EventQueue;

    std::unique_ptr<LinuxPerfEventVisitor> m_Visitor;

    uint64_t m_MaxTimestamp = 0;
};