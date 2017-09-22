/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/algorithms/bounded.hpp
 *
 * Purpose:     Bounded algorithms.
 *
 * Created:     23rd October 2004
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
 * ////////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/algorithms/bounded.hpp
 *
 * \brief [C++ only] Bounded algorithms
 *   (\ref group__library__algorithms "Algorithms" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_BOUNDED
#define STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_BOUNDED

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_BOUNDED_MAJOR       2
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_BOUNDED_MINOR       1
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_BOUNDED_REVISION    1
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_BOUNDED_EDIT        22
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHM_STD_HPP_ALT
# include <stlsoft/algorithms/std/alt.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHM_STD_HPP_ALT */
#ifdef STLSOFT_CF_std_NAMESPACE
# include <iterator>        // for the iterator tags
# include <utility>         // for std::pair
#endif /* STLSOFT_CF_std_NAMESPACE */

#ifdef STLSOFT_UNITTEST
# include <string.h>        // for strcmp
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

/** \brief Copies N elements from the range <code>[src, src + n)</code> to
 *    the range <code>[dest, dest + n)</code>.
 *
 * \ingroup group__library__algorithms
 *
 * \param src The iterator copied from
 * \param n The number of elements copied
 * \param dest The iterator copied to
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        >
// [[synesis:function:algorithm: copy_n(T<I> src, size_t n, T<O> dest)]]
inline O copy_n(I src, ss_size_t n, O dest)
{
    for(; n != 0; ++src, ++dest, --n)
    {
        *dest = *src;
    }

    return dest;
}


/** \brief Copies N elements from the range <code>[src, src + n)</code> to
 *    the range <code>[dest, dest + n)</code>.
 *
 * \ingroup group__library__algorithms
 *
 * \param src The iterator copied from
 * \param n The number of elements copied
 * \param oldValue The value to search for
 * \param newValue The value to replace with
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
// [[synesis:function:algorithm: replace_n(T<I> src, size_t n, T<T> const& oldValue, T<T> const& newValue)]]
inline void replace_n(I src, ss_size_t n, T const& oldValue, T const& newValue)
{
#if 0

    // NOTE: Why this is not valid!!!!!!!!

    std_replace(src, src + n, oldValue, newValue);
#else /* ? 0 */
    for(; 0 != n; ++src, --n)
    {
        if(oldValue == *src)
        {
            *src = newValue;
        }
    }
#endif /* 0 */
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/bounded_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_BOUNDED */

/* ///////////////////////////// end of file //////////////////////////// */
