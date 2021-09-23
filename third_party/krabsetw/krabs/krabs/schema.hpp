// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif

#define INITGUID

#include <memory>

#include <windows.h>
#include <tdh.h>
#include <evntrace.h>

#include "compiler_check.hpp"
#include "schema_locator.hpp"

#pragma comment(lib, "tdh.lib")


namespace krabs { namespace testing {
    class record_builder;
} /* namespace testing */ } /* namespace krabs */


namespace krabs {

    class schema;
    class parser;

    /**
     * <summary>
     * Used to query events for detailed information. Creation is rather
     * costly, so client code should try hard to delay creation of this.
     * </summary>
     */
    class schema {
    public:

        /**
         * <summary>
         * Constructs a schema from an event record instance
         * using the provided schema_locator.
         * </summary>
         *
         * <example>
         *   void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *   {
         *       krabs::schema schema(record, trace_context.schema_locator);
         *   }
         * </example>
         */
        schema(const EVENT_RECORD &, const krabs::schema_locator &);

        /**
         * <summary>Compares two schemas for equality.<summary>
         *
         * <example>
         *   schema1 == schema2;
         *   schema1 != schema2;
         * </example>
         */
        bool operator==(const schema &other) const;
        bool operator!=(const schema &other) const;

        /*
         * <summary>
         * Returns the name of an event via its schema.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        std::wstring name = krabs::event_name(schema);
         *    }
         * </example>
         */
        const wchar_t *event_name() const;

        /*
         * <summary>
         * Returns the name of an opcode via its schema.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        std::wstring name = krabs::opcode_name(schema);
         *    }
         * </example>
         */
        const wchar_t* opcode_name() const;

        /*
         * <summary>
         * Returns the taskname of an event via its schema.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        std::wstring name = krabs::task_name(schema);
         *    }
         * </example>
         */
        const wchar_t *task_name() const;

        /*
         * <summary>
         * Returns the DECODING_SOURCE of an event via its schema.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        DECODING_SOURCE source = krabs::decoding_source(schema);
         *    }
         * </example>
         */
        DECODING_SOURCE decoding_source() const;

        /**
         * <summary>
         * Returns the event ID via its schema.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        int id = schema.event_id();
         *    }
         * </example>
         */
        int event_id() const;

        /**
         * <summary>
         * Returns the event opcode.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        int opcode = schema.event_opcode();
         *    }
         * </example>
         */
        int event_opcode() const;

        /**
         * <summary>
         * Returns the version of the event.
         * </summary>
         */
        unsigned int event_version() const;

        /**
         * <summary>
         * Returns the flags of the event.
         * </summary>
         */
        unsigned int event_flags() const;

        /**
         * <summary>
         * Returns the provider name of an event via its schema.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
         *    {
         *        krabs::schema schema(record, trace_context.schema_locator);
         *        std::wstring name = krabs::provider_name(schema);
         *    }
         * </example>
         */
        const wchar_t *provider_name() const;

        /**
        * <summary>
        * Returns the PID associated with the event via its schema.
        * </summary>
        * <example>
        *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
        *    {
        *        krabs::schema schema(record, trace_context.schema_locator);
        *        unsigned int name = krabs::process_id(schema);
        *    }
        * </example>
        */
        unsigned int process_id() const;

        /**
        * <summary>
        * Returns the Thread ID associated with the event via its schema.
        * </summary>
        * <example>
        *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
        *    {
        *        krabs::schema schema(record, trace_context.schema_locator);
        *        unsigned int name = krabs::thread_id(schema);
        *    }
        * </example>
        */
        unsigned int thread_id() const;

        /**
        * <summary>
        * Returns the timestamp associated with the event via its schema.
        * </summary>
        * <example>
        *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
        *    {
        *        krabs::schema schema(record, trace_context.schema_locator);
        *        LARGE_INTEGER time = krabs::timestamp(schema);
        *    }
        * </example>
        */
        LARGE_INTEGER timestamp() const;

