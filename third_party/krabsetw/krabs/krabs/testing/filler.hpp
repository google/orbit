// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <cstdint>

#include "../compiler_check.hpp"
#include "../parse_types.hpp"

namespace krabs { namespace testing { namespace details {

    /**
     * <summary>
     *   Defines how much padding to inject into a synth_record when a property
     *   is not filled by calling code.
     * </summary>
     */
    inline size_t how_many_bytes_to_fill(_TDH_IN_TYPE type)
    {
        static_assert(sizeof(float) == 4, "sizeof(float) must be 4, defined on MSDN");

        switch (type) {
        case TDH_INTYPE_NULL:          throw std::runtime_error("supposed to be unusued -- something horrible is happening");
        case TDH_INTYPE_UNICODESTRING: return sizeof(wchar_t);
        case TDH_INTYPE_ANSISTRING:    return sizeof(char);
        case TDH_INTYPE_INT8:          return sizeof(int8_t);
        case TDH_INTYPE_UINT8:         return sizeof(uint8_t);
        case TDH_INTYPE_INT16:         return sizeof(int16_t);
        case TDH_INTYPE_UINT16:        return sizeof(uint16_t);
        case TDH_INTYPE_INT32:         return sizeof(int32_t);
        case TDH_INTYPE_UINT32:        return sizeof(uint32_t);
        case TDH_INTYPE_INT64:         return sizeof(int64_t);
        case TDH_INTYPE_UINT64:        return sizeof(int64_t);
        case TDH_INTYPE_FLOAT:         return sizeof(float);
        case TDH_INTYPE_DOUBLE:        return sizeof(double);
        case TDH_INTYPE_BOOLEAN:       return sizeof(uint32_t); // 4-byte bool, defined on MSDN
        case TDH_INTYPE_BINARY:        return sizeof(char);
        case TDH_INTYPE_GUID:          return sizeof(GUID);
        case TDH_INTYPE_POINTER:       return sizeof(char*);
        case TDH_INTYPE_FILETIME:      return sizeof(FILETIME);
        case TDH_INTYPE_SYSTEMTIME:    return sizeof(SYSTEMTIME);
        case TDH_INTYPE_SID:           return sizeof(PSID);
        case TDH_INTYPE_HEXINT32:      return sizeof(uint32_t);
        case TDH_INTYPE_HEXINT64:      return sizeof(uint64_t);
        default: break;
        };

        throw std::runtime_error("Unexpected fill type");
    }

    /**
     * <summary>
     * Maps C++ types to TDH types. Used to do runtime type checking of packed
     * synthetic properties.
     * </summary>
     */

     template <typename T>
     struct tdh_morphism {
        // This doesn't have a value field, so compilation will fail when we
        // try to use a type in our record_builder that isn't recognized.
     };

     template <typename T>
     struct tdh_morphism<T*> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_POINTER;
     };

     template <>
     struct tdh_morphism<std::wstring> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_UNICODESTRING;
     };

     template <>
     struct tdh_morphism<std::string> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_ANSISTRING;
     };

     template <>
     struct tdh_morphism<char> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_INT8;
     };

     template <>
     struct tdh_morphism<unsigned char> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_UINT8;
     };

     template <>
     struct tdh_morphism<short> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_INT16;
     };

     template <>
     struct tdh_morphism<unsigned short> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_UINT16;
     };

     template <>
     struct tdh_morphism<int> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_INT32;
     };

     template <>
     struct tdh_morphism<unsigned int> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_UINT32;
     };

     template <>
     struct tdh_morphism<long long> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_INT64;
     };

     template <>
     struct tdh_morphism<unsigned long long> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_UINT64;
     };

     template <>
     struct tdh_morphism<float> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_FLOAT;
     };

     template <>
     struct tdh_morphism<double> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_DOUBLE;
     };

     template <>
     struct tdh_morphism<bool> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_BOOLEAN;
     };

     template <>
     struct tdh_morphism<GUID> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_GUID;
     };

     template <>
     struct tdh_morphism<krabs::guid> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_GUID;
     };

     template <>
     struct tdh_morphism<FILETIME> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_FILETIME;
     };

     template <>
     struct tdh_morphism<SYSTEMTIME> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_SYSTEMTIME;
     };

     template <>
     struct tdh_morphism<krabs::hexint32> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_HEXINT32;
     };

     template <>
     struct tdh_morphism<krabs::hexint64> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_HEXINT64;
     };

     template <>
     struct tdh_morphism<SID> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_SID;
     };

     template <>
     struct tdh_morphism<krabs::binary> {
        static const _TDH_IN_TYPE value = TDH_INTYPE_BINARY;
     };


} /* namespace details */ } /* namespace testing */ } /* namespace krabs */
