// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PERFINFO_GROUPMASK_HPP
#define PERFINFO_GROUPMASK_HPP

#include <Windows.h>
#pragma comment(lib, "ntdll")

// https://geoffchappell.com/studies/windows/km/ntoskrnl/api/etw/tracesup/perfinfo_groupmask.htm

#define PERF_MASK_INDEX         (0xe0000000)
#define PERF_MASK_GROUP         (~PERF_MASK_INDEX)
#define PERF_NUM_MASKS          8

typedef ULONG PERFINFO_MASK;
typedef struct _PERFINFO_GROUPMASK {
    ULONG Masks[PERF_NUM_MASKS];
} PERFINFO_GROUPMASK, *PPERFINFO_GROUPMASK;

#define PERF_GET_MASK_INDEX(GM) (((GM) & PERF_MASK_INDEX) >> 29)
#define PERF_GET_MASK_GROUP(GM) ((GM) & PERF_MASK_GROUP)
#define PERFINFO_OR_GROUP_WITH_GROUPMASK(Group, pGroupMask) \
    (pGroupMask)->Masks[PERF_GET_MASK_INDEX(Group)] |= PERF_GET_MASK_GROUP(Group);

// Masks[0]
#define PERF_PROCESS            EVENT_TRACE_FLAG_PROCESS
#define PERF_THREAD             EVENT_TRACE_FLAG_THREAD
#define PERF_PROC_THREAD        EVENT_TRACE_FLAG_PROCESS | EVENT_TRACE_FLAG_THREAD
#define PERF_LOADER             EVENT_TRACE_FLAG_IMAGE_LOAD
#define PERF_PERF_COUNTER       EVENT_TRACE_FLAG_PROCESS_COUNTERS
#define PERF_FILENAME           EVENT_TRACE_FLAG_DISK_FILE_IO
#define PERF_DISK_IO            EVENT_TRACE_FLAG_DISK_FILE_IO | EVENT_TRACE_FLAG_DISK_IO
#define PERF_DISK_IO_INIT       EVENT_TRACE_FLAG_DISK_IO_INIT
#define PERF_ALL_FAULTS         EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS
#define PERF_HARD_FAULTS        EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS
#define PERF_VAMAP              EVENT_TRACE_FLAG_VAMAP
#define PERF_NETWORK            EVENT_TRACE_FLAG_NETWORK_TCPIP
#define PERF_REGISTRY           EVENT_TRACE_FLAG_REGISTRY
#define PERF_DBGPRINT           EVENT_TRACE_FLAG_DBGPRINT
#define PERF_JOB                EVENT_TRACE_FLAG_JOB
#define PERF_ALPC               EVENT_TRACE_FLAG_ALPC
#define PERF_SPLIT_IO           EVENT_TRACE_FLAG_SPLIT_IO
#define PERF_DEBUG_EVENTS       EVENT_TRACE_FLAG_DEBUG_EVENTS
#define PERF_FILE_IO            EVENT_TRACE_FLAG_FILE_IO
#define PERF_FILE_IO_INIT       EVENT_TRACE_FLAG_FILE_IO_INIT
#define PERF_NO_SYSCONFIG       EVENT_TRACE_FLAG_NO_SYSCONFIG

// Masks[1]
#define PERF_MEMORY             0x20000001
#define PERF_PROFILE            0x20000002  // equivalent to EVENT_TRACE_FLAG_PROFILE
#define PERF_CONTEXT_SWITCH     0x20000004  // equivalent to EVENT_TRACE_FLAG_CSWITCH
#define PERF_FOOTPRINT          0x20000008
#define PERF_DRIVERS            0x20000010  // equivalent to EVENT_TRACE_FLAG_DRIVER
#define PERF_REFSET             0x20000020
#define PERF_POOL               0x20000040
#define PERF_POOLTRACE          0x20000041
#define PERF_DPC                0x20000080  // equivalent to EVENT_TRACE_FLAG_DPC
#define PERF_COMPACT_CSWITCH    0x20000100
#define PERF_DISPATCHER         0x20000200  // equivalent to EVENT_TRACE_FLAG_DISPATCHER
#define PERF_PMC_PROFILE        0x20000400
#define PERF_PROFILING          0x20000402
#define PERF_PROCESS_INSWAP     0x20000800
#define PERF_AFFINITY           0x20001000
#define PERF_PRIORITY           0x20002000
#define PERF_INTERRUPT          0x20004000  // equivalent to EVENT_TRACE_FLAG_INTERRUPT
#define PERF_VIRTUAL_ALLOC      0x20008000  // equivalent to EVENT_TRACE_FLAG_VIRTUAL_ALLOC
#define PERF_SPINLOCK           0x20010000
#define PERF_SYNC_OBJECTS       0x20020000
#define PERF_DPC_QUEUE          0x20040000
#define PERF_MEMINFO            0x20080000
#define PERF_CONTMEM_GEN        0x20100000
#define PERF_SPINLOCK_CNTRS     0x20200000
#define PERF_SPININSTR          0x20210000
#define PERF_SESSION            0x20400000
#define PERF_PFSECTION          0x20400000
#define PERF_MEMINFO_WS         0x20800000
#define PERF_KERNEL_QUEUE       0x21000000
#define PERF_INTERRUPT_STEER    0x22000000
#define PERF_SHOULD_YIELD       0x24000000
#define PERF_WS                 0x28000000

