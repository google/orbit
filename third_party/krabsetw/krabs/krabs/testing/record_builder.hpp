// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define INITGUID
#include <vector>
#include <memory>
#include <sstream>
#include <utility>
#include <algorithm>
#include <iterator>

#include "../compiler_check.hpp"
#include "../guid.hpp"
#include "../schema.hpp"
#include "../parser.hpp"
#include "../tdh_helpers.hpp"
#include "filler.hpp"
#include "synth_record.hpp"
#include "record_property_thunk.hpp"
#include "extended_data_builder.hpp"

namespace krabs { namespace testing {

    class record_builder;

    namespace details {

        /**
         * <summary>
         *   Provides a convenient syntax for adding properties to a
         *   record_builder.
         * </summary>
         * <remarks>
         *   Really shouldn't be used by client code. An instance of this should
         *   be handed out by a specific record_builder's `add_properties`
         *   method.
         * </remarks>
         */
        struct property_adder {
        public:
            property_adder(record_builder &builder);

            /**
             * <summary>
             * Allows chaining of property addition.
             * </summary>
             * <example>
             *    record_builder builder;
             *    builder.add_properties()
             *         (L"Name", L"Bjarne Stroustrup")
             *         (L"Level", 9001);
             * </example>
             */
            template <typename T>
            property_adder &operator()(const std::wstring &name, T &&value);

        private:
            record_builder &builder_;
        };
    }

    /**
     * <summary>
     *   Enables creation of synthetic events in order to test client code.
     * </summary>
     * <remarks>
     *   This beast of a class enables the creation of EVENT_RECORD events
     *   for testing. The class accepts a collection of keyed pairs that are
     *   then packed into the event according to the schema on the local
     *   machine. Because a lot of this is Dark Arts kind of stuff, there
     *   really isn't a guarantee that this code works perfectly. Please
     *   file bugs.
     * </remarks>
     */
    class record_builder {
    private:
    public:
        record_builder(
            const krabs::guid &providerId,
            size_t id,
            size_t version,
            size_t opcode = 0,
            size_t level = 0,
            bool trim_string_null_terminator = false);

        /**
         * <summary>Enables adding new properties to the builder.</summary>
         * <example>
         *    record_builder builder;
         *    builder.add_properties()
         *         (L"Name", L"Bjarne Stroustrup")
         *         (L"Level", 9001);
         * </example>
         */
        details::property_adder add_properties();

        /**
         * <summary>Packs the event properties into an EVENT_RECORD.</summary>
         * <example>
         *    record_builder builder;
         *    builder.add_properties()
         *         (L"Name", L"Bjarne Stroustrup")
         *         (L"Level", 9001);
         *    auto event = builder.pack();
         * </example>
         */
       krabs::testing::synth_record pack() const;

        /**
         * <summary>
         *   Packs the event properties into an EVENT_RECORD, but
         *   doesn't throw when the properties are not complete.
         * </summary>
         * <example>
         *    record_builder builder;
         *    builder.add_properties()
         *         (L"Name", L"Grumpy Gills");
         *    auto event = builder.pack_incomplete();
         * </example>
         */
        krabs::testing::synth_record pack_incomplete() const;

        /**
         * <summary>
         * Provides access to the properties that have been added.
         * </summary>
         * <example>
         *    record_builder builder;
         *    builder.add_properties()(L"Foo", 10);
         *    for (auto &prop : builder.properties()) {
         *       // ...
         *    }
         * </example>
         */
         const std::vector<record_property_thunk> &properties() const;

         /**
         * <summary>
         * Adds extended data representing a GUID for an Windows container ID
         * </summary>
         */
         void add_container_id_extended_data(const GUID& container_id);

        /**
         * <summary>
         * Gives direct access to the EVENT_HEADER that will be packed into
         * the faked record.
         * </summary>
         */
        EVENT_HEADER &header();

        /**
         * <summary>
         *   Fills an EVENT_RECORD with the info necessary to grab its schema
         *   via Tdh.
         * </summary>
         */
         EVENT_RECORD create_stub_record() const;

    private:

        /**
         * <summary>
         * Does the dirty work of packing up an event record's user data.
         * </summary>
         * <returns>
         *   A pair, where the first item is the packed user data and
         *   the second is the properties that were not filled (because the
         *   user never specified them).
         * </returns>
         */
         std::pair<std::vector<BYTE>, std::vector<std::wstring>>
         pack_impl(const EVENT_RECORD &record) const;

    private:
        const krabs::guid &providerId_;
        const size_t id_;
        const size_t version_;
        const size_t opcode_;
        const size_t level_;
        EVENT_HEADER header_;
        std::vector<record_property_thunk> properties_;
        bool trim_string_null_terminator_;
        extended_data_builder extended_data_;

        friend struct details::property_adder;
    };


    // Implementation
    // ------------------------------------------------------------------------

    inline details::property_adder::property_adder(record_builder &builder)
    : builder_(builder)
    {
    }

    template <typename T>
    details::property_adder &details::property_adder::operator()(
        const std::wstring &name,
        T &&value)
    {
        builder_.properties_.emplace_back(name, value);
        return *this;
    }

    // ------------------------------------------------------------------------

