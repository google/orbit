// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Interface for ETW.

#pragma once

#define INITGUID

#include "compiler_check.hpp"
#include "trace.hpp"
#include "errors.hpp"

#include <cassert>
#include <map>

#include <evntrace.h>
#include <evntcons.h>

namespace krabs { namespace details {

    // The ETW API requires that we reserve enough memory behind
    // an EVENT_TRACE_PROPERTIES buffer in order to store an ETW trace name
    // and an optional ETW log file name. The easiest way to do this is to
    // use a struct to reserve this space -- the alternative is to malloc
    // the bytes at runtime (ew).
    class trace_info {
    public:
        EVENT_TRACE_PROPERTIES properties;
        wchar_t traceName[MAX_PATH];
        wchar_t logfileName[MAX_PATH];
    };

    /**
     * <summary>
     * Used to implement starting and stopping traces.
     * </summary>
     */
    template <typename T>
    class trace_manager {
    public:
        trace_manager(T &trace);

        /**
         * <summary>
         * Starts the ETW trace identified by the info in the trace type.
         * </summary>
         */
        void start();

        /**
         * <summary>
         * Stops the ETW trace identified by the info in the trace type.
         * </summary>
         */
        void stop();

        /**
        * <summary>
        * Opens the ETW trace identified by the info in the trace type.
        * </summary>
        */
        EVENT_TRACE_LOGFILE open();

        /**
        * <summary>
        * Starts processing the ETW trace identified by the info in the trace type.
        * open() needs to called for this to work first.
        * </summary>
        */
        void process();

        /**
         * <summary>
         * Queries the ETW trace identified by the info in the trace type.
         * </summary>
         */
        EVENT_TRACE_PROPERTIES query();

        /**
         * <summary>
         * Configures the ETW trace session settings.
         * See https://docs.microsoft.com/en-us/windows/win32/api/evntrace/nf-evntrace-tracesetinformation.
         * </summary>
         */
        void set_trace_information(
            TRACE_INFO_CLASS information_class,
            PVOID trace_information,
            ULONG information_length);

        /**
         * <summary>
         * Notifies the underlying trace of the buffers that were processed.
         * </summary>
         */
        void set_buffers_processed(size_t processed);

        /**
         * <summary>
         * Notifies the underlying trace that an event occurred.
         * </summary>
         */
        void on_event(const EVENT_RECORD &record);

    private:
        trace_info fill_trace_info();
        EVENT_TRACE_LOGFILE fill_logfile();
        void close_trace();
        void register_trace();
        EVENT_TRACE_PROPERTIES query_trace();
        void stop_trace();
        EVENT_TRACE_LOGFILE open_trace();
        void process_trace();
        void enable_providers();

    private:
        T &trace_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    /**
     * <summary>
     *   Called by ETW when an event occurs, forwards calls to the
     *   appropriate instance.
     * </summary>
     * <remarks>
     *   A pointer to the instance is stored in the UserContext
     *   field of the EVENT_RECORD. This is set via the Context field of the
     *   EVENT_TRACE_LOGFILE structure.
     * </remarks>
     */
    template <typename T>
    static void __stdcall trace_callback_thunk(EVENT_RECORD *pRecord)
    {
        auto *pUserTrace = (T*)(pRecord->UserContext);
        trace_manager<T> trace(*pUserTrace);
        trace.on_event(*pRecord);
    }

    /**
     * <summary>
     *   Called by ETW after the events for each buffer are delivered, gives
     *   statistics like the number of buffers processed and the number of
     *   events dropped.
     * </summary>
     * <remarks>
     *   A pointer to the instance is stored in the UserContext
     *   field of the EVENT_RECORD. This is set via the Context field of the
     *   EVENT_TRACE_LOGFILE structure.
     * </remarks>
     */
    template <typename T>
    static ULONG __stdcall trace_buffer_callback(EVENT_TRACE_LOGFILE *pLogFile)
    {
        auto *pTrace = (T*)(pLogFile->Context);
        trace_manager<T> trace(*pTrace);

        // NOTE: EventsLost is not set on this type
        trace.set_buffers_processed(pLogFile->BuffersRead);
        return TRUE;
    }

    template <typename T>
    trace_manager<T>::trace_manager(T &trace)
    : trace_(trace)
    {}

    template <typename T>
    void trace_manager<T>::start()
    {
        if (trace_.sessionHandle_ == INVALID_PROCESSTRACE_HANDLE) {
            (void)open();
        }
        process_trace();
    }

    template <typename T>
    EVENT_TRACE_LOGFILE trace_manager<T>::open()
    {
        register_trace();
        enable_providers();
        return open_trace();
    }

    template <typename T>
    void trace_manager<T>::process()
    {
        process_trace();
    }

    template <typename T>
    EVENT_TRACE_PROPERTIES trace_manager<T>::query()
    {
        return query_trace();
    }

    template <typename T>
    void trace_manager<T>::set_trace_information(
        TRACE_INFO_CLASS information_class,
        PVOID trace_information,
        ULONG information_length)
    {
        ULONG status = TraceSetInformation(
            trace_.registrationHandle_, 
            information_class,
            trace_information,
            information_length);

        error_check_common_conditions(status);
    }

    template <typename T>
    void trace_manager<T>::stop()
    {
        stop_trace();
        close_trace();
    }

    template <typename T>
    void trace_manager<T>::set_buffers_processed(size_t processed)
    {
        trace_.buffersRead_ = processed;
    }

    template <typename T>
    void trace_manager<T>::on_event(const EVENT_RECORD &record)
    {
        trace_.on_event(record);
    }

