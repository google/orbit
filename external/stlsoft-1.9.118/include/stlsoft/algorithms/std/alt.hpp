/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/algorithms/std/alt.hpp
 *
 * Purpose:     Standard-equivalent algorithms for use with implementations that
 *              do or do not have a standard library.
 *
 * Created:     17th January 2002
 * Updated:     15th July 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/algorithms/std/alt.hpp
 *
 * \brief [C++ only] Contains various standard functions, or their
 *   equivalents
 *   (\ref group__library__algorithms "Algorithms" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT
#define STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT

// For backwards compatibility, we define this here to disable stlsoft/util/std/algorithm.hpp
#define STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ALGORITHM

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_ALT_MAJOR       3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_ALT_MINOR       4
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_ALT_REVISION    4
# define STLSOFT_VER_STLSOFT_ALGORITHMS_STD_HPP_ALT_EDIT        77
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
#if defined(STLSOFT_CF_std_NAMESPACE)
# include <algorithm>
#endif /* STLSOFT_CF_std_NAMESPACE */
#if !defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
#  include <stlsoft/meta/yesno.hpp> // yes_type, no_type
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifdef STLSOFT_CF_NAMESPACE_SUPPORT
namespace std_alt_algorithms_util_
{
# endif /* STLSOFT_CF_NAMESPACE_SUPPORT */

template<ss_typename_param_k I>
void std_advance_impl(  I               &i
                    ,   ss_ptrdiff_t    n
# if !defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
                    ,   no_type
# endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
                    )
{
    if(n < 0)
    {
        for(; 0 != n; --i, ++n)
        {}
    }
    else
    {
        for(; 0 != n; ++i, --n)
        {}
    }
}

template<ss_typename_param_k I>
void std_advance_impl(  I               *&i
                    ,   ss_ptrdiff_t    n
# if !defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
                    ,   yes_type
# endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
                    )
{
    i += n;
}

# if !defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)

template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        >
no_type is_ptr_(T1 const&, T2 const*) { return no_type(); }

template<ss_typename_param_k T>
yes_type is_ptr_(T const*, T const**) { return yes_type(); }

# endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

# ifdef STLSOFT_CF_NAMESPACE_SUPPORT
} // namespace std_alt_algorithms_util_
# endif /* STLSOFT_CF_NAMESPACE_SUPPORT */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** \brief Equivalent to std::advance()
 *
 * \ingroup group__library__algorithms
 */
template<ss_typename_param_k I>
// [[synesis:function:algorithm: std_advance(T<I> &i, ss_ptrdiff_t n)]]
void std_advance(I &i, ss_ptrdiff_t n)
{
#ifdef STLSOFT_CF_std_NAMESPACE

    stlsoft_ns_qual_std(advance)(i, n);

#else /* ? STLSOFT_CF_std_NAMESPACE */

# ifdef STLSOFT_CF_NAMESPACE_SUPPORT
    using std_alt_algorithms_util_::std_advance_impl;
#  if !defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    using std_alt_algorithms_util_::is_ptr_;
#  endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
# endif /* STLSOFT_CF_NAMESPACE_SUPPORT */

# if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    std_advance_impl(i, n);
# else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
    std_advance_impl(i, n, is_ptr_(i, &i));
# endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::copy()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        >
// [[synesis:function:algorithm: std_copy(T<I> first, T<I> last, T<O> dest)]]
inline O std_copy(I first, I last, O dest)
{
#if defined(STLSOFT_CF_std_NAMESPACE) && \
    !defined(STLSOFT_USING_SAFE_STR_FUNCTIONS)

    return stlsoft_ns_qual_std(copy)(first, last, dest);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; first != last; ++first, ++dest)
    {
        *dest = *first;
    }

    return dest;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::count_if()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k UP
        >
// [[synesis:function:algorithm: std_count_if(T<I> first, T<I> last, T<UP> pred)]]
inline ss_size_t std_count_if(I first, I last, UP pred)
{
#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(count_if)(first, last, pred);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    ss_size_t r;

    for (r = 0; first != last; ++first)
    {
        if(pred(*first))
        {
            ++r;
        }
    }

    return r;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::fill()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k O
        ,   ss_typename_param_k V
        >
// [[synesis:function:algorithm: std_fill(T<O> first, T<O> last, T<V> const& value)]]
inline void std_fill(O first, O last, V const& value)
{
#ifdef STLSOFT_CF_std_NAMESPACE

    stlsoft_ns_qual_std(fill)(first, last, value);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; first != last; ++first)
    {
        *first = value;
    }

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::fill_n()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k O
        ,   ss_typename_param_k V
        >
// [[synesis:function:algorithm: std_fill_n(T<O> dest, ss_size_t n, T<V> const& value)]]
inline void std_fill_n(O dest, ss_size_t n, V const& value)
{
#if defined(STLSOFT_CF_std_NAMESPACE) && \
    !defined(STLSOFT_USING_SAFE_STR_FUNCTIONS)

    stlsoft_ns_qual_std(fill_n)(dest, n, value);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; 0 != n; ++dest, --n)
    {
        *dest = value;
    }

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::find()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k V
        >
// [[synesis:function:algorithm: std_find(T<I> first, T<I> last, T<V> const& value)]]
inline I std_find(I first, I last, V const& value)
{
#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(find)(first, last, value);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; first != last; ++first)
    {
        if(value == *first)
        {
            break;
        }
    }

    return first;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::find_if()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k UP
        >
// [[synesis:function:algorithm: std_find_if(T<I> first, T<I> last, T<UP> pred)]]
inline I std_find_if(I first, I last, UP pred)
{
#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(find_if)(first, last, pred);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; first != last; ++first)
    {
        if(pred(*first))
        {
            break;
        }
    }

    return first;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::for_each()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k UF
        >
// [[synesis:function:algorithm: std_for_each(T<I> first, T<I> last, T<UF> func)]]
inline UF std_for_each(I first, I last, UF func)
{
#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(for_each)(first, last, func);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; first != last; ++first)
    {
        func(*first);
    }

    return func;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::replace()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
// [[synesis:function:algorithm: std_replace(T<I> first, T<I> last, T<V> const& valFind, T<V> const& valReplace)]]
inline void std_replace(I first, I last, T const& valFind, T const& valReplace)
{
    for(; first != last; ++first)
    {
        if(valFind == *first)
        {
            *first = valReplace;
        }
    }
}

#ifdef STLSOFT_CF_std_NAMESPACE
/** \brief Equivalent to std::sort()
 *
 * \ingroup group__library__algorithms
 */
template <ss_typename_param_k RI>
// [[synesis:function:algorithm: std_sort(T<RI> first, T<RI> last)]]
inline void sort(RI first, RI last)
{
    stlsoft_ns_qual_std(sort)(first, last);
}

/** \brief Equivalent to std::sort()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k RI
        ,   ss_typename_param_k BP
        >
// [[synesis:function:algorithm: std_sort(T<RI> first, T<RI> last, T<BP> pred)]]
inline void std_sort(RI first, RI last, BP pred)
{
    stlsoft_ns_qual_std(sort)(first, last, pred);
}

#else /* ? STLSOFT_CF_std_NAMESPACE */

// Nothing current defined
template <ss_typename_param_k RI>
inline void sort(RI first, RI last);

// Nothing current defined
template<   ss_typename_param_k RI
        ,   ss_typename_param_k BP
        >
inline void std_sort(RI first, RI last, BP pred);

#endif /* STLSOFT_CF_std_NAMESPACE */

/** \brief Equivalent to std::transform()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        ,   ss_typename_param_k UF
        >
// [[synesis:function:algorithm: std_transform(T<I> first, T<I> last, T<O> dest, T<UF> func)]]
inline O std_transform(I first, I last, O dest, UF func)
{
#if defined(STLSOFT_CF_std_NAMESPACE) && \
    !defined(STLSOFT_USING_SAFE_STR_FUNCTIONS)

    return stlsoft_ns_qual_std(transform)(first, last, dest, func);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    for(; first != last; ++first, ++dest)
    {
        *dest = func(*first);
    }

    return dest;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::unique()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k FI
        ,   ss_typename_param_k BP
        >
// [[synesis:function:algorithm: std_unique(T<FI> first, T<FI> last, T<BP> pred)]]
inline FI std_unique(FI first, FI last, BP pred)
{
#ifdef STLSOFT_ENFORCE_ORDERING
    stlsoft_assert("range passed to unique() is not strict-weak ordered", is_strict_weak_ordered(first, last, pred));
#endif /* STLSOFT_ENFORCE_ORDERING */

#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(unique)(first, last, pred);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    if(first != last)
    {
# ifdef _DEBUG
        const FI    start   =   first;
# endif /* _DEBUG */
        FI          curr    =   first;  // The first elements is always unique

        for(++first; first != last; ++first)
        {
            if(pred(*first, *curr))
            {
                ; // Same, so skip it
            }
            else
            {
                ++curr;
                if(first != curr)
                {
                    *curr = *first;
                }
            }
        }

        first = ++curr;
    }

    return first;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::unique()
 *
 * \ingroup group__library__algorithms
 */
template <ss_typename_param_k FI>
// [[synesis:function:algorithm: std_unique(T<FI> first, T<FI> last)]]
inline FI std_unique(FI first, FI last)
{
#ifdef STLSOFT_ENFORCE_ORDERING
    stlsoft_assert("range passed to unique() is not strict-weak ordered", is_strict_weak_ordered(first, last));
#endif /* STLSOFT_ENFORCE_ORDERING */

#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(unique)(first, last);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    if(first != last)
    {
# ifdef _DEBUG
        const FI    start   =   first;
# endif /* _DEBUG */
        FI          curr    =   first;  // The first elements is always unique

        for(++first; first != last; ++first)
        {
            if(*first == *curr)
            {
                ; // Same, so skip it
            }
            else
            {
                ++curr;
                if(first != curr)
                {
                    *curr = *first;
                }
            }
        }

        first = ++curr;
    }

    return first;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::unique_copy()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k FI
        ,   ss_typename_param_k OI
        >
// [[synesis:function:algorithm: std_unique_copy(T<FI> first, T<FI> last, T<OI> dest)]]
inline OI std_unique_copy(FI first, FI last, OI dest)
{
#ifdef STLSOFT_ENFORCE_ORDERING
    stlsoft_assert("range passed to unique_copy() is not strict-weak ordered", is_strict_weak_ordered(first, last));
#endif /* STLSOFT_ENFORCE_ORDERING */

#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(unique_copy)(first, last, dest);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    if(first != last)
    {
# ifdef _DEBUG
        const FI    start   =   first;
# endif /* _DEBUG */
        FI          curr    =   first;  // The first elements is always unique

        *dest++ = *first++;
        for(; first != last; ++first)
        {
            if(*first == *curr)
            {
                ; // Same, so skip it
            }
            else
            {
                curr = first;
                *dest++ = *first;
            }
        }

        first = ++curr;
    }

    return dest;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

/** \brief Equivalent to std::unique_copy()
 *
 * \ingroup group__library__algorithms
 */
template<   ss_typename_param_k FI
        ,   ss_typename_param_k OI
        ,   ss_typename_param_k BP
        >
// [[synesis:function:algorithm: std_unique_copy(T<FI> first, T<FI> last, T<OI> dest, T<BP> pred)]]
inline OI std_unique_copy(FI first, FI last, OI dest, BP pred)
{
#ifdef STLSOFT_ENFORCE_ORDERING
    stlsoft_assert("range passed to unique_copy() is not strict-weak ordered", is_strict_weak_ordered(first, last, pred));
#endif /* STLSOFT_ENFORCE_ORDERING */

#ifdef STLSOFT_CF_std_NAMESPACE

    return stlsoft_ns_qual_std(unique_copy)(first, last, dest, pred);

#else /* ? STLSOFT_CF_std_NAMESPACE */

    if(first != last)
    {
# ifdef _DEBUG
        const FI    start   =   first;
# endif /* _DEBUG */
        FI          curr    =   first;  // The first elements is always unique

        *dest++ = *first++;
        for(; first != last; ++first)
        {
            if(pred(*first, *curr))
            {
                ; // Same, so skip it
            }
            else
            {
                curr = first;
                *dest++ = *first;
            }
        }

        first = ++curr;
    }

    return dest;

#endif /* STLSOFT_CF_std_NAMESPACE */
}

////////////////////////////////////////////////////////////////////////////

/// @} // end of group algorithms_std_alt

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/alt_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_STD_HPP_ALT */

/* ///////////////////////////// end of file //////////////////////////// */
