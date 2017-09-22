/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/access/string/std/basic_string.hpp
 *
 * Purpose:     Contains the c_str_ptr, c_str_ptr_null, c_str_len, and
 *              c_str_size accessors.
 *
 * Created:     16th January 2002
 * Updated:     10th August 2009
 *
 * Thanks to:   Robert Kreger for spotting a defect in the discrimination of
 *              wide character support on GCC 3.3.3.
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


/** \file stlsoft/shims/access/string/std/basic_string.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   std::basic_string (and its related compiler/library-dependent forms)
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING
#define STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING_MAJOR     4
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING_MINOR     0
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING_REVISION  3
# define STLSOFT_VER_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING_EDIT      92
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

#ifdef STLSOFT_CF_STRING_ACCESS_USE_std_char_traits
# undef STLSOFT_CF_STRING_ACCESS_USE_std_char_traits
#endif /* STLSOFT_CF_STRING_ACCESS_USE_std_char_traits */

/* No currently supported Watcom version can handle std::string. */
#ifdef STLSOFT_COMPILER_IS_WATCOM
# define STLSOFT_STRING_ACCESS_NO_STD_STRING
#endif /* compiler */

/* <string> is included for two reasons:
 *
 * (i) for std::string
 * (ii) for std::char_traits
 *
 * If STLSOFT_STRING_ACCESS_NO_STD_STRING is defined, then <string> is not
 * included, and STLSOFT_CF_STRING_ACCESS_USE_std_char_traits is not
 * defined.
 *
 * If _STLSOFT_NO_NAMESPACE is defined then
 * STLSOFT_CF_STRING_ACCESS_USE_std_char_traits is not defined.
 *
 * If STLSOFT_CF_std_char_traits_AVAILABLE is not defined then
 * STLSOFT_CF_STRING_ACCESS_USE_std_char_traits is not defined.
 *
 */

#ifdef _STLSOFT_STRING_ACCESS_NO_STD_STRING
# define STLSOFT_STRING_ACCESS_NO_STD_STRING
#endif /* _STLSOFT_STRING_ACCESS_NO_STD_STRING */

#if defined(STLSOFT_STRING_ACCESS_NO_STD_STRING)
# undef STLSOFT_CF_STRING_ACCESS_USE_std_char_traits
#elif defined(_STLSOFT_NO_NAMESPACE)
# undef STLSOFT_CF_STRING_ACCESS_USE_std_char_traits
#elif !defined(STLSOFT_CF_std_char_traits_AVAILABLE)
# undef STLSOFT_CF_STRING_ACCESS_USE_std_char_traits
#else /* ? Use char_traits? */
# define STLSOFT_CF_STRING_ACCESS_USE_std_char_traits
#endif /* Use char_traits? */

/* Are we going to cater for std::(w)string? */
#ifndef STLSOFT_STRING_ACCESS_NO_STD_STRING
# include <string>
#endif /* STLSOFT_STRING_ACCESS_NO_STD_STRING */

/* Include stlsoft_char_traits if not using std::string, or std::char_traits
 * is not available.
 */
#if !defined(STLSOFT_CF_STRING_ACCESS_USE_std_char_traits)
# ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
#  include <stlsoft/string/char_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#endif /* !STLSOFT_CF_STRING_ACCESS_USE_std_char_traits */

#if defined(STLSOFT_COMPILER_IS_GCC)
# include <wchar.h>
#endif /* compiler */

#ifdef STLSOFT_UNITTEST
# include <stdio.h>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Pre-processor control
 *
 * By default, conversions from non-const strings, or rather from pointers to
 * non-const characters, are not allowed, since the implied semantics for a
 * pointer-to-const character representing a null-terminated string are stronger
 * than those for a pointer-to-non-const character.
 *
 * However, you can override this by defining the symbol
 * _STLSOFT_STRING_ACCESS_ALLOW_NON_CONST, which will then treat both the types
 * (in fact all four types: char*; char const*; wchar_t*; wchar_t const*) as
 * representing null-terminated strings.
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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef STLSOFT_STRING_ACCESS_NO_STD_STRING
# if defined(STLSOFT_COMPILER_IS_GCC) && \
     (   __GNUC__ < 3 /* TODO: This is probably not needed now, so may remove it in a future version ... */ || \
         !defined(_GLIBCPP_USE_WCHAR_T)) // Thanks to Robert Kreger for suggesting this fix for GCC 3.3.3 on HP UX
  typedef stlsoft_ns_qual_std(basic_string)<ss_char_w_t>    stlsoft_wstring_t_;
