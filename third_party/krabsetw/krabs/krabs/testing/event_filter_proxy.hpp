// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define INITGUID


#include "../compiler_check.hpp"
#include "../filtering/event_filter.hpp"
#include "synth_record.hpp"

namespace krabs { namespace testing {

    /**
     * <summary>
     *   Serves as a fill-in for the event_filter class for testing purposes.
     *   It acts as a liason for the actual filter instance and allows for forced event
     *   testing.
     * </summary>
     */
    class event_filter_proxy {
    public:

        /**
         * <summary>
         * Constructs a proxy for the given event_filter.
         * </summary>
         * <example>
         *     krabs::event_filter event_filter;
         *     krabs::testing::event_filter_proxy proxy(event_filter);
         * </example>
         */
        event_filter_proxy(krabs::event_filter &filter);

        /**
         * <summary>
         * Pushes an event through to the proxied filter instance.
         * </summary>
         * <example>
         *     krabs::event_filter event_filter;
         *     krabs::testing::event_filter_proxy proxy(event_filter);
         *
         *     krabs::guid powershell(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *     krabs::testing::record_builder builder(powershell, krabs::id(7942), krabs::version(1));
         *
         *     builder.add_properties()
         *             (L"ClassName", L"FakeETWEventForRealz")
         *             (L"Message", L"This message is completely faked");
         *
         *     auto record = builder.pack_incomplete();
         *     proxy.push_event(record);
         * </example>
         */
        void push_event(const synth_record &record);

    private:
        krabs::event_filter &event_filter_;
        krabs::trace_context trace_context_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline event_filter_proxy::event_filter_proxy(krabs::event_filter &event_filter)
    : event_filter_(event_filter)
    {
    }

    inline void event_filter_proxy::push_event(const synth_record &record)
    {
        event_filter_.on_event(record, trace_context_);
    }

} /* namespace testing */ } /* namespace krabs */
