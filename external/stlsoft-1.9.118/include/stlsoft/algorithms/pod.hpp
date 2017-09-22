/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/algorithms/pod.hpp
 *
 * Purpose:     Algorithms for Plain-Old Data types.
 *
 * Created:     17th January 2002
 * Updated:     11th August 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/algorithms/pod.hpp
 *
 * \brief [C++ only] Algorithms for use with POD types
 *   (\ref group__library__algorithms "Algorithms" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD
#define STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_POD_MAJOR       3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_POD_MINOR       5
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_POD_REVISION    3
# define STLSOFT_VER_STLSOFT_ALGORITHMS_HPP_POD_EDIT        90
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHM_STD_HPP_ALT
# include <stlsoft/algorithms/std/alt.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHM_STD_HPP_ALT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
# include <stlsoft/meta/is_integral_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
# include <stlsoft/meta/is_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE
# include <stlsoft/meta/is_same_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_MSVC)
# include <memory.h>                    // for memcpy
#else /* ? compiler */
# include <string.h>                    // for memcpy
#endif /* compiler */

#ifdef STLSOFT_UNITTEST
# ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
#  include <stlsoft/error/exceptions.hpp>      // for null_exception_policy
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#endif /* STLSOFT_UNITTEST */

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
struct ximpl_stlsoft_algorithm_pod_helper_
{

    // Same-type operations

    template<
        ss_typename_param_k T
    >
    static void pod_copy_n_(
        T*          dest
    ,   T const*    src
    ,   ss_size_t   n
    ,   yes_type
    )
    {
        ::memcpy(dest, src, n * sizeof(*dest));
    }

    template<
        ss_typename_param_k T
    >
    static void pod_move_n_(
        T*          dest
    ,   T const*    src
    ,   ss_size_t   n
    ,   yes_type
    )
    {
        ::memmove(dest, src, n * sizeof(*dest));
    }


    // Different-type operations
    //
    // - ptr-to-ptr is fine
    // - int-to-int is fine

    template<
        ss_typename_param_k O
    ,   ss_typename_param_k I
    >
    static void pod_copy_n_(
        O*          dest
    ,   I const*    src
    ,   ss_size_t   n
    ,   no_type
    )
    {
        enum { O_IS_INTEGRAL_TYPE   =   (0 != is_integral_type<O>::value)   };
        enum { I_IS_INTEGRAL_TYPE   =   (0 != is_integral_type<I>::value)   };
        enum { O_IS_POINTER_TYPE    =   (0 != is_pointer_type<O>::value)    };
        enum { I_IS_POINTER_TYPE    =   (0 != is_pointer_type<I>::value)    };

        STLSOFT_STATIC_ASSERT(sizeof(*dest) == sizeof(*src));
        STLSOFT_STATIC_ASSERT(int(O_IS_INTEGRAL_TYPE) == int(I_IS_INTEGRAL_TYPE));
        STLSOFT_STATIC_ASSERT(int(O_IS_POINTER_TYPE) == int(I_IS_POINTER_TYPE));

        ::memcpy(dest, src, n * sizeof(*dest));
    }

    template<
        ss_typename_param_k O
    ,   ss_typename_param_k I
    >
    static void pod_move_n_(
        O*          dest
    ,   I const*    src
    ,   ss_size_t   n
    ,   no_type
    )
    {
        enum { O_IS_INTEGRAL_TYPE   =   (0 != is_integral_type<O>::value)   };
        enum { I_IS_INTEGRAL_TYPE   =   (0 != is_integral_type<I>::value)   };
        enum { O_IS_POINTER_TYPE    =   (0 != is_pointer_type<O>::value)    };
        enum { I_IS_POINTER_TYPE    =   (0 != is_pointer_type<I>::value)    };

        STLSOFT_STATIC_ASSERT(sizeof(*dest) == sizeof(*src));
        STLSOFT_STATIC_ASSERT(int(O_IS_INTEGRAL_TYPE) == int(I_IS_INTEGRAL_TYPE));
        STLSOFT_STATIC_ASSERT(int(O_IS_POINTER_TYPE) == int(I_IS_POINTER_TYPE));

        ::memmove(dest, src, n * sizeof(*dest));
    }

};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Algorithms
 */

/** \brief Copies one range of POD (Plain Old Data) entities to another
 *
 * \ingroup group__library__algorithms
 *
 * This algorithm has the same semantics as std::copy(), but it uses
 * ::memcpy() to copy elements en bloc, rather than copy assignment of one element
 * at a time.
 *
\code
  const int8_t  src_bytes[3] = { -1, 0, +1 };
  int8_t        dest_bytes[3];

  pod_copy(&src_bytes[0], &src_bytes[0] + STLSOFT_NUM_ELEMENTS(src_bytes), &dest_bytes[0]);
  assert(0 == memcmp(&src_bytes[0], &dest_bytes[0], sizeof(src_bytes)));
\endcode
 *
 * \note The implementation uses static assertions to ensure that the source and
 * destination element types are the same size.
 *
 * \note The implementation uses constraints ensure sure that the source and
 * destination element types are POD
 *
 * \param first Contiguous Iterator marking the start of the source range
 * \param last Contiguous Iterator marking the one-past-the-end of the source range
 * \param dest Contiguous Iterator marking the start of the source range
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        >
// [[synesis:function:algorithm: pod_copy(T<I> *first, T<I>* last, T<O>* dest)]]
inline void pod_copy(I* first, I* last, O* dest)
{
#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_DMC)

    std_copy(&first[0], &last[0], &dest[0]);

#else /* ? compiler */

    STLSOFT_STATIC_ASSERT(sizeof(*dest) == sizeof(*first));
    stlsoft_constraint_must_be_pod(O);
    stlsoft_constraint_must_be_pod(I);

    enum { TYPES_ARE_SAME = is_same_type<I, O>::value };

    typedef ss_typename_type_k value_to_yesno_type<TYPES_ARE_SAME>::type    yesno_t;

    ss_size_t n = static_cast<ss_size_t>(last - first);

    ximpl_stlsoft_algorithm_pod_helper_::pod_copy_n_(dest, first, n, yesno_t());

#endif /* compiler */
}

