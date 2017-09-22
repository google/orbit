/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/containers/static_array.hpp
 *
 * Purpose:     Statically sized multidimensional class template.
 *
 * Created:     4th August 1998
 * Updated:     10th August 2009
 *
 * Thanks to:   Neal Becker for suggesting the uninitialised mode.
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


/** \file stlsoft/containers/static_array.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::static_array_1d,
 *    stlsoft::static_array_2d, stlsoft::static_array_3d, and
 *    stlsoft::static_array_4d multidimensional array class templates
 *   (\ref group__library__containers "Containers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY
#define STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY_MAJOR     4
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY_MINOR     4
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY_REVISION  2
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY_EDIT      188
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:  __BORLANDC__<0x0564
STLSOFT_COMPILER_IS_DMC:  __DMC__<0x0844
STLSOFT_COMPILER_IS_GCC:  __GNUC__<3
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_BORLAND) && \
    __BORLANDC__ < 0x0560
# error stlsoft/containers/static_array.hpp is not compatible with Borland C/C++ prior to 5.6
#elif defined(STLSOFT_COMPILER_IS_DMC) && \
    __DMC__ < 0x0844
# error stlsoft/containers/static_array.hpp is not compatible with Digital Mars C/C++ 8.43 or earlier
#elif defined(STLSOFT_COMPILER_IS_GCC) && \
      __GNUC__ < 3
# error stlsoft/containers/static_array.hpp is not compatible with GNU C/C++ prior to 3.0
#elif defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/containers/static_array.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR
# include <stlsoft/memory/null_allocator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_CONTAINERS_UTIL_HPP_ARRAY_POLICIES
# include <stlsoft/containers/util/array_policies.hpp>  // for stlsoft::do_construction()
#endif /* !STLSOFT_INCL_STLSOFT_CONTAINERS_UTIL_HPP_ARRAY_POLICIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
# include <stlsoft/meta/is_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    defined(STLSOFT_CF_STD_LIBRARY_IS_STLPORT)
# include <string>                      // for std::string - sigh!
#endif /* compiler */
#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                    // for std::out_of_range
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# include <numeric>
#endif /* !STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Forward declarations
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
class static_array_1d;

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
class static_array_2d;

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
class static_array_3d;

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_size_t N3
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
class static_array_4d;

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_size_t N3
        ,   ss_size_t N4
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
class static_array_5d;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Classes

// class static_array_1d
/** \brief 1 dimensional static array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param N0 The first dimension extent
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
        ,   ss_size_t N0
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k P   =   do_construction<T>
        ,   ss_typename_param_k M   =   T[N0]
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class static_array_1d
    : public null_allocator<T>
    , public stl_collection_tag
{
public:
    typedef static_array_1d<T, N0, P, M>                    class_type;
    typedef T                                               dimension_type;
    typedef null_allocator<T>                               allocator_type;
    typedef P                                               policy_type;

    typedef T                                               value_type;
    typedef value_type&                                     reference;
    typedef value_type const&                               const_reference;
    typedef value_type*                                     pointer;
    typedef value_type const*                               const_pointer;
    typedef ss_size_t                                       size_type;
    typedef ss_size_t                                       index_type;
    typedef ss_ptrdiff_t                                    difference_type;

    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
//protected:
    static_array_1d(T *data/* [N0] */);
public:
    static_array_1d();
    static_array_1d(value_type const& t);
    static_array_1d(class_type const& rhs);
    ~static_array_1d() stlsoft_throw_0();

// Access
public:
    reference               at(index_type i0);
    const_reference         at(index_type i0) const;

    reference               at_unchecked(index_type i0);
    const_reference         at_unchecked(index_type i0) const;

    reference               operator [](index_type i0);
    const_reference         operator [](index_type i0) const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    static index_type       dimension0();
    static index_type       size();
    static ss_bool_t        empty();
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    value_type const        *data() const;

// Implementation
protected:
    pointer     data_();
    index_type  calc_index_(index_type i0) const;
    void        range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

// Members
private:
    M   m_data;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

