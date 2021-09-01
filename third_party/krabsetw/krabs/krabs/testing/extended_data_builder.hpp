// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <algorithm>
#include <vector>
#include <memory>

#include <evntcons.h>
#include <WinDef.h>
#include <objbase.h>

// TODO: Remove this #define once Krabs starts using Windows SDK v. 10.0.19041.0 or later.
// From evntcons.h starting in Windows SDK v. 10.0.19041.0.
#ifndef EVENT_HEADER_EXT_TYPE_CONTAINER_ID
    #define EVENT_HEADER_EXT_TYPE_CONTAINER_ID 16
#endif

namespace krabs { namespace testing {
    class extended_data_builder;

    /**
     * <summary>
     *   Since extended data items have to be packed later, we have to hold onto the data
     *   until we're ready to pack it.
     * </summary>
     */
    class extended_data_thunk 
    {
    public:
        extended_data_thunk(USHORT ext_type, BYTE* data, size_t data_length);

    private:
        // Intentionally not defined.
        extended_data_thunk();

        USHORT ext_type_;
        std::vector<BYTE> bytes_;

        friend class extended_data_builder;
    };

    /**
     * <summary>
     *   Generates fake packed EVENT_HEADER_EXTENDED_DATA_ITEM structures to later add into test
     *   synth_record objects. These are not guaranteed to be indistinguishable from the real
     *   thing, just good enough to unit test code that reads/interprets extended data.
     *
     *   Note for testing: this builder just appends extended data structures, it won't stop you
     *   from breaking any API invariants, such as only one of a specific extended data item type.
     * </summary>
     */
    class extended_data_builder
    {
    public:
        static constexpr size_t GUID_STRING_LENGTH_NO_BRACES = 36;
        static constexpr size_t GUID_STRING_LENGTH_WITH_BRACES = GUID_STRING_LENGTH_NO_BRACES + 2;


        extended_data_builder()
        : items_()
        {}

        // Mocks a container ID type extended data item.
        void add_container_id(const GUID& container_id);

        // This generates a contiguous buffer holding all of the data for
        // the extended data items. Non-trivial because the actual structs
        // have to be a contiguous array, and they each contain pointers,
        // not offsets, to dynamically sized data buffers.
        std::pair<std::shared_ptr<BYTE[]>, size_t> pack() const;

        // Returns the value that should correspond with EVENT_RECORD.ExtendedDataCount
        inline size_t count() const { return items_.size(); }

    private:
        std::vector<extended_data_thunk> items_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline extended_data_thunk::extended_data_thunk(USHORT ext_type, BYTE* data, size_t data_length)
    : ext_type_(ext_type)
    , bytes_()
    {
        bytes_.assign(data, data + data_length);
    }

    inline void extended_data_builder::add_container_id(const GUID& container_id)
    {
        // With null terminator
        wchar_t wide_guid_buffer[GUID_STRING_LENGTH_WITH_BRACES + 1] = {};

        // No null terminator
        BYTE guid_data[GUID_STRING_LENGTH_NO_BRACES] = {};

        StringFromGUID2(container_id, wide_guid_buffer, sizeof(wide_guid_buffer));

        for (int i = 0; i < GUID_STRING_LENGTH_NO_BRACES; i++)
        {
            // Offset by 1 to ignore the wrapping braces.
            guid_data[i] = static_cast<BYTE>(wide_guid_buffer[i + 1]);
        }

        items_.emplace_back(static_cast<USHORT>(EVENT_HEADER_EXT_TYPE_CONTAINER_ID), guid_data, GUID_STRING_LENGTH_NO_BRACES);
    }

    inline std::pair<std::shared_ptr<BYTE[]>, size_t> extended_data_builder::pack() const
    {
        // Return null for buffer if there are no extended data items.
        if (items_.size() == 0)
        {
            return std::make_pair(std::shared_ptr<BYTE[]>(nullptr), 0);
        }

        BYTE* data_buffer = nullptr;
        size_t data_buffer_size = 0;

        // Step 1: compute the required buffer size
        size_t array_part_size = sizeof(EVENT_HEADER_EXTENDED_DATA_ITEM) * items_.size();
        size_t data_part_size = 0;

        for (const extended_data_thunk& item : items_)
        {
            data_part_size += item.bytes_.size();
        }

        // Allocate the buffer and zero it
        data_buffer = new BYTE[array_part_size + data_part_size];
        data_buffer_size = array_part_size + data_part_size;
        ZeroMemory(data_buffer, data_buffer_size);

        // Step 2: Fill the buffer. For each extended data item, write the object into the buffer at the back.
        auto array_ptr = reinterpret_cast<EVENT_HEADER_EXTENDED_DATA_ITEM*>(data_buffer);
        auto data_ptr = data_buffer + array_part_size;

        for (int i = 0; i < items_.size(); i++)
        {
            // 2a: write the struct
            auto& destination = array_ptr[i];
            const auto& thunk = items_[i];
            const size_t thunk_size = thunk.bytes_.size();

            destination.ExtType = thunk.ext_type_;
            destination.DataSize = static_cast<USHORT>(thunk_size);
            // Assert that the conversion did not truncate thunk_size.
            assert(static_cast<size_t>(destination.DataSize) == thunk_size);

            // 2b: Write the data
            assert((data_buffer + data_buffer_size) > data_ptr); // prevent wraparound with unsigned int math
            size_t remaining = (data_buffer + data_buffer_size) - (data_ptr);
            // Assert that we will not truncate the data due to not allocating enough space in the buffer.
            assert(remaining >= thunk_size);
            // Make sure we rather not copy all of the data than overrun the buffer.
            memcpy_s(data_ptr, std::min<size_t>(remaining, thunk_size), thunk.bytes_.data(), thunk_size);

            // 2c: point the DataPtr field at the data
            destination.DataPtr = reinterpret_cast<ULONGLONG>(data_ptr);

            // 2d: increment the pointer for where to write the next piece of data
            data_ptr += destination.DataSize;
        }

        return std::make_pair(std::shared_ptr<BYTE[]>(data_buffer), data_buffer_size);
    }
} }
