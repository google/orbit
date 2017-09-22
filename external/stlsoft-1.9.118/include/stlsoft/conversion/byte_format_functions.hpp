/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/byte_format_functions.hpp
 *
 * Purpose:     Byte formatting functions.
 *
 * Created:     23rd July 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/conversion/byte_format_functions.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::byte_format() function(s)
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS_MAJOR     1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS_MINOR     0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS_REVISION  8
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS_EDIT      15
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
# include <stlsoft/conversion/sap_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>                // for strlen()
# endif /* !STLSOFT_INCL_H_STRING */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
# ifndef STLSOFT_INCL_H_MEMORY
#  include <memory.h>
# endif /* !STLSOFT_INCL_H_MEMORY */
#endif /* compiler */

//#define STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF

#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
# ifndef STLSOFT_INCL_H_STDIO
#  define STLSOFT_INCL_H_STDIO
#  include <stdio.h>
# endif /* !STLSOFT_INCL_H_STDIO */
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace conversion
{
namespace format
{
namespace impl
{
#endif /* _STLSOFT_NO_NAMESPACE */

inline char const* format_hex_chars(bool requestUppercaseAlpha)
{
    static const char   s_lower[] = "0123456789abcdef";
    static const char   s_upper[] = "0123456789ABCDEF";

    return requestUppercaseAlpha ? s_upper : s_lower;
}

inline void format_hex_uint8(char buff[2], ss_byte_t const* py, bool requestUppercaseAlpha)
{
    STLSOFT_ASSERT(NULL != py);

    const char*         bytes = format_hex_chars(requestUppercaseAlpha);
    const ss_uint8_t    byte0 = py[0];

    buff[1] = bytes[(byte0 >> 0) & 0x0f];
    buff[0] = bytes[(byte0 >> 4) & 0x0f];
}

inline void format_hex_uint16(char buff[4], ss_byte_t const* py, bool requestUppercaseAlpha, bool highByteFirst)
{
    const char*         bytes = format_hex_chars(requestUppercaseAlpha);
    const ss_uint8_t    byte0 = py[highByteFirst ? 1 : 0];
    const ss_uint8_t    byte1 = py[highByteFirst ? 0 : 1];

    buff[1] = bytes[(byte1 >> 0) & 0x0f];
    buff[0] = bytes[(byte1 >> 4) & 0x0f];

    buff[3] = bytes[(byte0 >> 0) & 0x0f];
    buff[2] = bytes[(byte0 >> 4) & 0x0f];
}

inline void format_hex_uint32(char buff[8], ss_byte_t const* py, bool requestUppercaseAlpha, bool highByteFirst)
{
    const char* bytes = format_hex_chars(requestUppercaseAlpha);
    const ss_uint8_t    byte0 = py[highByteFirst ? 3 : 0];
    const ss_uint8_t    byte1 = py[highByteFirst ? 2 : 1];
    const ss_uint8_t    byte2 = py[highByteFirst ? 1 : 2];
    const ss_uint8_t    byte3 = py[highByteFirst ? 0 : 3];

    buff[1] = bytes[(byte3 >> 0) & 0x0f];
    buff[0] = bytes[(byte3 >> 4) & 0x0f];

    buff[3] = bytes[(byte2 >> 0) & 0x0f];
    buff[2] = bytes[(byte2 >> 4) & 0x0f];

    buff[5] = bytes[(byte1 >> 0) & 0x0f];
    buff[4] = bytes[(byte1 >> 4) & 0x0f];

    buff[7] = bytes[(byte0 >> 0) & 0x0f];
    buff[6] = bytes[(byte0 >> 4) & 0x0f];
}

inline void format_hex_uint64(char buff[16], ss_byte_t const* py, bool requestUppercaseAlpha, bool highByteFirst)
{
#if 0
    format_hex_uint32(buff + (highByteFirst ? 8 : 0), py + (highByteFirst ? 0 : 4), requestUppercaseAlpha, highByteFirst);
    format_hex_uint32(buff + (highByteFirst ? 0 : 8), py + (highByteFirst ? 4 : 0), requestUppercaseAlpha, highByteFirst);
#else /* ? 0 */
    const char* bytes = format_hex_chars(requestUppercaseAlpha);
    const ss_uint8_t    byte0 = py[highByteFirst ? 7 : 0];
    const ss_uint8_t    byte1 = py[highByteFirst ? 6 : 1];
    const ss_uint8_t    byte2 = py[highByteFirst ? 5 : 2];
    const ss_uint8_t    byte3 = py[highByteFirst ? 4 : 3];
    const ss_uint8_t    byte4 = py[highByteFirst ? 3 : 4];
    const ss_uint8_t    byte5 = py[highByteFirst ? 2 : 5];
    const ss_uint8_t    byte6 = py[highByteFirst ? 1 : 6];
    const ss_uint8_t    byte7 = py[highByteFirst ? 0 : 7];

    buff[1] = bytes[(byte7 >> 0) & 0x0f];
    buff[0] = bytes[(byte7 >> 4) & 0x0f];

    buff[3] = bytes[(byte6 >> 0) & 0x0f];
    buff[2] = bytes[(byte6 >> 4) & 0x0f];

    buff[5] = bytes[(byte5 >> 0) & 0x0f];
    buff[4] = bytes[(byte5 >> 4) & 0x0f];

    buff[7] = bytes[(byte4 >> 0) & 0x0f];
    buff[6] = bytes[(byte4 >> 4) & 0x0f];

    buff[9] = bytes[(byte3 >> 0) & 0x0f];
    buff[8] = bytes[(byte3 >> 4) & 0x0f];

    buff[11] = bytes[(byte2 >> 0) & 0x0f];
    buff[10] = bytes[(byte2 >> 4) & 0x0f];

    buff[13] = bytes[(byte1 >> 0) & 0x0f];
    buff[12] = bytes[(byte1 >> 4) & 0x0f];

    buff[15] = bytes[(byte0 >> 0) & 0x0f];
    buff[14] = bytes[(byte0 >> 4) & 0x0f];
#endif /* 0 */
}

inline void format_hex_uint128(char buff[16], ss_byte_t const* py, bool requestUppercaseAlpha, bool highByteFirst)
{
    format_hex_uint64(buff + (highByteFirst ? 16 : 0), py + (highByteFirst ? 0 : 8), requestUppercaseAlpha, highByteFirst);
    format_hex_uint64(buff + (highByteFirst ? 0 : 16), py + (highByteFirst ? 8 : 0), requestUppercaseAlpha, highByteFirst);
}

inline void format_hex_uint256(char buff[16], ss_byte_t const* py, bool requestUppercaseAlpha, bool highByteFirst)
{
    format_hex_uint128(buff + (highByteFirst ? 32 : 0), py + (highByteFirst ? 0 : 16), requestUppercaseAlpha, highByteFirst);
    format_hex_uint128(buff + (highByteFirst ? 0 : 32), py + (highByteFirst ? 16 : 0), requestUppercaseAlpha, highByteFirst);
}


#ifndef _STLSOFT_NO_NAMESPACE
} // namespace impl
} // namespace conversion
} // namespace format
#endif /* _STLSOFT_NO_NAMESPACE */

