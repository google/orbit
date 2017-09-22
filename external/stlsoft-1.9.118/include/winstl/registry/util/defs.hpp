/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/util/defs.hpp
 *
 * Purpose:     Contains common type and feature discriminations for the Registry Library.
 *
 * Created:     19th January 2002
 * Updated:     3rd February 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2012, Matthew Wilson and Synesis Software
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


/** \file winstl/registry/util/defs.hpp
 *
 * \brief [C++ only] Common type and feature discriminations for
 *   the \ref group__library__windows_registry "Windows Registry" Library.
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS
#define WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_DEFS_MAJOR     4
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_DEFS_MINOR     0
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_DEFS_REVISION  1
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_DEFS_EDIT      57
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifdef _WINSTL_REG_NO_STD_STRING
# error _WINSTL_REG_NO_STD_STRING no longer supported. You should select _WINSTL_REG_STRING_USE_STD_STRING or _WINSTL_REG_STRING_USE_CUSTOM_STRING, or nothing (to use STLSoft simple_string)
#endif /* _WINSTL_REG_NO_STD_STRING */

/* Digital Mars seems to have trouble with basic_simple_string, so ...  */
#if 0 && \
    defined(STLSOFT_COMPILER_IS_DMC) && \
    !defined(_WINSTL_REG_STRING_USE_CUSTOM_STRING) && \
    __DMC__ <= 0x0834
# define _WINSTL_REG_STRING_USE_STD_STRING
#endif /* Digital Mars <= 0x0834 */

/* For some totally inexplicable reason VC 5 gets its namespace knickers in a twist if
 * we use simple_strings, so ...
 */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# define _WINSTL_REG_STRING_USE_STD_STRING
#endif /* compiler */

#if defined(_WINSTL_REG_STRING_USE_STD_STRING)
# include <string>
#elif defined(_WINSTL_REG_STRING_USE_CUSTOM_STRING)
# if !defined(_WINSTL_REG_STRING_CUSTOM_HEADER_FILE)
#  error You must define the header file (e.g. "#define _WINSTL_REG_STRING_CUSTOM_HEADER_FILE <mystring.h>") if you specify _WINSTL_REG_STRING_USE_CUSTOM_STRING
# else /* ? !_WINSTL_REG_STRING_CUSTOM_HEADER_FILE */
#  include _WINSTL_REG_STRING_CUSTOM_HEADER_FILE    // Your string class
# endif /* !_WINSTL_REG_STRING_CUSTOM_HEADER_FILE */
#else /* ? _WINSTL_REG_STRING_USE_CUSTOM_STRING */
# ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
#  include <stlsoft/string/simple_string.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#endif /* string type */

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
 * Constants
 */

/** \brief Number of characters in internal buffer of auto_buffer used throughout
 *   the Registry library.
 *
 * \ingroup group__library__windows_registry
 */
const ws_size_t CCH_REG_API_AUTO_BUFFER     =   512;

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * All the following typedefs may be overriden by use of the preprocessor.
 */

/** \def _WINSTL_REG_STRING_STRING_A_DEFINED
 *
 * \brief If defined, it indicates that the reg_string_a_t string type (for
 * ANSI character encoding) has been defined.
 */

/** \def _WINSTL_REG_STRING_STRING_W_DEFINED
 *
 * \brief If defined, it indicates that the reg_string_a_t string type (for
 * Unicode character encoding) has been defined.
 */

#if defined(_WINSTL_REG_STRING_USE_STD_STRING)

// ANSI

/** \brief For all compilers this is basic_string<ws_char_a_t> except Visual C++ 4.2
 * for which it is string
 *
 * \ingroup group__library__windows_registry
 */
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100
  typedef string                                            reg_string_a_t;
# else /* ? compiler */
  typedef winstl_ns_qual_std(basic_string)<ws_char_a_t>     reg_string_a_t;
# endif /* compiler */

# define _WINSTL_REG_STRING_STRING_A_DEFINED

// Unicode

/** \brief For all compilers this is basic_string<ws_char_w_t> except Visual C++ 4.2
 * for which it is wstring
 *
 * \ingroup group__library__windows_registry
 */
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100
  typedef wstring                                           reg_string_w_t;
# else /* ? compiler */
  typedef winstl_ns_qual_std(basic_string)<ws_char_w_t>     reg_string_w_t;
# endif /* compiler */

// TCHAR

/** \brief For all compilers this is basic_string<ws_char_w_t> except Visual C++ 4.2
 * for which it is wstring
 *
 * \ingroup group__library__windows_registry
 */
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1100
#  ifdef UNICODE
  typedef wstring                                           reg_string_t;
#  else /* ? UNICODE */
  typedef string                                            reg_string_t;
#  endif /* UNICODE */
# else /* ? compiler */
  typedef winstl_ns_qual_std(basic_string)<TCHAR>           reg_string_t;
# endif /* compiler */

# define _WINSTL_REG_STRING_STRING_W_DEFINED

#elif defined(_WINSTL_REG_STRING_USE_CUSTOM_STRING)

// ANSI

# ifndef _WINSTL_REG_STRING_STRING_A_DEFINED
#  error If using custom strings, must provide definition of reg_string_a_t type, and defined _WINSTL_REG_STRING_STRING_A_DEFINED
# endif /* !_WINSTL_REG_STRING_STRING_A_DEFINED */

// Unicode

# ifndef _WINSTL_REG_STRING_STRING_W_DEFINED
#  error If using custom strings, must provide definition of reg_string_w_t type, and defined _WINSTL_REG_STRING_STRING_W_DEFINED
# endif /* !_WINSTL_REG_STRING_STRING_W_DEFINED */

#else /* ? _WINSTL_REG_STRING_USE_STD_STRING */

// ANSI

 /// \brief String type for ANSI character encoding used by the Registry library.
 typedef stlsoft_ns_qual(basic_simple_string)<ws_char_a_t>  reg_string_a_t;

# define _WINSTL_REG_STRING_STRING_A_DEFINED

// Unicode

 /// \brief String type for Unicode character encoding used by the Registry library.
 typedef stlsoft_ns_qual(basic_simple_string)<ws_char_w_t>  reg_string_w_t;

// TCHAR

 /// \brief String type for TCHAR character encoding used by the Registry library.
 typedef stlsoft_ns_qual(basic_simple_string)<TCHAR>        reg_string_t;

# define _WINSTL_REG_STRING_STRING_W_DEFINED

#endif /* string type */


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifdef _STLSOFT_NO_NAMESPACES
struct registry_util
#else /* ? _STLSOFT_NO_NAMESPACES */
namespace registry_util
#endif /* _STLSOFT_NO_NAMESPACES */
{

    typedef reg_string_a_t          string_a_t;
    typedef reg_string_w_t          string_w_t;
    typedef reg_string_t            string_t;

    struct shared_handle;
    struct monitored_shared_handle;

#ifdef _STLSOFT_NO_NAMESPACES
}; // struct registry_util
#else /* ? _STLSOFT_NO_NAMESPACES */
} // namespace registry_util
#endif /* _STLSOFT_NO_NAMESPACES */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS */

/* ///////////////////////////// end of file //////////////////////////// */
