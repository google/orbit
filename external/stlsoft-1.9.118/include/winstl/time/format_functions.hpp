/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/time/format_functions.hpp
 *
 * Purpose:     Comparison functions for Windows time structures.
 *
 * Created:     21st November 2003
 * Updated:     7th August 2012
 *
 * Thanks to:   Mikael Pahmp, for spotting the failure to handle 24-hour
 *              time pictures.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2012, Matthew Wilson and Synesis Software
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


/** \file winstl/time/format_functions.hpp
 *
 * \brief [C, C++] Formatting functions for Windows time types
 *   (\ref group__library__time "Time" Library).
 */

#ifndef WINSTL_INCL_WINSTL_TIME_HPP_FORMAT_FUNCTIONS
#define WINSTL_INCL_WINSTL_TIME_HPP_FORMAT_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_TIME_HPP_FORMAT_FUNCTIONS_MAJOR      5
# define WINSTL_VER_WINSTL_TIME_HPP_FORMAT_FUNCTIONS_MINOR      1
# define WINSTL_VER_WINSTL_TIME_HPP_FORMAT_FUNCTIONS_REVISION   2
# define WINSTL_VER_WINSTL_TIME_HPP_FORMAT_FUNCTIONS_EDIT       62
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_HPP_MEMORY_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_HPP_MEMORY_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_INTEGER_TO_STRING
# include <stlsoft/conversion/integer_to_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_INTEGER_TO_STRING */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_FUNCTIONS
# include <winstl/registry/functions.hpp>   // for reg_get_string_value()
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_FUNCTIONS */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helper classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
struct time_format_functions_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct time_format_functions_traits<ws_char_a_t>
{
    typedef ws_char_a_t char_type;

