//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#if __linux__

// TODO: clean up
#include <asm/perf_regs.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

// TODO: Check this struct
struct perf_sample_id {
    uint32_t pid;
    uint32_t tid;
    uint64_t time;    
    uint64_t id;     
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
    uint32_t    id;
    uint32_t    lost;
    struct perf_sample_id sample_id;
};

// TODO: This struct might change. We should read this from debugfs.
struct sched_switch_tp {
    uint16_t common_type;
    unsigned char common_flags;
	unsigned char common_preempt_count;
	int32_t common_pid;

	char prev_comm[16];
	int32_t prev_pid;
	int32_t prev_prio;
	int64_t prev_state;
	char next_comm[16];
	int32_t next_pid;
	int32_t next_prio;
};

// TODO: this has currently the sched_switch_tp included, but it should be independend
// TODO: this struct must be in sync with the flags passed as "sample_type"
//      to perf_event_open
// this needs to be packed, such that the data is at the correct position
struct __attribute__((__packed__)) perf_event_sched_switch_tp {
    struct perf_event_header header;
    uint32_t                pid, tid;   /* if PERF_SAMPLE_TID */
    uint64_t                time;       /* if PERF_SAMPLE_TIME */
    uint32_t                cpu, res;   /* if PERF_SAMPLE_CPU */
    uint32_t                size;       /* if PERF_SAMPLE_RAW */
    struct sched_switch_tp  data;       /* if PERF_SAMPLE_RAW */
    uint32_t allignment; /* 4 bytes of allignment, as it must be 64-bit alligned */
};

//-----------------------------------------------------------------------------
namespace LinuxPerfUtils
{
    static int32_t perf_event_open(struct perf_event_attr* hw_event, pid_t pid,
                                int32_t cpu, int32_t group_fd, uint32_t flags) {
        return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
    }

    static void start_capturing(uint32_t a_FileDescriptor)
    {
        ioctl(a_FileDescriptor, PERF_EVENT_IOC_RESET, 0);
        ioctl(a_FileDescriptor, PERF_EVENT_IOC_ENABLE, 0);
    }

    static void stop_capturing(uint32_t a_FileDescriptor)
    {
        ioctl(a_FileDescriptor, PERF_EVENT_IOC_DISABLE, 0);
    }

    void read_from_ring_buffer(char* dest, char* buffer, uint64_t buffer_size,
                           uint64_t index, uint64_t count);

    int32_t task_event_open(int32_t cpu);

    int32_t tracepoint_event_open(uint64_t a_TracepointID, pid_t pid, int32_t cpu);

    static void* mmap_mapping(int32_t fd)
    {
        const size_t PAGE_SIZE = getpagesize();  // 4 KB
        const size_t PAGE_COUNT = 64;            // 64: 256 KB; 2048: 8 MB

        // "The mmap size should be 1+2^n pages, where the first page is a meta‐
        // data page (struct perf_event_mmap_page) that contains various bits of
        // information such as where the ring-buffer head is."
        const size_t mmap_length = (1 + PAGE_COUNT) * PAGE_SIZE;

        // http://man7.org/linux/man-pages/man2/mmap.2.html
        // Use mmap to get access to the ring buffer.
        void* mmap_ret =
            mmap(nullptr, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mmap_ret == reinterpret_cast<void*>(-1)) {
            PRINT("mmap error: %d\n", errno);
        }

        return mmap_ret;
    }
}
#endif