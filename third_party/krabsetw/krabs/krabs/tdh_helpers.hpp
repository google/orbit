// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define INITGUID

#include <tdh.h>
#include <string>
#include <stdexcept>

#include "compiler_check.hpp"
#include "parse_types.hpp"

namespace krabs {

#define CASE_TYPE(enum) case TDH_INTYPE_##enum: return #enum

    inline const char* in_type_to_string(_TDH_IN_TYPE type)
    {
        switch (type)
        {
            CASE_TYPE(NULL);
            CASE_TYPE(UNICODESTRING);
            CASE_TYPE(ANSISTRING);
            CASE_TYPE(INT8);
            CASE_TYPE(UINT8);
            CASE_TYPE(INT16);
            CASE_TYPE(UINT16);
            CASE_TYPE(INT32);
            CASE_TYPE(UINT32);
            CASE_TYPE(INT64);
            CASE_TYPE(UINT64);
            CASE_TYPE(FLOAT);
            CASE_TYPE(DOUBLE);
            CASE_TYPE(BOOLEAN);
            CASE_TYPE(BINARY);
            CASE_TYPE(GUID);
            CASE_TYPE(POINTER);
            CASE_TYPE(FILETIME);
            CASE_TYPE(SYSTEMTIME);
            CASE_TYPE(SID);
            CASE_TYPE(HEXINT32);
            CASE_TYPE(HEXINT64);
            CASE_TYPE(COUNTEDSTRING);
            CASE_TYPE(COUNTEDANSISTRING);
            CASE_TYPE(REVERSEDCOUNTEDSTRING);
            CASE_TYPE(REVERSEDCOUNTEDANSISTRING);
            CASE_TYPE(NONNULLTERMINATEDSTRING);
            CASE_TYPE(NONNULLTERMINATEDANSISTRING);
            CASE_TYPE(UNICODECHAR);
            CASE_TYPE(ANSICHAR);
            CASE_TYPE(SIZET);
            CASE_TYPE(HEXDUMP);
            CASE_TYPE(WBEMSID);
            default: return "<INVALID VALUE>";
        }
    }

#undef CASE_TYPE

    namespace debug {

        // this function provides a user-friendly compiler error
        // which shows the type in question in the error message.
        template <typename T>
        inline void missing_assert_specialization_for()
        {
            static_assert(sizeof(T) == 0, __FUNCSIG__);
        }

        // The "catch-all" implementation of assert_valid_assignment just
        // throws in debug to let us know that we are trying to parse a
        // type that does not have any assignment validation. This compiles
        // to a no-op in release.
        template <typename T>
        inline void assert_valid_assignment(const std::wstring&, const property_info&)
        {
#ifndef NDEBUG

            // NOTE: if you want compile time assignment assertion define TYPEASSERT
            // in the preprocessor or undefine it to disable compilation errors

#ifdef TYPEASSERT
            missing_assert_specialization_for<T>();
#endif // TYPEASSERT
#endif // NDEBUG
        }

#ifndef NDEBUG

        // These specializations will be removed in release builds and compilation
        // will fall back to the unspecialized version which is a no-op in release.

        inline void throw_if_invalid(
            const std::wstring& name,
            const property_info& info,
            _TDH_IN_TYPE requested)
        {
            auto actual = (_TDH_IN_TYPE)info.pEventPropertyInfo_->nonStructType.InType;

            if (requested == actual) return;

#pragma warning(push)
#pragma warning(disable: 4244) // narrowing property name wchar_t to char for this error message
            std::string ansiName(name.begin(), name.end());
#pragma warning(pop)

            throw type_mismatch_assert(
                ansiName.c_str(),
                in_type_to_string(actual),
                in_type_to_string(requested));
        }

