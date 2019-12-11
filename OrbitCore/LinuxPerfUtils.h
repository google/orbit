//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include <asm/unistd.h>
#include <asm/perf_regs.h>
#include <linux/perf_event.h>
#include <linux/version.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define HAS_UPROBE_PERF_EVENT_SUPPORT (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))

//-----------------------------------------------------------------------------
namespace LinuxPerfUtils
{
    // TODO: these constants should be made available in some settings
    static constexpr uint32_t STACK_SIZE = (1u << 13u) - 8;
    // Sample stack- and instruction pointer (used later for stack unwinding)
    static constexpr uint64_t SAMPLE_REGS_USER = (0x1 << PERF_REG_X86_SP) | (0x1 << PERF_REG_X86_IP);

    // This must be in sync with the perf_event_record and the perf_sample_id stucts.
    static constexpr uint64_t SAMPLE_TYPE_FLAGS = 
        PERF_SAMPLE_TID | 
        PERF_SAMPLE_TIME | 
        PERF_SAMPLE_CPU | 
        0u;

    // This struct must be in sync with the SAMPLE_TYPE_FLAGS.
    struct perf_sample_id {
        uint32_t pid, tid;  /* if PERF_SAMPLE_TID set */
        uint64_t time;      /* if PERF_SAMPLE_TIME set */
        uint32_t cpu, res;  /* if PERF_SAMPLE_CPU set */
    };

    // This struct must be in sync with the SAMPLE_REGS_USER.
    struct perf_sample_regs_user {
        /* if PERF_SAMPLE_REGS_USER */
        uint64_t    abi;
        uint64_t sp;
        uint64_t ip;
    };

    struct perf_sample_stack_user {
        uint64_t    size;        /* if PERF_SAMPLE_STACK_USER */
        char   data[STACK_SIZE]; /* if PERF_SAMPLE_STACK_USER */
        uint64_t    dyn_size;    /* if PERF_SAMPLE_STACK_USER && size != 0 */
    };

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

    perf_event_attr generic_perf_event_attr();

    int32_t task_event_open(int32_t a_CPU);

    int32_t tracepoint_event_open(uint64_t a_TracepointID, pid_t a_PID, int32_t a_CPU, uint64_t additonal_sample_type = 0);


    #if HAS_UPROBE_PERF_EVENT_SUPPORT
    int32_t uprobe_event_open(const char* a_Module, uint64_t a_FunctionOffset, pid_t a_PID, int32_t a_CPU, uint64_t additonal_sample_type = 0);
    int32_t uretprobe_event_open(const char* a_Module, uint64_t a_FunctionOffset, pid_t a_PID, int32_t a_CPU, uint64_t additonal_sample_type = 0);
    #endif
}
