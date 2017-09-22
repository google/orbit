/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/algorithms/deprecated.hpp
 *
 * Purpose:     Deprecated algorithms.
 *
 * Created:     17th January 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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
 * ////////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/algorithms/deprecated.hpp
 *
 * \brief [C++ only] Deprecated algorithms
 *   (\ref group__library__algorithms "Algorithms" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_DEPRECATED
#define STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_DEPRECATED

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_DEPRECATED_MAJOR    3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_DEPRECATED_MINOR    0
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_DEPRECATED_REVISION 4
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_DEPRECATED_EDIT     69
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_UNORDERED
# include <stlsoft/algorithms/unordered.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_UNORDERED */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_EXT
# include <stlsoft/algorithms/std/ext.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_EXT */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT
# include <stlsoft/algorithms/std/alt.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Algorithms
 */

/** \brief Counts the number of items in the sequence which the predicate is true.
 *
 * \ingroup group__library__algorithms
 *
 * \note This function is identical in semantics to std::count_if(). If you are
 * compiling in the context of a standard compliant library, you should prefer
 * std::count_if().
 *
 * \param first The start of the range to count
 * \param last The end of the range to count
 * \param pred The predicate
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k P
        >
// [[synesis:function:algorithm: for_each_count_success(T<I> first, T<I> last, T<UP> pred)]]
inline ss_size_t for_each_count_success(I first, I last, P pred)
{
    return std_count_if(first, last, pred);
}


/** \brief Sets the value of all items in the sequence.
 *
 * \ingroup group__library__algorithms
 *
 * \note This function is identical in semantics to std::fill(), except that
 * it returns the value. If you are compiling in the context of a standard
 * compliant library, and do not need the value returned, you should prefer
 * std::fill().
 *
 * \param begin The start of the sequence
 * \param end The end of the sequence
 * \param value The value to be applied to item N, for each N in [begin, end)
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k V
        >
// [[synesis:function:algorithm: for_each_set_value(T<I> first, T<I> last, T<V> value)]]
inline V const& for_each_set_value(I begin, I end, V const& value)
{
    std_fill(begin, end);

    return value;
}


/** \brief Sets the value of all items in the sequence.
 *
 * \ingroup group__library__algorithms
 *
 * \deprecated This is the old name for fill_if().
 *
 * \param begin The start of the sequence
 * \param end The end of the sequence
 * \param value The value to be applied to item N, for each N in [begin, end), when <code>pred(*(begin + N))</code> evaluates non-zero
 * \param pred The predicate that determines whether the value is to be modified
 */
// [[synesis:function:algorithm: for_each_set_value_if(T<I> first, T<I> last, T<V> value, T<UP> pred)]]
template<   ss_typename_param_k O
        ,   ss_typename_param_k V
        ,   ss_typename_param_k P
        >
inline V const& for_each_set_value_if(O begin, O end, V const& value, P pred)
{
    fill_if(begin, end, value, pred);

    return value;
}


#if 0
/** \brief This algorithm removes duplicate entries from unordered sequences. It
 * necessarily runs in O(n) time, since it must do a bubble-like double
 * pass on the sequence (in order to work with unordered sequences).
 *
 * \ingroup group__library__algorithms
 *
 * \deprecated This is the old name for unordered_unique().
 *
 * \param container The container
 * \param pred The predicate
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k P
        >
// [[synesis:function:algorithm: remove_duplicates_from_unordered_sequence(T<C> container, T<UP> pred)]]
inline void remove_duplicates_from_unordered_sequence(C &container, P pred)
{
//  unordered_unique(container.begin(), container.end()
    static_assert(0);
}
#endif /* 0 */


////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/deprecated_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_DEPRECATED */

/* ///////////////////////////// end of file //////////////////////////// */
