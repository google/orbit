// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif

#define INITGUID

#include <windows.h>
#include <tdh.h>
#include <evntrace.h>

#include <memory>
#include <unordered_map>

#include "compiler_check.hpp"
#include "errors.hpp"
#include "guid.hpp"

#pragma comment(lib, "tdh.lib")

namespace krabs {

    /**
     * <summary>
     * Type used as the key for cache lookup in a schema_locator.
     * </summary>
     */
    struct schema_key
    {
        guid      provider;
        uint16_t  id;
        uint8_t   opcode;
        uint8_t   version;
        uint8_t   level;

        schema_key(const EVENT_RECORD &record)
            : provider(record.EventHeader.ProviderId)
            , id(record.EventHeader.EventDescriptor.Id)
            , opcode(record.EventHeader.EventDescriptor.Opcode)
            , level(record.EventHeader.EventDescriptor.Level)
            , version(record.EventHeader.EventDescriptor.Version) { }

        bool operator==(const schema_key &rhs) const
        {
            return provider == rhs.provider &&
                   id == rhs.id &&
                   opcode == rhs.opcode &&
                   level == rhs.level &&
                   version == rhs.version;
        }

        bool operator!=(const schema_key &rhs) const { return !(*this == rhs); }
    };
}

namespace std {

    /**
     * <summary>
     * Builds a hash code for a schema_key
     * </summary>
     */
    template<>
    struct std::hash<krabs::schema_key>
    {
        size_t operator()(const krabs::schema_key &key) const
        {
            // Shift-Add-XOR hash - good enough for the small sets we deal with
            size_t h = 2166136261;

            h ^= (h << 5) + (h >> 2) + std::hash<krabs::guid>()(key.provider);
            h ^= (h << 5) + (h >> 2) + key.id;
            h ^= (h << 5) + (h >> 2) + key.opcode;
            h ^= (h << 5) + (h >> 2) + key.version;
            h ^= (h << 5) + (h >> 2) + key.level;

            return h;
        }
    };
}

namespace krabs {

    /**
     * <summary>
     * Get event schema from TDH.
     * </summary>
     */
    std::unique_ptr<char[]> get_event_schema_from_tdh(const EVENT_RECORD &);

    /**
     * <summary>
     * Fetches and caches schemas from TDH.
     * NOTE: this cache also reduces the number of managed to native transitions
     * when krabs is compiled into a managed assembly.
     * </summary>
     */
    class schema_locator {
    public:

        /**
         * <summary>
         * Retrieves the event schema from the cache or falls back to
         * TDH to load the schema.
         * </summary>
         */
        const PTRACE_EVENT_INFO get_event_schema(const EVENT_RECORD &record) const;

    private:
        mutable std::unordered_map<schema_key, std::unique_ptr<char[]>> cache_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline const PTRACE_EVENT_INFO schema_locator::get_event_schema(const EVENT_RECORD &record) const
    {
        // check the cache
        auto key = schema_key(record);
        auto& buffer = cache_[key];

        if (!buffer) {
            auto temp = get_event_schema_from_tdh(record);
            buffer.swap(temp);
        }

        return (PTRACE_EVENT_INFO)(buffer.get());
    }

    inline std::unique_ptr<char[]> get_event_schema_from_tdh(const EVENT_RECORD &record)
    {
        // get required size
        ULONG bufferSize = 0;
        ULONG status = TdhGetEventInformation(
            (PEVENT_RECORD)&record,
            0,
            NULL,
            NULL,
            &bufferSize);

        if (status != ERROR_INSUFFICIENT_BUFFER) {
            error_check_common_conditions(status, record);
        }

        // allocate and fill the schema from TDH
        auto buffer = std::unique_ptr<char[]>(new char[bufferSize]);

        error_check_common_conditions(
            TdhGetEventInformation(
            (PEVENT_RECORD)&record,
            0,
            NULL,
            (PTRACE_EVENT_INFO)buffer.get(),
            &bufferSize),
            record);

        return buffer;
    }
}
