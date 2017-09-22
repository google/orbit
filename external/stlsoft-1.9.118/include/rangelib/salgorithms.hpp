/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/salgorithms.hpp
 *
 * Purpose:     Range-adapted Sequence algorithms.
 *
 * Created:     19th July 2005
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


/** \file rangelib/salgorithms.hpp Range-adapted Sequence algorithms.
 *
 * This file includes the definition of the following algorithms:
 *
 * - sr_accumulate()
 * - sr_accumulate()
 * - sr_copy()
 * - sr_copy_if()
 * - sr_count()
 * - sr_count_if()
 * - sr_distance()
 * - sr_equal()
 * - sr_exists()
 * - sr_exists_if()
 * - sr_fill()
 * - sr_fill_n()
 * - sr_for_each()
 * - sr_generate()
 * - sr_max_element()
 * - sr_min_element()
 * - sr_replace()
 * - sr_replace_if()
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_SALGORITHMS
#define RANGELIB_INCL_RANGELIB_HPP_SALGORITHMS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_SALGORITHMS_MAJOR     1
# define RANGELIB_VER_RANGELIB_HPP_SALGORITHMS_MINOR     1
# define RANGELIB_VER_RANGELIB_HPP_SALGORITHMS_REVISION  3
# define RANGELIB_VER_RANGELIB_HPP_SALGORITHMS_EDIT      17
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_DMC:
STLSOFT_COMPILER_IS_MSVC:     _MSC_VER < 1200
STLSOFT_COMPILER_IS_MWERKS:   (__MWERKS__ & 0xFF00) < 0x3000
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGELIB
# include <rangelib/rangelib.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGELIB */
#ifndef RANGELIB_INCL_RANGELIB_HPP_ALGORITHMS
# include <rangelib/algorithms.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_ALGORITHMS */
#ifndef RANGELIB_INCL_RANGELIB_HPP_SEQUENCE_RANGE
# include <rangelib/sequence_range.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_SEQUENCE_RANGE */
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_ITERATOR
#  include <stlsoft/meta/detector/has_const_iterator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_ITERATOR */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_ITERATOR
#  include <stlsoft/meta/detector/has_iterator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_ITERATOR */
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::rangelib */
namespace rangelib
{
# else
/* Define stlsoft::rangelib_project */

namespace stlsoft
{

namespace rangelib_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        >
inline T sr_accumulate(S &s, T val)
{
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    STLSOFT_STATIC_ASSERT(stlsoft::has_iterator<S>::value || stlsoft::has_const_iterator<S>::value);
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

    return r_accumulate(sequence_range<S>(s), val);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        ,   ss_typename_param_k P
        >
inline T sr_accumulate(S &s, T val, P pred)
{
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    STLSOFT_STATIC_ASSERT(stlsoft::has_iterator<S>::value || stlsoft::has_const_iterator<S>::value);
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

    return r_accumulate(sequence_range<S>(s), val, pred);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k O
        >
inline O sr_copy(S &s, O o)
{
    return r_copy(sequence_range<S>(s), o);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k O
        ,   ss_typename_param_k P
        >
inline O sr_copy_if(S &s, O o, P pred)
{
    return r_copy_if(sequence_range<S>(s), o, pred);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k V
        >
inline size_t sr_count(S &s, V const& val)
{
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    STLSOFT_STATIC_ASSERT(stlsoft::has_iterator<S>::value || stlsoft::has_const_iterator<S>::value);
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

    return r_count(sequence_range<S>(s), val);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k P
        >
inline size_t sr_count_if(S &s, P pred)
{
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    STLSOFT_STATIC_ASSERT(stlsoft::has_iterator<S>::value || stlsoft::has_const_iterator<S>::value);
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

    return r_count_if(sequence_range<S>(s), pred);
}

template <ss_typename_param_k S>
inline ss_ptrdiff_t sr_distance(S &s)
{
    return r_distance(sequence_range<S>(s));
}

template<   ss_typename_param_k S1
        ,   ss_typename_param_k S2
        >
inline ss_bool_t sr_equal(S1 &s1, S2 &s2)
{
    STLSOFT_ASSERT(sr_distance(s1) <= sr_distance(s2));

    return r_equal(sequence_range<S1>(s1), sequence_range<S2>(s2));
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        >
inline ss_bool_t sr_exists(S &s, T const& val)
{
    return r_exists(sequence_range<S>(s), val);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k P
        >
inline ss_bool_t sr_exists_if(S &s, P pred)
{
    return r_exists_if(sequence_range<S>(s), pred);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        >
inline void sr_fill(S &s, T const& val)
{
    r_fill(sequence_range<S>(s), val);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k N
        ,   ss_typename_param_k T
        >
inline void sr_fill_n(S &s, N n, T const& val)
{
    STLSOFT_ASSERT(n <= r_distance(s));

    r_fill_n(sequence_range<S>(s), n, val);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k F
        >
inline F sr_for_each(S &s, F f)
{
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
    STLSOFT_STATIC_ASSERT(stlsoft::has_iterator<S>::value || stlsoft::has_const_iterator<S>::value);
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

    return r_for_each(sequence_range<S>(s), f);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k F
        >
inline void sr_generate(S &s, F f)
{
    r_generate(sequence_range<S>(s), f);
}

template <ss_typename_param_k S>
inline ss_typename_type_ret_k S::value_type sr_max_element(S &s)
{
    STLSOFT_ASSERT(sr_distance(s) > 0);

    return r_max_element(sequence_range<S>(s));
}
template<   ss_typename_param_k S
        ,   ss_typename_param_k F
        >
inline ss_typename_type_ret_k S::value_type sr_max_element(S &s, F f)
{
    STLSOFT_ASSERT(sr_distance(s) > 0);

    return r_max_element(sequence_range<S>(s), f);
}

template <ss_typename_param_k S>
inline ss_typename_type_ret_k S::value_type sr_min_element(S &s)
{
    STLSOFT_ASSERT(sr_distance(s) > 0);

    return r_min_element(sequence_range<S>(s));
}
template<   ss_typename_param_k S
        ,   ss_typename_param_k F
        >
inline ss_typename_type_ret_k S::value_type sr_min_element(S &s, F f)
{
    STLSOFT_ASSERT(sr_distance(s) > 0);

    return r_min_element(sequence_range<S>(s), f);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        >
inline void sr_replace(S &s, T oldVal, T newVal)
{
    r_replace(sequence_range<S>(s), oldVal, newVal);
}

template<   ss_typename_param_k S
        ,   ss_typename_param_k P
        ,   ss_typename_param_k T
        >
inline void sr_replace_if(S &s, P pred, T newVal)
{
    r_replace_if(sequence_range<S>(s), pred, newVal);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace rangelib
# else
} // namespace rangelib_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !RANGELIB_INCL_RANGELIB_HPP_SALGORITHMS */

/* ///////////////////////////// end of file //////////////////////////// */