// class static_array_2d
/** \brief 2 dimensional static array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param N0 The first dimension extent
 * \param N1 The second dimension extent
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k P   =   do_construction<T>
        ,   ss_typename_param_k M   =   T[N0 * N1]
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class static_array_2d
    : public null_allocator<T>
    , public stl_collection_tag
{
public:
    typedef static_array_2d<T, N0, N1, P, M>                class_type;
    typedef static_array_1d<T, N1, P, T*>                   dimension_type;
    typedef null_allocator<T>                               allocator_type;
    typedef P                                               policy_type;

    typedef T                                               value_type;
    typedef value_type&                                     reference;
    typedef value_type const&                               const_reference;
    typedef value_type*                                     pointer;
    typedef value_type const*                               const_pointer;
    typedef ss_size_t                                       size_type;
    typedef ss_size_t                                       index_type;
    typedef ss_ptrdiff_t                                    difference_type;

    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
//protected:
//    static_array_2d(T data[N0][N1]);
    static_array_2d(T *data);
public:
    static_array_2d();
    static_array_2d(value_type const& t);
    static_array_2d(class_type const& rhs);
    ~static_array_2d() stlsoft_throw_0();

// Operations
public:
    reference               at(index_type i0, index_type i1);
    const_reference         at(index_type i0, index_type i1) const;

    reference               at_unchecked(index_type i0, index_type i1);
    const_reference         at_unchecked(index_type i0, index_type i1) const;

    dimension_type          at(index_type i0);
    const dimension_type    at(index_type i0) const;

    dimension_type          at_unchecked(index_type i0);
    const dimension_type    at_unchecked(index_type i0) const;

    dimension_type          operator [](index_type i0);
    const dimension_type    operator [](index_type i0) const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    static index_type       dimension0();
    static index_type       dimension1();
    static index_type       size();
    static ss_bool_t        empty();
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    value_type const        *data() const;

// Implementation
protected:
    pointer     data_();
    index_type  calc_index_(index_type i0, index_type i1) const;
    void        range_check_(index_type i0, index_type i1) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void        range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

// Members
private:
    M   m_data;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

// class static_array_3d
/** \brief 3 dimensional static array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param N0 The first dimension extent
 * \param N1 The second dimension extent
 * \param N2 The third dimension extent
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k P   =   do_construction<T>
        ,   ss_typename_param_k M   =   T[N0 * N1 * N2]
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class static_array_3d
    : public null_allocator<T>
    , public stl_collection_tag
{
public:
    typedef static_array_3d<T, N0, N1, N2, P, M>            class_type;
    typedef static_array_2d<T, N1, N2, P, T*>               dimension_type;
    typedef null_allocator<T>                               allocator_type;
    typedef P                                               policy_type;

    typedef T                                               value_type;
    typedef value_type&                                     reference;
    typedef value_type const&                               const_reference;
    typedef value_type*                                     pointer;
    typedef value_type const*                               const_pointer;
    typedef ss_size_t                                       size_type;
    typedef ss_size_t                                       index_type;
    typedef ss_ptrdiff_t                                    difference_type;

    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
//protected:
//    static_array_3d(T data[N0][N1][N2]);
    static_array_3d(T *data);
public:
    static_array_3d();
    static_array_3d(value_type const& t);
    static_array_3d(class_type const& rhs);
    ~static_array_3d() stlsoft_throw_0();

// Operations
public:
    reference               at(index_type i0, index_type i1, index_type i2);
    const_reference         at(index_type i0, index_type i1, index_type i2) const;

    reference               at_unchecked(index_type i0, index_type i1, index_type i2);
    const_reference         at_unchecked(index_type i0, index_type i1, index_type i2) const;

    dimension_type          at(index_type i0);
    const dimension_type    at(index_type i0) const;

    dimension_type          at_unchecked(index_type i0);
    const dimension_type    at_unchecked(index_type i0) const;

    dimension_type          operator [](index_type i0);
    const dimension_type    operator [](index_type i0) const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    static index_type       dimension0();
    static index_type       dimension1();
    static index_type       dimension2();
    static index_type       size();
    static ss_bool_t        empty();
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    value_type const        *data() const;

// Implementation
protected:
    pointer     data_();
    index_type  calc_index_(index_type i0, index_type i1, index_type i2) const;
    void        range_check_(index_type i0, index_type i1, index_type i2) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void        range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

// Members
private:
    M   m_data;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};


// class static_array_4d
/** \brief 4 dimensional static array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param N0 The first dimension extent
 * \param N1 The second dimension extent
 * \param N2 The third dimension extent
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_size_t N3
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k P   =   do_construction<T>
        ,   ss_typename_param_k M   =   T[N0 * N1 * N2 * N3]
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class static_array_4d
    : public null_allocator<T>
    , public stl_collection_tag
{
public:
    typedef static_array_4d<T, N0, N1, N2, N3, P, M>        class_type;
    typedef static_array_3d<T, N1, N2, N3, P, T*>           dimension_type;
    typedef null_allocator<T>                               allocator_type;
    typedef P                                               policy_type;

    typedef T                                               value_type;
    typedef value_type&                                     reference;
    typedef value_type const&                               const_reference;
    typedef value_type*                                     pointer;
    typedef value_type const*                               const_pointer;
    typedef ss_size_t                                       size_type;
    typedef ss_size_t                                       index_type;
    typedef ss_ptrdiff_t                                    difference_type;

    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
//protected:
//    static_array_4d(T data[N0][N1][N2][N3]);
    static_array_4d(T *data);
public:
    static_array_4d();
    static_array_4d(value_type const& t);
    static_array_4d(class_type const& rhs);
    ~static_array_4d() stlsoft_throw_0();

// Operations
public:
    reference               at(index_type i0, index_type i1, index_type i2, index_type i3);
    const_reference         at(index_type i0, index_type i1, index_type i2, index_type i3) const;

    reference               at_unchecked(index_type i0, index_type i1, index_type i2, index_type i3);
    const_reference         at_unchecked(index_type i0, index_type i1, index_type i2, index_type i3) const;

    dimension_type          at(index_type i0);
    const dimension_type    at(index_type i0) const;

    dimension_type          at_unchecked(index_type i0);
    const dimension_type    at_unchecked(index_type i0) const;

    dimension_type          operator [](index_type i0);
    const dimension_type    operator [](index_type i0) const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    static index_type       dimension0();
    static index_type       dimension1();
    static index_type       dimension2();
    static index_type       dimension3();
    static index_type       size();
    static ss_bool_t        empty();
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    value_type const        *data() const;

// Implementation
protected:
    pointer     data_();
    index_type  calc_index_(index_type i0, index_type i1, index_type i2, index_type i3) const;
    void        range_check_(index_type i0, index_type i1, index_type i2, index_type i3) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void        range_check_(index_type i0, index_type i1, index_type i2) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void        range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

// Members
private:
    M   m_data;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// static_array_1d

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::pointer static_array_1d<T, N0, P, M>::data_()
{
    return &m_data[0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::index_type static_array_1d<T, N0, P, M>::calc_index_(ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0) const
{
    return i0;
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_1d<T, N0, P, M>::range_check_(ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < N0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_1d<T, N0, P, M>::static_array_1d(T *data/* [N0] */)
    : m_data(data)
{}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_1d<T, N0, P, M>::static_array_1d()
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_1d<T, N0, P, M>::static_array_1d(value_type const& t)
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_1d<T, N0, P, M>::static_array_1d(static_array_1d<T, N0, P, M> const& rhs)
{
    array_range_initialiser<T, allocator_type, P>::copy_construct(*this, data_(), rhs.data(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_1d<T, N0, P, M>::~static_array_1d() stlsoft_throw_0()
{
    if(!is_pointer_type<M>::value)
    {
        array_range_initialiser<T, allocator_type, P>::destroy(*this, data_(), size());
    }
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type &static_array_1d<T, N0, P, M>::at(ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0)
{
    range_check_(i0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type const& static_array_1d<T, N0, P, M>::at(ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0) const
{
    range_check_(i0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type &static_array_1d<T, N0, P, M>::at_unchecked(ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type const& static_array_1d<T, N0, P, M>::at_unchecked(ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type &static_array_1d<T, N0, P, M>::operator [](ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type const& static_array_1d<T, N0, P, M>::operator [](ss_typename_type_k static_array_1d<T, N0, P, M>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::reference static_array_1d<T, N0, P, M>::front()
{
    return at(0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::reference static_array_1d<T, N0, P, M>::back()
{
    return at(N0 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::const_reference static_array_1d<T, N0, P, M>::front() const
{
    return at(0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::const_reference static_array_1d<T, N0, P, M>::back() const
{
    return at(N0 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_1d<T, N0, P, M>::index_type static_array_1d<T, N0, P, M>::dimension0()
{
    return N0;
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_1d<T, N0, P, M>::index_type static_array_1d<T, N0, P, M>::size()
{
    return N0;
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_bool_t static_array_1d<T, N0, P, M>::empty()
{
    return false;
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_1d<T, N0, P, M>::index_type static_array_1d<T, N0, P, M>::max_size()
{
    return size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::iterator static_array_1d<T, N0, P, M>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::iterator static_array_1d<T, N0, P, M>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::const_iterator static_array_1d<T, N0, P, M>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::const_iterator static_array_1d<T, N0, P, M>::end() const
{
    return m_data + size();
}

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::reverse_iterator static_array_1d<T, N0, P, M>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::reverse_iterator static_array_1d<T, N0, P, M>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::const_reverse_iterator static_array_1d<T, N0, P, M>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::const_reverse_iterator static_array_1d<T, N0, P, M>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_size_t N0, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_1d<T, N0, P, M>::value_type const* static_array_1d<T, N0, P, M>::data() const
{
    return m_data;
}


// static_array_2d

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::pointer static_array_2d<T, N0, N1, P, M>::data_()
{
    return &m_data[0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::index_type static_array_2d<T, N0, N1, P, M>::calc_index_(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0, ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i1) const
{
    return (i0 * N1) + i1;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_2d<T, N0, N1, P, M>::range_check_(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0, ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i1) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0); STLSOFT_SUPPRESS_UNUSED(i1);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( !(i0 < N0) ||
        !(i1 < N1))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_2d<T, N0, N1, P, M>::range_check_(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < N0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_2d<T, N0, N1, P, M>::static_array_2d(T *data)
    : m_data(data)
{}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_2d<T, N0, N1, P, M>::static_array_2d()
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_2d<T, N0, N1, P, M>::static_array_2d(value_type const& t)
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_2d<T, N0, N1, P, M>::static_array_2d(static_array_2d<T, N0, N1, P, M> const& rhs)
{
    array_range_initialiser<T, allocator_type, P>::copy_construct(*this, data_(), rhs.data(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_2d<T, N0, N1, P, M>::~static_array_2d() stlsoft_throw_0()
{
    if(!is_pointer_type<M>::value)
    {
        array_range_initialiser<T, allocator_type, P>::destroy(*this, data_(), size());
    }
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::reference static_array_2d<T, N0, N1, P, M>::at(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0, ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i1)
{
    range_check_(i0, i1);

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_reference static_array_2d<T, N0, N1, P, M>::at(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0, ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i1) const
{
    range_check_(i0, i1);

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::reference static_array_2d<T, N0, N1, P, M>::at_unchecked(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0, ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i1)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1));

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_reference static_array_2d<T, N0, N1, P, M>::at_unchecked(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0, ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i1) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1));

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::dimension_type static_array_2d<T, N0, N1, P, M>::at(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0)
{
    range_check_(i0);

    return dimension_type(m_data + i0 * N1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::dimension_type const static_array_2d<T, N0, N1, P, M>::at(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0) const
{
    range_check_(i0);

    return dimension_type(m_data + i0 * N1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::dimension_type static_array_2d<T, N0, N1, P, M>::at_unchecked(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0)
{
    return dimension_type(m_data + i0 * N1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::dimension_type const static_array_2d<T, N0, N1, P, M>::at_unchecked(ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0) const
{
    return dimension_type(m_data + i0 * N1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::dimension_type static_array_2d<T, N0, N1, P, M>::operator [](ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0)
{
    return dimension_type(m_data + i0 * N1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_2d<T, N0, N1, P, M>::dimension_type static_array_2d<T, N0, N1, P, M>::operator [](ss_typename_type_k static_array_2d<T, N0, N1, P, M>::index_type i0) const
{
    return dimension_type(m_data + i0 * N1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::reference static_array_2d<T, N0, N1, P, M>::front()
{
    return at(0, 0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::reference static_array_2d<T, N0, N1, P, M>::back()
{
    return at(N0 - 1, N1 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_reference static_array_2d<T, N0, N1, P, M>::front() const
{
    return at(0, 0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_reference static_array_2d<T, N0, N1, P, M>::back() const
{
    return at(N0 - 1, N1 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::index_type static_array_2d<T, N0, N1, P, M>::dimension0()
{
    return N0;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::index_type static_array_2d<T, N0, N1, P, M>::dimension1()
{
    return N1;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::index_type static_array_2d<T, N0, N1, P, M>::size()
{
    return N0 * N1;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_bool_t static_array_2d<T, N0, N1, P, M>::empty()
{
    return false;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::index_type static_array_2d<T, N0, N1, P, M>::max_size()
{
    return size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::iterator static_array_2d<T, N0, N1, P, M>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::iterator static_array_2d<T, N0, N1, P, M>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_iterator static_array_2d<T, N0, N1, P, M>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_iterator static_array_2d<T, N0, N1, P, M>::end() const
{
    return m_data + size();
}

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::reverse_iterator static_array_2d<T, N0, N1, P, M>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::reverse_iterator static_array_2d<T, N0, N1, P, M>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_reverse_iterator static_array_2d<T, N0, N1, P, M>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::const_reverse_iterator static_array_2d<T, N0, N1, P, M>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_2d<T, N0, N1, P, M>::value_type const* static_array_2d<T, N0, N1, P, M>::data() const
{
    return m_data;
}

// static_array_3d

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::pointer static_array_3d<T, N0, N1, N2, P, M>::data_()
{
    return &m_data[0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::index_type static_array_3d<T, N0, N1, N2, P, M>::calc_index_(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i1, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i2) const
{
    return ((i0 * N1) + i1) * N2 + i2;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_3d<T, N0, N1, N2, P, M>::range_check_(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i1, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i2) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0); STLSOFT_SUPPRESS_UNUSED(i1); STLSOFT_SUPPRESS_UNUSED(i2);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( !(i0 < N0) ||
        !(i1 < N1) ||
        !(i2 < N2))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1 && i2 < N2));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_3d<T, N0, N1, N2, P, M>::range_check_(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < N0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_3d<T, N0, N1, N2, P, M>::static_array_3d(T *data)
    : m_data(data)
{}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_3d<T, N0, N1, N2, P, M>::static_array_3d()
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_3d<T, N0, N1, N2, P, M>::static_array_3d(value_type const& t)
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_3d<T, N0, N1, N2, P, M>::static_array_3d(static_array_3d<T, N0, N1, N2, P, M> const& rhs)
{
    array_range_initialiser<T, allocator_type, P>::copy_construct(*this, data_(), rhs.data(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_3d<T, N0, N1, N2, P, M>::~static_array_3d() stlsoft_throw_0()
{
    if(!is_pointer_type<M>::value)
    {
        array_range_initialiser<T, allocator_type, P>::destroy(*this, data_(), size());
    }
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::value_type &static_array_3d<T, N0, N1, N2, P, M>::at(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i1, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i2)
{
    range_check_(i0, i1, i2);

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::value_type const& static_array_3d<T, N0, N1, N2, P, M>::at(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i1, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i2) const
{
    range_check_(i0, i1, i2);

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::value_type &static_array_3d<T, N0, N1, N2, P, M>::at_unchecked(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i1, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i2)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1 && i2 < N2));

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::value_type const& static_array_3d<T, N0, N1, N2, P, M>::at_unchecked(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i1, ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i2) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1 && i2 < N2));

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::dimension_type static_array_3d<T, N0, N1, N2, P, M>::at(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0)
{
    range_check_(i0);

    return dimension_type(m_data + i0 * N1 * N2);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::dimension_type static_array_3d<T, N0, N1, N2, P, M>::at(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0) const
{
    range_check_(i0);

    return dimension_type(m_data + i0 * N1 * N2);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::dimension_type static_array_3d<T, N0, N1, N2, P, M>::at_unchecked(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0)
{
    return dimension_type(m_data + i0 * N1 * N2);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::dimension_type static_array_3d<T, N0, N1, N2, P, M>::at_unchecked(ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0) const
{
    return dimension_type(m_data + i0 * N1 * N2);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::dimension_type static_array_3d<T, N0, N1, N2, P, M>::operator [](ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0)
{
    return dimension_type(m_data + i0 * N1 * N2);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::dimension_type static_array_3d<T, N0, N1, N2, P, M>::operator [](ss_typename_type_k static_array_3d<T, N0, N1, N2, P, M>::index_type i0) const
{
    return dimension_type(m_data + i0 * N1 * N2);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::reference static_array_3d<T, N0, N1, N2, P, M>::front()
{
    return at(0, 0, 0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::reference static_array_3d<T, N0, N1, N2, P, M>::back()
{
    return at(N0 - 1, N1 - 1, N2 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::const_reference static_array_3d<T, N0, N1, N2, P, M>::front() const
{
    return at(0, 0, 0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::const_reference static_array_3d<T, N0, N1, N2, P, M>::back() const
{
    return at(N0 - 1, N1 - 1, N2 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::index_type static_array_3d<T, N0, N1, N2, P, M>::dimension0()
{
    return N0;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::index_type static_array_3d<T, N0, N1, N2, P, M>::dimension1()
{
    return N1;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::index_type static_array_3d<T, N0, N1, N2, P, M>::dimension2()
{
    return N2;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::index_type static_array_3d<T, N0, N1, N2, P, M>::size()
{
    return N0 * N1 * N2;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_bool_t static_array_3d<T, N0, N1, N2, P, M>::empty()
{
    return false;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::index_type static_array_3d<T, N0, N1, N2, P, M>::max_size()
{
    return size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::iterator static_array_3d<T, N0, N1, N2, P, M>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::iterator static_array_3d<T, N0, N1, N2, P, M>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::const_iterator static_array_3d<T, N0, N1, N2, P, M>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::const_iterator static_array_3d<T, N0, N1, N2, P, M>::end() const
{
    return m_data + size();
}

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::reverse_iterator static_array_3d<T, N0, N1, N2, P, M>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::reverse_iterator static_array_3d<T, N0, N1, N2, P, M>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::const_reverse_iterator static_array_3d<T, N0, N1, N2, P, M>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::const_reverse_iterator static_array_3d<T, N0, N1, N2, P, M>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_3d<T, N0, N1, N2, P, M>::value_type const* static_array_3d<T, N0, N1, N2, P, M>::data() const
{
    return m_data;
}

// static_array_4d

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::pointer static_array_4d<T, N0, N1, N2, N3, P, M>::data_()
{
    return &m_data[0];
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::calc_index_(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i1, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i2, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i3) const
{
    return (((i0 * N1) + i1) * N2 + i2) * N3 + i3;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_4d<T, N0, N1, N2, N3, P, M>::range_check_(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i1, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i2, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i3) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0); STLSOFT_SUPPRESS_UNUSED(i1); STLSOFT_SUPPRESS_UNUSED(i2); STLSOFT_SUPPRESS_UNUSED(i3);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( !(i0 < N0) ||
        !(i1 < N1) ||
        !(i2 < N2) ||
        !(i3 < N3))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1 && i2 < N2 && i3 < N3));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline void static_array_4d<T, N0, N1, N2, N3, P, M>::range_check_(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
    STLSOFT_SUPPRESS_UNUSED(i0);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < N0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("static array index out of range"));
    }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_4d<T, N0, N1, N2, N3, P, M>::static_array_4d()
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_4d<T, N0, N1, N2, N3, P, M>::static_array_4d(value_type const& t)
{
    array_range_initialiser<T, allocator_type, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_4d<T, N0, N1, N2, N3, P, M>::static_array_4d(static_array_4d<T, N0, N1, N2, N3, P, M> const& rhs)
{
    array_range_initialiser<T, allocator_type, P>::copy_construct(*this, data_(), rhs.data(), size());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline static_array_4d<T, N0, N1, N2, N3, P, M>::~static_array_4d() stlsoft_throw_0()
{
    if(!is_pointer_type<M>::value)
    {
        array_range_initialiser<T, allocator_type, P>::destroy(*this, data_(), size());
    }
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::value_type &static_array_4d<T, N0, N1, N2, N3, P, M>::at(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i1, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i2, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i3)
{
    range_check_(i0, i1, i2, i3);

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::value_type const& static_array_4d<T, N0, N1, N2, N3, P, M>::at(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i1, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i2, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i3) const
{
    range_check_(i0, i1, i2, i3);

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::value_type &static_array_4d<T, N0, N1, N2, N3, P, M>::at_unchecked(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i1, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i2, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i3)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1 && i2 < N2 && i3 < N3));

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::value_type const& static_array_4d<T, N0, N1, N2, N3, P, M>::at_unchecked(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i1, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i2, ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i3) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", (i0 < N0 && i1 < N1 && i2 < N2 && i3 < N3));

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::dimension_type static_array_4d<T, N0, N1, N2, N3, P, M>::at(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0)
{
    range_check_(i0);

    return dimension_type(m_data + i0 * N1 * N2 * N3);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::dimension_type static_array_4d<T, N0, N1, N2, N3, P, M>::at(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0) const
{
    range_check_(i0);

    return dimension_type(m_data + i0 * N1 * N2 * N3);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::dimension_type static_array_4d<T, N0, N1, N2, N3, P, M>::at_unchecked(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return dimension_type(m_data + i0 * N1 * N2 * N3);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::dimension_type static_array_4d<T, N0, N1, N2, N3, P, M>::at_unchecked(ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return dimension_type(m_data + i0 * N1 * N2 * N3);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::dimension_type static_array_4d<T, N0, N1, N2, N3, P, M>::operator [](ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return dimension_type(m_data + i0 * N1 * N2 * N3);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline const ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::dimension_type static_array_4d<T, N0, N1, N2, N3, P, M>::operator [](ss_typename_type_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("static array index out of range", i0 < N0);

    return dimension_type(m_data + i0 * N1 * N2 * N3);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::reference static_array_4d<T, N0, N1, N2, N3, P, M>::front()
{
    return at(0, 0, 0, 0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::reference static_array_4d<T, N0, N1, N2, N3, P, M>::back()
{
    return at(N0 - 1, N1 - 1, N2 - 1, N3 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::const_reference static_array_4d<T, N0, N1, N2, N3, P, M>::front() const
{
    return at(0, 0, 0, 0);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::const_reference static_array_4d<T, N0, N1, N2, N3, P, M>::back() const
{
    return at(N0 - 1, N1 - 1, N2 - 1, N3 - 1);
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::dimension0()
{
    return N0;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::dimension1()
{
    return N1;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::dimension2()
{
    return N2;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::dimension3()
{
    return N3;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::size()
{
    return N0 * N1 * N2 * N3;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_bool_t static_array_4d<T, N0, N1, N2, N3, P, M>::empty()
{
    return false;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline /* static */ ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::index_type static_array_4d<T, N0, N1, N2, N3, P, M>::max_size()
{
    return size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::iterator static_array_4d<T, N0, N1, N2, N3, P, M>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::iterator static_array_4d<T, N0, N1, N2, N3, P, M>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::const_iterator static_array_4d<T, N0, N1, N2, N3, P, M>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::const_iterator static_array_4d<T, N0, N1, N2, N3, P, M>::end() const
{
    return m_data + size();
}

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::reverse_iterator static_array_4d<T, N0, N1, N2, N3, P, M>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::reverse_iterator static_array_4d<T, N0, N1, N2, N3, P, M>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::const_reverse_iterator static_array_4d<T, N0, N1, N2, N3, P, M>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::const_reverse_iterator static_array_4d<T, N0, N1, N2, N3, P, M>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_size_t N0, ss_size_t N1, ss_size_t N2, ss_size_t N3, ss_typename_param_k P, ss_typename_param_k M>
inline ss_typename_type_ret_k static_array_4d<T, N0, N1, N2, N3, P, M>::value_type const* static_array_4d<T, N0, N1, N2, N3, P, M>::data() const
{
    return m_data;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

# if !defined(STLSOFT_COMPILER_IS_MSVC) || \
     _MSC_VER >= 1200

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
inline ss_size_t array_size(static_array_1d<T, N0, P, M> const& ar)
{
    STLSOFT_SUPPRESS_UNUSED(ar);    // Required by VC++ 7.1

    return ar.size();
}

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
inline ss_size_t array_size(static_array_2d<T, N0, N1, P, M> const& ar)
{
    STLSOFT_SUPPRESS_UNUSED(ar);    // Required by VC++ 7.1

    return ar.size();
}

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
inline ss_size_t array_size(static_array_3d<T, N0, N1, N2, P, M> const& ar)
{
    STLSOFT_SUPPRESS_UNUSED(ar);    // Required by VC++ 7.1

    return ar.size();
}

template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_size_t N3
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
inline ss_size_t array_size(static_array_4d<T, N0, N1, N2, N3, P, M> const& ar)
{
    STLSOFT_SUPPRESS_UNUSED(ar);    // Required by VC++ 7.1

    return ar.size();
}

#if 0
template<   ss_typename_param_k T
        ,   ss_size_t N0
        ,   ss_size_t N1
        ,   ss_size_t N2
        ,   ss_size_t N3
        ,   ss_size_t N4
        ,   ss_typename_param_k P
        ,   ss_typename_param_k M
        >
inline ss_size_t array_size(static_array_5d<T, N0, N1, N2, N3, N4, P, M> const& ar)
{
    STLSOFT_SUPPRESS_UNUSED(ar);    // Required by VC++ 7.1

    return ar.size();
}
#endif /* 0 */

#endif /* compiler */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/static_array_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_STATIC_ARRAY */

/* ///////////////////////////// end of file //////////////////////////// */
