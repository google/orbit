/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/std/iterator_generators.hpp (derived in part from stlsoft_iterator.h)
 *
 * Purpose:     Iterator generator classes, and helper macros.
 *
 * Created:     2nd January 2000
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2000-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/std/iterator_generators.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::reverse_iterator_generator
 *   and stlsoft::const_reverse_iterator_generator iterator
 *   \ref group__pattern__type_generator "generator" templates
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS
#define STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS_MAJOR     3
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS_MINOR     3
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS_REVISION  1
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS_EDIT      93
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Generator classes
 */

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT

template<   ss_typename_param_k I   /* iterator */
        ,   ss_typename_param_k V   /* value type */
        ,   ss_typename_param_k R   /* reference */
        ,   ss_typename_param_k P   /* pointer */
        ,   ss_typename_param_k D   /* distance type */
        >
struct reverse_iterator_generator
{
public:
#if defined(STLSOFT_COMPILER_IS_COMO)
    typedef std::reverse_iterator<I>                                                            type;
#elif 0
    typedef reverse_iterator_base<I, V, R, P, D>                                                type;
#else /* ? compiler */
    typedef ss_typename_type_k reverse_iterator_base<I, V, R, P, D>::parent_class_type          type;
#endif /* compiler */
};

template<   ss_typename_param_k I   /* iterator */
        ,   ss_typename_param_k V   /* value type */
        ,   ss_typename_param_k R   /* reference */
        ,   ss_typename_param_k P   /* pointer */
        ,   ss_typename_param_k D   /* distance type */
        >
struct const_reverse_iterator_generator
{
public:
#if defined(STLSOFT_COMPILER_IS_COMO)
    typedef std::reverse_iterator<I>                                                            type;
#elif 0
    typedef const_reverse_iterator_base<I, V, R, P, D>                                          type;
#else /* ? compiler */
    typedef ss_typename_type_k const_reverse_iterator_base<I, V, R, P, D>::parent_class_type    type;
#endif /* compiler */
};

#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS */

/* ///////////////////////////// end of file //////////////////////////// */
