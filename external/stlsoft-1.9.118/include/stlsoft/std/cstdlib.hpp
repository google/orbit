/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/std/cstdlib.hpp
 *
 * Purpose:     Mappings to stdlib string functions
 *
 * Created:     2nd December 2004
 * Updated:     31st March 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/std/cstdlib.hpp
 *
 * \brief [C++ only] Mappings of stdlib string functions that use
 *   \ref group__concept__shim__string_access string
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STD_HPP_CSTDLIB
#define STLSOFT_INCL_STLSOFT_STD_HPP_CSTDLIB

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTDLIB_MAJOR      2
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTDLIB_MINOR      0
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTDLIB_REVISION   1
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTDLIB_EDIT       25
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_STD_HPP_CBASE_
# include <stlsoft/std/cbase_.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STD_HPP_CBASE_ */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

#ifndef STLSOFT_INCL_H_STDLIB
# define STLSOFT_INCL_H_STDLIB
# include <stdlib.h>
#endif /* !STLSOFT_INCL_H_STDLIB */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler / library feature discrimination
 */

#define STLSOFT_STD_CSTDLIB_ATOI_SUPPORTED
#define STLSOFT_STD_CSTDLIB_ATOL_SUPPORTED
#define STLSOFT_STD_CSTDLIB_ATOF_SUPPORTED

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_DMC) || \
    (   defined(STLSOFT_COMPILER_IS_GCC) && \
        defined(WIN32)) || \
    (   defined(STLSOFT_COMPILER_IS_INTEL) && \
        defined(WIN32)) || \
    defined(STLSOFT_COMPILER_IS_MSVC) || \
    defined(STLSOFT_COMPILER_IS_MWERKS) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
# define STLSOFT_STD_CSTDLIB_WTOI_SUPPORTED
#endif /* compiler */

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_DMC) || \
    (   defined(STLSOFT_COMPILER_IS_GCC) && \
        defined(WIN32)) || \
    (   defined(STLSOFT_COMPILER_IS_INTEL) && \
        defined(WIN32)) || \
    defined(STLSOFT_COMPILER_IS_MSVC) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
# define STLSOFT_STD_CSTDLIB_WTOL_SUPPORTED
#endif /* compiler */

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER >= 1300) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
# define STLSOFT_STD_CSTDLIB_WTOF_SUPPORTED
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helper classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct atoi_helper
{
    static int call(char const* s)
    {
        return ::atoi(s);
    }
#ifdef STLSOFT_STD_CSTDLIB_WTOI_SUPPORTED
    static int call(wchar_t const* s)
    {
        return ::_wtoi(s);
    }
#endif /* STLSOFT_STD_CSTDLIB_WTOI_SUPPORTED */
};

struct atol_helper
{
    static long call(char const* s)
    {
        return ::atol(s);
    }
#ifdef STLSOFT_STD_CSTDLIB_WTOL_SUPPORTED
    static long call(wchar_t const* s)
    {
        return ::_wtol(s);
    }
#endif /* STLSOFT_STD_CSTDLIB_WTOL_SUPPORTED */
};

struct atof_helper
{
    static double call(char const* s)
    {
        return ::atof(s);
    }
#ifdef STLSOFT_STD_CSTDLIB_WTOF_SUPPORTED
    static double call(wchar_t const* s)
    {
        return ::_wtof(s);
    }
#endif /* STLSOFT_STD_CSTDLIB_WTOF_SUPPORTED */
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/// \name atoi family
/// @{

template <ss_typename_param_k S>
inline int atoi(S const& s)
{
    return atoi_helper::call(stlsoft_ns_qual(c_str_ptr)(s));
}

template <ss_typename_param_k S>
inline long atol(S const& s)
{
    return atol_helper::call(stlsoft_ns_qual(c_str_ptr)(s));
}

template <ss_typename_param_k S>
inline double atof(S const& s)
{
    return atof_helper::call(stlsoft_ns_qual(c_str_ptr)(s));
}

/// @}


/// \name strtol family
/// @{

inline long strtol(ss_char_a_t const* s, ss_char_a_t** endptr, int radix)
{
    return ::strtol(s, endptr, radix);
}

inline long strtol(ss_char_w_t const* s, ss_char_w_t** endptr, int radix)
{
    return ::wcstol(s, endptr, radix);
}

inline unsigned long strtoul(ss_char_a_t const* s, ss_char_a_t** endptr, int radix)
{
    return ::strtoul(s, endptr, radix);
}

inline unsigned long strtoul(ss_char_w_t const* s, ss_char_w_t** endptr, int radix)
{
    return ::wcstoul(s, endptr, radix);
}

inline double strtod(ss_char_a_t const* s, ss_char_a_t** endptr)
{
    return ::strtod(s, endptr);
}

inline double strtod(ss_char_w_t const* s, ss_char_w_t** endptr)
{
    return ::wcstod(s, endptr);
}

/// @}


/** \brief system()
 *
 * \ingroup group__library__utility
 */

template <ss_typename_param_k S>
inline int system(S const& s)
{
    return ::system(stlsoft_ns_qual(c_str_ptr)(s));
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/cstdlib_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STD_HPP_CSTDLIB */

/* ///////////////////////////// end of file //////////////////////////// */