/** \brief Copies one range of POD (Plain Old Data) entities to another
 *
 * \ingroup group__library__algorithms
 *
 * This algorithm uses ::memcpy() to copy elements en bloc, rather than
 * copy assignment of one element at a time.
 *
\code
  const int8_t  src_bytes[3] = { -1, 0, +1 };
  int8_t        dest_bytes[3];

  pod_copy_n(&dest_bytes[0], &src_bytes[0], STLSOFT_NUM_ELEMENTS(src_bytes));
  assert(0 == memcmp(&src_bytes[0], &dest_bytes[0], sizeof(src_bytes)));
\endcode
 *
 * \note The implementation uses static assertions to ensure that the source and
 * destination element types are the same size.
 *
 * \note The implementation uses constraints ensure sure that the source and
 * destination element types are POD
 *
 * \param dest Contiguous Iterator marking the start of the source range
 * \param src Contiguous Iterator marking the start of the source range
 * \param n Number of elements in the range
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        >
// [[synesis:function:algorithm: pod_copy_n(T<O>* dest, T<I> *src, size_t n)]]
inline void pod_copy_n(O *dest, I *src, ss_size_t n)
{
#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_DMC)

    std_copy(&src[0], &src[n], &dest[0]);

#else /* ? compiler */

    STLSOFT_STATIC_ASSERT(sizeof(*dest) == sizeof(*src));
    stlsoft_constraint_must_be_pod(O);
    stlsoft_constraint_must_be_pod(I);

    enum { TYPES_ARE_SAME = is_same_type<I, O>::value };

    typedef ss_typename_type_k value_to_yesno_type<TYPES_ARE_SAME>::type    yesno_t;

    ximpl_stlsoft_algorithm_pod_helper_::pod_copy_n_(dest, src, n, yesno_t());

#endif /* compiler */
}

