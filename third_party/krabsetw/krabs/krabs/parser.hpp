// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <cassert>
#include <deque>
#include <utility>
#include <stdexcept>

#include <functional>

#include "compiler_check.hpp"
#include "collection_view.hpp"
#include "property.hpp"
#include "parse_types.hpp"
#include "size_provider.hpp"
#include "tdh_helpers.hpp"

namespace krabs {

    class schema;

    /**
     * <summary>
     * Used to parse specific properties out of an event schema.
     * </summary>
     * <remarks>
     * The parser class dodges the task of trying to validate that the expected
     * type of a field matches the actual type of a field -- the onus is on
     * client code to get this right.
     * </remarks>
     */
    class parser {
    public:

        /**
         * <summary>
         * Constructs an event parser from an event schema.
         * </summary>
         * <example>
         *     void on_event(const EVENT_RECORD &record)
         *     {
         *         krabs::schema schema(record);
         *         krabs::parser parser(schema);
         *     }
         * </example>
         */
        parser(const schema &);

        /**
         * <summary>
         * Returns an iterator that returns each property in the event.
         * </summary>
         * <example>
         *    void on_event(const EVENT_RECORD &record)
         *    {
         *        krabs::schema schema(record);
         *        krabs::parser parser(schema);
         *        for (property &property : parser.properties())
         *        {
         *            // ...
         *        }
         *    }
         * </example>
         */
        property_iterator properties() const;

        /**
         * <summary>
         * Attempts to retrieve the given property by name and type.
         * </summary>
         * <remarks>
         * Type hinting here is taken as the authoritative source. There is no
         * validation that the request for type is correct.
         * </remarks>
         */
        template <typename T>
        bool try_parse(const std::wstring &name, T &out);

        /**
         * <summary>
         * Attempts to parse the given property by name and type. If the
         * property does not exist, an exception is thrown.
         * </summary>
         */
        template <typename T>
        T parse(const std::wstring &name);

        template <typename Adapter>
        auto view_of(const std::wstring &name, Adapter &adapter) -> collection_view<typename Adapter::const_iterator>;

    private:
        property_info find_property(const std::wstring &name);
        void cache_property(const wchar_t *name, property_info info);

    private:
        const schema &schema_;
        const BYTE *pEndBuffer_;
        BYTE *pBufferIndex_;
        ULONG lastPropertyIndex_;

        // Maintain a mapping from property name to blob data index.
        std::deque<std::pair<const wchar_t *, property_info>> propertyCache_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline parser::parser(const schema &s)
    : schema_(s)
    , pEndBuffer_((BYTE*)s.record_.UserData + s.record_.UserDataLength)
    , pBufferIndex_((BYTE*)s.record_.UserData)
    , lastPropertyIndex_(0)
    {}

    inline property_iterator parser::properties() const
    {
        return property_iterator(schema_);
    }

    inline property_info parser::find_property(const std::wstring &name)
    {
        // A schema contains a collection of properties that are keyed by name.
        // These properties are stored in a blob of bytes that needs to be
        // interpreted according to information that is packaged up in the
        // schema and that can be retrieved using the Tdh* APIs. This format
        // requires a linear traversal over the blob, incrementing according to
        // the contents within it. This is janky, so our strategy is to
        // minimize this as much as possible via caching.

        // The first step is to use our cache for the property to see if we've
        // discovered it already.
        for (auto &item : propertyCache_) {
            if (name == item.first) {
                return item.second;
            }
        }

        const ULONG totalPropCount = schema_.pSchema_->PropertyCount;

        assert((pBufferIndex_ <= pEndBuffer_ && pBufferIndex_ >= schema_.record_.UserData) &&
               "invariant: we should've already thrown for falling off the edge");

        // accept that last property can be omitted from buffer. this happens if last property
        // is string but empty and the provider strips the null terminator
        assert((pBufferIndex_ == pEndBuffer_ ? ((totalPropCount - lastPropertyIndex_) <= 1)
                                             : true)
               && "invariant: if we've exhausted our buffer, then we must've"
                  "exhausted the properties as well");

        // We've not looked up this property before, so we have to do the work
        // to find it. While we're going through the blob to find it, we'll
        // remember what we've seen to save time later.
        //
        // Question: Why don't we just populate the cache before looking up any
        //           properties and simplify our code (less state, etc)?
        //
        // Answer:   Doing that introduces overhead in the case that only a
        //           subset of properties are needed. While this code is a bit
        //           more complicated, we introduce no additional performance
        //           overhead at runtime.
        for (auto &i = lastPropertyIndex_; i < totalPropCount; ++i) {

            auto &currentPropInfo = schema_.pSchema_->EventPropertyInfoArray[i];
            const wchar_t *pName = reinterpret_cast<const wchar_t*>(
                                        reinterpret_cast<BYTE*>(schema_.pSchema_) +
                                        currentPropInfo.NameOffset);

            ULONG propertyLength = size_provider::get_property_size(
                                        pBufferIndex_,
                                        pName,
                                        schema_.record_,
                                        currentPropInfo);

            // verify that the length of the property doesn't exceed the buffer
            if (pBufferIndex_ + propertyLength > pEndBuffer_) {
                throw std::out_of_range("Property length past end of property buffer");
            }

            property_info propInfo(pBufferIndex_, currentPropInfo, propertyLength);
            cache_property(pName, propInfo);

            // advance the buffer index since we've already processed this property
            pBufferIndex_ += propertyLength;

            // The property was found, return it
            if (name == pName) {
                // advance the index since we've already processed this property
                ++i;
                return propInfo;
            }
        }

        // property wasn't found, return an empty propInfo
        return property_info();
    }

