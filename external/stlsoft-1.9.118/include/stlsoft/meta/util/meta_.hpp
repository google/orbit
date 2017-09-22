/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/util/meta_.hpp (originally MTBase.h, ::SynesisStl)
 *
 * Purpose:     Basic meta programming constructs.
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


/** \file stlsoft/meta/util/meta_.hpp
 *
 * \brief [C++ only] Definition of basic meta programming constructs
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_
#define STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_UTIL_HPP_META__MAJOR      4
# define STLSOFT_VER_STLSOFT_META_UTIL_HPP_META__MINOR      1
# define STLSOFT_VER_STLSOFT_META_UTIL_HPP_META__REVISION   1
# define STLSOFT_VER_STLSOFT_META_UTIL_HPP_META__EDIT       128
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
 * Macros
 */

/** \brief \def STLSOFT_GEN_TRAIT_SPECIALISATION
 * \ingroup group__project__stlsoft__code_modification_macros
 *
 * \brief Used to define a specialisation of a traits type
 *
 */
#define STLSOFT_GEN_TRAIT_SPECIALISATION(TR, T, V)  \
                                                    \
    STLSOFT_TEMPLATE_SPECIALISATION                 \
    struct TR<T>                                    \
    {                                               \
        enum { value = V };                         \
    };


/** \brief \def STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE
 * \ingroup group__project__stlsoft__code_modification_macros
 *
 * \brief Used to define a specialisation of a traits type that contains a member type
 *
 */
#define STLSOFT_GEN_TRAIT_SPECIALISATION_WITH_TYPE(TR, T, V, MT)    \
                                                                    \
    STLSOFT_TEMPLATE_SPECIALISATION                                 \
    struct TR<T>                                                    \
    {                                                               \
        enum { value = V };                                         \
                                                                    \
        typedef MT type;                                            \
    };

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs and basic support types
 */

#if (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER != 1200) || \
    defined(STLSOFT_META_ALLOW_INT_TO_TYPE)

/** \brief Converts compile time constants to type.
 *
 * \ingroup group__library__meta
 */
template <int N>
struct int_to_type
{};

#endif /* _MSC_VER == 1200 */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_ */

/* ///////////////////////////// end of file //////////////////////////// */