/** \brief Copies one range of POD (Plain Old Data) entities to another, where the two
 * may potentially overlap
 *
 * \ingroup group__library__algorithms
 *
 * This algorithm has the same semantics as std::copy(), but it uses
 * ::memmove() to copy elements en bloc, rather than copy assignment of one element
 * at a time.
 *
 * \note The implementation uses static assertions to ensure that the source and
 * destination element types are the same size.
 *
 * \note The implementation uses constraints ensure sure that the source and
 * destination element types are POD
 *
 * \param first Contiguous Iterator marking the start of the source range
 * \param last Contiguous Iterator marking the one-past-the-end of the source range
 * \param dest Contiguous Iterator marking the start of the source range
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        >
// [[synesis:function:algorithm: pod_move(T<I>* first, T<I>* last, T<O>* dest)]]
inline void pod_move(I* first, I* last, O* dest)
{
#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_DMC)

    std_copy(&first[0], &last[0], &dest[0]);

#else /* ? compiler */

    STLSOFT_STATIC_ASSERT(sizeof(*dest) == sizeof(*first));
    stlsoft_constraint_must_be_pod(O);
    stlsoft_constraint_must_be_pod(I);

    enum { TYPES_ARE_SAME = is_same_type<I, O>::value };

    typedef ss_typename_type_k value_to_yesno_type<TYPES_ARE_SAME>::type    yesno_t;

    ss_size_t n = static_cast<ss_size_t>(last - first);

    ximpl_stlsoft_algorithm_pod_helper_::pod_move_n_(dest, first, n, yesno_t());

#endif /* compiler */
}

/** \brief Copies one range of POD (Plain Old Data) entities to another, where the two
 * may potentially overlap
 *
 * \ingroup group__library__algorithms
 *
 * \note This algorithm uses ::memmove() to copy elements en bloc, rather than
 * copy assignment of one element at a time.
 *
 * \note The implementation uses static assertions to ensure that the source and
 * destination element types are the same size.
 *
 * \note The implementation uses constraints ensure sure that the source and
 * destination element types are POD
 *
 * \param dest Contiguous Iterator marking the start of the source range
 * \param src Contiguous Iterator marking the start of the source range
 * \param n Number of elements in the range
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k O
        >
// [[synesis:function:algorithm: pod_move_n(T<O> *dest, T<I> *src, size_t n)]]
inline void pod_move_n(O *dest, I *src, ss_size_t n)
{
#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_DMC)

    std_copy(&src[0], &src[n], &dest[0]);

#else /* ? compiler */

    STLSOFT_STATIC_ASSERT(sizeof(*dest) == sizeof(*src));
    stlsoft_constraint_must_be_pod(O);
    stlsoft_constraint_must_be_pod(I);

    enum { TYPES_ARE_SAME = is_same_type<I, O>::value };

    typedef ss_typename_type_k value_to_yesno_type<TYPES_ARE_SAME>::type    yesno_t;

    ximpl_stlsoft_algorithm_pod_helper_::pod_move_n_(dest, src, n, yesno_t());

#endif /* compiler */
}


/** \brief Sets all the elements in a range of POD (Plain Old Data) to a given value
 *
 * \ingroup group__library__algorithms
 *
 * This algorithm has the same semantics as std::fill_n(), but it uses
 * ::memset() for some types to set the range of elements, rather than copy
 * assignment of one element at a time.
 *
\code
  const int8_t  src_bytes[3] = { 3, 3, 3 };
  int8_t        dest_bytes[3];

  pod_fill_n(&dest_bytes[0], STLSOFT_NUM_ELEMENTS(src_bytes), 3);
  assert(0 == memcmp(&src_bytes[0], &dest_bytes[0], sizeof(src_bytes)));
\endcode
 *
 * The generic overload uses std::fill_n(), so performance is likely to be
 * identical to std::fill_n() in the general case. However, it is overloaded
 * to use ::memset() on \c char, \c signed \c char and \c unsigned \c char
 * types, for which performance gains are likely.
 *
 * \param dest Contiguous Iterator marking the start of the source range
 * \param n Number of elements in the range
 * \param value Value to which each element in dest[0, n) will be set
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k V
        >
// [[synesis:function:algorithm: pod_fill_n(T<T> *dest, T<V> const& value)]]
inline void pod_fill_n(T *dest, ss_size_t n, V const& value)
{
    // Constrain to POD type:
    stlsoft_constraint_must_be_pod(T);

    std_fill_n(dest, n, value);
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
// [[synesis:function:algorithm: pod_fill_n(char *dest, int value)]]
inline void pod_fill_n(char *dest, ss_size_t n, int value)
{
    ::memset(dest, value, n);
}
// [[synesis:function:algorithm: pod_fill_n(signed char *dest, int value)]]
inline void pod_fill_n(signed char *dest, ss_size_t n, int value)
{
    ::memset(dest, value, n);
}
// [[synesis:function:algorithm: pod_fill_n(unsigned char *dest, int value)]]
inline void pod_fill_n(unsigned char *dest, ss_size_t n, int value)
{
    ::memset(dest, value, n);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/pod_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD */

/* ///////////////////////////// end of file //////////////////////////// */
