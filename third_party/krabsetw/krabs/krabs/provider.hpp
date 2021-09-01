// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define INITGUID

#include <deque>
#include <functional>

#include "compiler_check.hpp"
#include "filtering/event_filter.hpp"
#include "perfinfo_groupmask.hpp"
#include "trace_context.hpp"
#include "wstring_convert.hpp"

#include <evntcons.h>
#include <guiddef.h>
#include <pla.h>
#include <comutil.h>

#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#endif

namespace krabs { namespace details {
    template <typename T> class trace_manager;

    struct kt;
    struct ut;

} /* namespace details */ } /* namespace krabs */

namespace krabs {

    template <typename T>
    class trace;

    typedef void(*c_provider_callback)(const EVENT_RECORD &, const krabs::trace_context &);
    typedef std::function<void(const EVENT_RECORD &, const krabs::trace_context &)> provider_callback;

    namespace details {

        /**
         * <summary>
         *   Serves as a base for providers and kernel_providers. Handles event
         *   registration and forwarding.
         * </summary>
         */
        template <typename T>
        class base_provider {
        public:

            /**
             * <summary>
             * Adds a function to call when an event for this provider is fired.
             * </summary>
             *
             * <param name="callback">the function to call into</param>
             * <example>
             *    void my_fun(const EVENT_RECORD &record, const krabs::trace_context &trace_context) { ... }
             *    // ...
             *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
             *    provider<> powershell(id);
             *    provider.add_on_event_callback(my_fun);
             * </example>
             *
             * <example>
             *    auto fun = [&](const EVENT_RECORD &record, const krabs::trace_context &trace_context) {...}
             *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
             *    provider<> powershell(id);
             *    provider.add_on_event_callback(fun);
             * </example>
             */
            void add_on_event_callback(c_provider_callback callback);

            template <typename U>
            void add_on_event_callback(U &callback);

            template <typename U>
            void add_on_event_callback(const U &callback);

            /**
             * <summary>
             *   Adds a new filter to a provider, where the filter is expected
             *   to have callbacks attached to it.
             * </summary>
             * <example>
             *   krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
             *   krabs::provider<> powershell(id);
             *   krabs::event_filter filter(krabs::filtering::any_event);
             *   filter.add_on_event_callback([&](const EVENT_RECORD &record, const krabs::trace_context &trace_context) {...});
             *   powershell.add_filter(filter);
             * </example>
             */
            void add_filter(const event_filter &f);

        protected:

            /**
             * <summary>
             *   Called when an event occurs, forwards to callbacks and filters.
             * </summary>
             */
            void on_event(const EVENT_RECORD &record, const krabs::trace_context &context) const;

        protected:
            std::deque<provider_callback> callbacks_;
            std::deque<event_filter> filters_;

        private:
            template <typename T>
            friend class details::trace_manager;

            template <typename T>
            friend class krabs::trace;

            template <typename S>
            friend class base_provider;
        };

    } // namespace details

    /**
     * <summary>
     *   Used to enable specific types of events from specific event sources in
     *   ETW. Corresponds tightly with the concept of an ETW provider. Used for
     *   user trace instances (not kernel trace instances).
     * </summary>
     *
     * <param name='T'>
     * The type of flags to use when filtering event types via any and all.
     * There is an implicitly requirement that T can be downcasted to a
     * ULONGLONG.
     * </param>
     */
    template <typename T = ULONGLONG>
    class provider : public details::base_provider<T> {
    public:

        /**
         * <summary>
         * Constructs a provider with the given guid identifier.
         * </summary>
         *
         * <param name="id">the GUID that identifies the provider.</param>
         * <example>
         *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         * </example>
         */
        provider(GUID id);

        /**
        * <summary>
        * Constructs a provider with the given a provider name.
        * </summary>
        *
        * <param name="id">the provider name.</param>
        * <example>
        *    krabs::guid id(L"Microsoft-Windows-WinINet");
        *    provider<> winINet(id);
        * </example>
        */
        provider(const std::wstring &providerName);

        /**
         * <summary>
         * Sets the "any" flag of the provider.
         * </summary>
         *
         * <param name="value">the value to set <c>any</c> to.</param>
         * <example>
         *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    powershell.any(0xFF00);
         * </example>
         */
        void any(T any);

        /**
         * <summary>
         * Sets the "all" flag of the provider.
         * </summary>
         *
         * <param name="value">the value to set <c>all</c> to.</param>
         * <example>
         *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<> powershell(id);
         *    powershell.all(0xFF00);
         * </example>
         */
        void all(T all);

        /**
        * <summary>
        * Sets the "level" flag of the provider. Valid values are 0~255 (0xFF).
        * </summary>
        *
        * <param name="value">the value to set <c>level</c> to.</param>
        * <example>
        *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
        *    provider<> powershell(id);
        *    powershell.level(0x00);
        * </example>
        */
        void level(T level);

