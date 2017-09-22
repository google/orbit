/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/algorithms/std/ext.hpp
 *
 * Purpose:     Extensions to standard algorithms.
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
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/algorithms/std/ext.hpp
 *
 * \brief [C++ only] Extensions to standard algorithms
 *   (\ref group__library__algorithms "Algorithms" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_EXT
#define STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_EXT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_EXT_MAJOR       3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_EXT_MINOR       2
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_EXT_REVISION    5
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_EXT_EDIT        72
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifdef STLSOFT_UNITTEST
# if defined(STLSOFT_COMPILER_IS_BORLAND)
#  include <memory.h>    // for memcpy()
# else /* ? compiler */
#  include <string.h>    // for memcpy()
# endif /* compiler */
#endif /* STLSOFT_UNITTEST */

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

/** \brief Copies elements from one range to another that satisfy a predicate
 *
 * \ingroup group__library__algorithms
 *
 * Copies each of the N elements in the source range [first, last) to the
 * destination range - of available length [dest, dest + N) - for which the
 * expression pred(*(i + n)) hold true, where i is the nth element in the
 * range
 *
 * \param first The start of the (unordered) sequence
 * \param last The (one past the) end point of the sequence
 * \param dest The output iterator to which the copies are written
 * \param pred The predicate used to determine whether the element in the input
 *   sequence is to be written to the output iterator
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        ,   ss_typename_param_k UP
        >
// [[synesis:function:algorithm: copy_if(T<I> first, T<I> last, T<O> dest, T<UP> pred)]]
inline O copy_if(I first, I last, O dest, UP pred)
{
    for(; first != last; ++first)
    {
        if(pred(*first))
        {
            *dest++ = *first;
        }
    }

    return dest;
}

/** \brief Applies the function to all items in a range that satisfy a predicate
 *
 * \ingroup group__library__algorithms
 *
 * Applies the function f to each of the N elements in the source range [first, last)
 * for which the expression pred(*(i + n)) hold true, where i is the nth element in the
 * range
 *
 * \param first The start of the (unordered) sequence
 * \param last The (one past the) end point of the sequence
 * \param func The function type to be applied to each selected element
 * \param pred The predicate used to determine whether the function is to be applied to
 *   the element in the input sequence
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k UF
        ,   ss_typename_param_k UP
        >
// [[synesis:function:algorithm: for_each_if(T<I> first, T<I> last, T<UF> func, T<UP> pred)]]
inline UF for_each_if(I first, I last, UF func, UP pred)
{
    for(; first != last; ++first)
    {
        if(pred(*first))
        {
            func(*first);
        }
    }

    return func;
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template<   ss_typename_param_k I
        ,   ss_typename_param_k UF
        ,   ss_typename_param_k UP
        >
// [[synesis:function:algorithm: for_each_ifnot(T<I> first, T<I> last, T<UF> func, T<UP> pred)]]
inline UF for_each_ifnot(I first, I last, UF func, UP pred)
{
    for(; first != last; ++first)
    {
        if(!pred(*first))
        {
            func(*first);
        }
    }

    return func;
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */




/** \brief Sets the value of all items in a range that satisfy a predicate.
 *
 * \ingroup group__library__algorithms
 *
 * Assigns the value v to each of the N elements in the range [first, last) for
 * which the expression pred(*(i + n)) hold true, where i is the nth element in
 * the range
 *
 * \param first The start of the sequence
 * \param last The last of the sequence
 * \param value The value to be assigned to each selected element
 * \param pred The predicate used to determine whether the value is to be assigned
 *   to the element in the input sequence
 */
template<   ss_typename_param_k O
        ,   ss_typename_param_k V
        ,   ss_typename_param_k UP
        >
// [[synesis:function:algorithm: fill_if(T<O> first, T<O> last, T<V> const& value, T<UP> pred)]]
inline void fill_if(O first, O last, V const& value, UP pred)
{
    for(; first != last; ++first)
    {
        if(pred(*first))
        {
            *first = value;
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/ext_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_EXT */

/* ///////////////////////////// end of file //////////////////////////// */
