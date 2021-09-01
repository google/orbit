// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <objbase.h>

#include <new>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "compiler_check.hpp"

namespace krabs {

    /** <summary>
     * Represents a GUID, allowing simplified construction from a string or
     * Windows GUID structure.
     * </summary>
     */
    class guid {
    public:
        guid(GUID guid);
        guid(const std::wstring &guid);

        bool operator==(const guid &rhs) const;
        bool operator==(const GUID &rhs) const;

        bool operator<(const guid &rhs) const;
        bool operator<(const GUID &rhs) const;

        operator GUID() const;
        operator const GUID*() const;

        /** <summary>
          * Constructs a new random guid.
          * </summary>
          */
        static inline guid random_guid();

    private:
        GUID guid_;

        friend struct std::hash<guid>;
    };

    /** <summary>
      * Helper functions for parsing GUID's.
      * </summary>
      */
    class guid_parser {
        // Implementing in Krabs instead of Lobsters so that we can test it in unmanaged code.
    private:
        // Number of characters in the UUID's 8-4-4-4-12 string format.
        static const size_t UUID_STRING_LENGTH = 36;

        static const unsigned char DELIMITER = '-';

        // Expected character positions of runs of hex digits in 8-4-4-4-12 format, e.g.
        // 00000000-0000-0000-0000-000000000000
        // Names correspond to struct members of GUID.
        static const size_t STR_POSITION_DATA1 = 0;
        static const size_t STR_POSITION_DATA2 = 8 + 1;
        static const size_t STR_POSITION_DATA3 = STR_POSITION_DATA2 + 4 + 1;
        static const size_t STR_POSITION_DATA4_PART1 = STR_POSITION_DATA3 + 4 + 1;
        static const size_t STR_POSITION_DATA4_PART2 = STR_POSITION_DATA4_PART1 + 4 + 1;

    public:
        // str_input must have at least 2 valid chars in allocated buffer
        static bool hex_octet_to_byte(const char* str_input, unsigned char& byte_output);
        // str_input must have at least 2*sizeof(T) valid chars in allocated buffer
        template<typename T>
        static bool hex_string_to_number(const char* str_input, T& int_output);
        // str_input must have at least 2*byte_count valid chars in allocated buffer,
        // and byte_output must have at least byte_count bytes in allocated buffer
        static bool hex_string_to_bytes(const char* str_input, unsigned char* byte_output, size_t byte_count);
        
        /** <summary>
          * Parses GUID of "D" format. For example, the nil GUID would be "00000000-0000-0000-0000-000000000000".
          * See: https://docs.microsoft.com/en-us/dotnet/api/system.guid.tostring?view=netframework-4.8
          *
          * (str) must have at least (length) valid characters for memory safety. A null terminator is not
          * required. Instead, (length) is used for the bounds check.
          *
          * Returns the parsed GUID. Throws a std::runtime_error if there is a bounds error or format error.
          *
          * This function is for performance, to help deal with container ID extended data, which has no null
          * terminator, which would force us to clone the data to append a null terminator in order to use
          * existing GUID parsing functions.
          * </summary>
          */
        static GUID parse_guid(const char* str, unsigned int length);
    };

    // Implementation
    // ------------------------------------------------------------------------

    inline guid::guid(GUID guid)
        : guid_(guid)
    {}

    inline guid::guid(const std::wstring &guid)
    {
        HRESULT hr = CLSIDFromString(guid.c_str(), &guid_);
        if (FAILED(hr)) {
#pragma warning(push)
#pragma warning(disable: 4244) // narrowing guid wchar_t to char for this error message
            std::string guidStr(guid.begin(), guid.end());
#pragma warning(pop)
            std::stringstream stream;
            stream << "Error in constructing guid from string (";
            stream << guidStr;
            stream << "), hr = 0x";
            stream << std::hex << hr;
            throw std::runtime_error(stream.str());
        }
    }

    inline bool guid::operator==(const guid &rhs) const
    {
        return (0 == memcmp(&guid_, &rhs.guid_, sizeof(GUID)));
    }

    inline bool guid::operator==(const GUID &rhs) const
    {
        return (0 == memcmp(&guid_, &rhs, sizeof(GUID)));
    }

    inline bool guid::operator<(const guid &rhs) const
    {
        return (memcmp(&guid_, &rhs.guid_, sizeof(guid_)) < 0);
    }

    inline bool guid::operator<(const GUID &rhs) const
    {
        return (memcmp(&guid_, &rhs, sizeof(guid_)) < 0);
    }

    inline guid::operator GUID() const
    {
        return guid_;
    }

    inline guid::operator const GUID*() const
    {
        return &guid_;
    }

    inline guid guid::random_guid()
    {
        GUID tmpGuid;
        CoCreateGuid(&tmpGuid);
        return guid(tmpGuid);
    }

    struct CoTaskMemDeleter {
        void operator()(wchar_t *mem) {
            CoTaskMemFree(mem);
        }
    };

