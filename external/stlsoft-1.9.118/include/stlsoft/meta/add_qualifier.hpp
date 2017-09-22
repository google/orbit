/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/add_qualifier.hpp
 *
 * Purpose:     Adds a const or volatile qualifier to a type.
 *
 * Created:     30th December 2005
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


/** \file stlsoft/meta/add_qualifier.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::add_const,
 *   stlsoft::add_volatile, stlsoft::add_const_ref and
 *   stlsoft::add_volatile_ref meta-programming type adjuster components
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_ADD_QUALIFIER
#define STLSOFT_INCL_STLSOFT_META_HPP_ADD_QUALIFIER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_ADD_QUALIFIER_MAJOR       1
# define STLSOFT_VER_STLSOFT_META_HPP_ADD_QUALIFIER_MINOR       1
# define STLSOFT_VER_STLSOFT_META_HPP_ADD_QUALIFIER_REVISION    2
# define STLSOFT_VER_STLSOFT_META_HPP_ADD_QUALIFIER_EDIT        11
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

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

/** \brief Adds <code>const</code> qualifier to a type.
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct add_const
{
    typedef const T     type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
struct add_const<void>
{
    typedef void        type;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \brief Adds <code>volatile</code> qualifier to a type.
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct add_volatile
{
    typedef volatile T  type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
struct add_volatile<void>
{
    typedef void        type;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \brief Adds <code>const&amp;</code> qualifier to a type.
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct add_const_ref
{
    typedef T const&    type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
struct add_const_ref<void>
{
    typedef void        type;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \brief Adds <code>volatile&amp;</code> qualifier to a type.
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct add_volatile_ref
{
    typedef volatile T& type;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
struct add_volatile_ref<void>
{
    typedef void        type;
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_ADD_QUALIFIER */

/* ///////////////////////////// end of file //////////////////////////// */
