//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "PrintVar.h"
#include "LinuxPerfUtils.h"


class LinuxPerfEventVisitor;

// Subclasses of this class, should be alligned with the
// structs in LinuxPerfEvent.h
class LinuxPerfEvent
{
public:
    LinuxPerfEvent(uint64_t a_Timestamp) : m_Timestamp(a_Timestamp) { }

    uint64_t Timestamp() { return m_Timestamp; }

    virtual void accept(LinuxPerfEventVisitor& v) = 0;
private:
    uint64_t m_Timestamp;
};

// TODO: we probably don't need this class,
// but log lost events immidiatly
class LinuxPerfLostEvent : public LinuxPerfEvent
{
public:
    LinuxPerfLostEvent(uint64_t a_Timestamp, uint32_t a_Lost): 
        LinuxPerfEvent(a_Timestamp), m_Lost(a_Lost)
    {}

    LinuxPerfLostEvent(const perf_event_lost* lost_event) : 
        LinuxPerfLostEvent(lost_event->sample_id.time, lost_event->lost)
    {}

    void accept(LinuxPerfEventVisitor& v) override;
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

    LinuxForkEvent(const perf_event_fork_exit* fork_event) : 
        LinuxForkExitEvent(
            fork_event->time, fork_event->pid, fork_event->ppid, 
            fork_event->tid, fork_event->ptid
        ) {}

    void accept(LinuxPerfEventVisitor& v) override;
};

// TODO: this could also apply for other Perf records such as uprobes.
// TODO: this could also contain the call stack information (either raw or processed).
class LinuxTracePointEvent : public LinuxPerfEvent
{
public:
    LinuxTracePointEvent(
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
class LinuxSchedSwitchEvent : public LinuxTracePointEvent
{
public:
    LinuxSchedSwitchEvent(
        uint64_t a_Timestamp, uint32_t a_PID, uint32_t a_TID, uint32_t a_CPU,
        uint32_t a_PrevPID, uint64_t a_PrevState, uint32_t a_NextPID
    ) : LinuxTracePointEvent(a_Timestamp, a_PID, a_TID, a_CPU),
        m_PrevPID(a_PrevPID), m_PrevState(a_PrevState), m_NextPID(a_NextPID)
    {
        if (a_PrevPID != PID()){
            //PRINT("Unexpected PID's on context switch - Please file a bug.\n");
        }
    }

    LinuxSchedSwitchEvent(const perf_event_sched_switch_tp* sched_switch) 
    : LinuxSchedSwitchEvent(
        sched_switch->time, sched_switch->pid, sched_switch->tid, 
        sched_switch->cpu, sched_switch->data.prev_pid, 
        sched_switch->data.prev_state, sched_switch->data.next_pid
    )
    {
    }

    void accept(LinuxPerfEventVisitor& v) override;
private:
	int32_t m_PrevPID;
	int64_t m_PrevState;
	int32_t m_NextPID;
};