/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/access/string/std/c_string.h
 *
 * Purpose:     Contains the c_str_ptr, c_str_ptr_null, c_str_len, and
 *              c_str_size accessors.
 *
 * Created:     16th January 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/shims/access/string/std/c_string.h
 *
 * \brief [C, C++] Definition of the string access shims for C-style
 *   strings
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING
#define STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING_MAJOR       4
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING_MINOR       0
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING_REVISION    5
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING_EDIT        94
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

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */
#if defined(STLSOFT_COMPILER_IS_GCC) || \
    defined(STLSOFT_COMPILER_IS_MWERKS) || \
    defined(STLSOFT_COMPILER_IS_SUNPRO)
# include <wchar.h>
#endif /* compiler */

#ifdef STLSOFT_UNITTEST
# include <stdio.h>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Pre-processor control
 *
 * By default, conversions from non-const strings, or rather from pointers
 * to non-const characters, are not allowed, since the implied semantics for
 * a pointer-to-const character representing a null-terminated string are
 * stronger than those for a pointer-to-non-const character.
 *
 * However, you can override this by defining the symbol
 * _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST, which will then treat both the
 * types (in fact all four types: char*; char const*; wchar_t*; wchar_t
 * const*) as representing null-terminated strings.
 */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 */

