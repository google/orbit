//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#if __linux__

// TODO: clean up.
#include "PrintVar.h"
#include "Capture.h"
#include "LinuxPerfUtils.h"

#include <asm/perf_regs.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

//-----------------------------------------------------------------------------
void LinuxPerfUtils::read_from_ring_buffer(char* dest, char* buffer, uint64_t buffer_size,
                        uint64_t index, uint64_t count) {
    if (count > buffer_size) {
        // If mmap has been called with PROT_WRITE and
        // perf_event_mmap_page::data_tail is used properly, this should not happen,
        // as the kernel would no overwrite unread data.
        PRINT("Error %u: too slow reading from the ring buffer\n", stderr);
    } else if (index / buffer_size == (index + count - 1) / buffer_size) {
        memcpy(dest, buffer + index % buffer_size, count);
    } else if (index / buffer_size == (index + count - 1) / buffer_size - 1) {
        // Need two copies as the data to read wraps around the ring buffer.
        memcpy(dest, buffer + index % buffer_size,
            buffer_size - index % buffer_size);
        memcpy(dest + (buffer_size - index % buffer_size), buffer,
            count - (buffer_size - index % buffer_size));
    } else {
        PRINT("Unexpected error while reading from the ring buffer\n");
    }
}

//-----------------------------------------------------------------------------
int32_t LinuxPerfUtils::task_event_open(int32_t cpu) {
    perf_event_attr pe{};
    pe.size = sizeof(struct perf_event_attr);
    pe.type = PERF_TYPE_SOFTWARE;
    pe.config = PERF_COUNT_SW_DUMMY;
    pe.task = 1;
    pe.sample_period = 1;
    // we want the sample_id present in all events
    pe.sample_id_all = 1;
    pe.disabled = 1;

    pe.sample_type = 0;
    int32_t fd = perf_event_open(&pe, -1, cpu, -1 /*group_fd*/, 0 /*flags*/);
    return fd;
}

//-----------------------------------------------------------------------------
int32_t LinuxPerfUtils::tracepoint_event_open(uint64_t a_TracepointID, pid_t pid, int32_t cpu) {
    perf_event_attr pe{};
    pe.size = sizeof(struct perf_event_attr);
    pe.type = PERF_TYPE_TRACEPOINT;
    pe.config = a_TracepointID;
    pe.sample_period = 1;
    // we want the sample_id struct in the lost events
    pe.sample_id_all = 1;
    pe.disabled = 1;
    // TODO: This would be needed if we also want to capture the stack
    pe.sample_stack_user = (1u << 16u) - 8;


    // if you modify this, you also need to modify the perf_event_tracepoint struct
    pe.sample_type = PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_CPU | 
                PERF_SAMPLE_RAW | 0u;


    int32_t fd = perf_event_open(&pe, pid, cpu, -1 /*grpup_fd*/, 0 /*flags*/);

    if (fd == -1)
    {
        PRINT("perf_event_open error: %d\n", errno);
    }

    return fd;
}
#endif