    inline bool guid_parser::hex_octet_to_byte(const char* str_input, unsigned char& byte_output)
    {
        // Accepts chars '0' through '9' (0x30 to 0x39),
        // 'A' through 'F' (0x41 to 0x46)
        // 'a' through 'f' (0x61 to 0x66)

        // Narrow the value later, for safety checking.
        auto value = 0;
        // most significant digit in the octet
        auto msd = str_input[0];
        // least significant digit in the octet
        auto lsd = str_input[1];

        if (msd >= '0' && msd <= '9')
        {
            value |= ((int)msd & 0x0F) << 4;
        }
        else if ((msd >= 'A' && msd <= 'F') || (msd >= 'a' && msd <= 'f'))
        {
            value |= (((int)msd & 0x0F) + 9) << 4;
        }
        else
        {
            return false;
        }

        if (lsd >= '0' && lsd <= '9')
        {
            value |= ((int)lsd & 0x0F);
        }
        else if ((lsd >= 'A' && lsd <= 'F') || (lsd >= 'a' && lsd <= 'f'))
        {
            value |= (((int)lsd & 0x0F) + 9);
        }
        else
        {
            return false;
        }

        assert(value >= 0 && value <= UCHAR_MAX);
        byte_output = static_cast<unsigned char>(value);
        return true;
    }

    template<typename T>
    bool guid_parser::hex_string_to_number(const char* str_input, T& int_output)
    {
        auto byte_count = sizeof(T);
        T value = 0;
        unsigned char byte = 0;

        for (size_t i = 0; i < byte_count; i++)
        {
            if (!guid_parser::hex_octet_to_byte(str_input + i * 2, byte))
            {
                return false;
            }

            value = (value << 8) | static_cast<T>(byte);
        }

        int_output = value;
        return true;
    }

    inline bool guid_parser::hex_string_to_bytes(const char* str_input, unsigned char* byte_output, size_t byte_count)
    {
        for (size_t i = 0; i < byte_count; i++)
        {
            if (!hex_octet_to_byte(str_input + (i * 2), byte_output[i]))
            {
                return false;
            }
        }

        return true;
    }

    /** <summary>
      * Parses GUID of "D" format. For example, the nil GUID would be "00000000-0000-0000-0000-000000000000".
      * See: https://docs.microsoft.com/en-us/dotnet/api/system.guid.tostring?view=netframework-4.8
      *
      * (str) must have at least (length) valid characters for memory safety. A null terminator is not
      * required. Instead, (length) is used for the bounds check.
      *
      * Returns the parsed GUID. Throws a std::runtime_error if there is a bounds error or format error.
      * 
      * This function is for performance, to help deal with container ID extended data, which has no null 
      * terminator, which would force us to clone the data to append a null terminator in order to use 
      * existing GUID parsing functions.
      * </summary>
      */
    inline GUID guid_parser::parse_guid(const char* str, unsigned int length)
    {
        if (length != UUID_STRING_LENGTH)
        {
            std::stringstream message;
            message << "Input data has incorrect length. Expected "
                << UUID_STRING_LENGTH
                << ", got "
                << length;
            throw std::runtime_error(message.str());
        }

        GUID guid = { 0 };

        // Check that hyphens are in expected places as a formatting issue.
        if (str[STR_POSITION_DATA2 - 1] != DELIMITER ||
            str[STR_POSITION_DATA3 - 1] != DELIMITER ||
            str[STR_POSITION_DATA4_PART1 - 1] != DELIMITER ||
            str[STR_POSITION_DATA4_PART2 - 1] != DELIMITER)
        {
            throw std::runtime_error("Missing a hyphen where one was expected.");
        }

        // Use from_hex_string for Data1, Data2, and Data3 because of endianness of the data
        // Use hex_string_to_bytes for Data4's array elements because it's byte by byte instead
        auto success = guid_parser::hex_string_to_number(str + STR_POSITION_DATA1, guid.Data1)
            && guid_parser::hex_string_to_number(str + STR_POSITION_DATA2, guid.Data2)
            && guid_parser::hex_string_to_number(str + STR_POSITION_DATA3, guid.Data3)
            && guid_parser::hex_string_to_bytes(str + STR_POSITION_DATA4_PART1, reinterpret_cast<unsigned char*>(&guid.Data4[0]), 2)
            && guid_parser::hex_string_to_bytes(str + STR_POSITION_DATA4_PART2, reinterpret_cast<unsigned char*>(&guid.Data4[2]), 6);

        if (!success)
        {
            throw std::runtime_error("GUID string contains non-hex digits where hex digits are expected.");
        }

        return guid;
    }
}

namespace std
{
    inline std::wstring to_wstring(const krabs::guid& guid)
    {
        wchar_t* guidString;
        HRESULT hr = StringFromCLSID(guid, &guidString);

        if (FAILED(hr)) throw std::bad_alloc();

        std::unique_ptr<wchar_t, krabs::CoTaskMemDeleter> managed(guidString);

        return { managed.get() };
    }

    template<>
    struct std::hash<krabs::guid>
    {
        size_t operator()(const krabs::guid& guid) const
        {
            // This algorithm comes from .NET's reference source for Guid.GetHashCode()
            return guid.guid_.Data1 ^
                 ((guid.guid_.Data2    << 16) | guid.guid_.Data3   ) ^
                 ((guid.guid_.Data4[2] << 24) | guid.guid_.Data4[7]);
        }
    };
}
