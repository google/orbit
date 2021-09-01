// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif

#define INITGUID

#include <memory>
#include <vector>

#include "compiler_check.hpp"
#include "schema.hpp"
#include "errors.hpp"

#include <windows.h>
#include <tdh.h>
#include <evntrace.h>

#pragma comment(lib, "tdh.lib")

namespace krabs {

    /**
     * <summary>
     * Represents a single property of the record schema.
     * </summary>
     * <remarks>
     *   Noticeably absent from this property is the ability to ask what its
     *   value is. The reason for this is that this property instance is
     *   intended to work with synth_records, which don't always have data to
     *   correspond with properties. This class *cannot* return a value because
     *   there isn't always a value to return.
     * </remarks>
     */
    class property {
    public:

        /**
         * <summary>
         * Constructs a property.
         * </summary>
         * <remarks>
         *   This should be instantiated by client code -- let the parser
         *   object do this for you with its `properties` method.
         * </remarks>
         */
        property(const std::wstring &name, _TDH_IN_TYPE type);

        /**
         * <summary>
         * Retrieves the name of the property.
         * </summary>
         */
        const std::wstring &name() const;

        /**
         * <summary>
         * Retrieves the Tdh type of the property.
         * </summary>
         */
        _TDH_IN_TYPE type() const;

    private:
        std::wstring name_;
        _TDH_IN_TYPE type_;
    };


    /**
     * <summary>
     * Iterates the properties in a given event record.
     * </summary>
     */
    class property_iterator {
    public:

        /**
         * <summary>
         *   Constructs a new iterator that lazily retrieves the properties of
         *   the given event record.
         * </summary>
         * <remarks>
         *   Don't construct this yourself. Let the `parser` class do it for you.
         * </remarks>
         */
        property_iterator(const schema &s);

        /**
         * <summary>
         * Returns an iterator that hasn't yielded any properties yet.
         * </summary>
         */
        std::vector<property>::iterator begin();

        /**
         * <summary>
         * Returns an iterator that has yielded all properties.
         * </summary>
         */
        std::vector<property>::iterator end();

    private:

        /**
         * <summary>
         *   Constructs a property instance out of the raw data of the
         *   given property.
         * </summary>
         */
        property get_property(size_t index) const;

        /**
         * <summary>
         * Collects the names of the properties in the schema.
         * </summary>
         * <remarks>
         *   This is a little lazy of us, as we end up iterating the properties
         *   entirely before allowing enumeration by the client. Because this
         *   code is most likely called in a non-critical path, there's not
         *   much to worry about here.
         */
         std::vector<property> enum_properties() const;


    private:
        const krabs::schema &schema_;
        size_t numProperties_;
        std::vector<property> properties_;
        std::vector<property>::iterator beg_;
        std::vector<property>::iterator end_;
        std::vector<property>::iterator curr_;
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline property::property(const std::wstring &name, _TDH_IN_TYPE type)
    : name_(name)
    , type_(type)
    {}

    inline const std::wstring &property::name() const
    {
        return name_;
    }

    inline _TDH_IN_TYPE property::type() const
    {
        return type_;
    }

    // ------------------------------------------------------------------------

    inline property_iterator::property_iterator(const schema &s)
    : schema_(s)
    , numProperties_(s.pSchema_->TopLevelPropertyCount)
    , properties_(enum_properties())
    , beg_(properties_.begin())
    , end_(properties_.end())
    , curr_(properties_.begin())
    {}

    inline std::vector<property>::iterator property_iterator::begin()
    {
        return beg_;
    }

    inline std::vector<property>::iterator property_iterator::end()
    {
        return end_;
    }

    inline property property_iterator::get_property(size_t index) const
    {
        const auto &curr_prop = schema_.pSchema_->EventPropertyInfoArray[index];

        const wchar_t *pName = reinterpret_cast<const wchar_t*>(
            reinterpret_cast<BYTE*>(schema_.pSchema_) +
            curr_prop.NameOffset);

        auto tdh_type = (_TDH_IN_TYPE)curr_prop.nonStructType.InType;

        return property(pName, tdh_type);
    }

    inline std::vector<property> property_iterator::enum_properties() const
    {
        std::vector<property> props;
        for (size_t i = 0; i < numProperties_; ++i) {
            props.emplace_back(get_property(i));
        }

        return props;
    }


} /* namespace krabs */
