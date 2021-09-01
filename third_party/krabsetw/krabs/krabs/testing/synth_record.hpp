// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define INITGUID

#include <vector>
#include <algorithm>

#include "../compiler_check.hpp"
#include "../guid.hpp"
#include "../schema.hpp"
#include "../parser.hpp"

#include <evntrace.h>
#include <evntcons.h>
#include <memory>

namespace krabs { namespace testing {

    /**
     * <summary>
     *   Represents a property that is faked -- one that is built by hand for the
     *   purpose of testing event reaction code.
     * </summary>
     */
    class synth_record {
    public:

        /**
         * <summary>
         *   Constructs a synthetic property, given a partially filled
         *   EVENT_RECORD and a packed sequence of bytes that represent the
         *   event's user data.
         * </summary>
         * <remarks>
         *   This class should not be directly instantiated -- an record_builder
         *   should return this with its `pack` methods.
         * </remarks>
         */
        synth_record(const EVENT_RECORD& record,
                     const std::vector<BYTE>& user_data);

        /**
         * <summary>
         *   Constructs a synthetic property, given a partially filled
         *   EVENT_RECORD and a packed sequence of bytes that represent the
         *   event's user data.
         * </summary>
         * <remarks>
         *   This class should not be directly instantiated -- an record_builder
         *   should return this with its `pack` methods.
         * </remarks>
         */
        synth_record(const EVENT_RECORD &record,
                     const std::vector<BYTE> &user_data,
                     const std::shared_ptr<BYTE[]> &extended_data);

        /**
         * <summary>
         *   Copies a synth_record and updates the pointers
         *   in the EVENT_RECORD appropriately.
         * </summary>
         */
        synth_record(const synth_record& other);

        /**
         * <summary>
         *   Moves a synth_record into a new instance.
         * </summary>
         */
        synth_record(synth_record&& other);

        /**
         * <summary>
         *   Assigns a synth_record to another.
         * </summary>
         * <remarks>by value to take advantage of move ctor</remarks>
         */
        synth_record& operator=(synth_record);

        /**
         * <summary>
         *   Allows implicit casts to an EVENT_RECORD.
         * </summary>
         */
         operator const EVENT_RECORD&() const;

        /**
         * <summary>
         *   Swaps two synth_records.
         * </summary>
         */
        friend void swap(synth_record& left, synth_record& right)
        {
            using std::swap; // ADL

            swap(left.record_, right.record_);
            swap(left.data_, right.data_);
            swap(left.extended_data_, right.extended_data_);
        }

    private:
        synth_record()
            : record_()
            , data_() { }

        EVENT_RECORD record_;
        std::vector<BYTE> data_;

        // extended_data shared PTR is passed around to make sure that the data
        // buffer is only deleted after all dependent synth_records are deleted.
        // since the extended data structure uses direct pointers to data
        // instead of offsets, we can't pass around a vector<BYTE> unless we
        // also want to redo the pointers every time the buffer is copied.
        std::shared_ptr<BYTE[]> extended_data_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline synth_record::synth_record(const EVENT_RECORD& record,
        const std::vector<BYTE>& user_data)
    : synth_record(record, user_data, std::shared_ptr<BYTE[]>())
    {
        // Empty shared_ptr is fine here because there's no concern
        // about managing lifetime of an extended data buffer if there
        // is no extended data buffer.
    }

    inline synth_record::synth_record(
        const EVENT_RECORD &record,
        const std::vector<BYTE> &user_data,
        const std::shared_ptr<BYTE[]> &extended_data)
    : record_(record)
    , data_(user_data)
    , extended_data_(extended_data)
    {
        if (data_.size() > 0) {
            record_.UserData = &data_[0];
        } else {
            record_.UserData = 0;
        }

        record_.UserDataLength = static_cast<USHORT>(data_.size());
    }

    inline synth_record::synth_record(const synth_record& other)
        : synth_record(other.record_, other.data_, other.extended_data_)
    { }

    inline synth_record::synth_record(synth_record&& other)
        : synth_record()
    {
        swap(*this, other);
    }

    inline synth_record& synth_record::operator=(synth_record other)
    {
        swap(*this, other);
        return *this;
    }

    inline synth_record::operator const EVENT_RECORD&() const
    {
        return record_;
    }

} /* namespace testing */ } /* namespace krabs */
