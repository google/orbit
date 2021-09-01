// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "compiler_check.hpp"
#include "kernel_guids.hpp"
#include "perfinfo_groupmask.hpp"
#include "provider.hpp"

#define INITGUID
#include <Evntrace.h>

namespace krabs { namespace kernel {

#define CREATE_CONVENIENCE_KERNEL_PROVIDER(__name__, __value__, __guid__)     \
    struct __name__ : public krabs::kernel_provider                           \
    {                                                                         \
        __name__()                                                            \
        : krabs::kernel_provider(__value__, __guid__)                         \
        {}                                                                    \
    };

#define CREATE_CONVENIENCE_KERNEL_PROVIDER_MASK(__name__, __guid__, __mask__) \
    struct __name__ : public krabs::kernel_provider                           \
    {                                                                         \
        __name__()                                                            \
        : krabs::kernel_provider(__guid__, __mask__)                          \
        {}                                                                    \
    };

    /**
     * <summary>A provider that enables ALPC events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        alpc_provider,
        EVENT_TRACE_FLAG_ALPC,
        krabs::guids::alpc);

     /**
      * <summary>A provider that enables context switch events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER(
         context_switch_provider,
         EVENT_TRACE_FLAG_CSWITCH,
         krabs::guids::thread);

    /**
     * <summary>A provider that enables debug print events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        debug_print_provider,
        EVENT_TRACE_FLAG_DBGPRINT,
        krabs::guids::debug);

    /**
     * <summary>A provider that enables file I/O name events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        disk_file_io_provider,
        EVENT_TRACE_FLAG_DISK_FILE_IO,
        krabs::guids::file_io);

    /**
     * <summary>A provider that enables disk I/O completion events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        disk_io_provider,
        EVENT_TRACE_FLAG_DISK_IO,
        krabs::guids::disk_io);

    /**
     * <summary>A provider that enables disk I/O start events.</summary>
    */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        disk_init_io_provider,
        EVENT_TRACE_FLAG_DISK_IO_INIT,
        krabs::guids::disk_io);

    /**
    * <summary>A provider that enables file I/O completion events.</summary>
    */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        file_io_provider,
        EVENT_TRACE_FLAG_FILE_IO,
        krabs::guids::file_io);

    /**
    * <summary>A provider that enables file I/O start events.</summary>
    */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        file_init_io_provider,
        EVENT_TRACE_FLAG_FILE_IO_INIT,
        krabs::guids::file_io);

    /**
     * <summary>A provider that enables thread dispatch events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        thread_dispatch_provider,
        EVENT_TRACE_FLAG_DISPATCHER,
        krabs::guids::thread);

     /**
      * <summary>A provider that enables device deferred procedure call events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER(
         dpc_provider,
         EVENT_TRACE_FLAG_DPC,
         krabs::guids::perf_info);

     /**
      * <summary>A provider that enables driver events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER(
         driver_provider,
         EVENT_TRACE_FLAG_DRIVER,
         krabs::guids::disk_io);

    /**
     * <summary>A provider that enables image load events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        image_load_provider,
        EVENT_TRACE_FLAG_IMAGE_LOAD,
        krabs::guids::image_load);

     /**
      * <summary>A provider that enables interrupt events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER(
         interrupt_provider,
         EVENT_TRACE_FLAG_INTERRUPT,
         krabs::guids::perf_info);

    /**
     * <summary>A provider that enables memory hard fault events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        memory_hard_fault_provider,
        EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS,
        krabs::guids::page_fault);

    /**
     * <summary>A provider that enables memory page fault events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        memory_page_fault_provider,
        EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS,
        krabs::guids::page_fault);

    /**
     * <summary>A provider that enables network tcp/ip events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        network_tcpip_provider,
        EVENT_TRACE_FLAG_NETWORK_TCPIP,
        krabs::guids::tcp_ip);

    /**
     * <summary>A provider that enables process events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        process_provider,
        EVENT_TRACE_FLAG_PROCESS,
        krabs::guids::process);

    /**
     * <summary>A provider that enables process counter events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        process_counter_provider,
        EVENT_TRACE_FLAG_PROCESS_COUNTERS,
        krabs::guids::process);

    /**
     * <summary>A provider that enables profiling events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        profile_provider,
        EVENT_TRACE_FLAG_PROFILE,
        krabs::guids::perf_info);

    /**
     * <summary>A provider that enables registry events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        registry_provider,
        EVENT_TRACE_FLAG_REGISTRY,
        krabs::guids::registry);

    /**
     * <summary>A provider that enables split I/O events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        split_io_provider,
        EVENT_TRACE_FLAG_SPLIT_IO,
        krabs::guids::split_io);

    /**
     * <summary>A provider that enables system call events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        system_call_provider,
        EVENT_TRACE_FLAG_SYSTEMCALL,
        krabs::guids::perf_info);

    /**
     * <summary>A provider that enables thread start and stop events.</summary>
     */
    CREATE_CONVENIENCE_KERNEL_PROVIDER(
        thread_provider,
        EVENT_TRACE_FLAG_THREAD,
        krabs::guids::thread);

     /**
      * <summary>A provider that enables file map and unmap (excluding images) events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER(
         vamap_provider,
         EVENT_TRACE_FLAG_VAMAP,
         krabs::guids::file_io);

     /**
      * <summary>A provider that enables VirtualAlloc and VirtualFree events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER(
         virtual_alloc_provider,
         EVENT_TRACE_FLAG_VIRTUAL_ALLOC,
         krabs::guids::page_fault);

     /**
      * <summary>A provider that enables Object Manager events.</summary>
      */
     CREATE_CONVENIENCE_KERNEL_PROVIDER_MASK(
         object_manager_provider,
         krabs::guids::ob_trace,
         PERF_OB_HANDLE);

#undef CREATE_CONVENIENCE_KERNEL_PROVIDER
#undef CREATE_CONVENIENCE_KERNEL_PROVIDER_MASK

} /* namespace kernel */ } /* namespace krabs */