/** \brief Inert class that connotes an invalid use of a string access shim
 *   function by forcing a compile-time error.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
struct cannot_use_untyped_0_or_NULL_with_shims;

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* C-style ANSI string */
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
STLSOFT_INLINE ss_char_a_t const* c_str_data_a(ss_char_a_t const* s)
{
    return (NULL != s) ? s : "";
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_data(ss_char_a_t const* s)
{
    return c_str_data_a(s);
}
#endif /* __cplusplus */

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
STLSOFT_INLINE ss_char_w_t const* c_str_data_w(ss_char_w_t const* s)
{
    return (NULL != s) ? s : stlsoft_static_cast(ss_char_w_t const*, L"");
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_data(ss_char_w_t const* s)
{
    return c_str_data_w(s);
}
#endif /* __cplusplus */

#ifdef __cplusplus
/* C-style ANSI string */
# ifdef _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>char*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_data_a(ss_char_a_t *s)
{
    return (NULL != s) ? s : "";
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>char*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_data(ss_char_a_t *s)
{
    return c_str_data_a(s);
}

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>wchar_t*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_data_w(ss_char_w_t *s)
{
    return (NULL != s) ? s : L"";
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>wchar_t*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_data(ss_char_w_t *s)
{
    return c_str_data_w(s);
}
# endif /* _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST */

# if 0 /* TODO: Try and make this work. Sometime. Maybe ... */
/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_data
 *    function for any type for which c_str_ptr_a is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
template <ss_typename_param_k S>
inline ss_char_a_t const* c_str_data_a(S const& s)
{
    return stlsoft_ns_qual(c_str_data_a)(static_cast<ss_char_a_t const*>(stlsoft_ns_qual(c_str_ptr_a)(s))));
}
/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_data
 *    function for any type for which c_str_ptr_w is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
template <ss_typename_param_k S>
inline ss_char_w_t const* c_str_data_w(S const& s)
{
    return stlsoft_ns_qual(c_str_data_w)(static_cast<ss_char_w_t const*>(stlsoft_ns_qual(c_str_ptr_w)(s))));
}
# endif /* 0 */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/* C-style ANSI string */
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
STLSOFT_INLINE ss_size_t c_str_len_a(ss_char_a_t const* s)
{
    return (s == 0) ? 0 : STLSOFT_NS_GLOBAL(strlen)(s);
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
inline ss_size_t c_str_len(ss_char_a_t const* s)
{
    return c_str_len_a(s);
}
#endif /* __cplusplus */

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in characters) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
STLSOFT_INLINE ss_size_t c_str_len_w(ss_char_w_t const* s)
{
    return (s == 0) ? 0 : STLSOFT_NS_GLOBAL(wcslen)(s);
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in characters) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
inline ss_size_t c_str_len(ss_char_w_t const* s)
{
    return c_str_len_w(s);
}
#endif /* __cplusplus */

#ifdef __cplusplus

/* C-style ANSI string */
# ifdef _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
inline ss_size_t c_str_len_a(ss_char_a_t *s)
{
    return c_str_len_a(static_cast<ss_char_a_t const*>(s));
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
inline ss_size_t c_str_len(ss_char_a_t *s)
{
    return c_str_len_a(s);
}

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in characters) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
inline ss_size_t c_str_len_w(ss_char_w_t *s)
{
    return c_str_len_w(static_cast<ss_char_w_t const*>(s));
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in characters) of the C-style string <code>s</code>, or 0 if
 *   <code>s</code> is NULL.
 */
inline ss_size_t c_str_len(ss_char_w_t *s)
{
    return c_str_len_w(s);
}
# endif /* _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* C-style ANSI string */
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
STLSOFT_INLINE ss_char_a_t const* c_str_ptr_a(ss_char_a_t const* s)
{
    return (NULL != s) ? s : "";
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr(ss_char_a_t const* s)
{
    return c_str_ptr_a(s);
}
#endif /* __cplusplus */

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
STLSOFT_INLINE ss_char_w_t const* c_str_ptr_w(ss_char_w_t const* s)
{
    return (NULL != s) ? s : stlsoft_static_cast(ss_char_w_t const*, L"");
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>wchar_t const*</code>.
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_ptr(ss_char_w_t const* s)
{
    return c_str_ptr_w(s);
}
#endif /* __cplusplus */

#ifdef __cplusplus
/* C-style ANSI string */
# ifdef _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>char const*</code>.
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr_a(ss_char_a_t *s)
{
    return (NULL != s) ? s : "";
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr(ss_char_a_t *s)
{
    return c_str_ptr_a(s);
}

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_ptr_w(ss_char_w_t *s)
{
    return (NULL != s) ? s : L"";
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_ptr(ss_char_w_t *s)
{
    return c_str_ptr_w(s);
}
# endif /* _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST */
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/* C-style ANSI string */
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
STLSOFT_INLINE ss_char_a_t const* c_str_ptr_null_a(ss_char_a_t const* s)
{
    return (NULL == s || '\0' == *s) ? NULL : s;
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
inline ss_char_a_t const* c_str_ptr_null(ss_char_a_t const* s)
{
    return c_str_ptr_null_a(s);
}
#endif /* __cplusplus */

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
STLSOFT_INLINE ss_char_w_t const* c_str_ptr_null_w(ss_char_w_t const* s)
{
    return (NULL == s || L'\0' == *s) ? NULL : s;
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
inline ss_char_w_t const* c_str_ptr_null(ss_char_w_t const* s)
{
    return c_str_ptr_null_w(s);
}
#endif /* __cplusplus */

#ifdef __cplusplus

/* C-style ANSI string */
# ifdef _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
inline ss_char_a_t const* c_str_ptr_null_a(ss_char_a_t *s)
{
    return (NULL == s || '\0' == *s) ? NULL : s;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
inline ss_char_a_t const* c_str_ptr_null(ss_char_a_t *s)
{
    return c_str_ptr_null_a(s);
}

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
inline ss_char_w_t const* c_str_ptr_null_w(ss_char_w_t *s)
{
    return (NULL == s || L'\0' == *s) ? NULL : s;
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
inline ss_char_w_t const* c_str_ptr_null(ss_char_w_t *s)
{
    return c_str_ptr_null_w(s);
}
# endif /* _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST */

# if 0 /* TODO: Try and make this work. Sometime. Maybe ... */
/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_ptr_null
 *    function for any type for which c_str_ptr_a is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
template <ss_typename_param_k S>
inline ss_char_a_t const* c_str_ptr_null_a(S const& s)
{
    return stlsoft_ns_qual(c_str_ptr_null_a)(static_cast<ss_char_a_t const*>(stlsoft_ns_qual(c_str_ptr_a)(s))));
}
/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_ptr_null
 *    function for any type for which c_str_ptr_w is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>. If <code>s</code> is NULL, or has zero
 *   length, NULL is returned.
 */
template <ss_typename_param_k S>
inline ss_char_w_t const* c_str_ptr_null_w(S const& s)
{
    return stlsoft_ns_qual(c_str_ptr_null_w)(static_cast<ss_char_w_t const*>(stlsoft_ns_qual(c_str_ptr_w)(s))));
}
# endif /* 0 */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_size
 *
 * This can be applied to an expression, and the return value is the number of
 * bytes required to store the character string in the expression, NOT including
 * the null-terminating character.
 */

/* C-style ANSI string */
/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
STLSOFT_INLINE ss_size_t c_str_size_a(ss_char_a_t const* s)
{
    return c_str_len_a(s) * sizeof(ss_char_a_t);
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
inline ss_size_t c_str_size(ss_char_a_t const* s)
{
    return c_str_size_a(s);
}
#endif /* __cplusplus */

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
STLSOFT_INLINE ss_size_t c_str_size_w(ss_char_w_t const* s)
{
    return c_str_len_w(s) * sizeof(ss_char_w_t);
}

#ifdef __cplusplus
/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
inline ss_size_t c_str_size(ss_char_w_t const* s)
{
    return c_str_size_w(s);
}
#endif /* __cplusplus */

#ifdef __cplusplus

/* C-style ANSI string */
# ifdef _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST
/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
inline ss_size_t c_str_size_a(ss_char_a_t *s)
{
    return c_str_len(s) * sizeof(ss_char_a_t);
}

/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>char const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
inline ss_size_t c_str_size(ss_char_a_t *s)
{
    return c_str_size_a(s);
}

/* C-style Unicode string */
/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
inline ss_size_t c_str_size_w(ss_char_w_t *s)
{
    return c_str_len(s) * sizeof(ss_char_w_t);
}

/** \brief \ref group__concept__shim__string_access__c_str_size function
 *    for <code>wchar_t const*</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
inline ss_size_t c_str_size(ss_char_w_t *s)
{
    return c_str_size_w(s);
}
# endif /* _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST */

/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_size
 *    function for any type for which c_str_len_a is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
template <ss_typename_param_k S>
inline ss_size_t c_str_size_a(S const& s)
{
    return sizeof(ss_char_a_t) * c_str_len_a(s);
}

/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_size
 *    function for any type for which c_str_len_w is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
template <ss_typename_param_k S>
inline ss_size_t c_str_size_w(S const& s)
{
    return sizeof(ss_char_w_t) * c_str_len_w(s);
}

/** \brief Generic implementation of \ref group__concept__shim__string_access__c_str_size
 *    function for any type for which c_str_len is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The number of bytes required to represent the contents of the
 *   character string pointed to by <code>s</code>, excluding a
 *   nul-terminating character.
 */
template <ss_typename_param_k S>
inline ss_size_t c_str_size(S const& s)
{
    return sizeof(*c_str_ptr(s)) * c_str_len(s);
}

/** \brief Implementation of \ref group__concept__shim__string_access__c_str_size
 *    for trapping use of literal <code>0</code> or <code>NULL</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return The return type is <code>cannot_use_untyped_0_or_NULL_with_shims</code>,
 *   which serves to remind users, via compilation error message, that
 *   they're attempting something disallowed.
 */
inline cannot_use_untyped_0_or_NULL_with_shims c_str_size(int deny_literal_NULL);

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/c_string_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} /* namespace stlsoft */
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
