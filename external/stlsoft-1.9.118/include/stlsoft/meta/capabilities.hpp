/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/capabilities.hpp
 *
 * Purpose:     Pre-processor abilities for the meta library.
 *
 * Created:     5th March 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/meta/capabilities.hpp
 *
 * \brief [C++ only] Pre-processor discrimination of meta programming
 *   support
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
#define STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_CAPABILITIES_MAJOR    1
# define STLSOFT_VER_STLSOFT_META_HPP_CAPABILITIES_MINOR    1
# define STLSOFT_VER_STLSOFT_META_HPP_CAPABILITIES_REVISION 1
# define STLSOFT_VER_STLSOFT_META_HPP_CAPABILITIES_EDIT     11
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * Capabilities
 */

/** \def STLSOFT_META_HAS_IS_SAME_TYPE
 *
 * \ingroup group__library__meta
 *
 * \brief If defined, this indicates that the <code>stlsoft::is_same_type</code>
 * is supported <code>and stlsoft/meta/is_same_type.hpp</code> can be included.
 */

#ifdef STLSOFT_META_HAS_IS_SAME_TYPE
# undef STLSOFT_META_HAS_IS_SAME_TYPE
#endif /* STLSOFT_META_HAS_IS_SAME_TYPE */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
    defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
  // If partial template specialisation is supported, then is_same_type is
  // supported ...
# define STLSOFT_META_HAS_IS_SAME_TYPE
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1200
  // . . . except if it's Visual C++ 5.0 or earlier.
#  define STLSOFT_META_HAS_IS_SAME_TYPE
# endif /* compiler */
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */


/** \def STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
 *
 * \ingroup group__library__meta
 *
 * \brief If defined, this indicates that the <code>stlsoft::select_first_type_if</code>
 * is supported <code>and stlsoft/meta/select_first_type_if.hpp</code> can be included.
 */

#ifdef STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
# undef STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
#endif /* STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF */

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
  // If partial template specialisation is supported, then select_first_type_if is
  // supported ...
# define STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1200
  // . . . except if it's Visual C++ 5.0 or earlier.
#  define STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
# endif /* compiler */
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */


/** \def STLSOFT_META_HAS_MEMBER_TYPE_DETECTION
 *
 * \ingroup group__library__meta
 *
 * \brief If defined, this indicates that the member type detection is
 *   supported.
 */

#ifdef STLSOFT_META_HAS_MEMBER_TYPE_DETECTION
# undef STLSOFT_META_HAS_MEMBER_TYPE_DETECTION
#endif /* STLSOFT_META_HAS_MEMBER_TYPE_DETECTION */

#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
# undef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

#if !defined(STLSOFT_COMPILER_IS_BORLAND) && \
    (   !defined(STLSOFT_COMPILER_IS_DMC) || \
        __DMC__ >= 0x0845) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER >= 1310) && \
    !defined(STLSOFT_COMPILER_IS_VECTORC) && \
    !defined(STLSOFT_COMPILER_IS_WATCOM)

# define STLSOFT_META_HAS_MEMBER_TYPE_DETECTION
# define STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED

#endif /* compiler */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */

/* ///////////////////////////// end of file //////////////////////////// */
