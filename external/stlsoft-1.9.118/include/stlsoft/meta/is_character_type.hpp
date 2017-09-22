/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/is_character_type.hpp (originally MTBase.h, ::SynesisStl)
 *
 * Purpose:     is_character_type meta class.
 *
 * Created:     19th November 1998
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


/** \file stlsoft/meta/is_character_type.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::is_character_type meta class
 *  template
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE
#define STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_IS_CHARACTER_TYPE_MAJOR       1
# define STLSOFT_VER_STLSOFT_META_HPP_IS_CHARACTER_TYPE_MINOR       0
# define STLSOFT_VER_STLSOFT_META_HPP_IS_CHARACTER_TYPE_REVISION    2
# define STLSOFT_VER_STLSOFT_META_HPP_IS_CHARACTER_TYPE_EDIT        8
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#ifndef STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_
# include <stlsoft/meta/util/meta_.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_ */

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

/** \brief Traits type used to determine whether the given type is a
 *    character type (i.e. <code>char</code> or <code>wchar_t</code>).
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct is_character_type
{
    enum { value = 0 };

    typedef no_type     type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_character_type, char, 1, yes_type)
STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(is_character_type, wchar_t, 1, yes_type)

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief Obsolete form of \link stlsoft::is_character_type is_character_type\endlink.
 *
 * \ingroup group__library__meta
 *
 * \deprecated Use \link stlsoft::is_character_type is_character_type\endlink instead.
 */
template <ss_typename_param_k T>
struct is_char_type
{
    enum { value = is_character_type<T>::value };
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/is_character_type_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE */

/* ///////////////////////////// end of file //////////////////////////// */
