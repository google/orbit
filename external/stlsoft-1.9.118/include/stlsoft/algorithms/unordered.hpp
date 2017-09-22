/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/algorithms/unordered.hpp
 *
 * Purpose:     Algorithms for manipulating unordered sequences.
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


/** \file stlsoft/algorithms/unordered.hpp
 *
 * \brief [C++ only] Algorithms for manipulating unordered sequences
 *   (\ref group__library__algorithms "Algorithms" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_UNORDERED
#define STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_UNORDERED

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_UNORDERED_MAJOR     3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_UNORDERED_MINOR     3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_UNORDERED_REVISION  2
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_UNORDERED_EDIT      71
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT
# include <stlsoft/algorithms/std/alt.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT */
#ifdef STLSOFT_CF_std_NAMESPACE
# include <functional>
#endif /* STLSOFT_CF_std_NAMESPACE */

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

#ifdef STLSOFT_CF_std_NAMESPACE
/** \brief Finds the first duplicate item in the unordered sequence
 *    <code>[first, last)</code>.
 *
 * \ingroup group__library__algorithms
 *
 * If a duplicate is found, the return value is a pair of the iterators
 * referring to the first and second elements comprising the duplicate.
 * If no duplicate is found, the return value is a pair containing the
 * \c last iterator in both its members
 *
 * \param first The start of the (unordered) sequence
 * \param last The (one past the) end point of the sequence
 *
 * \note This algorithm works for ordered sequences, but \c std::adjacent_find
 * is more suitable for such cases
 */
template<ss_typename_param_k I>
// [[synesis:function:algorithm: find_first_duplicate(T<I> first, T<I> last)]]
#if defined(STLSOFT_COMPILER_IS_DMC) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
inline stlsoft_ns_qual_std_(pair)<I, I> find_first_duplicate(I first, I last)
#else /* ? compiler */
inline stlsoft_ns_qual_std(pair)<I, I> find_first_duplicate(I first, I last)
#endif /* compiler */
{
    for(; first != last; ++first)
    {
        I next = first;

        for(++next; next != last; ++next)
        {
            if(*next == *first)
            {
                return stlsoft_ns_qual_std(make_pair)(first, next);
            }
        }
    }

    return stlsoft_ns_qual_std(make_pair)(last, last);
}

/** \brief
 *
 * \ingroup group__library__algorithms
 *
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k BP
        >
// [[synesis:function:algorithm: find_first_duplicate(T<I> first, T<I> last, T<BP> pred)]]
#if defined(STLSOFT_COMPILER_IS_DMC) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
inline stlsoft_ns_qual_std_(pair)<I, I> find_first_duplicate(I first, I last, BP pred)
#else /* ? compiler */
inline stlsoft_ns_qual_std(pair)<I, I> find_first_duplicate(I first, I last, BP pred)
#endif /* compiler */
{
    for(; first != last; ++first)
    {
        I next = first;

        for(++next; next != last; ++next)
        {
            if(pred(*next, *first))
            {
                return stlsoft_ns_qual_std(make_pair)(first, next);
            }
        }
    }

    return stlsoft_ns_qual_std(make_pair)(last, last);
}

#endif /* STLSOFT_CF_std_NAMESPACE */


/** \brief
 *
 * \ingroup group__library__algorithms
 *
 */
template <ss_typename_param_k FI>
// [[synesis:function:algorithm: unordered_unique(T<I> first, T<I> last)]]
inline FI unordered_unique(FI first, FI last)
{
    if(first != last)
    {
        // Because this is unordered, we need to enumerate through the
        // elements in the sequence, and ...
        const FI    start   =   first;
        FI          dest    =   ++first;

        for(; first != last; ++first)
        {
            // ... for each element in the sequence, we see if it has
            // already in the 'accepted' sequence, and, if not, ...
            if(dest == std_find(start, dest, *first))
            {
                // ... add it into the accepted sequence at the
                // current point.
                //
                // Effectively, this is to overwrite the element
                // if the source and destination points are different,
                // or simply moving past it if not.
                if(dest != first)
                {
                    *dest = *first;
                }
                ++dest;
            }
        }

        first = dest;
    }

    return first;
}

/** \brief
 *
 * \ingroup group__library__algorithms
 *
 */
template<   ss_typename_param_k FI
        ,   ss_typename_param_k BP
        >
// [[synesis:function:algorithm: unordered_unique(T<I> first, T<I> last, T<BP> pred)]]
inline FI unordered_unique(FI first, FI last, BP pred)
{
    if(first != last)
    {
        // Because this is unordered, we need to enumerate through the
        // elements in the sequence, and ...
        const FI    start   =   first;
        FI          dest    =   ++first;

        for(; first != last; ++first)
        {
            // ... for each element in the sequence, we see if it has
            // already in the 'accepted' sequence, and, if not, ...
            if(dest == std_find_if(start, dest, std::bind2nd(pred, *first)))
            {
                // ... add it into the accepted sequence at the
                // current point.
                //
                // Effectively, this is to overwrite the element
                // if the source and destination points are different,
                // or simply moving past it if not.
                if(dest != first)
                {
                    *dest = *first;
                }
                ++dest;
            }
        }

        first = dest;
    }

    return first;
}

/** \brief
 *
 * \ingroup group__library__algorithms
 *
 */