# else /* ? GCC */
  typedef stlsoft_ns_qual_std(wstring)                      stlsoft_wstring_t_;
# endif /* STLSOFT_COMPILER_IS_GCC) && __GNUC__ < 3 */
#endif /* STLSOFT_STRING_ACCESS_NO_STD_STRING */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* std::basic_string */
#ifndef STLSOFT_STRING_ACCESS_NO_STD_STRING
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100
  /* Have to implement in terms of explicit specialisations in global
   * namespace for MSVC++ <5.
   */

inline ss_char_a_t const* c_str_data_a(string const& s)
{
    return s.data();
}

inline ss_char_a_t const* c_str_data(string const& s)
{
    return c_str_data_a(s);
}

inline ss_char_w_t const* c_str_data_w(wstring const& s)
{
    return s.data();
}

inline ss_char_w_t const* c_str_data(wstring const& s)
{
    return c_str_data_w(s);
}

# else /* ? compiler */
  /* Compilers other than MSVC++ <5.
   */

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>std::string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_data_a(stlsoft_ns_qual_std(string) const& s)
{
    return s.data();
}

#  if !defined(STLSOFT_COMPILER_IS_GCC) || \
      !(__GNUC__ < 3)
/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for <code>std::wstring</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_data_w(stlsoft_wstring_t_ const& s)
{
    return s.data();
}
#  endif /* compiler */

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>char</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>char</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_char_a_t const* c_str_data_a(stlsoft_ns_qual_std(basic_string)<ss_char_a_t, T, A> const& s)
{
    return s.data();
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>wchar_t</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style
 *   string of <code>wchar_t</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_char_w_t const* c_str_data_w(stlsoft_ns_qual_std(basic_string)<ss_char_w_t, T, A> const& s)
{
    return s.data();
}

/** \brief \ref group__concept__shim__string_access__c_str_data function
 *    for arbitrary specialisations of <code>std::basic_string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a C-style string.
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_data(stlsoft_ns_qual_std(basic_string)<C, T, A> const& s)
{
    return s.data();
}
# endif /* STLSOFT_COMPILER_IS_MSVC && _MSC_VER < 1100 */
#endif /* STLSOFT_STRING_ACCESS_NO_STD_STRING */

/* stlport::basic_string */
#if defined(_STLP_USE_NAMESPACES) && \
    defined(_STLP_USE_OWN_NAMESPACE) && \
    !defined(_STLP_REDEFINE_STD) && \
    (   !defined(_STLPORT_MAJOR) || \
        _STLPORT_MAJOR < 5)

inline ss_char_a_t const* c_str_data_a(stlport::string const& s)
{
    return s.data();
}

inline ss_char_w_t const* c_str_data_w(stlport::wstring const& s)
{
    return s.data();
}

template <ss_typename_param_k C>
inline C const* c_str_data(stlport::basic_string<C> const& s)
{
    return s.data();
}

#endif /* _STLP_USE_NAMESPACES && _STLP_USE_OWN_NAMESPACE */

#if 0
/** \brief Function template that provide generic implementations of
 *   c_str_data_a for any type for which c_str_ptr_a is defined.
 *
 * \ingroup group__shims__string_access
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k S>
inline ss_char_a_t const* c_str_data_a(S const& s)
{
    return stlsoft_ns_qual(c_str_data_a)(static_cast<ss_char_a_t const*>(stlsoft_ns_qual(c_str_ptr_a)(s))));
}
/** \brief Function template that provide generic implementations of
 *   c_str_data_w for any type for which c_str_ptr_w is defined.
 *
 * \ingroup group__shims__string_access
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k S>
inline ss_char_w_t const* c_str_data_w(S const& s)
{
    return stlsoft_ns_qual(c_str_data_w)(static_cast<ss_char_w_t const*>(stlsoft_ns_qual(c_str_ptr_w)(s))));
}
#endif /* 0 */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/* std::basic_string */
#ifndef STLSOFT_STRING_ACCESS_NO_STD_STRING
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100

inline ss_size_t c_str_len_a(string const& s)
{
    return s.length();
}

inline ss_size_t c_str_len(string const& s)
{
    return c_str_len_a(s);
}

inline ss_size_t c_str_len_w(wstring const& s)
{
    return s.length();
}

inline ss_size_t c_str_len(wstring const& s)
{
    return c_str_len_w(s);
}

# else /* ? compiler */

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>std::string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string <code>s</code>.
 */
inline ss_size_t c_str_len_a(stlsoft_ns_qual_std(string) const& s)
{
    return s.length();
}

#  if !defined(STLSOFT_COMPILER_IS_GCC) || \
      !(__GNUC__ < 3)
/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for <code>std::wstring</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in characters) of the string <code>s</code>.
 */
