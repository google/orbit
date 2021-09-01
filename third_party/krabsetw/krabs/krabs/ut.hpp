// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <set>

#include "compiler_check.hpp"
#include "trace.hpp"
#include "provider.hpp"

namespace krabs { namespace details {

    /**
     * <summary>
     *   Used as a template argument to a trace instance. This class implements
     *   code paths for user traces. Should never be used or seen by client
     *   code.
     * </summary>
     */
    struct ut {

        typedef krabs::provider<> provider_type;
        
        struct filter_flags {
            UCHAR level_;
            ULONGLONG any_;
            ULONGLONG all_;
            ULONG trace_flags_;
        };

        struct filter_settings{
            std::set<unsigned short> provider_filter_event_ids_;
            filter_flags filter_flags_{};
            bool rundown_enabled_ = false;
        };

        typedef std::map<krabs::guid, filter_settings> provider_filter_settings;
        /**
         * <summary>
         *   Used to assign a name to the trace instance that is being
         *   instantiated.
         * </summary>
         * <remarks>
         *   There really isn't a name policy to enforce with user traces, but
         *   kernel traces do have specific naming requirements.
         * </remarks>
         */
        static const std::wstring enforce_name_policy(
            const std::wstring &name);

        /**
         * <summary>
         *   Generates a value that fills the EnableFlags field in an
         *   EVENT_TRACE_PROPERTIES structure. This controls the providers that
         *   get enabled for a kernel trace. For a user trace, it doesn't do
         *   much of anything.
         * </summary>
         */
        static const unsigned long construct_enable_flags(
            const krabs::trace<krabs::details::ut> &trace);

        /**
         * <summary>
         *   Enables the providers that are attached to the given trace.
         * </summary>
         */
        static void enable_providers(
            const krabs::trace<krabs::details::ut> &trace);

        /**
         * <summary>
         *   Enables the configured rundown events for each provider.
         *   Should be called immediately prior to ProcessTrace.
         * </summary>
         */
        static void enable_rundown(
            const krabs::trace<krabs::details::ut>& trace);

        /**
         * <summary>
         *   Decides to forward an event to any of the providers in the trace.
         * </summary>
         */
        static void forward_events(
            const EVENT_RECORD &record,
            const krabs::trace<krabs::details::ut> &trace);

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

    inline const std::wstring ut::enforce_name_policy(
        const std::wstring &name_hint)
    {
        if (name_hint.empty()) {
            return std::to_wstring(krabs::guid::random_guid());
        }

        return name_hint;
    }

    inline const unsigned long ut::construct_enable_flags(
        const krabs::trace<krabs::details::ut> &)
    {
        return 0;
    }

    inline void ut::enable_providers(
        const krabs::trace<krabs::details::ut> &trace)
    {
        if (trace.registrationHandle_ == INVALID_PROCESSTRACE_HANDLE)
            return;

        provider_filter_settings provider_flags;

        // This function essentially takes the union of all the provider flags
        // for a given provider GUID. This comes about when multiple providers
        // for the same GUID are provided and request different provider flags.
        // TODO: Only forward the calls that are requested to each provider.
        for (auto &provider : trace.providers_) {
            auto& settings = provider_flags[provider.get().guid_];
            settings.filter_flags_.level_       |= provider.get().level_;
            settings.filter_flags_.any_         |= provider.get().any_;
            settings.filter_flags_.all_         |= provider.get().all_;
            settings.filter_flags_.trace_flags_ |= provider.get().trace_flags_;
            settings.rundown_enabled_           |= provider.get().rundown_enabled_;

            for (const auto& filter : provider.get().filters_) {
                settings.provider_filter_event_ids_.insert(
                    filter.provider_filter_event_ids().begin(),
                    filter.provider_filter_event_ids().end());
            }
        }

        for (auto &provider : provider_flags) {
            ENABLE_TRACE_PARAMETERS parameters;
            parameters.ControlFlags = 0;
            parameters.Version = ENABLE_TRACE_PARAMETERS_VERSION_2;
            parameters.SourceId = provider.first;
            
            GUID guid = provider.first;
            auto& settings = provider.second;

            parameters.EnableProperty = settings.filter_flags_.trace_flags_;
            parameters.EnableFilterDesc = nullptr;
            parameters.FilterDescCount = 0;
            EVENT_FILTER_DESCRIPTOR filterDesc{};
            std::vector<BYTE> filterEventIdBuffer;
            auto filterEventIdCount = settings.provider_filter_event_ids_.size();

            if (filterEventIdCount > 0) {
                //event filters existing, set native filters using API
                parameters.FilterDescCount = 1;
                filterDesc.Type = EVENT_FILTER_TYPE_EVENT_ID;

                //allocate + size of expected events in filter
                DWORD size = FIELD_OFFSET(EVENT_FILTER_EVENT_ID, Events[filterEventIdCount]);
                filterEventIdBuffer.resize(size, 0);

                auto filterEventIds = reinterpret_cast<PEVENT_FILTER_EVENT_ID>(&(filterEventIdBuffer[0]));
                filterEventIds->FilterIn = TRUE;
                filterEventIds->Count = static_cast<USHORT>(filterEventIdCount);

                auto index = 0;
                for (auto filter : settings.provider_filter_event_ids_) {
                    filterEventIds->Events[index] = filter;
                    index++;
                }

                filterDesc.Ptr = reinterpret_cast<ULONGLONG>(filterEventIds);
                filterDesc.Size = size;

                parameters.EnableFilterDesc = &filterDesc;
            }

            ULONG status = EnableTraceEx2(trace.registrationHandle_,
                                          &guid,
                                          EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                                          settings.filter_flags_.level_,
                                          settings.filter_flags_.any_,
                                          settings.filter_flags_.all_,
                                          0,
                                          &parameters);
            error_check_common_conditions(status);
        }
    }

    inline void ut::enable_rundown(
        const krabs::trace<krabs::details::ut>& trace)
    {
        if (trace.registrationHandle_ == INVALID_PROCESSTRACE_HANDLE)
            return;

        for (auto& provider : trace.providers_) {
            if (!provider.get().rundown_enabled_)
                continue;

            ULONG status = EnableTraceEx2(trace.registrationHandle_,
                &provider.get().guid_,
                EVENT_CONTROL_CODE_CAPTURE_STATE,
                0,
                0,
                0,
                0,
                NULL);
            error_check_common_conditions(status);
        }
    }

    inline void ut::forward_events(
        const EVENT_RECORD &record,
        const krabs::trace<krabs::details::ut> &trace)
    {
        // for manifest providers, EventHeader.ProviderId is the Provider GUID
        for (auto& provider : trace.providers_) {
            if (record.EventHeader.ProviderId == provider.get().guid_) {
                provider.get().on_event(record, trace.context_);
                return;
            }
        }

        // for MOF providers, EventHeader.Provider is the *Message* GUID
        // we need to ask TDH for event information in order to determine the
        // correct provider to pass this event to
        auto schema = get_event_schema_from_tdh(record);
        auto eventInfo = reinterpret_cast<PTRACE_EVENT_INFO>(schema.get());
        for (auto& provider : trace.providers_) {
            if (eventInfo->ProviderGuid == provider.get().guid_) {
                provider.get().on_event(record, trace.context_);
                return;
            }
        }

        if (trace.default_callback_ != nullptr)
            trace.default_callback_(record, trace.context_);
    }

    inline unsigned long ut::augment_file_mode()
    {
        return 0;
    }

    inline krabs::guid ut::get_trace_guid()
    {
        return krabs::guid::random_guid();
    }

} /* namespace details */ } /* namespace krabs */
