/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/select_first_type_if.hpp (originally MTBase.h, ::SynesisStl)
 *
 * Purpose:     Compile-time if component.
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


/** \file stlsoft/meta/select_first_type_if.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::select_first_type_if
 *  meta if component
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF
#define STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF_MAJOR       3
# define STLSOFT_VER_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF_MINOR       18
# define STLSOFT_VER_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF_REVISION    2
# define STLSOFT_VER_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF_EDIT        122
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC:   _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */

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

#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

/** \brief Compile-time if component
 *
 * \ingroup group__library__meta
 *
 * This template provides compile-time type selection between the two types
 * specified in its first two parameters, based on a (compile-time) boolean
 * value specified as its third parameter. If the third parameter evaluates
 * to non-zero, then the member type \c type is defined to be equivalent to
 * the first type, otherwise it is defined to be equivalent to the second.
 */
template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        ,   bool                B //!< Selects T1
        >
struct select_first_type_if
{
    typedef T1          type;   //!< The selected type
};

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        >
struct select_first_type_if<T1, T2, false>
{
    typedef T2          type;
};

// Obsolete
template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        ,   bool                B //!< Selects T1
        >
struct select_first_type
{
    typedef T1          type;   //!< The
};

template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        >
struct select_first_type<T1, T2, false>
{
    typedef T2          type;
};

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        ,   bool                B //!< Selects T1 if true. T2 otherwise
        >
struct select_first_type_if
{
private:
    template <bool>
    struct if_
    {
        typedef T2      type;
    };
    STLSOFT_TEMPLATE_SPECIALISATION
    struct if_<true>
    {
        typedef T1      type;
    };

public:
    typedef ss_typename_type_k if_<B>::type type;   //!< The selected type
};

#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF */

/* ///////////////////////////// end of file //////////////////////////// */