/** \brief Formats the contents of a contiguous block of memory into
 *    hexadecimal text, optionally aligning into groups and/or lines.
 *
 * \ingroup group__library__conversion
 *
 * \param pv Pointer to the block
 * \param cb Number of bytes in the block
 * \param buff Pointer to the destination character buffer to receive the formatted contents
 * \param cchBuff Number of character spaces available in the buffer
 * \param byteGrouping Number of bytes in a group. Must be 0, 1, 2, 4, 6, 16 or 32. If 0, is reevaluated to sizeof(int)
 * \param groupSeparator Group separator. If NULL, defaults to ""
 * \param groupsPerLine Number of groups per line
 * \param lineSeparator Line separator. If NULL, no line separation is done
 *
 * \return If sufficient space was available, then the number of characters
 *   written to the buffer. Otherwise, returns a size that is guaranteed to
 *   be large enough to write the result.
 */
inline ss_size_t format_bytes(  void const* pv
                            ,   ss_size_t   cb
                            ,   char*       buff
                            ,   ss_size_t   cchBuff
                            ,   ss_size_t   byteGrouping
                            ,   char const* groupSeparator
                            ,   int         groupsPerLine   =   -1
                            ,   char const* lineSeparator   =   "\n") stlsoft_throw_0()
/*
                            ,   int         flags           =   format::lowercase | format::highByteFirst */
{
    STLSOFT_ASSERT( 0 == byteGrouping
                ||  1 == byteGrouping
                ||  2 == byteGrouping
                ||  4 == byteGrouping
                ||  8 == byteGrouping
                ||  16 == byteGrouping
                ||  32 == byteGrouping);

#ifdef _DEBUG
    ::memset(buff, '~', cchBuff);
#endif /* _DEBUG */

    if(0 == cb)
    {
        return 0;
    }
    else
    {
        if(0 == byteGrouping)
        {
            byteGrouping = sizeof(int);
        }

        const ss_size_t cchSeparator        =   (NULL == groupSeparator) ? (groupSeparator = "", 0) : ::strlen(groupSeparator);
        const ss_size_t cchLineSeparator    =   (NULL == lineSeparator) ? 0 : ::strlen(lineSeparator);
        const ss_size_t numGroups           =   (cb + (byteGrouping - 1)) / byteGrouping;
        const ss_size_t numLines            =   (groupsPerLine < 1) ? 1 : (numGroups + (groupsPerLine - 1)) / groupsPerLine;
        const ss_size_t numLineSeparators   =   numLines - 1;
        ss_size_t       size                =   (numGroups * (cchSeparator + (2 * byteGrouping))) + (numLineSeparators * cchLineSeparator) - (numLines * cchSeparator);

        if(size <= cchBuff)
        {
            byte_t const*   py =   static_cast<byte_t const*>(pv);
            ss_size_t       lineIndex;
            ss_size_t       groupIndex;

            for(lineIndex = 0, groupIndex = 0; 0 != cb; py += byteGrouping)
            {
                byte_t  remaining[32];
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                int     cch;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */

                if(cb < byteGrouping)
                {
                    ::memcpy(&remaining[0], py, cb);
                    ::memset(&remaining[0] + cb, 0x00, STLSOFT_NUM_ELEMENTS(remaining) - cb);

                    py = &remaining[0];
                    cb = byteGrouping;  // Cause iteration to complete cleanly after this round
                }

#if defined(STLSOFT_COMPILER_IS_GCC)
                typedef unsigned        int8x_t;
#else /* ? compiler */
                typedef uint32_t        int8x_t;
#endif /* compiler */

#ifndef _STLSOFT_NO_NAMESPACE
                using ::stlsoft::conversion::format::impl::format_hex_uint8;
                using ::stlsoft::conversion::format::impl::format_hex_uint16;
                using ::stlsoft::conversion::format::impl::format_hex_uint32;
                using ::stlsoft::conversion::format::impl::format_hex_uint64;
                using ::stlsoft::conversion::format::impl::format_hex_uint128;
                using ::stlsoft::conversion::format::impl::format_hex_uint256;
#endif /* _STLSOFT_NO_NAMESPACE */

                const bool  requestUppercaseAlpha = false;
                const bool  highByteFirst = false;

                switch(byteGrouping)
                {
                    default:
                        STLSOFT_MESSAGE_ASSERT(0, "invalid byte grouping");
                        cb = 0;
                        break;
                    case    1:
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                        cch     =   ::sprintf(  buff, /* (0 == uppercaseHex) ? */ "%02x" /* : "%04X" */
                                            ,   *sap_cast<uint8_t const*>(py));
                        buff    +=  cch;
#else /* ? STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        format_hex_uint8(buff, py, requestUppercaseAlpha);
                        buff    +=  2;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        cb      -=  1;
                        break;
                    case    2:
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                        cch     =   ::sprintf(  buff, /* (0 == uppercaseHex) ? */ "%04x" /* : "%04X" */
                                            ,   *sap_cast<uint16_t const*>(py));
                        buff    +=  cch;
#else /* ? STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        format_hex_uint16(buff, py, requestUppercaseAlpha, highByteFirst);
                        buff    +=  4;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        cb      -=  2;
                        break;
                    case    4:
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                        cch     =   ::sprintf(  buff, /* (0 == uppercaseHex) ? */ "%08x" /* : "%08X" */
                                            ,   *sap_cast<int8x_t const*>(py));
                        buff    +=  cch;
#else /* ? STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        format_hex_uint32(buff, py, requestUppercaseAlpha, highByteFirst);
                        buff    +=  8;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        cb      -=  4;
                        break;
                    case    8:
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                        cch     =   ::sprintf(  buff, /* (0 == uppercaseHex) ? */ "%08x%08x" /* : "%08X%08X" */
                                            ,   *(sap_cast<int8x_t const*>(py) + 1)
                                            ,   *sap_cast<int8x_t const*>(py));
                        buff    +=  cch;
#else /* ? STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        format_hex_uint64(buff, py, requestUppercaseAlpha, highByteFirst);
                        buff    +=  16;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        cb      -=  8;
                        break;
                    case    16:
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                        cch     =   ::sprintf(  buff, /* (0 == uppercaseHex) ? */ "%08x%08x%08x%08x" /* : "%08X%08X%08X%08X" */
                                            ,   *(sap_cast<int8x_t const*>(py) + 3)
                                            ,   *(sap_cast<int8x_t const*>(py) + 2)
                                            ,   *(sap_cast<int8x_t const*>(py) + 1)
                                            ,   *sap_cast<int8x_t const*>(py));
                        buff    +=  cch;
#else /* ? STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        format_hex_uint128(buff, py, requestUppercaseAlpha, highByteFirst);
                        buff    +=  32;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        cb      -=  16;
                        break;
                    case    32:
#ifdef STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF
                        cch     =   ::sprintf(  buff, /* (0 == uppercaseHex) ? */ "%08x%08x%08x%08x%08x%08x%08x%08x" /* : "%08X%08X%08X%08X%08X%08X%08X%08X" */
                                            ,   *(sap_cast<int8x_t const*>(py) + 7)
                                            ,   *(sap_cast<int8x_t const*>(py) + 6)
                                            ,   *(sap_cast<int8x_t const*>(py) + 5)
                                            ,   *(sap_cast<int8x_t const*>(py) + 4)
                                            ,   *(sap_cast<int8x_t const*>(py) + 3)
                                            ,   *(sap_cast<int8x_t const*>(py) + 2)
                                            ,   *(sap_cast<int8x_t const*>(py) + 1)
                                            ,   *sap_cast<int8x_t const*>(py));
                        buff    +=  cch;
#else /* ? STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        format_hex_uint256(buff, py, requestUppercaseAlpha, highByteFirst);
                        buff    +=  64;
#endif /* STLSOFT_CONVERSION_BYTE_FORMAT_FUNCTIONS_USE_SPRINTF */
                        cb      -=  32;
                        break;
                }

                if(static_cast<ss_size_t>(groupsPerLine) == ++groupIndex)
                {
                    if(++lineIndex < numLines)
                    {
                        ::memcpy(buff, lineSeparator, cchLineSeparator * sizeof(char));
                        buff += cchLineSeparator;
                    }
                    groupIndex = 0;
                }
                else if(0 != cb)
                {
                    ::memcpy(buff, groupSeparator, cchSeparator * sizeof(char));
                    buff += cchSeparator;
                }
            }

            if(size < cchBuff)
            {
                0[buff] = '\0';
            }
        }

        return size;
    }
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/byte_format_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_BYTE_FORMAT_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
