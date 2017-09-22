/* /////////////////////////////////////////////////////////////////////////
 * File:        acestl/shims/access/string/time_value.hpp
 *
 * Purpose:     Helper functions for the ACE_Time_Value class.
 *
 * Created:     2nd December 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file acestl/shims/access/string/time_value.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>ACE_Time_Value</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef ACESTL_INCL_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE
#define ACESTL_INCL_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE_MAJOR     2
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE_MINOR     0
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE_REVISION  5
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE_EDIT      40
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ACESTL_INCL_ACESTL_HPP_ACESTL
# include <acestl/acestl.hpp>
#endif /* !ACESTL_INCL_ACESTL_HPP_ACESTL */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */

#ifndef STLSOFT_INCL_ACE_H_TIME_VALUE
# define STLSOFT_INCL_ACE_H_TIME_VALUE
# include <ace/Time_Value.h>                // for ACE_Time_Value
#endif /* !STLSOFT_INCL_ACE_H_TIME_VALUE */
#ifndef STLSOFT_INCL_ACE_H_ACE_WCHAR
# define STLSOFT_INCL_ACE_H_ACE_WCHAR
# include <ace/ace_wchar.h>                 // for ACE_Wide_To_Ascii, ACE_Ascii_To_Wide
#endif /* !STLSOFT_INCL_ACE_H_ACE_WCHAR */
#if ACESTL_ACE_VERSION >= 0x00050004
#ifndef STLSOFT_INCL_ACE_H_OS_NS_STDIO
# define STLSOFT_INCL_ACE_H_OS_NS_STDIO
#  include <ace/OS_NS_stdio.h>
#endif /* !STLSOFT_INCL_ACE_H_OS_NS_STDIO */
#ifndef STLSOFT_INCL_ACE_H_OS_NS_TIME
# define STLSOFT_INCL_ACE_H_OS_NS_TIME
#  include <ace/OS_NS_time.h>
#endif /* !STLSOFT_INCL_ACE_H_OS_NS_TIME */
#endif /* ACE_VER >= 5.4 */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _ACESTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::acestl */
namespace acestl
{
# else
/* Define stlsoft::acestl_project */

namespace stlsoft
{

namespace acestl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ACESTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace acestl_time_access_string_util
{
    template <ss_typename_param_k S>
    void stream_insert(S& s, ACE_Time_Value const& t)
    {
        char                s1[20];
        ACE_TCHAR           s2[24];

        const long          s   =   t.sec();
        const long          us  =   t.usec();
        struct tm   *const  tm  =   ACE_OS::localtime(&static_cast<time_t const&>(s));
        as_size_t           len =   ACE_OS::strftime(s1, STLSOFT_NUM_ELEMENTS(s1), "%Y-%m-%d %H:%M:%S", tm);

        ACESTL_ASSERT(len == 1 + STLSOFT_NUM_ELEMENTS(s1));

        len = ACE_OS::snprintf(s2, STLSOFT_NUM_ELEMENTS(s2), ACE_TEXT("%s.%03ld"), ACE_TEXT_CHAR_TO_TCHAR(s1), us / 1000);

        ACESTL_ASSERT(len == 1 + STLSOFT_NUM_ELEMENTS(s2));

        s.write(&s2[0], len);
    }

    inline int invoke_ACE_OS_snprintf(ACE_TCHAR s2[], as_size_t size, ACE_TCHAR const* fmt, as_char_a_t const* s1, long ms)
    {
        return ACE_OS::snprintf(s2, size, fmt, ACE_TEXT_CHAR_TO_TCHAR(s1), ms);
    }

# ifdef ACE_USES_WCHAR
    inline int invoke_ACE_OS_snprintf(as_char_a_t s2[], as_size_t size, as_char_w_t const* fmt, as_char_a_t const* s1, long ms)
    {
        return ACE_OS::snprintf(s2, size, ACE_TEXT_ALWAYS_CHAR(fmt), s1, ms);
    }
# else /* ? ACE_USES_WCHAR */
    inline int invoke_ACE_OS_snprintf(as_char_w_t s2[], as_size_t size, as_char_a_t const* fmt, as_char_a_t const* s1, long ms)
    {
        stlsoft::auto_buffer<as_char_a_t>   buff(1 + size);
        int                                 res;

        s2[0] = '\0';

        res = ACE_OS::snprintf(&buff[0], buff.size(), ACE_TEXT_ALWAYS_CHAR(fmt), s1, ms);

        if(0 < res)
        {
            ACESTL_ASSERT(static_cast<ss_size_t>(res) < buff.size());

            ::mbstowcs(&s2[2], buff.data(), res);
            s2[res] = '\0';
        }

        return res;
    }
# endif /* ACE_USES_WCHAR */