// Masks[2]
#define PERF_ANTI_STARVATION    0x40000001
#define PERF_PROCESS_FREEZE     0x40000002
#define PERF_PFN_LIST           0x40000004
#define PERF_WS_DETAIL          0x40000008
#define PERF_WS_ENTRY           0x40000010
#define PERF_HEAP               0x40000020
#define PERF_SYSCALL            0x40000040  // equivalent to EVENT_TRACE_FLAG_SYSTEMCALL
#define PERF_UMS                0x40000080
#define PERF_BACKTRACE          0x40000100
#define PERF_VULCAN             0x40000200
#define PERF_OBJECTS            0x40000400
#define PERF_EVENTS             0x40000800
#define PERF_FULLTRACE          0x40001000
#define PERF_DFSS               0x40002000
#define PERF_PREFETCH           0x40004000
#define PERF_PROCESSOR_IDLE     0x40008000
#define PERF_CPU_CONFIG         0x40010000
#define PERF_TIMER              0x40020000
#define PERF_CLOCK_INTERRUPT    0x40040000
#define PERF_LOAD_BALANCER      0x40080000
#define PERF_CLOCK_TIMER        0x40100000
#define PERF_IDLE_SELECTION     0x40200000
#define PERF_IPI                0x40400000
#define PERF_IO_TIMER           0x40800000
#define PERF_REG_HIVE           0x41000000
#define PERF_REG_NOTIF          0x42000000
#define PERF_PPM_EXIT_LATENCY   0x44000000
#define PERF_WORKER_THREAD      0x48000000

// Masks[4]
#define PERF_OPTICAL_IO         0x80000001
#define PERF_OPTICAL_IO_INIT    0x80000002
#define PERF_DLL_INFO           0x80000008
#define PERF_DLL_FLUSH_WS       0x80000010
#define PERF_OB_HANDLE          0x80000040
#define PERF_OB_OBJECT          0x80000080
#define PERF_WAKE_DROP          0x80000200
#define PERF_WAKE_EVENT         0x80000400
#define PERF_DEBUGGER           0x80000800
#define PERF_PROC_ATTACH        0x80001000
#define PERF_WAKE_COUNTER       0x80002000
#define PERF_POWER              0x80008000
#define PERF_SOFT_TRIM          0x80010000
#define PERF_CC                 0x80020000
#define PERF_FLT_IO_INIT        0x80080000
#define PERF_FLT_IO             0x80100000
#define PERF_FLT_FASTIO         0x80200000
#define PERF_FLT_IO_FAILURE     0x80400000
#define PERF_HV_PROFILE         0x80800000
#define PERF_WDF_DPC            0x81000000
#define PERF_WDF_INTERRUPT      0x82000000
#define PERF_CACHE_FLUSH        0x84000000

// Masks[5]
#define PERF_HIBER_RUNDOWN      0xA0000001

// Masks[6]
#define PERF_SYSCFG_SYSTEM      0xC0000001
#define PERF_SYSCFG_GRAPHICS    0xC0000002
#define PERF_SYSCFG_STORAGE     0xC0000004
#define PERF_SYSCFG_NETWORK     0xC0000008
#define PERF_SYSCFG_SERVICES    0xC0000010
#define PERF_SYSCFG_PNP         0xC0000020
#define PERF_SYSCFG_OPTICAL     0xC0000040
#define PERF_SYSCFG_ALL         0xDFFFFFFF

// Masks[7] - Control Mask. All flags that change system behavior go here.
#define PERF_CLUSTER_OFF        0xE0000001
#define PERF_MEMORY_CONTROL     0xE0000002

// TraceQueryInformation wasn't introduced until Windows 8, so we need to use
// NtQuerySystemInformation instead in order to maintain support for Windows 7.
// This requires the below additional definitions.

typedef enum _EVENT_TRACE_INFORMATION_CLASS {
    EventTraceKernelVersionInformation,
    EventTraceGroupMaskInformation,
    EventTracePerformanceInformation,
    EventTraceTimeProfileInformation,
    EventTraceSessionSecurityInformation,
    EventTraceSpinlockInformation,
    EventTraceStackTracingInformation,
    EventTraceExecutiveResourceInformation,
    EventTraceHeapTracingInformation,
    EventTraceHeapSummaryTracingInformation,
    EventTracePoolTagFilterInformation,
    EventTracePebsTracingInformation,
    EventTraceProfileConfigInformation,
    EventTraceProfileSourceListInformation,
    EventTraceProfileEventListInformation,
    EventTraceProfileCounterListInformation,
    EventTraceStackCachingInformation,
    EventTraceObjectTypeFilterInformation,
    MaxEventTraceInfoClass
} EVENT_TRACE_INFORMATION_CLASS;

typedef struct _EVENT_TRACE_GROUPMASK_INFORMATION {
    EVENT_TRACE_INFORMATION_CLASS EventTraceInformationClass;
    TRACEHANDLE TraceHandle;
    PERFINFO_GROUPMASK EventTraceGroupMasks;
} EVENT_TRACE_GROUPMASK_INFORMATION, * PEVENT_TRACE_GROUPMASK_INFORMATION;

#ifndef _WINTERNL_

typedef enum _SYSTEM_INFORMATION_CLASS {
} SYSTEM_INFORMATION_CLASS;

typedef LONG NTSTATUS;

extern "C" NTSTATUS NTAPI NtQuerySystemInformation(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Out_writes_bytes_to_opt_(SystemInformationLength, *ReturnLength) PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
);

#endif // _WINTERNL_

extern "C" NTSTATUS NTAPI NtSetSystemInformation(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _In_reads_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
    _In_ ULONG SystemInformationLength
);

constexpr auto SystemPerformanceTraceInformation{ static_cast<SYSTEM_INFORMATION_CLASS>(0x1f) };

#endif // PERFINFO_GROUPMASK_HPP