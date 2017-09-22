/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/access/string/time.hpp
 *
 * Purpose:     Helper functions for the SYSTEMTIME and FILETIME structures.
 *
 * Created:     2nd December 2004
 * Updated:     28th May 2014
 *
 * Thanks to:   David Wang, for spotting an error in one of the shim
 *              functions.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2014, Matthew Wilson and Synesis Software
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


/** \file winstl/shims/access/string/time.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>FILETIME</code> and <code>SYSTEMTIME</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME
#define WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME_MAJOR       2
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME_MINOR       3
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME_REVISION    9
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME_EDIT        57
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR
# include <winstl/error/conversion_error.hpp>
#endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR */
#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_FILETIME
# include <winstl/shims/conversion/to_SYSTEMTIME/FILETIME.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_FILETIME */
#ifndef WINSTL_INCL_WINSTL_TIME_HPP_FORMAT_FUNCTIONS
# include <winstl/time/format_functions.hpp>
#endif /* !WINSTL_INCL_WINSTL_TIME_HPP_FORMAT_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING
# include <stlsoft/shims/access/string/std/c_string.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */
#ifdef WINSTL_UDATE_DEFINED
# include <objbase.h>
# include <oleauto.h>
#endif /* WINSTL_UDATE_DEFINED */

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
 * Helper Classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct winstl_shims_access_string_time_impl
{
    typedef int (STLSOFT_STDCALL *pfnGetDateTimeFmtA_t)(LCID                Locale      // locale
                                                    ,   DWORD               dwFlags     // options
                                                    ,   CONST SYSTEMTIME    *lpTime     // time
                                                    ,   ws_char_a_t const   *lpFormat   // time format string
                                                    ,   ws_char_a_t         *lpTimeStr  // formatted string buffer
                                                    ,   int                 cchTime);   // size of string buffer

    typedef int (STLSOFT_STDCALL *pfnGetDateTimeFmtW_t)(LCID                Locale      // locale
                                                    ,   DWORD               dwFlags     // options
                                                    ,   CONST SYSTEMTIME    *lpTime     // time
                                                    ,   ws_char_w_t const   *lpFormat   // time format string
                                                    ,   ws_char_w_t         *lpTimeStr  // formatted string buffer
                                                    ,   int                 cchTime);   // size of string buffer

    //
    static
    ws_size_t
    calc_sizes(
        SYSTEMTIME const&       t
    ,   pfnGetDateTimeFmtA_t    pfnGetDateFmtA
    ,   pfnGetDateTimeFmtA_t    pfnGetTimeFmtA
    ,   ws_size_t&              cchDate
    ,   ws_size_t&              cchTime
    )
    {
        cchDate = static_cast<ws_size_t>(pfnGetDateFmtA(LOCALE_USER_DEFAULT, 0, &t, NULL, NULL, 0));

        if(0 != cchDate)
        {
            cchTime = static_cast<ws_size_t>(pfnGetTimeFmtA(LOCALE_USER_DEFAULT, 0, &t, NULL, NULL, 0));

            if(0 != cchTime)
            {
                return (cchDate - 1) + 1 + (cchTime - 1);
            }
        }

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        DWORD const e = ::GetLastError();

        // TODO: discriminate on e and on the values of t to determine
        // which condition (of invalid value, out-of-range, etc.)

        STLSOFT_THROW_X(conversion_error("failed to convert date/time", e));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return 0;
    }

    //
    static
    ws_size_t
    calc_sizes(
        SYSTEMTIME const&       t
    ,   pfnGetDateTimeFmtW_t    pfnGetDateFmtW
    ,   pfnGetDateTimeFmtW_t    pfnGetTimeFmtW
    ,   ws_size_t&              cchDate
    ,   ws_size_t&              cchTime
    )
    {
        cchDate = static_cast<ws_size_t>(pfnGetDateFmtW(LOCALE_USER_DEFAULT, 0, &t, NULL, NULL, 0));

        if(0 != cchDate)
        {
            cchTime = static_cast<ws_size_t>(pfnGetTimeFmtW(LOCALE_USER_DEFAULT, 0, &t, NULL, NULL, 0));

            if(0 != cchTime)
            {
                return (cchDate - 1) + 1 + (cchTime - 1);
            }
        }

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        DWORD const e = ::GetLastError();

        // TODO: discriminate on e and on the values of t to determine
        // which condition (of invalid value, out-of-range, etc.)

        STLSOFT_THROW_X(conversion_error("failed to convert date/time", e));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return 0;
    }
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Helper Functions
 */

/** \brief [DEPRECATED] Converts a <code>FILETIME</code> value to a
 *   <code>SYSTEMTIME</code> value.
 *
 * \deprecated Instead use winstl::to_SYSTEMTIME(FILETIME const&)
 *
 * \ingroup group__library__shims__string_access
 *
 * \param ft The <code>FILETIME</code> instance whose value is to be converted
 *
 * \note If <code>ft</code> does not represent a valid time value, the
 *  return value is undefined. The caller may check
 *  <code>GetLastError()</code> to determine whether the conversion has been
 *  successful.
 */
inline
SYSTEMTIME
FILETIME2SYSTEMTIME(
    FILETIME const& ft
)
{
    return winstl_ns_qual(to_SYSTEMTIME)(ft);
}

template <ss_typename_param_k S>
inline
void
stream_insert(S &stm, SYSTEMTIME const& t)
{
    typedef stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>     string_t;
    typedef winstl_shims_access_string_time_impl                impl_t;

    ws_size_t       cchDate     =   0;
    ws_size_t       cchTime     =   0;
    const ws_size_t cchTotal    =   impl_t::calc_sizes(t, ::GetDateFormatA, ::GetTimeFormatA, cchDate, cchTime);

#ifdef STLSOFT_CD_EXCEPTION_SUPPORT
    WINSTL_ASSERT(0 != cchTotal);
#else /* ? STLSOFT_CD_EXCEPTION_SUPPORT */
    if(0 != cchTotal)
#endif /* STLSOFT_CD_EXCEPTION_SUPPORT */
    {
        string_t s(cchTotal);

        if(cchTotal == s.size())
        {
            ::GetDateFormatA(LOCALE_USER_DEFAULT, 0, &t, NULL, &s.data()[0], static_cast<int>(cchDate));
            s.data()[cchDate - 1] = ' ';
            ::GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &t, NULL, &s.data()[0] + cchDate, static_cast<int>(cchTime));

            // Because it's possible for external action to change the date
            // and/or time pictures between the call to calc_sizes and the
            // two preceeding calls, we guard against an unterminated
            // C-style string by forcibly appending the nul-terminator that
            // GetTimeFormat() will do for us normally.
            s.data()[cchTotal] = '\0';
        }

        stm << s.data();
    }
}