    static int GetTimeFormat(LCID Locale, DWORD dwFlags, SYSTEMTIME const* lpTime, char_type const* lpFormat, char_type* lpTimeStr, int cchTime)
    {
        return ::GetTimeFormatA(Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
    }
    static int GetLocaleInfo(LCID Locale, LCTYPE LCType, char_type* lpLCData, int cchData)
    {
        return ::GetLocaleInfoA(Locale, LCType, lpLCData, cchData);
    }
    static ss_size_t lstrlen(char_type const* s)
    {
        return static_cast<ss_size_t>(::lstrlenA(s));
    }
    static char_type* lstrcpy(char_type* dest, char_type const* src)
    {
        return ::lstrcpyA(dest, src);
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct time_format_functions_traits<ws_char_w_t>
{
    typedef ws_char_w_t char_type;

    static int GetTimeFormat(LCID Locale, DWORD dwFlags, SYSTEMTIME const* lpTime, char_type const* lpFormat, char_type* lpTimeStr, int cchTime)
    {
        return ::GetTimeFormatW(Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
    }
    static int GetLocaleInfo(LCID Locale, LCTYPE LCType, char_type* lpLCData, int cchData)
    {
        return ::GetLocaleInfoW(Locale, LCType, lpLCData, cchData);
    }
    static ss_size_t lstrlen(char_type const* s)
    {
        return static_cast<ss_size_t>(::lstrlenW(s));
    }
    static char_type* lstrcpy(char_type* dest, char_type const* src)
    {
        return ::lstrcpyW(dest, src);
    }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

template <ss_typename_param_k C>
inline
int
STLSOFT_STDCALL GetTimeFormat_ms_(
    LCID                locale      // locale
,   DWORD               dwFlags     // options
,   CONST SYSTEMTIME*   lpTime      // time
,   C const*            lpFormat    // time format string
,   C const* const*     timeMarkers // pointer to array of two pointers to time markers
,   C*                  lpTimeStr   // formatted string buffer
,   int const           cchTime     // size of string buffer
)
{
    typedef C                                       char_t;
    typedef time_format_functions_traits<char_t>    traits_t;
    typedef stlsoft_ns_qual(auto_buffer_old)<
        char_t
    ,   processheap_allocator<char_t>
    >                                               buffer_t;

    if(dwFlags & (TIME_NOMINUTESORSECONDS | TIME_NOSECONDS))
    {
        return traits_t::GetTimeFormat(locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
    }

    if(dwFlags & LOCALE_NOUSEROVERRIDE)
    {
        locale = LOCALE_SYSTEM_DEFAULT;
    }

    buffer_t            timePicture(1 + ((NULL == lpFormat) ? static_cast<ss_size_t>(::GetLocaleInfoA(locale, LOCALE_STIMEFORMAT, NULL, 0)) : 0));

    if(NULL == lpFormat)
    {
        ss_size_t   n = static_cast<ss_size_t>(traits_t::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, &timePicture[0], static_cast<int>(timePicture.size())));
        lpFormat = &timePicture[0];
        if(n < timePicture.size())
        {
            timePicture[n] = static_cast<C>('\0');
        }
    }

    ss_size_t const cchPicture  =   1 + traits_t::lstrlen(lpFormat);

    // Following need to be front-padded to be forward compatible with STLSoft 1.10 (which uses
    // 'safer' i2s functions

    char_t          hours12_[]  =   { '\0', '\0', '\0', '0', '0', '\0' };                       // "...00"
    char_t          hours24_[]  =   { '\0', '\0', '\0', '0', '0', '\0' };                       // "...00"
    char_t          minutes_[]  =   { '\0', '\0', '\0', '0', '0', '\0' };                       // "...00"
    char_t          seconds_[]  =   { '\0', '\0', '\0', '0', '0', '.', '0', '0', '0', '\0' };   // "...00.000"

    uint16_t const  hour12      =   (lpTime->wHour > 12) ? uint16_t(lpTime->wHour - 12) : lpTime->wHour;

#if defined(STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT)
    char_t const*   hours12    =   stlsoft_ns_qual(integer_to_string)(hours12_, hour12);
    char_t const*   hours24    =   stlsoft_ns_qual(integer_to_string)(hours24_, lpTime->wHour);
    char_t const*   minutes    =   stlsoft_ns_qual(integer_to_string)(minutes_, lpTime->wMinute);
#else /* ? STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */
    char_t const*   hours12    =   stlsoft_ns_qual(integer_to_string)(&hours12_[0], STLSOFT_NUM_ELEMENTS(hours12_), hour12);
    char_t const*   hours24    =   stlsoft_ns_qual(integer_to_string)(&hours24_[0], STLSOFT_NUM_ELEMENTS(hours24_), lpTime->wHour);
    char_t const*   minutes    =   stlsoft_ns_qual(integer_to_string)(&minutes_[0], STLSOFT_NUM_ELEMENTS(minutes_), lpTime->wMinute);
#endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */
                                   stlsoft_ns_qual(integer_to_string)(&seconds_[3], STLSOFT_NUM_ELEMENTS(seconds_) - 3, lpTime->wMilliseconds);
    char_t const*   seconds    =   stlsoft_ns_qual(integer_to_string)(&seconds_[0], 6, lpTime->wSecond);

    seconds_[5] = '.';

    // Get the time markers
    char_t const*   amMarker    =   (NULL != timeMarkers && NULL != timeMarkers[0]) ? timeMarkers[0] : NULL;
    char_t const*   pmMarker    =   (NULL != timeMarkers && NULL != timeMarkers[1]) ? timeMarkers[1] : NULL;
    buffer_t        am(0);
    buffer_t        pm(0);

    if( NULL == amMarker ||
        NULL == pmMarker)
    {
        HKEY    hkey;
        LONG    res =   ::RegOpenKeyA(HKEY_CURRENT_USER, "Control Panel\\International", &hkey);

        if(ERROR_SUCCESS == res)
        {
            static char_t const s1159[] =   { 's', '1', '1', '5', '9', '\0' };
            static char_t const s2359[] =   { 's', '2', '3', '5', '9', '\0' };
            ws_size_t           cchAM   =   0;
            ws_size_t           cchPM   =   0;
            LONG                r;

            if( ERROR_SUCCESS != (r = reg_get_string_value(hkey, s1159, static_cast<char_t*>(NULL), cchAM)) ||
                ERROR_SUCCESS != (r = (am.resize(cchAM), cchAM = am.size(), reg_get_string_value(hkey, s1159, &am[0], cchAM))))
            {
                res = r;
            }
            else if(ERROR_SUCCESS != (r = reg_get_string_value(hkey, s2359, static_cast<char_t*>(NULL), cchPM)) ||
                    ERROR_SUCCESS != (r = (pm.resize(cchPM), cchPM = pm.size(), reg_get_string_value(hkey, s2359, &pm[0], cchPM))))
            {
                res = r;
            }

            ::RegCloseKey(hkey);
        }

        if(ERROR_SUCCESS == res)
        {
            if(NULL == amMarker)
            {
                amMarker = &am[0];
            }
            if(NULL == pmMarker)
            {
                pmMarker = &pm[0];
            }
        }
    }

    if(NULL == amMarker)
    {
        static char_t const AM[]    =   { 'A', 'M', '\0' };

        am.resize(3);

        amMarker = traits_t::lstrcpy(&am[0], AM);
    }
    if(NULL == pmMarker)
    {
        static char_t const PM[]    =   { 'P', 'M', '\0' };

        pm.resize(3);

        pmMarker = traits_t::lstrcpy(&pm[0], PM);
    }

    ws_size_t const cchAmMarker =   traits_t::lstrlen(amMarker);
    ws_size_t const cchPmMarker =   traits_t::lstrlen(pmMarker);
    char_t const*   timeMarker  =   (lpTime->wHour < 12) ? amMarker : pmMarker;
    ws_size_t const cchMarker   =   (cchAmMarker < cchPmMarker) ? cchPmMarker : cchAmMarker;
    ws_size_t const cchTimeMax  =   (cchPicture - 1) + (2 - 1) + (2 - 1) + (6 - 1) + 1 + (1 + cchMarker);
    buffer_t        buffer(1 + cchTimeMax);
    ws_size_t       len         =   0;

    if(!buffer.empty())
    {
        char_t const*   r;
        char_t*         w          =   &buffer[0];
        char_t          prev        =   '\0';
        ws_bool_t       bMarker1    =   true;

        for(r = lpFormat; r != lpFormat + cchPicture; ++r)
        {
            char_t const ch = *r;

            switch(ch)
            {
                case    'h':
                    if( 'h' == prev &&
                        '\0' == *(hours12 + 1))
                    {
                        --hours12;
                    }
                    break;
                case    'H':
                    if( 'H' == prev &&
                        '\0' == *(hours24 + 1))
                    {
                        --hours24;
                    }
                    break;
                case    'm':
                    if( 'm' == prev &&
                        '\0' == *(minutes + 1))
                    {
                        --minutes;
                    }
                    break;
                case    's':
                    if( 's' == prev &&
                        '.' == *(seconds + 1))
                    {
                        --seconds;
                    }
                    break;
                case    't':
                    if('t' == prev)
                    {
                        bMarker1 = false;
                    }
                    break;
                default:
                    {
                        static char_t const s_emptyString[] = { '\0' };
                        char_t const*       p;

                        switch(prev)
                        {
                            case    'h':    p = hours12;        break;
                            case    'H':    p = hours24;        break;
                            case    'm':    p = minutes;        break;
                            case    's':    p = seconds;        break;
                            case    't':
                                if(0 == (dwFlags & TIME_NOTIMEMARKER))
                                {
                                    if(!bMarker1)
                                    {
                                        p = timeMarker;
                                        break;
                                    }
                                    else
                                    {
                                        // Fall through
                                        *w++ = *timeMarker;
                                        ++len;
                                    }
                                }
                                // Fall through
                            default:        p = s_emptyString;   break;
                        }

                        for(; '\0' != *p; *w++ = *p++, ++len)
                        {}
                    }
                    *w++ = ch;
                    ++len;
                    break;
            }

            if('\0' == ch)
            {
                break;
            }

            prev = ch;
        }
    }

    // If 0 was specified, or

    if( 0 == cchTime ||
        len <= ws_size_t(cchTime))
    {
        if(0 != cchTime)
        {
            traits_t::lstrcpy(lpTimeStr, &buffer[0]);
        }

        return static_cast<int>(len);
    }
    else
    {
        return 0;
    }
}

/** \brief Analogue to the Win32 API <code>GetTimeFormat()</code>, but also
 *    provides milliseconds as part of the time picture.
 *
 * \param locale
 * \param dwFlags
 * \param lpTime
 * \param lpFormat
 * \param lpTimeStr The buffer into which the result will be written
 * \param cchTime Number of character spaces available in
 *   <code>lpTimeStr</code>. If 0, the required length is returned, and
 *   <code>lpTimeStr</code> is ignored.
 *
 * \return The number of characters written to <code>lpTimeStr</code>
 *    (if <code>0 != cchTime</code>), or required
 *    (if <code>0 == cchTime</code>).
 */
inline
int
STLSOFT_STDCALL GetTimeFormat_msA(
    LCID                locale      // locale
,   DWORD               dwFlags     // options
,   CONST SYSTEMTIME*   lpTime      // time
,   ws_char_a_t const*  lpFormat    // time format string
,   ws_char_a_t*        lpTimeStr   // formatted string buffer
,   int                 cchTime     // size of string buffer
)
{
    WINSTL_ASSERT(0 == cchTime || NULL != lpTimeStr);

    return GetTimeFormat_ms_<ws_char_a_t>(locale, dwFlags, lpTime, lpFormat, NULL, lpTimeStr, cchTime);
}

inline
int
STLSOFT_STDCALL GetTimeFormat_msW(
    LCID                locale      // locale
,   DWORD               dwFlags     // options
,   CONST SYSTEMTIME*   lpTime      // time
,   ws_char_w_t const*  lpFormat    // time format string
,   ws_char_w_t*        lpTimeStr   // formatted string buffer
,   int                 cchTime     // size of string buffer
)
{
    WINSTL_ASSERT(0 == cchTime || NULL != lpTimeStr);

    return GetTimeFormat_ms_<ws_char_w_t>(locale, dwFlags, lpTime, lpFormat, NULL, lpTimeStr, cchTime);
}

inline
int
STLSOFT_STDCALL GetTimeFormat_msExA(
    LCID                locale      // locale
,   DWORD               dwFlags     // options
,   CONST SYSTEMTIME*   lpTime      // time
,   ws_char_a_t const*  lpFormat    // time format string
,   ws_char_a_t const*  (*timeMarkers)[2]
,   ws_char_a_t*        lpTimeStr   // formatted string buffer
,   int                 cchTime     // size of string buffer
)
{
    WINSTL_ASSERT(0 == cchTime || NULL != lpTimeStr);

    return GetTimeFormat_ms_<ws_char_a_t>(locale, dwFlags, lpTime, lpFormat, *timeMarkers, lpTimeStr, cchTime);
}

inline
int
STLSOFT_STDCALL GetTimeFormat_msExW(
    LCID                locale      // locale
,   DWORD               dwFlags     // options
,   CONST SYSTEMTIME*   lpTime      // time
,   ws_char_w_t const*  lpFormat    // time format string
,   ws_char_w_t const*  (*timeMarkers)[2]
,   ws_char_w_t*        lpTimeStr   // formatted string buffer
,   int                 cchTime     // size of string buffer
)
{
    WINSTL_ASSERT(0 == cchTime || NULL != lpTimeStr);

    return GetTimeFormat_ms_<ws_char_w_t>(locale, dwFlags, lpTime, lpFormat, *timeMarkers, lpTimeStr, cchTime);
}


////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/format_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace winstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_TIME_HPP_FORMAT_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