template<   ss_typename_param_k FI
        ,   ss_typename_param_k BP
        >
// [[synesis:function:algorithm: unordered_unique_if(T<I> first, T<I> last, T<BP> pred)]]
inline FI unordered_unique_if(FI first, FI last, BP pred)
{
    return unordered_unique(first, last, pred);
}

/** \brief
 *
 * \ingroup group__library__algorithms
 *
 */
template<   ss_typename_param_k FI
        ,   ss_typename_param_k OI
        >
// [[synesis:function:algorithm: unordered_unique(T<I> first, T<I> last, T<OI> dest)]]
inline OI unordered_unique_copy(FI first, FI last, OI dest)
{
    if(first != last)
    {
        // Because this is unordered, we need to enumerate through the
        // elements in the sequence, and ...
        const OI    start   =   dest;
        FI          curr    =   first;  // The first elements is always unique

        *dest++ = *first++;
        for(; first != last; ++first)
        {
            // ... for each element in the sequence, we see if it has
            // already in the 'accepted' sequence, and, if not, ...
            if(dest == std_find(start, dest, *first))
            {
                // ... add it into the accepted sequence at the
                // current point.
                *dest = *first;
                ++dest;
            }
        }
    }

    return dest;
}


/** \brief This algorithm removes duplicate entries from unordered sequences.
 *
 * \ingroup group__library__algorithms
 *
 * It necessarily runs in O(n2) time, since it must do a bubble-like double
 * pass on the sequence (in order to work with unordered sequences).
 *
 * \param container The container
 * \param pred The predicate used to determine the equivalence of items
 */
// [[synesis:function:algorithm: remove_duplicates_from_unordered_sequence(T<C> &container, T<BP> pred)]]
template<   ss_typename_param_k C
        ,   ss_typename_param_k BP
        >
inline void remove_duplicates_from_unordered_sequence(C &container, BP pred)
{
    typedef ss_typename_type_k C::iterator  iterator_t;

    ss_size_t   index;
    iterator_t  begin;

    for(index = 0, begin = container.begin(); begin != container.end(); )
    {
        iterator_t  it  =   begin;
        iterator_t  end =   container.end();

        if(++it == end)
        {
            ++begin;
        }
        else
        {
            for(;;)
            {
                if(pred(*begin, *it))
                {
                    ss_size_t   last;

                    // Erase the element. We now assume that all iterators
                    // are invalidated
                    container.erase(it);
                    // Remember the last erasure point
                    last = index;
                    // Set begin back to the start of the sequence
                    begin = container.begin();
                    // Move begin to the point where it was at the last erasure
                    std_advance(begin, static_cast<ss_ptrdiff_t>(last)); // Need to cast so that RAI impl doesn't incur impl conv from unsigned to signed in i += n;
                    break;
                }
                else
                {
                    if(++it == end)
                    {
                        ++begin;
                        ++index;
                        break;
                    }
                }
            }
        }
    }
}


#ifdef STLSOFT_CF_std_NAMESPACE
/** \brief This algorithm removes duplicate entries from unordered sequences.
 *
 * \ingroup group__library__algorithms
 *
 * It necessarily runs in O(n2) time, since it must do a bubble-like double
 * pass on the sequence (in order to work with unordered sequences).
 *
 * \param container The container
 */
// [[synesis:function:algorithm: remove_duplicates_from_unordered_sequence(T<C> &container)]]
template<ss_typename_param_k C>
inline void remove_duplicates_from_unordered_sequence(C &container)
{
    typedef ss_typename_type_k C::value_type    value_t;

    remove_duplicates_from_unordered_sequence(container, stlsoft_ns_qual_std(equal_to)<value_t>());
}

#endif /* STLSOFT_CF_std_NAMESPACE */


/** \brief Skips along from a given iteration point to the first subsequent
 * iteration point whose value is not equal to that of the starting
 * point
 *
 * \ingroup group__library__algorithms
 *
 *
 * \param first The start of the sequence
 * \param last The (one past the) end point of the sequence
 */
template<ss_typename_param_k I>
// [[synesis:function:algorithm: fill_all(T<I> first, T<I> last)]]
inline I skip_equal(I first, I last)
{
    if(first == last)
    {
        return last;
    }
    else
    {
        for(I next = first; next != last; ++next)
        {
            if(*next != *first)
            {
                return next;
            }
        }

        return last;
    }
}


/** \brief Determines whether all elements from the range
 *    <code>[first2, last2)</code> are contained within the range
 *    <code>[first1, last1)</code>.
 *
 * \ingroup group__library__algorithms
 *
 * \note The algorithm does <i>not</i> assume that the ranges are ordered, and
 * so does linear searches. If the ranges are ordered, you should use \c std::includes
 */
template<   ss_typename_param_k I1
        ,   ss_typename_param_k I2
        >
inline ss_bool_t unordered_includes(I1 first1, I1 last1, I2 first2, I2 last2)
{
    for(; first2 != last2; ++first2)
    {
        ss_bool_t   bFound  =   false;

        for(I1 i1 = first1; i1 != last1; ++i1)
        {
            if(*first2 == *i1)
            {
                bFound = true;
                break;
            }
        }

        if(!bFound)
        {
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/unordered_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_UNORDERED */

/* ///////////////////////////// end of file //////////////////////////// */