inline ss_size_t c_str_len_w(stlsoft_wstring_t_ const& s)
{
    return s.length();
}
#  endif /* compiler */

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>char</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in bytes) of the string <code>s</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_size_t c_str_len_a(stlsoft_ns_qual_std(basic_string)<ss_char_a_t, T, A> const& s)
{
    return s.length();
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>wchar_t</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length (in characters) of the string <code>s</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_size_t c_str_len_w(stlsoft_ns_qual_std(basic_string)<ss_char_w_t, T, A> const& s)
{
    return s.length();
}

/** \brief \ref group__concept__shim__string_access__c_str_len function
 *    for arbitrary specialisations of <code>std::basic_string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Length of the string <code>s</code>.
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_size_t c_str_len(stlsoft_ns_qual_std(basic_string)<C, T, A> const& s)
{
    return s.length();
}
# endif /* STLSOFT_COMPILER_IS_MSVC && _MSC_VER < 1100 */
#endif /* STLSOFT_STRING_ACCESS_NO_STD_STRING */

/* stlport::basic_string */
#if defined(_STLP_USE_NAMESPACES) && \
    defined(_STLP_USE_OWN_NAMESPACE) && \
    !defined(_STLP_REDEFINE_STD) && \
    (   !defined(_STLPORT_MAJOR) || \
        _STLPORT_MAJOR < 5)

inline ss_size_t c_str_len_a(stlport::string const& s)
{
    return s.length();
}

inline ss_size_t c_str_len_w(stlport::wstring const& s)
{
    return s.length();
}

template <ss_typename_param_k C>
inline ss_size_t c_str_len(stlport::basic_string<C> const& s)
{
    return s.length();
}

#endif /* _STLP_USE_NAMESPACES && _STLP_USE_OWN_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* std::basic_string */
#ifndef STLSOFT_STRING_ACCESS_NO_STD_STRING
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100

inline ss_char_a_t const* c_str_ptr_a(string const& s)
{
    return s.c_str();
}

inline ss_char_a_t const* c_str_ptr(string const& s)
{
    return c_str_ptr_a(s);
}

inline ss_char_w_t const* c_str_ptr_w(wstring const& s)
{
    return s.c_str();
}

inline ss_char_w_t const* c_str_ptr(wstring const& s)
{
    return c_str_ptr_w(s);
}

# else /* ? compiler */
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>std::string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr_a(stlsoft_ns_qual_std(string) const& s)
{
    return s.c_str();
}

#  if !defined(STLSOFT_COMPILER_IS_GCC) || \
      !(__GNUC__ < 3)
/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for <code>std::wstring</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_ptr_w(stlsoft_wstring_t_ const& s)
{
    return s.c_str();
}
#  endif /* compiler */

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>char</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_char_a_t const* c_str_ptr_a(stlsoft_ns_qual_std(basic_string)<ss_char_a_t, T, A> const& s)
{
    return s.c_str();
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>wchar_t</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_char_w_t const* c_str_ptr_w(stlsoft_ns_qual_std(basic_string)<ss_char_w_t, T, A> const& s)
{
    return s.c_str();
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr function
 *    for arbitrary specialisations of <code>std::basic_string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return None-NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr(stlsoft_ns_qual_std(basic_string)<C, T, A> const& s)
{
    return s.c_str();
}
# endif /* STLSOFT_COMPILER_IS_MSVC && _MSC_VER < 1100 */
#endif /* STLSOFT_STRING_ACCESS_NO_STD_STRING */

/* stlport::basic_string */
#if defined(_STLP_USE_NAMESPACES) && \
    defined(_STLP_USE_OWN_NAMESPACE) && \
    !defined(_STLP_REDEFINE_STD) && \
    (   !defined(_STLPORT_MAJOR) || \
        _STLPORT_MAJOR < 5)

inline ss_char_a_t const* c_str_ptr_a(stlport::string const& s)
{
    return s.c_str();
}

inline ss_char_w_t const* c_str_ptr_w(stlport::wstring const& s)
{
    return s.c_str();
}

template <ss_typename_param_k C>
inline C const* c_str_ptr(stlport::basic_string<C> const& s)
{
    return s.c_str();
}

#endif /* _STLP_USE_NAMESPACES && _STLP_USE_OWN_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/* std::basic_string */
#ifndef STLSOFT_STRING_ACCESS_NO_STD_STRING
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100

inline ss_char_a_t const* c_str_ptr_null_a(string const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

inline ss_char_a_t const* c_str_ptr_null(string const& s)
{
    return c_str_ptr_null_a(s);
}

inline ss_char_w_t const* c_str_ptr_null_w(wstring const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

inline ss_char_w_t const* c_str_ptr_null(wstring const& s)
{
    return c_str_ptr_null_w(s);
}

# else /* ? compiler */
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>std::string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
inline ss_char_a_t const* c_str_ptr_null_a(stlsoft_ns_qual_std(string) const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

#  if !defined(STLSOFT_COMPILER_IS_GCC) || \
      !(__GNUC__ < 3)
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for <code>std::wstring</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
inline ss_char_w_t const* c_str_ptr_null_w(stlsoft_wstring_t_ const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}
#  endif /* compiler */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>char</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>char</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_char_a_t const* c_str_ptr_null_a(stlsoft_ns_qual_std(basic_string)<ss_char_a_t, T, A> const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for specialisations of <code>std::basic_string</code> with
 *    <code>wchar_t</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string of <code>wchar_t</code>.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_char_w_t const* c_str_ptr_null_w(stlsoft_ns_qual_std(basic_string)<ss_char_w_t, T, A> const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null function
 *    for arbitrary specialisations of <code>std::basic_string</code>.
 *
 * \ingroup group__concept__shim__string_access
 *
 * \return Possibly NULL, non-mutating pointer to a nul-terminated C-style
 *   string.
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr_null(stlsoft_ns_qual_std(basic_string)<C, T, A> const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

# endif /* STLSOFT_COMPILER_IS_MSVC && _MSC_VER < 1100 */
#endif /* STLSOFT_STRING_ACCESS_NO_STD_STRING */

/* stlport::basic_string */
#if defined(_STLP_USE_NAMESPACES) && \
    defined(_STLP_USE_OWN_NAMESPACE) && \
    !defined(_STLP_REDEFINE_STD) && \
    (   !defined(_STLPORT_MAJOR) || \
        _STLPORT_MAJOR < 5)

inline ss_char_a_t const* c_str_ptr_null_a(stlport::string const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

inline ss_char_w_t const* c_str_ptr_null_w(stlport::wstring const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

template <ss_typename_param_k C>
inline C const* c_str_ptr_null(stlport::basic_string<C> const& s)
{
    return (0 == s.length()) ? 0 : s.c_str();
}

#endif /* _STLP_USE_NAMESPACES && _STLP_USE_OWN_NAMESPACE */

#if 0
/** \brief Function template that provide generic implementations of
 *   c_str_ptr_null_a for any type for which c_str_ptr_a is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k S>
inline ss_char_a_t const* c_str_ptr_null_a(S const& s)
{
    return stlsoft_ns_qual(c_str_ptr_null_a)(static_cast<ss_char_a_t const*>(stlsoft_ns_qual(c_str_ptr_a)(s))));
}
/** \brief Function template that provide generic implementations of
 *   c_str_ptr_null_w for any type for which c_str_ptr_w is defined.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k S>
inline ss_char_w_t const* c_str_ptr_null_w(S const& s)
{
    return stlsoft_ns_qual(c_str_ptr_null_w)(static_cast<ss_char_w_t const*>(stlsoft_ns_qual(c_str_ptr_w)(s))));
}
#endif /* 0 */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/basic_string_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