    inline void parser::cache_property(const wchar_t *name, property_info propInfo)
    {
        propertyCache_.push_front(std::make_pair(name, propInfo));
    }

    inline void throw_if_property_not_found(const property_info &propInfo)
    {
        if (!propInfo.found()) {
            throw std::runtime_error("Property with the given name does not exist");
        }
    }

    template <typename T>
    size_t get_string_content_length(const T* string, size_t lengthBytes)
    {
        // for some string types the length includes the null terminator
        // so we need to find the length of just the content part

        T nullChar {0};
        auto length = lengthBytes / sizeof(T);

        for (auto i = length; i >= 1; --i)
            if (string[i - 1] != nullChar) return i;

        return 0;
    }

    // try_parse
    // ------------------------------------------------------------------------

    template <typename T>
    bool parser::try_parse(const std::wstring &name, T &out)
    {
        try {
            out = parse<T>(name);
            return true;
        }

#ifndef NDEBUG
        // in debug builds we want any mismatch asserts
        // to get back to the caller. This is removed
        // in release builds.
        catch (const krabs::type_mismatch_assert&) {
            throw;
        }
#endif // NDEBUG

        catch (...) {
            return false;
        }
    }

    // parse
    // ------------------------------------------------------------------------

    template <typename T>
    T parser::parse(const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<T>(name, propInfo);

        // ensure that size of the type we are requesting is
        // the same size of the property in the event
        if (sizeof(T) != propInfo.length_)
            throw std::runtime_error("Property size doesn't match requested size");

        return *(T*)propInfo.pPropertyIndex_;
    }

    template<>
    inline bool parser::parse<bool>(const std::wstring& name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<bool>(name, propInfo);

        // Boolean in ETW is 4 bytes long
        return static_cast<bool>(*(unsigned*)propInfo.pPropertyIndex_);
    }

    template <>
    inline std::wstring parser::parse<std::wstring>(const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<std::wstring>(name, propInfo);

        auto string = reinterpret_cast<const wchar_t*>(propInfo.pPropertyIndex_);
        auto length = get_string_content_length(string, propInfo.length_);

        return std::wstring(string, length);
    }

    template <>
    inline std::string parser::parse<std::string>(const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<std::string>(name, propInfo);

        auto string = reinterpret_cast<const char*>(propInfo.pPropertyIndex_);
        auto length = get_string_content_length(string, propInfo.length_);

        return std::string(string, length);
    }

    template<>
    inline const counted_string* parser::parse<const counted_string*>(const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<const counted_string*>(name, propInfo);

        return reinterpret_cast<const counted_string*>(propInfo.pPropertyIndex_);
    }

    template<>
    inline binary parser::parse<binary>(const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        // no type asserts for binary - anything can be read as binary

        return binary(propInfo.pPropertyIndex_, propInfo.length_);
    }

    template<>
    inline ip_address parser::parse<ip_address>(
        const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<ip_address>(name, propInfo);

        auto outType = propInfo.pEventPropertyInfo_->nonStructType.OutType;

        switch (outType) {
        case TDH_OUTTYPE_IPV6:
            return ip_address::from_ipv6(propInfo.pPropertyIndex_);

        case TDH_OUTTYPE_IPV4:
            return ip_address::from_ipv4(*(DWORD*)propInfo.pPropertyIndex_);

        default:
            throw std::runtime_error("IP Address was not IPV4 or IPV6");
        }
    }

    template<>
    inline socket_address parser::parse<socket_address>(
        const std::wstring &name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<socket_address>(name, propInfo);

        return socket_address::from_bytes(propInfo.pPropertyIndex_, propInfo.length_);
    }

    template<>
    inline sid parser::parse<sid>(
        const std::wstring& name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<sid>(name, propInfo);
        auto InType = propInfo.pEventPropertyInfo_->nonStructType.InType;

        // A WBEMSID is actually a TOKEN_USER structure followed by the SID.
        // We only care about the SID. From MSDN:
        //
        //      The size of the TOKEN_USER structure differs
        //      depending on whether the events were generated on a 32 - bit
        //      or 64 - bit architecture. Also the structure is aligned
        //      on an 8 - byte boundary, so its size is 8 bytes on a
        //      32 - bit computer and 16 bytes on a 64 - bit computer.
        //      Doubling the pointer size handles both cases.
        ULONG sid_start = 16;
        if (EVENT_HEADER_FLAG_32_BIT_HEADER == (schema_.record_.EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER)) {
            sid_start = 8;
        }
        switch (InType) {
        case TDH_INTYPE_SID:
            return sid::from_bytes(propInfo.pPropertyIndex_, propInfo.length_);
        case TDH_INTYPE_WBEMSID:
            // Safety measure to make sure we don't overflow
            if (propInfo.length_ <= sid_start) {
                throw std::runtime_error(
                    "Requested a WBEMSID property but data is too small");
            }
            return sid::from_bytes(propInfo.pPropertyIndex_ + sid_start, propInfo.length_ - sid_start);

        default:
            throw std::runtime_error("SID was not a SID or WBEMSID");
        }
    }

    template<>
    inline pointer parser::parse<pointer>(const std::wstring& name)
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        krabs::debug::assert_valid_assignment<pointer>(name, propInfo);

        return pointer::from_bytes(propInfo.pPropertyIndex_, propInfo.length_);
    }

    // view_of
    // ------------------------------------------------------------------------

    template <typename Adapter>
    auto parser::view_of(const std::wstring &name, Adapter &adapter)
        -> collection_view<typename Adapter::const_iterator>
    {
        auto propInfo = find_property(name);
        throw_if_property_not_found(propInfo);

        // TODO: type asserts?

        return adapter(propInfo);
    }
}
