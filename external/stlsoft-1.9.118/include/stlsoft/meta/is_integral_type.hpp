/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/is_integral_type.hpp (originally MTBase.h, ::SynesisStl)
 *
 * Purpose:     Tests whether a type is integral.
 *
 * Created:     19th November 1998
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/meta/is_integral_type.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::is_integral_type meta class
 *  template
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
#define STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_IS_INTEGRAL_TYPE_MAJOR    4
# define STLSOFT_VER_STLSOFT_META_HPP_IS_INTEGRAL_TYPE_MINOR    1
# define STLSOFT_VER_STLSOFT_META_HPP_IS_INTEGRAL_TYPE_REVISION 2
# define STLSOFT_VER_STLSOFT_META_HPP_IS_INTEGRAL_TYPE_EDIT     127
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_
# include <stlsoft/meta/util/meta_.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_ */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Traits type used to determine whether the given type is an
 *    integral type.
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct is_integral_type
{
    enum { value = 0 };

    typedef no_type     type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_sint8_t, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_uint8_t, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_sint16_t, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_uint16_t, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_sint32_t, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_uint32_t, 1, yes_type)
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_sint64_t, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_uint64_t, 1, yes_type)
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

#ifdef STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, signed char, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, unsigned char, 1, yes_type)
#endif /* STLSOFT_CF_CHAR_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, signed short, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, unsigned short, 1, yes_type)
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, signed int, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, unsigned int, 1, yes_type)
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, signed long, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, unsigned long, 1, yes_type)
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_bool_t, 1, yes_type)
#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_char_a_t, 1, yes_type)
#ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_integral_type, ss_char_w_t, 1, yes_type)
#endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */

/* ///////////////////////////// end of file //////////////////////////// */
