//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include "PrintVar.h"
#include "LinuxPerfUtils.h"

using namespace LinuxPerfUtils;

class LinuxPerfEventVisitor;

class LinuxPerfEventVisitor;

// This base class will be used in order to do processing of
// different perf events using the visitor patter.
// The data of the perf events will be copied from the ring buffer
// via memcpy dirrectly into the concrete subclass (depending on the
// event typ).
// The target of the memcpy will be a field "ring_buffer_data" that
// must be present for the subclass at compile time.
// As the perf_event_open ringbuffer, is 8byte alligned, this field
// must also be extended with dummy bytes at the end of the record.

class LinuxPerfEvent
{
public:
    virtual uint64_t Timestamp() = 0;
    virtual void accept(LinuxPerfEventVisitor* a_Visitor) = 0;
};

struct __attribute__((__packed__)) perf_fork_exit_event {
    struct perf_event_header header;
    uint32_t    pid, ppid;
    uint32_t    tid, ptid;
    uint64_t    time;
    struct perf_sample_id sample_id;
};

class LinuxForkEvent : public LinuxPerfEvent {
public: 
    perf_fork_exit_event ring_buffer_data;

    virtual uint64_t Timestamp() override {
        return ring_buffer_data.time;
    }

    virtual void accept(LinuxPerfEventVisitor* a_Visitor) override;

    uint32_t PID() { return ring_buffer_data.pid; }
    uint32_t ParentPID() { return ring_buffer_data.ppid; }
    uint32_t TID() { return ring_buffer_data.tid; }
    uint32_t ParentTID() { return ring_buffer_data.ptid; }
};

struct __attribute__((__packed__)) perf_lost_event 
{
    struct perf_event_header header;
    uint64_t    id;
    uint64_t    lost;
    struct perf_sample_id sample_id;
};

class LinuxPerfLostEvent : public LinuxPerfEvent {
public:
    perf_lost_event ring_buffer_data;

    virtual uint64_t Timestamp() override {
        return ring_buffer_data.sample_id.time;
    }

    virtual void accept(LinuxPerfEventVisitor* a_Visitor) override;

    uint64_t Lost() {
        return ring_buffer_data.lost;
    }
};

template<typename perf_record_data_t>
class LinuxPerfEventRecord : public LinuxPerfEvent {
public:
    perf_record_data_t ring_buffer_data;

    virtual uint64_t Timestamp() override {
        return ring_buffer_data.record.time;
    }

    uint32_t PID() { return ring_buffer_data.record.pid; }
    uint32_t TID() { return ring_buffer_data.record.tid; }
    uint32_t CPU() { return ring_buffer_data.record.cpu; }
};

// TODO: This struct might change. We should read this from debugfs.
struct __attribute__((__packed__)) sched_switch_trace_point {
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

    uint32_t alignment;
};

struct __attribute__((__packed__)) perf_record_sched_switch_event
{
    struct perf_event_header header;
    struct perf_sample_id basic_sample_data; /* common PERF_SAMPLE fields */
    struct sched_switch_trace_point trace_point; /* PERF_SAMPLE_RAW */
};

// Currently, we do not record callstacks for sched.
class LinuxSchedSwitchEvent : public LinuxPerfEventRecord<perf_record_sched_switch_event>
{
public:
    virtual void accept(LinuxPerfEventVisitor* a_Visitor) override;

    uint32_t PrevTID() { return ring_buffer_data.trace_point.prev_pid; }
    uint32_t NextTID() { return ring_buffer_data.trace_point.next_pid; }
};