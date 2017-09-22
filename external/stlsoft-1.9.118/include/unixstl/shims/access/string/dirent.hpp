/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/shims/access/string/dirent.hpp
 *
 * Purpose:     Support for the STLSoft string access shims for UNIX types.
 *
 * Created:     11th January 2003
 * Updated:     10th August 2009
 *
 * Thanks:      To Carlos Santander Bernal, for providing feedback for Mac
 *              builds
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file unixstl/shims/access/string/dirent.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>struct dirent</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT
#define UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT_MAJOR       4
# define UNIXSTL_VER_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT_MINOR       0
# define UNIXSTL_VER_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT_REVISION    2
# define UNIXSTL_VER_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT_EDIT        56
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD
# include <stlsoft/shims/access/string/fwd.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING
# include <stlsoft/shims/access/string/std/c_string.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING */

#ifndef STLSOFT_INCL_H_DIRENT
# define STLSOFT_INCL_H_DIRENT
# include <dirent.h>
#endif /* !STLSOFT_INCL_H_DIRENT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/** \brief \ref group__concept__shim__string_access__c_str_data for
 *    <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline us_char_a_t const* c_str_data_a(struct dirent const* d)
{
    return (NULL == d) ? "" : d->d_name;
}

/** \brief \ref group__concept__shim__string_access__c_str_data for
 *    <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style string.
 */
inline us_char_a_t const* c_str_data(struct dirent const* d)
{
    return (NULL == d) ? "" : d->d_name;
}


/** \brief \ref group__concept__shim__string_access__c_str_data for
 *    <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline us_char_a_t const* c_str_data_a(struct dirent const& d)
{
    return d.d_name;
}

/** \brief \ref group__concept__shim__string_access__c_str_data for
 *    <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style string.
 */
inline us_char_a_t const* c_str_data(struct dirent const& d)
{
    return d.d_name;
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string form of <code>d</code>.
 */
inline us_size_t c_str_len_a(struct dirent const* d)
{
    return stlsoft_ns_qual(c_str_len)(c_str_data(d));
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length of the string form of <code>d</code>.
 */
inline us_size_t c_str_len(struct dirent const* d)
{
    return c_str_len_a(d);
}


/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string form of <code>d</code>.
 */
inline us_size_t c_str_len_a(struct dirent const& d)
{
    return c_str_len_a(&d);
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length of the string form of <code>d</code>.
 */
inline us_size_t c_str_len(struct dirent const& d)
{
    return c_str_len(&d);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline us_char_a_t const* c_str_ptr_a(struct dirent const* d)
{
    return (NULL == d) ? "" : d->d_name;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline us_char_a_t const* c_str_ptr(struct dirent const* d)
{
    return (NULL == d) ? "" : d->d_name;
}


/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline us_char_a_t const* c_str_ptr_a(struct dirent const& d)
{
    return d.d_name;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline us_char_a_t const* c_str_ptr(struct dirent const& d)
{
    return d.d_name;
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline us_char_a_t const* c_str_ptr_null_a(struct dirent const* d)
{
    return (NULL == d || 0 == d->d_name[0]) ? static_cast<us_char_a_t const*>(NULL) : d->d_name;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline us_char_a_t const* c_str_ptr_null(struct dirent const* d)
{
    return (NULL == d || 0 == d->d_name[0]) ? static_cast<us_char_a_t const*>(NULL) : d->d_name;
}


/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline us_char_a_t const* c_str_ptr_null_a(struct dirent const& d)
{
    return 0 == d.d_name[0] ? static_cast<us_char_a_t const*>(NULL) : d.d_name;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>struct dirent</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline us_char_a_t const* c_str_ptr_null(struct dirent const& d)
{
    return 0 == d.d_name[0] ? static_cast<us_char_a_t const*>(NULL) : d.d_name;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/dirent_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace stlsoft::unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::unixstl::c_str_data;
using ::unixstl::c_str_data_a;
//using ::unixstl::c_str_data_w;

using ::unixstl::c_str_len;
using ::unixstl::c_str_len_a;
//using ::unixstl::c_str_len_w;

using ::unixstl::c_str_ptr;
using ::unixstl::c_str_ptr_a;
//using ::unixstl::c_str_ptr_w;

using ::unixstl::c_str_ptr_null;
using ::unixstl::c_str_ptr_null_a;
//using ::unixstl::c_str_ptr_null_w;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_STRING_HPP_DIRENT */

/* ///////////////////////////// end of file //////////////////////////// */