        /**
        * <summary>
        * Sets the "EnableProperty" flag on the ENABLE_TRACE_PARAMETER struct.
        * These properties configure behaviours for a specified user-mode provider.
        * Valid values can be found here:
        *   https://msdn.microsoft.com/en-us/library/windows/desktop/dd392306(v=vs.85).aspx
        * </summary>
        *
        * <param name="value">the value to set</param>
        * <example>
        *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
        *    provider<> powershell(id);
        *    powershell.trace_flags(EVENT_ENABLE_PROPERTY_STACK_TRACE);
        * </example>
        */
        void trace_flags(T trace_flags);

        /**
        * <summary>
        * Gets the configured value for the "EnableProperty" flag on the 
        * ENABLE_TRACE_PARAMETER struct. See void trace_flags(T trace_flags)
        * for details on what the values mean.
        * </summary>
        * <returns>The value to set when the provider is enabled.</returns>
        * <example>
        *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
        *    provider<> powershell(id);
        *    powershell.trace_flags(EVENT_ENABLE_PROPERTY_STACK_TRACE);
        *    auto flags = powershell.get_trace_flags(); 
        *    assert(flags == EVENT_ENABLE_PROPERTY_STACK_TRACE);
        * </example>
        */
        T trace_flags() const;

        /**
        * <summary>
        * Requests that the provider log its state information. See:
        *   https://docs.microsoft.com/en-us/windows/win32/api/evntrace/nf-evntrace-enabletraceex2
        * </summary>
        *
        * <example>
        *     krabs::provider<> process_provider(L"Microsoft-Windows-Kernel-Process");
        *     process_provider.any(0x10);  // WINEVENT_KEYWORD_PROCESS
        *     process_provider.enable_rundown_events();
        * </example>
        */
        void enable_rundown_events();

        /**
         * <summary>
         * Turns a strongly typed provider<T> to provider<> (useful for
         * creating collections of providers).
         * </summary>
         *
         * <example>
         *    krabs::guid id(L"{A0C1853B-5C40-4B15-8766-3CF1C58F985A}");
         *    provider<CustomFlagType> powershell(id);
         *    provider<> blah(powershell);
         * </example>
         */
        operator provider<>() const;

    private:
        GUID guid_;
        T any_;
        T all_;
        T level_;
        T trace_flags_;
        bool rundown_enabled_;

    private:
        template <typename T>
        friend class details::trace_manager;

        template <typename T>
        friend class krabs::trace;

        template <typename S>
        friend class base_provider;

        friend struct details::ut;

    };

    /**
     * <summary>
     *   Used to enable specific types of event sources from an ETW kernel
     *   trace.
     * </summary>
     */
    class kernel_provider : public details::base_provider<ULONGLONG> {
    public:

        /**
         * <summary>
         *   Constructs a kernel_provider that enables events of the given
         *   flags.
         * </summary>
         */
        kernel_provider(unsigned long flags, const GUID &id)
        : p_(flags)
        , id_(id)
        , gm_(0)
        , r_(0)
        , rundown_enabled_(false)
        {}

        /**
         * <summary>
         *   Constructs a kernel_provider that enables events of the given
         *   group mask.
         * </summary>
         * <remarks>
         *   Only supported on Windows 8 and newer.
         * </remarks>
         */
        kernel_provider(const GUID& id, PERFINFO_MASK group_mask)
        : p_(0)
        , id_(id)
        , gm_(group_mask)
        , r_(0)
        , rundown_enabled_(false)
        {}

        /**
         * <summary>
         *   Retrieves the GUID associated with this provider.
         * </summary>
         */
         const krabs::guid &id() const;

         /**
         * <summary>
         *   Sets flags to be enabled for the kernel rundown GUID.
         * </summary>
         * <remarks>
         *   This ETW feature is undocumented and should be used with caution.
         * </remarks>
         */
         void set_rundown_flags(unsigned long rundown_flags) {
             r_ = rundown_flags;
             rundown_enabled_ = true;
         };

    private:

        /**
         * <summary>
         *   Retrieves the flag value associated with this provider.
         * </summary>
         */
        unsigned long flags() const { return p_; }

        /**
         * <summary>
         *   Retrieves the group mask value associated with this provider.
         * </summary>
         */
        PERFINFO_MASK group_mask() const { return gm_; }

        /**
         * <summary>
         *   Retrieves the rundown flag value associated with this provider.
         * </summary>
         */
        unsigned long rundown_flags() const { return r_; }

        /**
         * <summary>
         *   Have rundown flags been set for this this provider?
         * </summary>
         */
        bool rundown_enabled() const { return rundown_enabled_; }

    private:
        unsigned long p_;
        const krabs::guid id_;
        PERFINFO_MASK gm_;
        unsigned long r_;
        bool rundown_enabled_;

    private:
        friend struct details::kt;
    };


    // Implementation
    // ------------------------------------------------------------------------

    namespace details {

        template <typename T>
        void base_provider<T>::add_on_event_callback(c_provider_callback callback)
        {
            // C function pointers don't interact well with std::ref, so we
            // overload to take care of this scenario.
            callbacks_.push_back(callback);
        }

        template <typename T>
        template <typename U>
        void base_provider<T>::add_on_event_callback(U &callback)
        {
            // std::function copies its argument -- because our callbacks list
            // is a list of std::function, this causes problems when a user
            // intended for their particular instance to be called.
            // std::ref lets us get around this and point to a specific instance
            // that they handed us.
            callbacks_.push_back(std::ref(callback));
        }