        // The macro below generates a specialized version of assert_valid_assignment
        // only in debug builds. The specialized overload will be selected instead
        // of the unspecialized version defined above. This allows us to have
        // type-driven assertions only in debug builds.

#define BUILD_ASSERT(type, tdh_type) \
        template <> \
        inline void assert_valid_assignment<type>(               \
            const std::wstring& name, const property_info& info) \
        {                                                        \
            throw_if_invalid(name, info, tdh_type);              \
        }

        // NOTE: don't just blindly add assertions here, some types
        // that seem trivial (e.g. bool) are not because of differences
        // between the representation in C++ and the representation in ETW.
        // Ensure that type sizes match and that the ETW form isn't
        // a variant or variable length. A type that requires a specialized
        // assertion will also require a specialized parser.

        // strings
        BUILD_ASSERT(std::wstring, TDH_INTYPE_UNICODESTRING);
        BUILD_ASSERT(std::string, TDH_INTYPE_ANSISTRING);
        BUILD_ASSERT(const counted_string*, TDH_INTYPE_COUNTEDSTRING);

        // integers
        BUILD_ASSERT(int8_t, TDH_INTYPE_INT8);
        BUILD_ASSERT(uint8_t, TDH_INTYPE_UINT8);
        BUILD_ASSERT(int16_t, TDH_INTYPE_INT16);
        BUILD_ASSERT(uint16_t, TDH_INTYPE_UINT16);
        BUILD_ASSERT(int32_t, TDH_INTYPE_INT32);
        BUILD_ASSERT(uint32_t, TDH_INTYPE_UINT32);
        BUILD_ASSERT(int64_t, TDH_INTYPE_INT64);
        BUILD_ASSERT(uint64_t, TDH_INTYPE_UINT64);

        // floating
        BUILD_ASSERT(float, TDH_INTYPE_FLOAT);
        BUILD_ASSERT(double, TDH_INTYPE_DOUBLE);

        // FILETIME
        BUILD_ASSERT(::FILETIME, TDH_INTYPE_FILETIME);
        BUILD_ASSERT(::SYSTEMTIME, TDH_INTYPE_SYSTEMTIME);

#undef BUILD_ASSERT

        template <>
        inline void assert_valid_assignment<ip_address>(
            const std::wstring&, const property_info& info)
        {
            auto outType = info.pEventPropertyInfo_->nonStructType.OutType;

            if (outType != TDH_OUTTYPE_IPV6 &&
                outType != TDH_OUTTYPE_IPV4) {
                throw std::runtime_error(
                    "Requested an IP address from non-IP address property");
            }
        }

        template <>
        inline void assert_valid_assignment<socket_address>(
            const std::wstring&, const property_info& info)
        {
            auto outType = info.pEventPropertyInfo_->nonStructType.OutType;

            if (outType != TDH_OUTTYPE_SOCKETADDRESS) {
                throw std::runtime_error(
                    "Requested a socket address from property that does not contain a socket address");
            }
        }

        template <>
        inline void assert_valid_assignment<sid>(
            const std::wstring&, const property_info& info)
        {
            auto inType = info.pEventPropertyInfo_->nonStructType.InType;

            if (inType != TDH_INTYPE_WBEMSID && inType != TDH_INTYPE_SID) {
                throw std::runtime_error(
                    "Requested a SID but was neither a SID nor WBEMSID");
            }
        }

        template <>
        inline void assert_valid_assignment<pointer>(
            const std::wstring&, const property_info& info)
        {
            auto inType = info.pEventPropertyInfo_->nonStructType.InType;

            if (inType != TDH_INTYPE_POINTER) {
                throw std::runtime_error(
                    "Requested a POINTER from property that is not one");
            }
        }

        template <>
        inline void assert_valid_assignment<bool>(
            const std::wstring&, const property_info& info)
        {
            auto inType = info.pEventPropertyInfo_->nonStructType.InType;

            if (inType != TDH_INTYPE_BOOLEAN) {
                throw std::runtime_error(
                    "Requested a BOOLEAN from property that is not one");
            }
        }

#endif // NDEBUG

    } /* namespace debug */

} /* namespace krabs */
