//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------

#include "PrintVar.h"
#include "LinuxPerfUtils.h"

#include <linux/perf_event.h>
#include <sys/errno.h>
#include <sys/mman.h>

//-----------------------------------------------------------------------------
int32_t LinuxPerfUtils::task_event_open(int32_t cpu) {
    perf_event_attr pe{};
    pe.size = sizeof(struct perf_event_attr);
    pe.type = PERF_TYPE_SOFTWARE;
    pe.config = PERF_COUNT_SW_DUMMY;
    pe.task = 1;
    pe.sample_period = 1;
    pe.clockid = CLOCK_MONOTONIC;
    pe.sample_id_all = 1; // also include timestamps for lost events
    pe.disabled = 1;

    pe.sample_type = SAMPLE_TYPE_FLAGS;

    int32_t fd = perf_event_open(&pe, -1, cpu, -1 /*group_fd*/, 0 /*flags*/);

    if (fd == -1)
    {
        PRINT("perf_event_open error: %d\n", errno);
    }

    return fd;
}

//-----------------------------------------------------------------------------
int32_t LinuxPerfUtils::tracepoint_event_open(uint64_t a_TracepointID, pid_t pid, int32_t cpu) {
    perf_event_attr pe{};
    pe.size = sizeof(struct perf_event_attr);
    pe.type = PERF_TYPE_TRACEPOINT;
    pe.config = a_TracepointID;
    pe.sample_period = 1;
    pe.clockid = CLOCK_MONOTONIC;
    pe.sample_id_all = 1; // also include timestamps for lost events
    pe.disabled = 1;
    pe.sample_stack_user = STACK_SIZE;


    pe.sample_type = SAMPLE_TYPE_FLAGS;


    int32_t fd = perf_event_open(&pe, pid, cpu, -1 /*grpup_fd*/, 0 /*flags*/);

    if (fd == -1)
    {
        PRINT("perf_event_open error: %d\n", errno);
    }

    return fd;
}

//-----------------------------------------------------------------------------
void* LinuxPerfUtils::mmap_mapping(int32_t fd)
{
    const size_t PAGE_SIZE = getpagesize();  // 4 KB

    // "The mmap size should be 1+2^n pages, where the first page is a meta‐
    // data page (struct perf_event_mmap_page) that contains various bits of
    // information such as where the ring-buffer head is."
    const size_t mmap_length = (1 + RING_BUFFER_PAGE_COUNT) * PAGE_SIZE;

    // http://man7.org/linux/man-pages/man2/mmap.2.html
    // Use mmap to get access to the ring buffer.
    void* mmap_ret =
        mmap(nullptr, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_ret == reinterpret_cast<void*>(-1)) {
        PRINT("mmap error: %d\n", errno);
    }

    return mmap_ret;
}