/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/typefixer/reference.hpp
 *
 * Purpose:     Definition of the fixer_reference and fixer_const_reference types.
 *
 * Created:     9th February 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/meta/typefixer/reference.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::typefixer::reference
 *   typefixer class template
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE
#define STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_HPP_REFERENCE_MAJOR     4
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_HPP_REFERENCE_MINOR     0
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_HPP_REFERENCE_REVISION  1
# define STLSOFT_VER_STLSOFT_META_TYPEFIXER_HPP_REFERENCE_EDIT      31
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
# error stlsoft/typefixer/reference.hpp is only compatible with compilers that support partial template specialisation
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

/** \brief Type-fixer for \c reference member type
 *
 * \ingroup group__library__meta
 *
 * If type \c T has a \c reference member type, then the type of
 * <code>fixer_reference<T>::reference</code> is
 * <code>T::reference</code>, otherwise it is \c void
 */

template <ss_typename_param_k T, ss_bool_t>
struct fixer_reference
{
    typedef ss_typename_type_k T::reference         reference;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// When not present it uses void as a placeholder
template <ss_typename_param_k T>
struct fixer_reference<T, false>
{
    typedef void                                    reference;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief Type-fixer for \c const_reference member type
 *
 * \ingroup group__library__meta
 *
 * If type \c T has a \c const_reference member type, then the type of
 * <code>fixer_const_reference<T>::const_reference</code> is
 * <code>T::const_reference</code>, otherwise it is \c void
 */

template <ss_typename_param_k T, ss_bool_t>
struct fixer_const_reference
{
    typedef ss_typename_type_k T::const_reference   const_reference;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// When not present it uses void as a placeholder
template <ss_typename_param_k T>
struct fixer_const_reference<T, false>
{
    typedef void                                    const_reference;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace typefixer

} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE */

/* ///////////////////////////// end of file //////////////////////////// */
