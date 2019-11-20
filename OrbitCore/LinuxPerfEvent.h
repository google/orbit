//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "PrintVar.h"
#include "LinuxPerfUtils.h"

class LinuxPerfEventVisitor;

// Subclasses of this class, should be in synch with the
// structs in LinuxPerfEvent.h
class LinuxPerfEvent
{
public:
    LinuxPerfEvent(uint64_t a_Timestamp) : m_Timestamp(a_Timestamp) { }

    uint64_t Timestamp() { return m_Timestamp; }

    virtual void accept(LinuxPerfEventVisitor* a_Visitor) = 0;
private:
    uint64_t m_Timestamp;
};

class LinuxPerfLostEvent : public LinuxPerfEvent
{
public:
    LinuxPerfLostEvent(uint64_t a_Timestamp, uint32_t a_Lost): 
        LinuxPerfEvent(a_Timestamp), m_Lost(a_Lost)
    {}

    LinuxPerfLostEvent(const perf_event_lost& a_LostEvent) : 
        LinuxPerfLostEvent(a_LostEvent.sample_id.time, a_LostEvent.lost)
    {}

    uint32_t Lost() { return m_Lost; }

    void accept(LinuxPerfEventVisitor* a_Visitor) override;
private:
    uint32_t m_Lost;
};

class LinuxForkExitEvent : public LinuxPerfEvent
{
public:
    LinuxForkExitEvent(
        uint64_t a_Timestamp, uint32_t a_PID, uint32_t a_PPID,
        uint32_t a_TID, uint32_t a_PTID
    ) : LinuxPerfEvent(a_Timestamp), m_PID(a_PID),
        m_PPID(a_PPID), m_TID(a_TID), m_PTID(a_PTID)
    {
    }
    uint32_t PID() { return m_PID; }
    uint32_t ParentPID() { return m_PPID; }
    uint32_t TID() { return m_TID; }
    uint32_t ParentTID() { return m_PTID; }

private:
    uint32_t    m_PID, m_PPID;
    uint32_t    m_TID, m_PTID;
};

class LinuxForkEvent : public LinuxForkExitEvent
{
public:
    LinuxForkEvent(
        uint64_t a_Timestamp, uint32_t a_PID, uint32_t a_PPID,
        uint32_t a_TID, uint32_t a_PTID
    ) : LinuxForkExitEvent( a_Timestamp, a_PID, a_PPID, a_TID, a_PTID )
    {
    }

    explicit LinuxForkEvent(const perf_event_fork_exit& a_ForkEvent) : 
        LinuxForkExitEvent(
            a_ForkEvent.time, a_ForkEvent.pid, a_ForkEvent.ppid, 
            a_ForkEvent.tid, a_ForkEvent.ptid
        ) {}

    void accept(LinuxPerfEventVisitor* v) override;
};

// TODO: this could also contain the call stack information (either raw or processed).
class LinuxPerfRecordEvent : public LinuxPerfEvent
{
public:
    LinuxPerfRecordEvent(
        uint64_t a_Timestamp, uint32_t a_PID, uint32_t a_TID, uint32_t a_CPU
    ) : LinuxPerfEvent(a_Timestamp), m_PID(a_PID), m_TID(a_TID), m_CPU(a_CPU) 
    {
    }

    uint32_t PID() { return m_PID; }
    uint32_t TID() { return m_TID; }
    uint32_t CPU() { return m_CPU; }
private:
    uint32_t                m_PID, m_TID;
    uint32_t                m_CPU;
};

// TODO: we could also add the names, a.k.a. comm.
//  as well as the prev/next prio
class LinuxSchedSwitchEvent : public LinuxPerfRecordEvent
{
public:
    LinuxSchedSwitchEvent(
        uint64_t a_Timestamp, uint32_t a_PID, uint32_t a_TID, uint32_t a_CPU,
        uint32_t a_PrevPID, uint64_t a_PrevState, uint32_t a_NextPID
    ) : LinuxPerfRecordEvent(a_Timestamp, a_PID, a_TID, a_CPU),
        m_PrevPID(a_PrevPID), m_PrevState(a_PrevState), m_NextPID(a_NextPID)
    {
    }

    explicit LinuxSchedSwitchEvent(const perf_event_record<sched_switch_tp>& a_SchedSwitchEvent) 
    : LinuxSchedSwitchEvent(
        a_SchedSwitchEvent.time, a_SchedSwitchEvent.pid, a_SchedSwitchEvent.tid, 
        a_SchedSwitchEvent.cpu, a_SchedSwitchEvent.data.prev_pid, 
        a_SchedSwitchEvent.data.prev_state, a_SchedSwitchEvent.data.next_pid
    )
    {
    }

    int32_t PrevPID() { return m_PrevPID; }
    int64_t PrevState() { return m_PrevState; }
    int32_t NextPID() { return m_NextPID; }

    void accept(LinuxPerfEventVisitor* a_Visitor) override;
private:
	int32_t m_PrevPID;
	int64_t m_PrevState;
	int32_t m_NextPID;
};