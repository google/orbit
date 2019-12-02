//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "PrintVar.h"
#include "LinuxPerfUtils.h"

using namespace LinuxPerfUtils;

class LinuxPerfEventVisitor;

typedef int NullType[0];

class LinuxPerfEventVisitor;

// We will use the subclasses of LinuxPerfEvent in order to memcpy
// the event data from the ringbuffer and access/process the data via
// their actuall names (and given their types) instead of doing the
// pointer arithmetic by ourself.
// However, this requires the memory layout of the corresponding
// subclass to be exactly the same as the one from the perf data in 
// the ringbuffer (for that particular event).
class LinuxPerfEvent
{
public:
    // On offset 0, in the memory layout of objects of this class,
    //  there will be always the vtable. 
    // This header filed will always be at the beginnning of the data blocks,
    //  containing the sample data. So we can use its address
    //  for memcopy (copy the data from the ringbuffer to this object).
    // We need to make sure, that each subclass has the subsequent data fields
    //  followed by this header.
    struct perf_event_header header;

    virtual uint64_t Timestamp() = 0;
    virtual void accept(LinuxPerfEventVisitor* a_Visitor) = 0;
    LinuxPerfEvent() {};
};

class __attribute__((__packed__)) LinuxForkEvent : public LinuxPerfEvent {
private:
    uint32_t    pid, ppid;
    uint32_t    tid, ptid;
    uint64_t    time;
    struct perf_sample_id sample_id;
public: 
    virtual uint64_t Timestamp() override {
        return time;
    }

    virtual void accept(LinuxPerfEventVisitor* a_Visitor) override;

    uint32_t PID() { return pid; }
    uint32_t ParentPID() { return ppid; }
    uint32_t TID() { return tid; }
    uint32_t ParentTID() { return ptid; }
};

class __attribute__((__packed__)) LinuxPerfLostEvent : public LinuxPerfEvent {
private:
    uint64_t    id;
    uint64_t    lost;
    struct perf_sample_id sample_id;
public:
    virtual uint64_t Timestamp() override {
        return sample_id.time;
    }

    virtual void accept(LinuxPerfEventVisitor* a_Visitor) override;

    uint64_t Lost() {
        return lost;
    }
};

// This struct must be in sync with the SAMPLE_TYPE_FLAGS.
template<typename raw_data_t, typename registers_t, typename stack_t>
class __attribute__((__packed__)) LinuxPerfEventRecord : public LinuxPerfEvent {
protected:
    uint32_t                 pid, tid;   /* if PERF_SAMPLE_TID */
    uint64_t                 time;       /* if PERF_SAMPLE_TIME */
    uint32_t                 cpu, res;   /* if PERF_SAMPLE_CPU */
    raw_data_t               raw_data;   /* if PERF_SAMPLE_RAW */
    registers_t              registers;  /* if PERF_SAMPLE_REGS_USER */
    stack_t                  stack;      /* if PERF_SAMPLE_STACK_USER */

public:
    LinuxPerfEventRecord() {};
    virtual uint64_t Timestamp() override {
        return time;
    }

    uint32_t PID() { return pid; }
    uint32_t TID() { return tid; }
    uint32_t CPU() { return cpu; }

};

// TODO: This struct might change. We should read this from debugfs.
struct __attribute__((__packed__)) sched_switch_tp {
    uint32_t size; /* if PERF_SAMPLE_RAW */
    uint16_t common_type;
    unsigned char common_flags;
    unsigned char common_preempt_count;
    int32_t common_pid;

    char prev_comm[16];
    // this is actually a thread id
    int32_t prev_pid;
    int32_t prev_prio;
    int64_t prev_state;
    char next_comm[16];
    // this is actually a thread id
    int32_t next_pid;
    int32_t next_prio;
};

// Currently, we do not record callstacks for sched. switch events, so we use the
// NullType here.
class LinuxSchedSwitchEvent : public LinuxPerfEventRecord<sched_switch_tp, NullType, NullType>
{
public:
    LinuxSchedSwitchEvent() {};
    virtual void accept(LinuxPerfEventVisitor* a_Visitor) override;

    uint32_t PrevTID() { return raw_data.prev_pid; }
    uint32_t NextTID() { return raw_data.next_pid; }
};