    template <typename T>
    trace_info trace_manager<T>::fill_trace_info()
    {
        trace_info info = {};
        info.properties.Wnode.BufferSize    = sizeof(trace_info);
        info.properties.Wnode.Guid          = T::trace_type::get_trace_guid();
        info.properties.Wnode.Flags         = WNODE_FLAG_TRACED_GUID;
        info.properties.Wnode.ClientContext = 1; // QPC clock resolution
        info.properties.BufferSize          = trace_.properties_.BufferSize;
        info.properties.MinimumBuffers      = trace_.properties_.MinimumBuffers;
        info.properties.MaximumBuffers      = trace_.properties_.MaximumBuffers;
        info.properties.FlushTimer          = trace_.properties_.FlushTimer;

        if (trace_.properties_.LogFileMode)
            info.properties.LogFileMode     = trace_.properties_.LogFileMode;
        else
            info.properties.LogFileMode     = EVENT_TRACE_REAL_TIME_MODE
                                            | EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING;

        info.properties.LogFileMode         |= T::trace_type::augment_file_mode();
        info.properties.LoggerNameOffset    = offsetof(trace_info, logfileName);
        info.properties.EnableFlags         = T::trace_type::construct_enable_flags(trace_);
        assert(info.traceName[0] == '\0');
        assert(info.logfileName[0] == '\0');
        trace_.name_._Copy_s(info.traceName, ARRAYSIZE(info.traceName), trace_.name_.length());
        return info;
    }

    template <typename T>
    EVENT_TRACE_LOGFILE trace_manager<T>::fill_logfile()
    {
        EVENT_TRACE_LOGFILE file = {};
        file.LoggerName          = const_cast<wchar_t*>(trace_.name_.c_str());
        file.ProcessTraceMode    = PROCESS_TRACE_MODE_EVENT_RECORD |
                                   PROCESS_TRACE_MODE_REAL_TIME;
        file.Context             = (void *)&trace_;
        file.EventRecordCallback = trace_callback_thunk<T>;
        file.BufferCallback      = trace_buffer_callback<T>;
        return file;
    }

    template <typename T>
    void trace_manager<T>::stop_trace()
    {
        trace_info info = fill_trace_info();
        ULONG status = ControlTrace(
            NULL,
            trace_.name_.c_str(),
            &info.properties,
            EVENT_TRACE_CONTROL_STOP);

        if (status != ERROR_WMI_INSTANCE_NOT_FOUND) {
            error_check_common_conditions(status);
        }
    }

    template <typename T>
    EVENT_TRACE_PROPERTIES trace_manager<T>::query_trace()
    {
        trace_info info = fill_trace_info();
        ULONG status = ControlTrace(
            NULL,
            trace_.name_.c_str(),
            &info.properties,
            EVENT_TRACE_CONTROL_QUERY);

        if (status != ERROR_WMI_INSTANCE_NOT_FOUND) {
            error_check_common_conditions(status);

            return info.properties;
        }

        return { };
    }

    template <typename T>
    void trace_manager<T>::register_trace()
    {
        trace_info info = fill_trace_info();

        ULONG status = StartTrace(&trace_.registrationHandle_,
                                  trace_.name_.c_str(),
                                  &info.properties);
        if (status == ERROR_ALREADY_EXISTS) {
            try {
                stop_trace();
                status = StartTrace(&trace_.registrationHandle_,
                    trace_.name_.c_str(),
                    &info.properties);
            }
            catch (need_to_be_admin_failure) {
                (void)open_trace();
                close_trace();
                // insufficient privilege to stop/configure
                // but if open/close didn't throw also
                // then we're okay to process events
                status = ERROR_SUCCESS;
                // we also invalidate the registrationHandle_
                // StartTrace() actually sets this to 0 on failure
                trace_.registrationHandle_ = INVALID_PROCESSTRACE_HANDLE;
            }
            catch (invalid_parameter) {
                // In some versions, the error code is 87 when using
                // SystemTraceControlGuid session. If open/close doesn't
                // throw, then we can continually processing events.
                (void)open_trace();
                close_trace();
                status = ERROR_SUCCESS;
                trace_.registrationHandle_ = INVALID_PROCESSTRACE_HANDLE;
            }
        }

        error_check_common_conditions(status);
    }

    template <typename T>
    EVENT_TRACE_LOGFILE trace_manager<T>::open_trace()
    {
        auto file = fill_logfile();
        trace_.sessionHandle_ = OpenTrace(&file);
        if (trace_.sessionHandle_ == INVALID_PROCESSTRACE_HANDLE) {
            throw open_trace_failure();
        }
        return file;
    }

    template <typename T>
    void trace_manager<T>::process_trace()
    {
        if (trace_.sessionHandle_ == INVALID_PROCESSTRACE_HANDLE) {
            throw open_trace_failure();
        }

        // Refactoring warning.
        // During the testing of the (slower) C++/CLI implementation it became evident that
        // EnableTraceEx2(EVENT_CONTROL_CODE_CAPTURE_STATE) must be called very shortly
        // before ProcessTrace() in order for the rundown events to be generated.
        T::trace_type::enable_rundown(trace_);

        ULONG status = ProcessTrace(&trace_.sessionHandle_, 1, NULL, NULL);
        error_check_common_conditions(status);
    }

    template <typename T>
    void trace_manager<T>::close_trace()
    {
        if (trace_.sessionHandle_ != INVALID_PROCESSTRACE_HANDLE) {
            ULONG status = CloseTrace(trace_.sessionHandle_);
            trace_.sessionHandle_ = INVALID_PROCESSTRACE_HANDLE;

            if (status != ERROR_CTX_CLOSE_PENDING) {
                error_check_common_conditions(status);
            }
        }
    }

    template <typename T>
    void trace_manager<T>::enable_providers()
    {
        T::trace_type::enable_providers(trace_);
    }
} /* namespace details */ } /* namespace krabs */