    template <ss_typename_param_k C>
    inline ::stlsoft::basic_shim_string<C> c_str_ptr_(ACE_Time_Value const& t)
    {
        typedef C                       char_t;

        as_char_a_t         s1[20];
        char_t              s2[24];
        const long          s   =   t.sec();
        const long          us  =   t.usec();
        struct tm   *const  tm  =   ACE_OS::localtime(&static_cast<time_t const&>(s));
        as_size_t           len =   ACE_OS::strftime(s1, STLSOFT_NUM_ELEMENTS(s1), "%Y-%m-%d %H:%M:%S", tm);

        ACESTL_ASSERT(1 + len == STLSOFT_NUM_ELEMENTS(s1));

        len = invoke_ACE_OS_snprintf(s2, STLSOFT_NUM_ELEMENTS(s2), ACE_TEXT("%s.%03ld"), s1, us / 1000);

        ACESTL_ASSERT(1 + len == STLSOFT_NUM_ELEMENTS(s2));

        return ::stlsoft::basic_shim_string<char_t>(&s2[0], len);
    }

} // namespace acestl_time_access_string_util
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * String Access Shims
 */

// c_str_data

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ::stlsoft::basic_shim_string<as_char_a_t> c_str_data_a(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<as_char_a_t>(t);
}
inline ::stlsoft::basic_shim_string<as_char_w_t> c_str_data_w(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<as_char_w_t>(t);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_data for ACE_Time_Value
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ::stlsoft::basic_shim_string<ACE_TCHAR> c_str_data(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<ACE_TCHAR>(t);
}

// c_str_len

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline as_size_t c_str_len_a(ACE_Time_Value const& /* t */)
{
    return 23;
}
inline as_size_t c_str_len_w(ACE_Time_Value const& /* t */)
{
    return 23;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_len for ACE_Time_Value
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline as_size_t c_str_len(ACE_Time_Value const& /* t */)
{
    return 23;
}

// c_str_ptr

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ::stlsoft::basic_shim_string<as_char_a_t> c_str_ptr_a(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<as_char_a_t>(t);
}
inline ::stlsoft::basic_shim_string<as_char_w_t> c_str_ptr_w(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<as_char_w_t>(t);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_ptr for ACE_Time_Value
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ::stlsoft::basic_shim_string<ACE_TCHAR> c_str_ptr(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<ACE_TCHAR>(t);
}

// c_str_ptr_null

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ::stlsoft::basic_shim_string<as_char_a_t> c_str_ptr_null_a(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<as_char_a_t>(t);
}
inline ::stlsoft::basic_shim_string<as_char_w_t> c_str_ptr_null_w(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<as_char_w_t>(t);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_ptr_null for ACE_Time_Value
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ::stlsoft::basic_shim_string<ACE_TCHAR> c_str_ptr_null(ACE_Time_Value const& t)
{
    return acestl_time_access_string_util::c_str_ptr_<ACE_TCHAR>(t);
}


/* /////////////////////////////////////////////////////////////////////////
 * Stream Insertion Shims
 */

/** A \ref group__concept__shim__stream_insertion "stream insertion shim" for ACE_Time_Value
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template <ss_typename_param_k S>
inline S& operator <<(S& s, ACE_Time_Value const& t)
{
    acestl_time_access_string_util::stream_insert(s, t);

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/time_value_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ACESTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace acestl
# else
} // namespace acestl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */

/** A \ref group__concept__shim__stream_insertion "stream insertion shim" for ACE_Time_Value
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template <ss_typename_param_k S>
inline S& operator <<(S& s, ACE_Time_Value const& t)
{
    ::acestl::acestl_time_access_string_util::stream_insert(s, t);

    return s;
}

namespace stlsoft
{

    using ::acestl::c_str_data;
    using ::acestl::c_str_data_a;
    using ::acestl::c_str_data_w;

    using ::acestl::c_str_len;
    using ::acestl::c_str_len_a;
    using ::acestl::c_str_len_w;

    using ::acestl::c_str_ptr;
    using ::acestl::c_str_ptr_a;
    using ::acestl::c_str_ptr_w;

    using ::acestl::c_str_ptr_null;
    using ::acestl::c_str_ptr_null_a;
    using ::acestl::c_str_ptr_null_w;

} // namespace stlsoft

#endif /* !_ACESTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !ACESTL_INCL_ACESTL_SHIMS_ACCESS_STRING_HPP_TIME_VALUE */

/* ///////////////////////////// end of file //////////////////////////// */
