//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

// This struct must be in sync with the SAMPLE_TYPE_FLAGS.
struct perf_sample_id {
    uint32_t pid, tid;  /* if PERF_SAMPLE_TID set */
    uint64_t time;      /* if PERF_SAMPLE_TIME set */
    uint32_t cpu, res;  /* if PERF_SAMPLE_CPU set */
};

struct perf_event_fork_exit {
    struct perf_event_header header;
    uint32_t    pid, ppid;
    uint32_t    tid, ptid;
    uint64_t    time;
    struct perf_sample_id sample_id;
};

struct perf_event_lost {
    struct perf_event_header header;
    uint64_t    id;
    uint64_t    lost;
    struct perf_sample_id sample_id;
};

// TODO: This struct might change. We should read this from debugfs.
struct sched_switch_tp {
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


// This struct must be in sync with the SAMPLE_TYPE_FLAGS.
template<typename record_data>
struct __attribute__((__packed__)) perf_event_record {
    struct perf_event_header header;
    uint32_t                 pid, tid;   /* if PERF_SAMPLE_TID */
    uint64_t                 time;       /* if PERF_SAMPLE_TIME */
    uint32_t                 cpu, res;   /* if PERF_SAMPLE_CPU */
    uint32_t                 size;       /* if PERF_SAMPLE_RAW */
    record_data              data;       /* if PERF_SAMPLE_RAW */
};

//-----------------------------------------------------------------------------
namespace LinuxPerfUtils
{
    // TODO: these constants should be made available in some settings
    static const uint32_t STACK_SIZE = (1u << 16u) - 8; // TODO: the stack is not yet recorded
    static const size_t RING_BUFFER_PAGE_COUNT = 64; // 64: 256 KB; 2048: 8 MB

    // This must be in sync with the perf_event_record and the perf_sample_id stucts.
    static const uint64_t SAMPLE_TYPE_FLAGS = 
        PERF_SAMPLE_TID | 
        PERF_SAMPLE_TIME | 
        PERF_SAMPLE_CPU | 
        PERF_SAMPLE_RAW | 
        0u;

    inline int32_t perf_event_open(struct perf_event_attr* a_HWEvent, pid_t a_PID,
                                int32_t a_CPU, int32_t a_GroupFD, uint32_t a_Flags) {
        return syscall(__NR_perf_event_open, a_HWEvent, a_PID, a_CPU, a_GroupFD, a_Flags);
    }

    inline void start_capturing(uint32_t a_FileDescriptor)
    {
        ioctl(a_FileDescriptor, PERF_EVENT_IOC_RESET, 0);
        ioctl(a_FileDescriptor, PERF_EVENT_IOC_ENABLE, 0);
    }

    inline void stop_capturing(uint32_t a_FileDescriptor)
    {
        ioctl(a_FileDescriptor, PERF_EVENT_IOC_DISABLE, 0);
    }

    int32_t task_event_open(int32_t a_CPU);

    int32_t tracepoint_event_open(uint64_t a_TracepointID, pid_t a_PID, int32_t a_CPU);

    void* mmap_mapping(int32_t a_FileDescriptor);
}