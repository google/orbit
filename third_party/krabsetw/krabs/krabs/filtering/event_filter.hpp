// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <evntcons.h>
#include <functional>
#include <deque>
#include <vector>

#include "../compiler_check.hpp"
#include "../trace_context.hpp"

namespace krabs { namespace testing {
    class event_filter_proxy;
} /* namespace testing */} /* namespace krabs */

namespace krabs { namespace details {
    template <typename T> class base_provider;
} /* namespace details */} /* namespace krabs */


namespace krabs {

    typedef void(*c_provider_callback)(const EVENT_RECORD &, const krabs::trace_context &);
    typedef void(*c_provider_error_callback)(const EVENT_RECORD&, const std::string&);
    typedef std::function<void(const EVENT_RECORD &, const krabs::trace_context &)> provider_event_callback;
    typedef std::function<void(const EVENT_RECORD&, const std::string&)> provider_error_callback;
    typedef std::function<bool(const EVENT_RECORD &, const krabs::trace_context &)> filter_predicate;

    template <typename T> class provider;

    /**
     * <summary>
     *   Use this to provide event filtering before an event bubbles to
     *   specific callbacks.
     * </summary>
     * <remarks>
     *   Each event_filter has a single predicate (which can do complicated
     *   checks and logic on the event). All callbacks registered under the
     *   filter are invoked only if the predicate returns true for a given
     *   event.
     * </remarks>
     */
    class event_filter {
    public:

        /**
         * <summary>
         *   Constructs an event_filter that applies the given predicate to all
         *   events.
         * </summary>
         */
        event_filter(filter_predicate predicate);

        /**
         * <summary>
         *   Constructs an event_filter that applies event id filtering by event_id
         *   which will be added to list of filtered event ids in ETW API.
         *   This way is more effective from performance point of view.
         *   Given optional predicate will be applied to ETW API filtered results
         * </summary>
         */
        event_filter(unsigned short event_id, filter_predicate predicate=nullptr);

        /**
         * <summary>
         *   Constructs an event_filter that applies event id filtering by event_id
         *   which will be added to list of filtered event ids in ETW API.
         *   This way is more effective from performance point of view.
         *   Given optional predicate will be applied to ETW API filtered results
         * </summary>
         */
        event_filter(std::vector<unsigned short> event_ids, filter_predicate predicate = nullptr);

        /**
         * <summary>
         * Adds a function to call when an event for this filter is fired.
         * </summary>
         */
        void add_on_event_callback(c_provider_callback callback);

        template <typename U>
        void add_on_event_callback(U &callback);

        template <typename U>
        void add_on_event_callback(const U &callback);

        /**
         * <summary>
         * Adds a function to call when an error occurs.
         * </summary>
         */
        void add_on_error_callback(c_provider_error_callback callback);

        template <typename U>
        void add_on_error_callback(U& callback);

        template <typename U>
        void add_on_error_callback(const U& callback);

        const std::vector<unsigned short>& provider_filter_event_ids() const
        {
            return provider_filter_event_ids_;
        }

    private:

        /**
         * <summary>
         *   Called when an event occurs, forwards to callbacks if the event
         *   satisfies the predicate.
         * </summary>
         */
        void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context) const;

        /**
         * <summary>
         *   Called when an error occurs, forwards to the error callback
         * </summary>
         */
        void on_error(const EVENT_RECORD& record, const std::string& error_message) const;

    private:
        std::deque<provider_event_callback> event_callbacks_;
        std::deque<provider_error_callback> error_callbacks_;
        filter_predicate predicate_{ nullptr };
        std::vector<unsigned short> provider_filter_event_ids_;

    private:
        template <typename T>
        friend class details::base_provider;

        friend class krabs::testing::event_filter_proxy;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline event_filter::event_filter(filter_predicate predicate)
    : predicate_(predicate)
    {}

    inline event_filter::event_filter(std::vector<unsigned short> event_ids, filter_predicate predicate/*=nullptr*/)
    : provider_filter_event_ids_{ event_ids },
      predicate_(predicate)
    {}

    inline event_filter::event_filter(unsigned short event_id, filter_predicate predicate/*=nullptr*/)
    : provider_filter_event_ids_{ event_id },
      predicate_(predicate)
    {}

    inline void event_filter::add_on_event_callback(c_provider_callback callback)
    {
        // C function pointers don't interact well with std::ref, so we
        // overload to take care of this scenario.
        event_callbacks_.push_back(callback);
    }

    template <typename U>
    void event_filter::add_on_event_callback(U &callback)
    {
        // std::function copies its argument -- because our callbacks list
        // is a list of std::function, this causes problems when a user
        // intended for their particular instance to be called.
        // std::ref lets us get around this and point to a specific instance
        // that they handed us.
        event_callbacks_.push_back(std::ref(callback));
    }

    template <typename U>
    void event_filter::add_on_event_callback(const U &callback)
    {
        // This is where temporaries bind to. Temporaries can't be wrapped in
        // a std::ref because they'll go away very quickly. We are forced to
        // actually copy these.
        event_callbacks_.push_back(callback);
    }

    inline void event_filter::add_on_error_callback(c_provider_error_callback callback)
    {
        // C function pointers don't interact well with std::ref, so we
        // overload to take care of this scenario.
        error_callbacks_.push_back(callback);
    }

    template <typename U>
    void event_filter::add_on_error_callback(U& callback)
    {
        // std::function copies its argument -- because our callbacks list
        // is a list of std::function, this causes problems when a user
        // intended for their particular instance to be called.
        // std::ref lets us get around this and point to a specific instance
        // that they handed us.
        error_callbacks_.push_back(std::ref(callback));
    }

    template <typename U>
    void event_filter::add_on_error_callback(const U& callback)
    {
        // This is where temporaries bind to. Temporaries can't be wrapped in
        // a std::ref because they'll go away very quickly. We are forced to
        // actually copy these.
        error_callbacks_.push_back(callback);
    }

    inline void event_filter::on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context) const
    {
        if (event_callbacks_.empty()) {
            return;
        }

        try
        {
            if (predicate_ != nullptr && !predicate_(record, trace_context)) {
                return;
            }

            for (auto& callback : event_callbacks_) {
                callback(record, trace_context);
            }
        }
        catch (const krabs::could_not_find_schema& ex)
        {
            // this occurs when a predicate is applied to an event for which
            // no schema exists. instead of allowing the exception to halt
            // the entire trace, send a notification to the filter's error callback
            for (auto& error_callback : error_callbacks_) {
                error_callback(record, ex.what());
            }
        }
    }
} /* namespace krabs */
