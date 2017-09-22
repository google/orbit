/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/typefixer/mapped_type.hpp
 *
 * Purpose:     Definition of the fixer_mapped_type type.
 *
 * Created:     26th February 2005
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


/** \file stlsoft/meta/typefixer/mapped_type.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::typefixer::mapped_type
 *   typefixer class template
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_MAPPED_TYPE
#define STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_MAPPED_TYPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_MAPPED_TYPE_MAJOR       2
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_MAPPED_TYPE_MINOR       0
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_MAPPED_TYPE_REVISION    1
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_MAPPED_TYPE_EDIT        15
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER < 1310
[Incompatibilies-end]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
# error stlsoft/typefixer/mapped_type.hpp is only compatible with compilers that support partial template specialisation
#endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{

namespace typefixer
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Type-fixer for \c mapped_type member type
 *
 * \ingroup group__library__meta
 *
 * If type \c T has a \c mapped_type member type, then the type of
 * <code>fixer_mapped_type<T>::mapped_type</code> is
 * <code>T::mapped_type</code>, otherwise it is \c void
 */

template <ss_typename_param_k T, ss_bool_t>
struct fixer_mapped_type
{
    typedef ss_typename_type_k T::mapped_type   mapped_type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// When not present it uses void as a placeholder
template <ss_typename_param_k T>
struct fixer_mapped_type<T, false>
{
    typedef void                                mapped_type;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace typefixer

} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_MAPPED_TYPE */

/* ///////////////////////////// end of file //////////////////////////// */