template <ss_typename_param_k S>
inline
void
stream_insert(S &stm, FILETIME const& ft)
{
    stream_insert(stm, FILETIME2SYSTEMTIME(ft));
}

#ifdef WINSTL_UDATE_DEFINED

template <ss_typename_param_k S>
inline
void
stream_insert(S &stm, UDATE const& ud)
{
    stream_insert(stm, ud.st);
}

#endif /* WINSTL_UDATE_DEFINED */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 */

// SYSTEMTIME

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_a(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    typedef stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>     string_t;
    typedef winstl_shims_access_string_time_impl                impl_t;

    int (STLSOFT_STDCALL *pfnGetTimeFormatA)(   LCID                Locale      // locale
                                            ,   DWORD               dwFlags     // options
                                            ,   CONST SYSTEMTIME    *lpTime     // time
                                            ,   ws_char_a_t const   *lpFormat   // time format string
                                            ,   ws_char_a_t         *lpTimeStr  // formatted string buffer
                                            ,   int                 cchTime)    // size of string buffer

                                            =   bMilliseconds ? GetTimeFormat_msA : ::GetTimeFormatA;

    ws_size_t       cchDate     =   0;
    ws_size_t       cchTime     =   0;
    const ws_size_t cchTotal    =   impl_t::calc_sizes(t, ::GetDateFormatA, pfnGetTimeFormatA, cchDate, cchTime);
    string_t        s(cchTotal);

    if( 0 != cchTotal &&
        cchTotal == s.size())
    {
        ::GetDateFormatA(LOCALE_USER_DEFAULT, 0, &t, NULL, &s.data()[0], static_cast<int>(cchDate));
        s.data()[cchDate - 1] = ' ';
        pfnGetTimeFormatA(LOCALE_USER_DEFAULT, 0, &t, NULL, &s.data()[0] + cchDate, static_cast<int>(cchTime));

        // Because it's possible for external action to change the date
        // and/or time pictures between the call to calc_sizes and the
        // two preceeding calls, we guard against an unterminated
        // C-style string by forcibly appending the nul-terminator that
        // GetTimeFormat() will do for us normally.
        s.data()[cchTotal] = '\0';
    }

    return s;
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_a(
    SYSTEMTIME const& t
)
{
    return c_str_ptr_a(t, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_w(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    typedef stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>     string_t;
    typedef winstl_shims_access_string_time_impl                impl_t;

    int (STLSOFT_STDCALL *pfnGetTimeFormatW)(   LCID                Locale      // locale
                                            ,   DWORD               dwFlags     // options
                                            ,   CONST SYSTEMTIME    *lpTime     // time
                                            ,   ws_char_w_t const   *lpFormat   // time format string
                                            ,   ws_char_w_t         *lpTimeStr  // formatted string buffer
                                            ,   int                 cchTime)    // size of string buffer

                                            =   bMilliseconds ? GetTimeFormat_msW : ::GetTimeFormatW;

    ws_size_t       cchDate     =   0;
    ws_size_t       cchTime     =   0;
    const ws_size_t cchTotal    =   impl_t::calc_sizes(t, ::GetDateFormatW, pfnGetTimeFormatW, cchDate, cchTime);
    string_t        s(cchTotal);

    if( 0 != cchTotal &&
        cchTotal == s.size())
    {
        ::GetDateFormatW(LOCALE_USER_DEFAULT, 0, &t, NULL, &s.data()[0], static_cast<int>(cchDate));
        s.data()[cchDate - 1] = ' ';
        pfnGetTimeFormatW(LOCALE_USER_DEFAULT, 0, &t, NULL, &s.data()[0] + cchDate, static_cast<int>(cchTime));

        // Because it's possible for external action to change the date
        // and/or time pictures between the call to calc_sizes and the
        // two preceeding calls, we guard against an unterminated
        // C-style string by forcibly appending the nul-terminator that
        // GetTimeFormat() will do for us normally.
        s.data()[cchTotal] = '\0';
    }

    return s;
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_w(
    SYSTEMTIME const& t
)
{
    return c_str_ptr_w(t, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
#ifdef UNICODE
    return c_str_ptr_w(t, bMilliseconds);
#else /* ? UNICODE */
    return c_str_ptr_a(t, bMilliseconds);
#endif /* UNICODE */
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr(
    SYSTEMTIME const& t
)
{
    return c_str_ptr(t, ws_false_v);
}


// FILETIME

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_a(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_a(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_a(
    FILETIME const& t
)
{
    return c_str_ptr_a(t, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_w(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_w(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_w(
    FILETIME const& t
)
{
    return c_str_ptr_w(t, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr(
    FILETIME const& t
)
{
    return c_str_ptr(t, ws_false_v);
}


#ifdef WINSTL_UDATE_DEFINED

// UDATE

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_a(
    UDATE const&    ud
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_a(ud.st, bMilliseconds);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_a(
    UDATE const& ud
)
{
    return c_str_ptr_a(ud.st);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_w(
    UDATE const&    ud
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_w(ud.st, bMilliseconds);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_w(
    UDATE const& ud
)
{
    return c_str_ptr_w(ud.st);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr(
    UDATE const&    ud
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr(ud.st, bMilliseconds);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr(
    UDATE const& ud
)
{
    return c_str_ptr(ud.st);
}

#endif /* WINSTL_UDATE_DEFINED */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 */

// SYSTEMTIME

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_data_a(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    return c_str_ptr_a(t, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_data_a(
    SYSTEMTIME const& t
)
{
    return c_str_data_a(t, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_data_w(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    return c_str_ptr_w(t, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_data_w(
    SYSTEMTIME const& t
)
{
    return c_str_data_w(t, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_data(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
#ifdef UNICODE
    return c_str_data_w(t, bMilliseconds);
#else /* ? UNICODE */
    return c_str_data_a(t, bMilliseconds);
#endif /* UNICODE */
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_data(
    SYSTEMTIME const& t
)
{
    return c_str_data(t, ws_false_v);
}


// FILETIME

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_data_a(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_a(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_data_a(
    FILETIME const& t
)
{
    return c_str_data_a(FILETIME2SYSTEMTIME(t), ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_data_w(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_w(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_data_w(
    FILETIME const& t
)
{
    return c_str_data_w(FILETIME2SYSTEMTIME(t), ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_data(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_data(
    FILETIME const& t
)
{
    return c_str_data(FILETIME2SYSTEMTIME(t), ws_false_v);
}


// UDATE

#ifdef WINSTL_UDATE_DEFINED

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_data_a(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_a(t.st, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_data_a(
    UDATE const& t
)
{
    return c_str_data_a(t.st, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_data_w(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_w(t.st, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_data_w(
    UDATE const& t
)
{
    return c_str_data_w(t.st, ws_false_v);
}

inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_data(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr(t.st, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_data(
    UDATE const& t
)
{
    return c_str_data(t.st, ws_false_v);
}

#endif /* WINSTL_UDATE_DEFINED */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 */

// SYSTEMTIME

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_null_a(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    return c_str_ptr_a(t, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_null_a(
    SYSTEMTIME const& t
)
{
    return c_str_ptr_null_a(t, ws_false_v);
}


inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_null_w(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    return c_str_ptr_w(t, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_null_w(
    SYSTEMTIME const& t
)
{
    return c_str_ptr_null_w(t, ws_false_v);
}


inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr_null(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    return c_str_ptr(t, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr_null(
    SYSTEMTIME const& t
)
{
    return c_str_ptr_null(t, ws_false_v);
}



// FILETIME

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_null_a(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_null_a(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_null_a(
    FILETIME const& t
)
{
    return c_str_ptr_null_a(FILETIME2SYSTEMTIME(t), ws_false_v);
}


inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_null_w(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_null_w(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_null_w(
    FILETIME const& t
)
{
    return c_str_ptr_null_w(FILETIME2SYSTEMTIME(t), ws_false_v);
}


inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr_null(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_null(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr_null(
    FILETIME const& t
)
{
    return c_str_ptr_null(FILETIME2SYSTEMTIME(t), ws_false_v);
}


// UDATE

#ifdef WINSTL_UDATE_DEFINED

inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_null_a(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_null_a(t.st, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_a_t>
c_str_ptr_null_a(
    UDATE const& t
)
{
    return c_str_ptr_null_a(t.st, ws_false_v);
}


inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_null_w(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_null_w(t.st, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<ws_char_w_t>
c_str_ptr_null_w(
    UDATE const& t
)
{
    return c_str_ptr_null_w(t.st, ws_false_v);
}


inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr_null(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_ptr_null(t.st, bMilliseconds);
}
inline
stlsoft_ns_qual(basic_shim_string)<TCHAR>
c_str_ptr_null(
    UDATE const& t
)
{
    return c_str_ptr_null(t.st, ws_false_v);
}

#endif /* WINSTL_UDATE_DEFINED */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * NOTE: The following are provided as function overloads, rather than, as
 * originally implemented, with defaulted bMilliseconds argument, because
 * DMC++ gives "ambiguous reference to symbol" errors. (And I didn't have
 * time to investigate further.)
 */

// SYSTEMTIME

inline
ws_size_t
c_str_len_a(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    typedef winstl_shims_access_string_time_impl impl_t;

    int (STLSOFT_STDCALL *pfnGetTimeFormatA)(   LCID                Locale      // locale
                                            ,   DWORD               dwFlags     // options
                                            ,   CONST SYSTEMTIME    *lpTime     // time
                                            ,   ws_char_a_t const   *lpFormat   // time format string
                                            ,   ws_char_a_t         *lpTimeStr  // formatted string buffer
                                            ,   int                 cchTime)    // size of string buffer

                                            =   bMilliseconds ? GetTimeFormat_msA : ::GetTimeFormatA;

    ws_size_t cchDate = 0;
    ws_size_t cchTime = 0;

    return impl_t::calc_sizes(t, ::GetDateFormatA, pfnGetTimeFormatA, cchDate, cchTime);
}
inline
ws_size_t
c_str_len_a(
    SYSTEMTIME const& t
)
{
    return c_str_len_a(t, ws_false_v);
}

inline
ws_size_t
c_str_len_w(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
    typedef winstl_shims_access_string_time_impl impl_t;

    int (STLSOFT_STDCALL *pfnGetTimeFormatW)(   LCID                Locale      // locale
                                            ,   DWORD               dwFlags     // options
                                            ,   CONST SYSTEMTIME    *lpTime     // time
                                            ,   ws_char_w_t const   *lpFormat   // time format string
                                            ,   ws_char_w_t         *lpTimeStr  // formatted string buffer
                                            ,   int                 cchTime)    // size of string buffer

                                            =   bMilliseconds ? GetTimeFormat_msW : ::GetTimeFormatW;

    ws_size_t cchDate = 0;
    ws_size_t cchTime = 0;

    return impl_t::calc_sizes(t, ::GetDateFormatW, pfnGetTimeFormatW, cchDate, cchTime);
}
inline
ws_size_t
c_str_len_w(
    SYSTEMTIME const& t
)
{
    return c_str_len_w(t, ws_false_v);
}

inline
ws_size_t
c_str_len(
    SYSTEMTIME const&   t
,   ws_bool_t           bMilliseconds
)
{
#ifdef UNICODE
    return c_str_len_w(t, bMilliseconds);
#else /* ? UNICODE */
    return c_str_len_a(t, bMilliseconds);
#endif /* UNICODE */
}
inline
ws_size_t
c_str_len(
    SYSTEMTIME const& t
)
{
    return c_str_len(t, ws_false_v);
}


// FILETIME

inline
ws_size_t
c_str_len_a(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_len_a(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
ws_size_t
c_str_len_a(
    FILETIME const& t
)
{
    return c_str_len_a(FILETIME2SYSTEMTIME(t), ws_false_v);
}

inline
ws_size_t
c_str_len_w(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_len_w(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
ws_size_t
c_str_len_w(
    FILETIME const& t
)
{
    return c_str_len_w(FILETIME2SYSTEMTIME(t), ws_false_v);
}


inline
ws_size_t
c_str_len(
    FILETIME const& t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_len(FILETIME2SYSTEMTIME(t), bMilliseconds);
}
inline
ws_size_t
c_str_len(
    FILETIME const& t
)
{
    return c_str_len(FILETIME2SYSTEMTIME(t), ws_false_v);
}


// UDATE

#ifdef WINSTL_UDATE_DEFINED

inline
ws_size_t
c_str_len_a(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_len_a(t.st, bMilliseconds);
}
inline
ws_size_t
c_str_len_a(
    UDATE const& t
)
{
    return c_str_len_a(t.st, ws_false_v);
}

inline
ws_size_t
c_str_len_w(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_len_w(t.st, bMilliseconds);
}
inline
ws_size_t
c_str_len_w(
    UDATE const& t
)
{
    return c_str_len_w(t.st, ws_false_v);
}

inline
ws_size_t
c_str_len(
    UDATE const&    t
,   ws_bool_t       bMilliseconds
)
{
    return c_str_len(t.st, bMilliseconds);
}
inline
ws_size_t
c_str_len(
    UDATE const& t
)
{
    return c_str_len(t.st, ws_false_v);
}

#endif /* WINSTL_UDATE_DEFINED */


/* /////////////////////////////////////////////////////////////////////////
 * Stream inserter
 */

/** \brief An inserter function for SYSTEMTIME into output streams
 *
 * \ingroup group__library__shims__string_access
 *
 */
template <ss_typename_param_k S>
inline
S&
operator <<(
    S&                  s
,   SYSTEMTIME const&   st
)
{
    stream_insert(s, st);

    return s;
}

/** \brief An inserter function for FILETIME into output streams
 *
 * \ingroup group__library__shims__string_access
 *
 */
template <ss_typename_param_k S>
inline
S&
operator <<(
    S&              s
,   FILETIME const& ft
)
{
    stream_insert(s, ft);

    return s;
}

#ifdef WINSTL_UDATE_DEFINED

/** \brief An inserter function for UDATE into output streams
 *
 * \ingroup group__library__shims__string_access
 *
 */
template <ss_typename_param_k S>
inline
S&
operator <<(
    S&              s
,   UDATE const&    ud
)
{
    stream_insert(s, ud);

    return s;
}

#endif /* WINSTL_UDATE_DEFINED */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */

/** \brief An inserter function for SYSTEMTIME into output streams
 *
 * \ingroup group__library__shims__string_access
 *
 */
template <ss_typename_param_k S>
inline
S&
operator <<(
    S&                  s
,   SYSTEMTIME const&   st
)
{
    ::winstl::stream_insert(s, st);

    return s;
}

/** \brief An inserter function for FILETIME into output streams
 *
 * \ingroup group__library__shims__string_access
 *
 */
template <ss_typename_param_k S>
inline
S&
operator <<(
    S&              s
,   FILETIME const& st
)
{
    ::winstl::stream_insert(s, st);

    return s;
}

namespace stlsoft
{
    using ::winstl::c_str_ptr_null;
    using ::winstl::c_str_ptr_null_a;
    using ::winstl::c_str_ptr_null_w;

    using ::winstl::c_str_ptr;
    using ::winstl::c_str_ptr_a;
    using ::winstl::c_str_ptr_w;

    using ::winstl::c_str_data;
    using ::winstl::c_str_data_a;
    using ::winstl::c_str_data_w;

    using ::winstl::c_str_len;

    using ::winstl::c_str_len_a;
    using ::winstl::c_str_len_w;

} // namespace stlsoft

#endif /* !_WINSTL_NO_NAMESPACE */

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

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/time_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME */

/* ///////////////////////////// end of file //////////////////////////// */