        /**
        * <summary>
        * Returns the Activity ID associated with the event via its schema.
        * </summary>
        * <example>
        *    void on_event(const EVENT_RECORD &record, const krabs::trace_context &trace_context)
        *    {
        *        krabs::schema schema(record, trace_context.schema_locator);
        *        GUID activity_id = krabs::activity_id(schema);
        *    }
        * </example>
        */
        GUID activity_id() const;

    private:
        const EVENT_RECORD &record_;
        TRACE_EVENT_INFO *pSchema_;

    private:
        friend std::wstring event_name(const schema &);
        friend std::wstring opcode_name(const schema &);
        friend std::wstring task_name(const schema &);
        friend DECODING_SOURCE decoding_source(const schema &);
        friend std::wstring provider_name(const schema &);
        friend unsigned int process_id(const schema &);
        friend LARGE_INTEGER timestamp(const schema &);
        friend GUID activity_id(const schema&);
        friend int event_id(const EVENT_RECORD &);
        friend int event_id(const schema &);

        friend class parser;
        friend class property_iterator;
        friend class record_builder;
    };


    // Implementation
    // ------------------------------------------------------------------------

    inline schema::schema(const EVENT_RECORD &record, const krabs::schema_locator &schema_locator)
        : record_(record)
        , pSchema_(schema_locator.get_event_schema(record))
    { }

    inline bool schema::operator==(const schema &other) const
    {
        return (pSchema_->ProviderGuid == other.pSchema_->ProviderGuid &&
                pSchema_->EventDescriptor.Id == other.pSchema_->EventDescriptor.Id &&
                pSchema_->EventDescriptor.Version == other.pSchema_->EventDescriptor.Version);
    }

    inline bool schema::operator!=(const schema &other) const
    {
        return !(*this == other);
    }

    inline const wchar_t *schema::event_name() const
    {
        /*
        EventNameOffset will be 0 if the event does not have an assigned name or
        if this event is decoded on a system that does not support decoding
        manifest event names. Event name decoding is supported on Windows
        10 Fall Creators Update (2017) and later.
        */
        if (pSchema_->EventNameOffset != 0) {
            return reinterpret_cast<const wchar_t*>(
                reinterpret_cast<const char*>(pSchema_) +
                pSchema_->EventNameOffset);
        }
        else {
            return L"";
        }
    }

    inline const wchar_t* schema::opcode_name() const
    {
        /*
        In WPP Traces OpcodeName is not used
        */
        if (pSchema_->OpcodeNameOffset != 0) {
            return reinterpret_cast<const wchar_t*>(
                reinterpret_cast<const char*>(pSchema_) +
                pSchema_->OpcodeNameOffset);
        }
        else {
            return L"";
        }
    }

    inline const wchar_t *schema::task_name() const
    {
        if (pSchema_->TaskNameOffset != 0) {
            return reinterpret_cast<const wchar_t*>(
                reinterpret_cast<const char*>(pSchema_) +
                pSchema_->TaskNameOffset);
        }
        else {
            return L"";
        }
    }

    inline DECODING_SOURCE schema::decoding_source() const
    {
        return pSchema_->DecodingSource;
    }

    inline int schema::event_id() const
    {
        return record_.EventHeader.EventDescriptor.Id;
    }

    inline int schema::event_opcode() const
    {
        return record_.EventHeader.EventDescriptor.Opcode;
    }

    inline unsigned int schema::event_version() const
    {
        return record_.EventHeader.EventDescriptor.Version;
    }

    inline unsigned int schema::event_flags() const
    {
        return record_.EventHeader.Flags;
    }

    inline const wchar_t *schema::provider_name() const
    {
       return reinterpret_cast<const wchar_t*>(
           reinterpret_cast<const char*>(pSchema_) +
           pSchema_->ProviderNameOffset);
    }

    inline unsigned int schema::process_id() const
    {
        return record_.EventHeader.ProcessId;
    }

    inline unsigned int schema::thread_id() const
    {
        return record_.EventHeader.ThreadId;
    }

    inline LARGE_INTEGER schema::timestamp() const
    {
        return record_.EventHeader.TimeStamp;
    }

    inline GUID schema::activity_id() const
    {
        return record_.EventHeader.ActivityId;
    }
}
