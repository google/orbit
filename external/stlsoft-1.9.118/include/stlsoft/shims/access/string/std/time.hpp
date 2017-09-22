/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/access/string/std/time.hpp
 *
 * Purpose:     String shims for standard time structures.
 *
 * Created:     25th July 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/shims/access/string/std/time.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   standard time structures
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME
#define STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME_MAJOR     2
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME_MINOR     1
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME_REVISION  6
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME_EDIT      23
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING
# include <stlsoft/shims/access/string/std/c_string.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */

#ifndef STLSOFT_INCL_H_TIME
# define STLSOFT_INCL_H_TIME
# include <time.h>
#endif /* !STLSOFT_INCL_H_TIME */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 *
 * All the struct tm-related conversions assume a format of 20 characters
 */

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline basic_shim_string<ss_char_a_t> c_str_data_a(struct tm const* t)
{
    typedef basic_shim_string<ss_char_a_t>  shim_string_t;

    shim_string_t   s(20);

    if(NULL == t)
    {
        s.truncate(0);
    }
    else
    {
        const ss_size_t cch = ::strftime(s.data(), 1 + s.size(), "%b %d %H:%M:%S %Y", t);

        STLSOFT_ASSERT(20 == cch);

        s.truncate(cch);
    }

    return s;
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style string.
 */
inline basic_shim_string<ss_char_a_t> c_str_data(struct tm const* t)
{
    return c_str_data_a(t);
}



/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string <code>s</code>.
 */
inline ss_size_t c_str_len_a(struct tm const* t)
{
    return static_cast<ss_size_t>((NULL != t) ? 20 : 0);
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length of the string <code>s</code>.
 */
inline ss_size_t c_str_len(struct tm const* t)
{
    return c_str_len_a(t);
}



/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, nul-terminated, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr_a(struct tm const* t)
{
    return c_str_data_a(t);
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, nul-terminated, non-mutating pointer to a C-style
 *   string.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr(struct tm const* t)
{
    return c_str_data_a(t);
}



/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, nul-terminated, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr_null_a(struct tm const* t)
{
    return c_str_data_a(t);
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, nul-terminated, non-mutating pointer to a C-style
 *   string.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr_null(struct tm const* t)
{
    return c_str_data_a(t);
}






/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline basic_shim_string<ss_char_a_t> c_str_data(struct tm const& t)
{
    return c_str_data(&t);
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style string.
 */
inline basic_shim_string<ss_char_a_t> c_str_data_a(struct tm const& t)
{
    return c_str_data_a(&t);
}


/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string <code>s</code>.
 */
inline ss_size_t c_str_len_a(struct tm const& t)
{
    return c_str_len_a(&t);
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length of the string <code>s</code>.
 */
inline ss_size_t c_str_len(struct tm const& t)
{
    return c_str_len(&t);
}


/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, nul-terminated, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr_a(struct tm const& t)
{
    return c_str_ptr_a(&t);
}
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, nul-terminated, non-mutating pointer to a C-style
 *   string.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr(struct tm const& t)
{
    return c_str_ptr(&t);
}


/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, nul-terminated, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr_null_a(struct tm const& t)
{
    return c_str_ptr_null_a(&t);
}
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct tm</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, nul-terminated, non-mutating pointer to a C-style
 *   string.
 */
inline basic_shim_string<ss_char_a_t> c_str_ptr_null(struct tm const& t)
{
    return c_str_ptr_null(&t);
}


/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_TIME */

/* ///////////////////////////// end of file //////////////////////////// */
