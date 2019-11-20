//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "LinuxPerfEvent.h"
#include "LinuxPerfEventVisitor.h"
#include "PrintVar.h"

#include <queue>

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
    // than 1/10 sec smaller than the most recent one in the queue.
    // This way we can ensure, that all events (from different sources)
    // are processed in the correct order.
    uint64_t DELAY_IN_NS = 100000000 /*ns*/;

    LinuxPerfEventProcessor(std::unique_ptr<LinuxPerfEventVisitor> a_Visitor) :
        m_Visitor(std::move(a_Visitor))
    { 
    }

    void Push(std::shared_ptr<LinuxPerfEvent> a_Event);

    void ProcessAll();

    void ProcessTillOffset();

private:
    std::priority_queue<
        std::shared_ptr<LinuxPerfEvent>, 
        std::vector<std::shared_ptr<LinuxPerfEvent>>, 
        TimestampCompare
    > m_EventQueue;

    std::unique_ptr<LinuxPerfEventVisitor> m_Visitor;

    uint64_t m_MaxTimestamp = 0;
    #ifndef NDEBUG
    uint64_t m_LastProcessTimestamp = 0;
    #endif
};