    inline record_builder::record_builder(
        const krabs::guid &providerId,
        size_t id,
        size_t version,
        size_t opcode,
        size_t level,
        bool trim_string_null_terminator)
    : providerId_(providerId)
    , id_(id)
    , version_(version)
    , opcode_(opcode)
    , level_(level)
    , trim_string_null_terminator_(trim_string_null_terminator)
    {
        ZeroMemory(&header_, sizeof(EVENT_HEADER));
        header_.EventDescriptor.Id      = static_cast<USHORT>(id_);
        header_.EventDescriptor.Version = static_cast<UCHAR>(version_);
        header_.EventDescriptor.Opcode  = static_cast<UCHAR>(opcode_);
        header_.EventDescriptor.Level   = static_cast<UCHAR>(level_);
        memcpy(&header_.ProviderId, (const GUID *)&providerId_, sizeof(GUID));
    }

    inline EVENT_HEADER &record_builder::header()
    {
        return header_;
    }

    inline details::property_adder record_builder::add_properties()
    {
        return details::property_adder(*this);
    }

    inline synth_record record_builder::pack() const
    {
        EVENT_RECORD record = create_stub_record();

        auto results = pack_impl(record);
        if (!results.second.empty()) {
            std::string msg = "Not all the properties of the event were filled:";

            for (auto& s : results.second) {
#pragma warning(push)
#pragma warning(disable: 4244) // narrowing property name wchar_t to char for this error message
                msg += " " + std::string(s.begin(), s.end());
#pragma warning(pop)
            }

            throw std::invalid_argument(msg);
        }

        // If it's a size 0 list, pack() will return (nullptr, 0) and no buffer is allocated.
        auto extended_data_buffer = extended_data_.pack();
        record.ExtendedData = reinterpret_cast<EVENT_HEADER_EXTENDED_DATA_ITEM*>(extended_data_buffer.first.get());
        record.ExtendedDataCount = static_cast<USHORT>(extended_data_.count());

        // Pass shared_ptr of the extended data buffer to make sure the buffer isn't deleted before the synth_record is.
        return krabs::testing::synth_record(record, results.first, extended_data_buffer.first);
    }

    inline synth_record record_builder::pack_incomplete() const
    {
        EVENT_RECORD record = create_stub_record();
        auto results = pack_impl(record);

        // If it's a size 0 list, pack() will return (nullptr, 0) and no buffer is allocated.
        auto extended_data_buffer = extended_data_.pack();
        record.ExtendedData = reinterpret_cast<EVENT_HEADER_EXTENDED_DATA_ITEM*>(extended_data_buffer.first.get());
        record.ExtendedDataCount = static_cast<USHORT>(extended_data_.count());

        // Pass shared_ptr of the extended data buffer to make sure the buffer isn't deleted before the synth_record is.
        return krabs::testing::synth_record(record, results.first, extended_data_buffer.first);
    }

    inline EVENT_RECORD record_builder::create_stub_record() const
    {
        EVENT_RECORD record = {0};
        memcpy(&record.EventHeader, &header_, sizeof(EVENT_HEADER));
        if (record.EventHeader.Size == 0) {
            record.EventHeader.Size = sizeof(record.EventHeader);
        }

        return record;
    }

    inline const std::vector<record_property_thunk> &
    record_builder::properties() const
    {
        return properties_;
    }

    inline void record_builder::add_container_id_extended_data(const GUID& container_id)
    {
        extended_data_.add_container_id(container_id);
    }

    inline std::pair<std::vector<BYTE>, std::vector<std::wstring>>
    record_builder::pack_impl(const EVENT_RECORD &record) const
    {
        std::pair<std::vector<BYTE>, std::vector<std::wstring>> results;
        krabs::schema_locator schema_locator;
        krabs::schema event_schema(record, schema_locator);
        krabs::parser event_parser(event_schema);

        // When the last property in a record is of string type (ansi or unicode), 
        // ETW may omit the string NULL terminator. bytes_to_trim below will eventually be
        // set to the number of bytes that can be trimmed from the generated buffer.

        auto bytes_to_trim = 0;
        for (auto prop : event_parser.properties()) {
            bytes_to_trim = 0;

            auto found_prop = std::find_if(properties_.begin(),
                                           properties_.end(),
                                           [&](const record_property_thunk &thunk) {
                                                return prop.name() == thunk.name();
                                           });

            if (found_prop != properties_.end()) {

                // Verify that the user-provided property data matches the type
                // that the schema expects.
                if (found_prop->type() != prop.type()) {
                    std::string ansi(prop.name().begin(), prop.name().end());
                    auto msg = std::string(
                        "Invalid property type given for property " + ansi +
                        " Expected: " + krabs::in_type_to_string(prop.type()) +
                        " Received: " + krabs::in_type_to_string(found_prop->type()));

                    throw std::invalid_argument(msg.c_str());
                }

                // if this is a string type, we could trim the null terminator
                // (assuming that there are no other properties after this one)
                if (prop.type() == TDH_INTYPE_UNICODESTRING) {
                    bytes_to_trim = sizeof(L'\0');
                }
                else if (prop.type() == TDH_INTYPE_ANSISTRING) {
                    bytes_to_trim = sizeof('\0');
                }

                std::copy(found_prop->bytes().begin(),
                          found_prop->bytes().end(),
                          std::back_inserter(results.first));
            } else {

                // If the property wasn't filled by the user's tests, we fill
                // it with empty data that is the size that is expected
                // according to the schema. We also remember these properties,
                // because it may be considered an error to not fill all
                // properties manually.
                results.second.emplace_back(prop.name());
                std::fill_n(std::back_inserter(results.first),
                            details::how_many_bytes_to_fill(prop.type()), static_cast<UCHAR>(0));
            }
        }

        if (trim_string_null_terminator_) {
            results.first.resize(results.first.size() - bytes_to_trim);
        }

        return results;
    }
} /* namespace testing */ } /* namespace krabs */
