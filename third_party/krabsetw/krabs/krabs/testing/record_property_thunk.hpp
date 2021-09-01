// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "../compiler_check.hpp"
#include "filler.hpp"

namespace krabs { namespace testing {

    class record_builder;

    /**
     * <summary>
     *   Takes any value and turns it into a sequence of serialized bytes.
     * </summary>
     * <remarks>
     *   When we're composing an event, we need to store heterogeneous types in
     *   a collection while we wait until we know exactly how to pack the actual
     *   event. Because the actual EVENT_RECORD structure properties are packed
     *   into a byte collection, we take our cue from that and do similarly. We
     *   keep all of the random property byte blobs separate until we know the
     *   particular order to stash them in so we have less futzing to do later.
     * </remarks>
     */
    class record_property_thunk {
    public:

        template <typename T>
        record_property_thunk(const std::wstring &property, const T &value);

        record_property_thunk(const std::wstring &property, const wchar_t *value);
        record_property_thunk(const std::wstring &property, const char *value);
        record_property_thunk(const std::wstring &property, bool value);

        const std::wstring &name() const;
        const std::vector<BYTE> &bytes() const;
        const _TDH_IN_TYPE type() const;

    private:

        // We need this because we don't have delegating constructors in VS 2012.
        template <typename T>
        void common_string_init(const std::wstring &property, const T &value);

        template <typename T>
        void common_init(const std::wstring &property, const T &value);

    private:
        std::wstring name_;
        std::vector<BYTE> bytes_;
        _TDH_IN_TYPE type_;

        friend class record_builder;
    };

    // Implementation
    // ------------------------------------------------------------------------
    template <typename T>
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        const T &value)
    {
        common_init(property, value);
    }

    // Specialization for wstrings
    template <>
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        const std::wstring &value)
    {
        common_string_init(property, value);
    }

    // Specialization for strings
    template <>
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        const std::string &value)
    {
        common_string_init(property, value);
    }

    // Overload for wchar_t strings.
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        const wchar_t *value)
    {
        common_string_init(property, std::move(std::wstring(value)));
    }

    // Overload for char strings.
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        const char *value)
    {
        common_string_init(property, std::move(std::string(value)));
    }

    // Specialization for binary blobs
    template <>
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        const krabs::binary &bin)
    : name_(property)
    , bytes_(bin.bytes())
    , type_(krabs::testing::details::tdh_morphism<krabs::binary>::value)
    {
    }

    // Overload for booleans
    inline record_property_thunk::record_property_thunk(
        const std::wstring &property,
        bool value)
    {
        common_init(property, (int)value);
        type_ = krabs::testing::details::tdh_morphism<bool>::value;
    }

    template <typename T>
    void record_property_thunk::common_init(
        const std::wstring &property,
        const T &value)
    {
        name_  = property;
        bytes_ = std::move(std::vector<BYTE>((BYTE*)&value, (BYTE*)&value + sizeof(T)));
        type_  = krabs::testing::details::tdh_morphism<T>::value;
    }

    template <typename T>
    void record_property_thunk::common_string_init(
        const std::wstring &property,
        const T &value)
    {
        name_ = property;

        const size_t size = value.size() * sizeof(typename T::value_type);
        bytes_ = std::move(std::vector<BYTE>((BYTE*)&value[0], (BYTE*)&value[0] + size));
        type_  =  krabs::testing::details::tdh_morphism<T>::value;

        // Null terminate the string
        for (size_t i = 0; i < sizeof(typename T::value_type); ++i) {
            bytes_.push_back('\0');
        }
    }

    inline const std::wstring &record_property_thunk::name() const
    {
        return name_;
    }

    inline const std::vector<BYTE> &record_property_thunk::bytes() const
    {
        return bytes_;
    }

    inline const _TDH_IN_TYPE record_property_thunk::type() const
    {
        return type_;
    }

} /* namespace testing */ } /* namespace krabs */
