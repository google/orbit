// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "compiler_check.hpp"
#include "kernel_guids.hpp"
#include "perfinfo_groupmask.hpp"
#include "provider.hpp"
#include "trace.hpp"
#include "ut.hpp"
#include "version_helpers.hpp"

#include <Evntrace.h>

namespace krabs { namespace details {

    /**
     * <summary>
     *   Used as a template argument to a trace instance. This class implements
     *   code paths for kernel traces. Should never be used or seen by client
     *   code.
     * </summary>
     */
    struct kt {

        typedef krabs::kernel_provider provider_type;

        /**
         * <summary>
         *   Used to assign a name to the trace instance that is being
         *   instantiated.
         * </summary>
         * <remarks>
         *   In pre-Win8 days, there could only be a single kernel trace
         *   instance on an entire machine, and that instance had to be named
         *   a particular name. This restriction was loosened in Win8, but
         *   the trace still needs to do the right thing on older OSes.
         * </remarks>
         */
        static const std::wstring enforce_name_policy(
            const std::wstring &name);

        /**
         * <summary>
         *   Generates a value that fills the EnableFlags field in an
         *   EVENT_TRACE_PROPERTIES structure. This controls the providers that
         *   get enabled for a kernel trace.
         * </summary>
         */
        static const unsigned long construct_enable_flags(
            const krabs::trace<krabs::details::kt> &trace);

        /** 
         * <summary>
         *   Enables the providers that are attached to the given trace.
         * </summary>
         */
        static void enable_providers(
            const krabs::trace<krabs::details::kt> &trace);

        /**
         * <summary>
         *   Enables the configured kernel rundown flags.
         * </summary>
         * <remarks>
         *   This ETW feature is undocumented and should be used with caution.
         * </remarks>
         */
        static void enable_rundown(
            const krabs::trace<krabs::details::kt>& trace);

        /**
         * <summary>
         *   Decides to forward an event to any of the providers in the trace.
         * </summary>
         */
        static void forward_events(
            const EVENT_RECORD &record,
            const krabs::trace<krabs::details::kt> &trace);

        /**
         * <summary>
         *   Sets the ETW trace log file mode.
         * </summary>
         */
        static unsigned long augment_file_mode();

        /**
         * <summary>
         *   Returns the GUID of the trace session.
         * </summary>
         */
        static krabs::guid get_trace_guid();

    };

    // Implementation
    // ------------------------------------------------------------------------

    inline const std::wstring kt::enforce_name_policy(
        const std::wstring &name_hint)
    {
        if (IsWindows8OrGreater()) {
            return krabs::details::ut::enforce_name_policy(name_hint);
        }

        return KERNEL_LOGGER_NAME;
    }

    inline const unsigned long kt::construct_enable_flags(
        const krabs::trace<krabs::details::kt> &trace)
    {
        unsigned long flags = 0;
        for (auto &provider : trace.providers_) {
            flags |= provider.get().flags();
        }

        return flags;
    }

    inline void kt::enable_providers(
        const krabs::trace<krabs::details::kt> &trace)
    {
        EVENT_TRACE_GROUPMASK_INFORMATION gmi = { 0 };
        gmi.EventTraceInformationClass = EventTraceGroupMaskInformation;
        gmi.TraceHandle = trace.registrationHandle_;

        // initialise EventTraceGroupMasks to the values that have been enabled via the trace flags
        ULONG status = NtQuerySystemInformation(SystemPerformanceTraceInformation, &gmi, sizeof(gmi), nullptr);
        error_check_common_conditions(status);

        auto group_mask_set = false;
        for (auto& provider : trace.providers_) {
            auto group = provider.get().group_mask();
            PERFINFO_OR_GROUP_WITH_GROUPMASK(group, &(gmi.EventTraceGroupMasks));
            group_mask_set |= (group != 0);
        }

        if (group_mask_set) {
            // This will fail on Windows 7, so only call it if truly neccessary
            status = NtSetSystemInformation(SystemPerformanceTraceInformation, &gmi, sizeof(gmi));
            error_check_common_conditions(status);
        }

        return;
    }

    inline void kt::enable_rundown(
        const krabs::trace<krabs::details::kt>& trace)
    {
        bool rundown_enabled = false;
        ULONG rundown_flags = 0;
        for (auto& provider : trace.providers_) {
            rundown_enabled |= provider.get().rundown_enabled();
            rundown_flags |= provider.get().rundown_flags();
        }

        if (rundown_enabled) {
            ULONG status = EnableTraceEx2(trace.registrationHandle_,
                                          &krabs::guids::rundown,
                                          EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                                          0,
                                          rundown_flags,
                                          0,
                                          0,
                                          NULL);
            error_check_common_conditions(status);
        }
    }


    inline void kt::forward_events(
        const EVENT_RECORD &record,
        const krabs::trace<krabs::details::kt> &trace)
    {
        for (auto &provider : trace.providers_) {
            if (provider.get().id() == record.EventHeader.ProviderId) {
                provider.get().on_event(record, trace.context_);
                return;
            }
        }

        if (trace.default_callback_ != nullptr)
            trace.default_callback_(record, trace.context_);
    }

    inline unsigned long kt::augment_file_mode()
    {
        if (IsWindows8OrGreater()) {
            return EVENT_TRACE_SYSTEM_LOGGER_MODE;
        }

        return 0;
    }

    inline krabs::guid kt::get_trace_guid()
    {
        if (IsWindows8OrGreater()) {
            return krabs::guid::random_guid();
        }

        return krabs::guid(SystemTraceControlGuid);
    }

} /* namespace details */ } /* namespace krabs */
