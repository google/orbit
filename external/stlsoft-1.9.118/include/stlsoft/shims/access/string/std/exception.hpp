/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/access/string/std/exception.hpp
 *
 * Purpose:     Contains the string access shims for std::exception.
 *
 * Created:     2nd May 2003
 * Updated:     14th March 2012
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


/** \file stlsoft/shims/access/string/std/exception.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>std::exception</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION
#define STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION_MAJOR       2
# define _STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION_MINOR       2
# define _STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION_REVISION    7
# define _STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION_EDIT        43
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD
# include <stlsoft/shims/access/string/fwd.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING
# include <stlsoft/shims/access/string/std/c_string.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_CONVERSION_ERROR
#  include <stlsoft/error/conversion_error.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_CONVERSION_ERROR */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

#ifndef STLSOFT_INCL_EXCEPTION
# define STLSOFT_INCL_EXCEPTION
#  include <exception>              // for std::exception
#endif /* !STLSOFT_INCL_EXCEPTION */

#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helpers
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct ximpl_stlsoft_shims_access_string_std_exception_
{
public:
    //
    // *numConverted excludes the nul-terminator
    static
    int
    mbstowcs_(
        ss_char_a_t const*  mbs
    ,   ss_size_t           mbsLen
    ,   ss_char_w_t*        ws
    ,   ss_size_t           wsSizeInChars
    ,   ss_size_t*          numConverted
    )
    {
        // Always assume the nul-terminator
        STLSOFT_ASSERT(NULL != mbs);

        STLSOFT_ASSERT(NULL != numConverted);

#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS

        int r = ::mbstowcs_s(numConverted, ws, wsSizeInChars, mbs, mbsLen);

        if(0 == r)
        {
            STLSOFT_ASSERT(0 != *numConverted);

            --*numConverted; // mbstowcs_s() always adds one for nul-terminator
        }

        return r;

#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */

        STLSOFT_SUPPRESS_UNUSED(mbsLen);
        STLSOFT_SUPPRESS_UNUSED(wsSizeInChars);

        *numConverted = ::mbstowcs(ws, mbs, mbsLen);
        if(static_cast<ss_size_t>(-1) == *numConverted)
        {
            return errno;
        }

        return 0;
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */
    }

    //
    // *numConverted excludes the nul-terminator
    static
    int
    mbstowcs_len(
        ss_char_a_t const*  s
    ,   ss_size_t*          len
    )
    {
        STLSOFT_ASSERT(NULL != len);

#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS

        int r = ::mbstowcs_s(len, NULL, 0, s, 0);

        if(0 == r)
        {
            STLSOFT_ASSERT(0 != *len);

            --*len; // mbstowcs_s() always adds one for nul-terminator
        }

        return r;

#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */

        *len = ::mbstowcs(NULL, s, 0);
        if(static_cast<ss_size_t>(-1) == *len)
        {
            return errno;
        }

        return 0;

#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */
    }

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if defined(STLSOFT_COMPILER_IS_MSVC)
#  if _MSC_VER >= 1200
#   pragma warning(push)
#   pragma warning(disable : 4702)
#  endif /* _MSC_VER */
# endif /* compiler */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

    static
    int
    throw_conversion_error_or_return_(
        int         err
    ,   char const* message
    )
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(conversion_error(message, err));
#else /* ?STLSOFT_CF_EXCEPTION_SUPPORT */
        STLSOFT_SUPPRESS_UNUSED(message);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return err;
    }

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if defined(STLSOFT_COMPILER_IS_MSVC)
#  if _MSC_VER >= 1200
#   pragma warning(pop)
#  endif /* _MSC_VER */
# endif /* compiler */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_data_a(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_data_a(x.what());
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
inline
basic_shim_string<ss_char_w_t, 100>
c_str_data_w(stlsoft_ns_qual_std(exception) const& x)
{
    // C-string pointer and length
    ss_char_a_t const* const    s   = x.what();
    ss_size_t const             len = ::strlen(s);

    // Calculate the wide-string length
    ss_size_t   wlen;
    int         err = ximpl_stlsoft_shims_access_string_std_exception_::mbstowcs_len(s, &wlen);

    // handle failure to get length
    if(0 != err)
    {
        ximpl_stlsoft_shims_access_string_std_exception_::throw_conversion_error_or_return_(err, "cannot elicit wide-string length of exception message");
    }

    // Create the shim string, which will be returned
    basic_shim_string<ss_char_w_t, 100> r(wlen);
    ss_size_t                           wlen2;

    err = ximpl_stlsoft_shims_access_string_std_exception_::mbstowcs_(s, len, r.data(), 1u + r.size(), &wlen2);

    // Sanity checks
    STLSOFT_ASSERT(wlen2 == r.size());
    STLSOFT_SUPPRESS_UNUSED(wlen2);

    // handle failure to convert
    if(0 != err)
    {
        ximpl_stlsoft_shims_access_string_std_exception_::throw_conversion_error_or_return_(err, "cannot elicit wide-string equivalent of exception message");
    }

    return r;
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style string.
 */
inline ss_char_a_t const* c_str_data(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_data_a(x);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string <code>s</code>.
 */
inline ss_size_t c_str_len_a(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_len_a(x.what());
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string <code>s</code>.
 */
inline
ss_size_t
c_str_len_w(stlsoft_ns_qual_std(exception) const& x)
{
    ss_size_t   len;
    int         err = ximpl_stlsoft_shims_access_string_std_exception_::mbstowcs_len(x.what(), &len);

    if(0 != err)
    {
        len = 0;

        ximpl_stlsoft_shims_access_string_std_exception_::throw_conversion_error_or_return_(err, "failed to elicit length of multibyte string");
    }

    return len;
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length of the string <code>s</code>.
 */
inline ss_size_t c_str_len(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_len_a(x);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr_a(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_ptr_a(x.what());
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline
basic_shim_string<ss_char_w_t, 100>
c_str_ptr_w(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_data_w(x);
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline ss_char_a_t const* c_str_ptr(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_ptr_a(x);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr_null_a(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_ptr_null_a(x.what());
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline
basic_shim_string<ss_char_w_t, 100, true>
c_str_ptr_null_w(stlsoft_ns_qual_std(exception) const& x)
{
    // C-string pointer and length
    ss_char_a_t const* const    s   = x.what();
    ss_size_t const             len = ::strlen(s);

    // Calculate the wide-string length
    ss_size_t   wlen;
    int         err = ximpl_stlsoft_shims_access_string_std_exception_::mbstowcs_len(s, &wlen);

    // handle failure to get length
    if(0 != err)
    {
        ximpl_stlsoft_shims_access_string_std_exception_::throw_conversion_error_or_return_(err, "cannot elicit wide-string length of exception message");
    }

    // Create the shim string, which will be returned
    basic_shim_string<ss_char_w_t, 100, true>  r(wlen);
    ss_size_t                                  wlen2;

    err = ximpl_stlsoft_shims_access_string_std_exception_::mbstowcs_(s, len, r.data(), 1u + r.size(), &wlen2);

    // Sanity checks
    STLSOFT_ASSERT(wlen2 == r.size());
    STLSOFT_SUPPRESS_UNUSED(wlen2);

    // handle failure to convert
    if(0 != err)
    {
        ximpl_stlsoft_shims_access_string_std_exception_::throw_conversion_error_or_return_(err, "cannot elicit wide-string equivalent of exception message");
    }

    return r;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>std::exception</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
inline ss_char_a_t const* c_str_ptr_null(stlsoft_ns_qual_std(exception) const& x)
{
    return c_str_ptr_null_a(x);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_EXCEPTION */

/* ///////////////////////////// end of file //////////////////////////// */