        template <typename T>
        template <typename U>
        void base_provider<T>::add_on_event_callback(const U &callback)
        {
            // This is where temporaries bind to. Temporaries can't be wrapped in
            // a std::ref because they'll go away very quickly. We are forced to
            // actually copy these.
            callbacks_.push_back(callback);
        }

        template <typename T>
        void base_provider<T>::add_filter(const event_filter &f)
        {
            filters_.push_back(f);
        }

        template <typename T>
        void base_provider<T>::on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context) const
        {
            for (auto &callback : callbacks_) {
                callback(record, trace_context);
            }

            for (auto &filter : filters_) {
                filter.on_event(record, trace_context);
            }
        }
    } // namespace details

    // ------------------------------------------------------------------------

    static const GUID emptyGuid = { 0 };

    template <typename T>
    provider<T>::provider(GUID id)
    : guid_(id)
    , any_(0)
    , all_(0)
    , level_(5)
    , trace_flags_(0)
    , rundown_enabled_(false)
    {}


    inline void check_com_hr(HRESULT hr) {
        if (FAILED(hr)) {
            std::stringstream stream;
            stream << "Error in creating instance of trace providers";
            stream << ", hr = 0x";
            stream << std::hex << hr;
            throw std::runtime_error(stream.str());
        }
    }

    inline void check_provider_hr(HRESULT hr, const std::wstring &providerName) {
        if (FAILED(hr)) {
            std::stringstream stream;
            stream << "Error in constructing guid from provider name (";
            stream << from_wstring(providerName);
            stream << "), hr = 0x";
            stream << std::hex << hr;
            throw std::runtime_error(stream.str());
        }
    }

    template <typename T>
    provider<T>::provider(const std::wstring &providerName)
    {
        ITraceDataProviderCollection *allProviders;

        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        check_com_hr(hr);
        {
            hr = CoCreateInstance(
                CLSID_TraceDataProviderCollection,
                NULL,
                CLSCTX_SERVER,
                IID_ITraceDataProviderCollection,
                (void**)&allProviders);
            check_com_hr(hr);

            auto release_ptr = [](IUnknown* ptr) { ptr->Release(); };
            std::unique_ptr<ITraceDataProviderCollection, decltype(release_ptr)> allProvidersPtr(allProviders, release_ptr);

            hr = allProviders->GetTraceDataProviders(NULL);
            check_provider_hr(hr, providerName);

            ULONG count;
            hr = allProviders->get_Count((long*)&count);
            check_provider_hr(hr, providerName);

            VARIANT index;
            index.vt = VT_UI4;

            GUID providerGuid = { 0 };

            for (index.ulVal = 0; index.ulVal < count; index.ulVal++){
                ITraceDataProvider *provider;
                hr = allProviders->get_Item(index, &provider);
                check_provider_hr(hr, providerName);

                std::unique_ptr<ITraceDataProvider, decltype(release_ptr)> providerPtr(provider, release_ptr);

                _bstr_t name;
                hr = provider->get_DisplayName(name.GetAddress());
                check_provider_hr(hr, providerName);

                if (wcscmp(name, providerName.c_str()) == 0){
                    hr = provider->get_Guid(&providerGuid);
                    check_provider_hr(hr, providerName);
                    break;
                }
            }

            if (memcmp((void*)&providerGuid, (void*)&emptyGuid, sizeof(emptyGuid)) == 0)
            {
                std::stringstream stream;
                stream << "Provider name does not exist. (";
                stream << from_wstring(providerName);
                stream << "), hr = 0x";
                stream << std::hex << hr;
                throw std::runtime_error(stream.str());
            }

            guid_ = providerGuid;
            any_ = 0;
            all_ = 0;
            level_ = 5;
            trace_flags_ = 0;
            rundown_enabled_ = false;
        }

        CoUninitialize();
    }

    template <typename T>
    void provider<T>::any(T any)
    {
        any_ = any;
    }

    template <typename T>
    void provider<T>::all(T all)
    {
        all_ = all;
    }

    template <typename T>
    void provider<T>::level(T level)
    {
        level_ = level;
    }

    template <typename T>
    void provider<T>::trace_flags(T trace_flags)
    {
        trace_flags_ = trace_flags;
    }

    template <typename T>
    T provider<T>::trace_flags() const
    {
        return static_cast<T>(trace_flags_);
    }

    template <typename T>
    void provider<T>::enable_rundown_events()
    {
        rundown_enabled_ = true;
    }

    template <typename T>
    provider<T>::operator provider<>() const
    {
        provider<> tmp(guid_);
        tmp.any_            = static_cast<ULONGLONG>(any_);
        tmp.all_            = static_cast<ULONGLONG>(all_);
        tmp.level_          = static_cast<UCHAR>(level_);
        tmp.trace_flags_    = static_cast<ULONG>(trace_flags_);
        tmp.callbacks_      = this.callbacks_;

        return tmp;
    }

    inline const krabs::guid &kernel_provider::id() const
    {
        return id_;
    }

}
