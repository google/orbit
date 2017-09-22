/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/transform_filter_iterator.hpp
 *
 * Purpose:     A combination of the transform_iterator and the filter_iterator.
 *
 * Created:     2nd January 2006
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


/** \file stlsoft/iterators/transform_filter_iterator.hpp
 *
 * \brief [C++ only] Functions for simultaneous transformation and filtering
 *   of iterator ranges
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR_MAJOR     1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR_MINOR     0
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR_REVISION  3
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR_EDIT      11
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1310
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR
# include <stlsoft/iterators/transform_iterator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR */
#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR
# include <stlsoft/iterators/filter_iterator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
# include <stlsoft/meta/is_integral_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

/** \brief Creator function for transform_iterator + filter_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param from The iterator marking the start of the range to transform and filter
 * \param to The iterator marking (one past) the end of the range to transform and filter
 * \param pr The predicate used to filter the given range
 * \param fn The predicate used to transform values of the filtered range
 *
 * \return An instance of the specialisation transform_iterator&lt;filter_iterator&lt;I, FP>, TF>
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k TF
        ,   ss_typename_param_k FP
        >
inline transform_iterator<filter_iterator<I, FP>, TF> make_transform_filter_iterator(I from, I to, TF fn, FP pr)
{
    typedef ss_typename_param_k FP::result_type  predicate_result_t;

    // If this fires, you've either specified the transforming function and
    // the filtering predicate in the wrong order, or your predicate has a
    // non-integral return type (which would be decidedly odd).
    STLSOFT_STATIC_ASSERT(0 != is_integral_type<predicate_result_t>::value);

    return transformer(filter(from, to, pr), fn);
}

/** \brief Creator function for transform_iterator + filter_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param from The iterator marking the start of the range to transform and filter
 * \param to The iterator marking (one past) the end of the range to transform and filter
 * \param pr The predicate used to filter the given range
 * \param fn The predicate used to transform values of the filtered range
 *
 * \return An instance of the specialisation transform_iterator&lt;filter_iterator&lt;I, FP>, TF>
 *
 * \note Short-hand for make_filter_iterator()
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k TF
        ,   ss_typename_param_k FP
        >
inline transform_iterator<filter_iterator<I, FP>, TF> transform_filter(I from, I to, TF fn, FP pr)
{
    return make_transform_filter_iterator(from, to, fn, pr);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
//# include "./unittest/transform_filter_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_FILTER_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
