/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/containers/fixed_array.hpp
 *
 * Purpose:     Contains the fixed_array_1d, fixed_array_2d, fixed_array_3d,
 *              fixed_array_4d template classes.
 *
 * Created:     4th August 1998
 * Updated:     10th August 2009
 *
 * Thanks to:   Neal Becker for suggesting the uninitialised mode,
 *              requesting the function call operator, and for requesting
 *              the with-allocator constructor overloads.
 *
 *              Thorsten Ottosen for suggesting swap() and mutating data(),
 *              and for providing suggested implementations.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/containers/fixed_array.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::fixed_array_1d,
 *    stlsoft::fixed_array_2d, stlsoft::fixed_array_3d, and
 *    stlsoft::fixed_array_4d multidimensional array class templates
 *   (\ref group__library__containers "Containers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY
#define STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY_MAJOR      4
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY_MINOR      9
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY_REVISION   5
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY_EDIT       191
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
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

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/containers/fixed_array.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_CONTAINERS_UTIL_HPP_ARRAY_POLICIES
# include <stlsoft/containers/util/array_policies.hpp>  // for stlsoft::do_construction()
#endif /* !STLSOFT_INCL_STLSOFT_CONTAINERS_UTIL_HPP_ARRAY_POLICIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    defined(STLSOFT_CF_STD_LIBRARY_IS_STLPORT)
# ifndef STLSOFT_INCL_STRING
#  define STLSOFT_INCL_STRING
#  include <string>                      // for std::string - sigh!
# endif /* !STLSOFT_INCL_STRING */
#endif /* compiler */
#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                    // for std::out_of_range
#endif /* !STLSOFT_INCL_STDEXCEPT */

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
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
class fixed_array_1d;

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
class fixed_array_2d;

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
class fixed_array_3d;

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
class fixed_array_4d;

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
class fixed_array_5d;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief 1 dimensional fixed array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param A The allocator type
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
        ,   ss_typename_param_k P = do_construction<T>
        ,   ss_bool_t           R = true
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_typename_param_k R
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
// class fixed_array_1d
class fixed_array_1d
    : protected A
    , public stl_collection_tag
{
public:
    typedef fixed_array_1d<T, A, P, R>              class_type;
    typedef T                                       dimension_element_type;
    typedef /* const */ dimension_element_type          const_dimension_element_type;
    typedef A                                       allocator_type;
    typedef T                                       value_type;
    typedef value_type&                             reference;
    typedef value_type const&                       const_reference;
    typedef value_type*                             pointer;
    typedef value_type const*                       const_pointer;
    typedef ss_size_t                               size_type;
    typedef ss_size_t                               index_type;
    typedef ss_ptrdiff_t                            difference_type;
    typedef ss_bool_t                               bool_type;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type     iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type     const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >           reverse_iterator;
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >           const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
private:
                  fixed_array_1d(T *data, index_type d0);
public:
    ss_explicit_k fixed_array_1d(index_type d0);
                  fixed_array_1d(index_type d0, allocator_type const& ator);
                  fixed_array_1d(index_type d0, value_type const& t);
                  fixed_array_1d(index_type d0, value_type const& t, allocator_type const& ator);
                  fixed_array_1d(class_type const& rhs);
                 ~fixed_array_1d() stlsoft_throw_0();

    allocator_type  get_allocator() const;

    void          swap(class_type& rhs) stlsoft_throw_0();

// Access
public:
    reference               at(index_type i0);
    const_reference         at(index_type i0) const;

    reference               at_unchecked(index_type i0);
    const_reference         at_unchecked(index_type i0) const;

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
    reference               operator ()(index_type i0);
    const_reference         operator ()(index_type i0) const;
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

    reference               operator [](index_type i0);
    const_reference         operator [](index_type i0) const;

    /// Providing the evil operator, in order to support &ar[0]
//    pointer                 operator &();
    /// Providing the evil operator, in order to support &ar[0]
//    const_pointer           operator &() const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    index_type              dimension0() const;
    index_type              size() const;
    bool_type               empty() const;
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    pointer                 data();
    const_pointer           data() const;

// Implementation
private:
    pointer         allocate_(size_type n);
    void            deallocate_(pointer p, size_type n);

    pointer         data_();
    index_type      calc_index_(index_type i0) const;
    void            range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

    allocator_type& get_allocator_();

// Members
private:
    T*          m_data;
    index_type  m_d0;

    friend class fixed_array_2d<T, A, P, true>;
    friend class fixed_array_2d<T, A, P, false>;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

/** \brief 2 dimensional fixed array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param A The allocator type
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
        ,   ss_typename_param_k P = do_construction<T>
        ,   ss_bool_t           R = true
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
// class fixed_array_2d
class fixed_array_2d
    : protected A
    , public stl_collection_tag
{
public:
    typedef fixed_array_2d<T, A, P, R>              class_type;
    typedef fixed_array_1d<T, A, P, false>          dimension_element_type;
    typedef /* const */ dimension_element_type          const_dimension_element_type;
    typedef A                                       allocator_type;
    typedef T                                       value_type;
    typedef value_type&                             reference;
    typedef value_type const&                       const_reference;
    typedef value_type*                             pointer;
    typedef value_type const*                       const_pointer;
    typedef ss_size_t                               size_type;
    typedef ss_size_t                               index_type;
    typedef ss_ptrdiff_t                            difference_type;
    typedef ss_bool_t                               bool_type;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type     iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type     const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >           reverse_iterator;
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >           const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
private:
    fixed_array_2d(T *data, index_type d0, index_type d1);
public:
    fixed_array_2d(index_type d0, index_type d1);
    fixed_array_2d(index_type d0, index_type d1, allocator_type const& ator);
    fixed_array_2d(index_type d0, index_type d1, value_type const& t);
    fixed_array_2d(index_type d0, index_type d1, value_type const& t, allocator_type const& ator);
    fixed_array_2d(class_type const& rhs);
    ~fixed_array_2d() stlsoft_throw_0();

    allocator_type  get_allocator() const;

    void swap(class_type& rhs) stlsoft_throw_0();

// Access
public:
    reference               at(index_type i0, index_type i1);
    const_reference         at(index_type i0, index_type i1) const;

    reference               at_unchecked(index_type i0, index_type i1);
    const_reference         at_unchecked(index_type i0, index_type i1) const;

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
    reference               operator ()(index_type i0, index_type i1);
    const_reference         operator ()(index_type i0, index_type i1) const;
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

    dimension_element_type          at(index_type i0);
    const_dimension_element_type    at(index_type i0) const;

    dimension_element_type          at_unchecked(index_type i0);
    const_dimension_element_type    at_unchecked(index_type i0) const;

    dimension_element_type          operator [](index_type i0);
    const_dimension_element_type    operator [](index_type i0) const;

    /// A reference to the first element in the array
    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    index_type              dimension0() const;
    index_type              dimension1() const;
    index_type              size() const;
    bool_type               empty() const;
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    pointer                 data();
    const_pointer           data() const;

// Implementation
private:
    pointer         allocate_(size_type n);
    void            deallocate_(pointer p, size_type n);

    pointer         data_();
    index_type      calc_index_(index_type i0, index_type i1) const;
    void            range_check_(index_type i0, index_type i1) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void            range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

    allocator_type& get_allocator_();

// Members
private:
    T*          m_data;
    index_type  m_d0;
    index_type  m_d1;
    size_type   m_size;

    friend class fixed_array_3d<T, A, P, true>;
    friend class fixed_array_3d<T, A, P, false>;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

/** \brief 3 dimensional fixed array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param A The allocator type
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
        ,   ss_typename_param_k P = do_construction<T>
        ,   ss_bool_t           R = true
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
// class fixed_array_3d
class fixed_array_3d
    : protected A
    , public stl_collection_tag
{
public:
    typedef fixed_array_3d<T, A, P, R>              class_type;
    typedef fixed_array_2d<T, A, P, false>          dimension_element_type;
    typedef /* const */ dimension_element_type          const_dimension_element_type;
    typedef A                                       allocator_type;
    typedef T                                       value_type;
    typedef value_type&                             reference;
    typedef value_type const&                       const_reference;
    typedef value_type*                             pointer;
    typedef value_type const*                       const_pointer;
    typedef ss_size_t                               size_type;
    typedef ss_size_t                               index_type;
    typedef ss_ptrdiff_t                            difference_type;
    typedef ss_bool_t                               bool_type;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type     iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type     const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >           reverse_iterator;
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >           const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
private:
    fixed_array_3d(pointer data, index_type d0, index_type d1, index_type d2);
public:
    fixed_array_3d(index_type d0, index_type d1, index_type d2);
    fixed_array_3d(index_type d0, index_type d1, index_type d2, allocator_type const& ator);
    fixed_array_3d(index_type d0, index_type d1, index_type d2, value_type const& t);
    fixed_array_3d(index_type d0, index_type d1, index_type d2, value_type const& t, allocator_type const& ator);
    fixed_array_3d(class_type const& rhs);
    ~fixed_array_3d() stlsoft_throw_0();

    allocator_type  get_allocator() const;

    void swap(class_type& rhs) stlsoft_throw_0();

// Access
public:
    reference               at(index_type i0, index_type i1, index_type i2);
    const_reference         at(index_type i0, index_type i1, index_type i3) const;

    reference               at_unchecked(index_type i0, index_type i1, index_type i2);
    const_reference         at_unchecked(index_type i0, index_type i1, index_type i2) const;

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
    reference               operator ()(index_type i0, index_type i1, index_type i2);
    const_reference         operator ()(index_type i0, index_type i1, index_type i2) const;
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

    dimension_element_type          at(index_type i0);
    const_dimension_element_type    at(index_type i0) const;

    dimension_element_type          at_unchecked(index_type i0);
    const_dimension_element_type    at_unchecked(index_type i0) const;

    dimension_element_type          operator [](index_type i0);
    const_dimension_element_type    operator [](index_type i0) const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    index_type              dimension0() const;
    index_type              dimension1() const;
    index_type              dimension2() const;
    index_type              size() const;
    bool_type               empty() const;
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    pointer                 data();
    const_pointer           data() const;

// Implementation
private:
    pointer         allocate_(size_type n);
    void            deallocate_(pointer p, size_type n);

    pointer         data_();
    index_type      calc_index_(index_type i0, index_type i1, index_type i2) const;
    void            range_check_(index_type i0, index_type i1, index_type i2) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void            range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

    allocator_type& get_allocator_();

// Members
private:
    T*          m_data;
    index_type  m_d0;
    index_type  m_d1;
    index_type  m_d2;

    friend class fixed_array_4d<T, A, P, true>;
    friend class fixed_array_4d<T, A, P, false>;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

/** \brief 4 dimensional fixed array
 *
 * \ingroup group__library__containers
 *
 * \param T The value type
 * \param A The allocator type
 * \param P The construction policy type
 */
template<   ss_typename_param_k T
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
        ,   ss_typename_param_k P = do_construction<T>
        ,   ss_bool_t           R = true
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
// class fixed_array_4d
class fixed_array_4d
    : protected A
    , public stl_collection_tag
{
public:
    typedef fixed_array_4d<T, A, P, R>              class_type;
    typedef fixed_array_3d<T, A, P, false>          dimension_element_type;
    typedef /* const */ dimension_element_type          const_dimension_element_type;
    typedef A                                       allocator_type;
    typedef T                                       value_type;
    typedef value_type&                             reference;
    typedef value_type const&                       const_reference;
    typedef value_type*                             pointer;
    typedef value_type const*                       const_pointer;
    typedef ss_size_t                               size_type;
    typedef ss_size_t                               index_type;
    typedef ss_ptrdiff_t                            difference_type;
    typedef ss_bool_t                               bool_type;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type     iterator;
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type     const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >           reverse_iterator;
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >           const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
private:
    fixed_array_4d(T *data, index_type d0, index_type d1, index_type d2, index_type d3);
public:
    fixed_array_4d(index_type d0, index_type d1, index_type d2, index_type d3);
    fixed_array_4d(index_type d0, index_type d1, index_type d2, index_type d3, allocator_type const& ator);
    fixed_array_4d(index_type d0, index_type d1, index_type d2, index_type d3, value_type const& t);
    fixed_array_4d(index_type d0, index_type d1, index_type d2, index_type d3, value_type const& t, allocator_type const& ator);
    fixed_array_4d(class_type const& rhs);
    ~fixed_array_4d() stlsoft_throw_0();

    allocator_type  get_allocator() const;

    void swap(class_type& rhs) stlsoft_throw_0();

// Access
public:
    reference               at(index_type i0, index_type i1, index_type i2, index_type i3);
    const_reference         at(index_type i0, index_type i1, index_type i2, index_type i3) const;

    reference               at_unchecked(index_type i0, index_type i1, index_type i2, index_type i3);
    const_reference         at_unchecked(index_type i0, index_type i1, index_type i2, index_type i3) const;

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
    reference               operator ()(index_type i0, index_type i1, index_type i2, index_type i3);
    const_reference         operator ()(index_type i0, index_type i1, index_type i2, index_type i3) const;
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

    dimension_element_type          at(index_type i0);
    const_dimension_element_type    at(index_type i0) const;

    dimension_element_type          at_unchecked(index_type i0);
    const_dimension_element_type    at_unchecked(index_type i0) const;

    dimension_element_type          operator [](index_type i0);
    const_dimension_element_type    operator [](index_type i0) const;

    reference               front();
    reference               back();
    const_reference         front() const;
    const_reference         back() const;

// State
public:
    index_type              dimension0() const;
    index_type              dimension1() const;
    index_type              dimension2() const;
    index_type              dimension3() const;
    index_type              size() const;
    bool_type               empty() const;
    static size_type        max_size();

// Iteration
public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    reverse_iterator        rbegin();
    reverse_iterator        rend();
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    pointer                 data();
    const_pointer           data() const;

// Implementation
private:
    pointer         allocate_(size_type n);
    void            deallocate_(pointer p, size_type n);

    pointer         data_();
    index_type      calc_index_(index_type i0, index_type i1, index_type i2, index_type i3) const;
    void            range_check_(index_type i0, index_type i1, index_type i2, index_type i3) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );
    void            range_check_(index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) );

    allocator_type& get_allocator_();

// Members
private:
    T*          m_data;
    index_type  m_d0;
    index_type  m_d1;
    index_type  m_d2;
    index_type  m_d3;

    friend class fixed_array_5d<T, A, P, true>;
    friend class fixed_array_5d<T, A, P, false>;

// Not to be implemented
private:
    class_type const& operator =(class_type const& rhs);
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/fixed_array_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// fixed_array_1d

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::pointer fixed_array_1d<T, A, P, R>::allocate_(ss_typename_type_k fixed_array_1d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    return ator.allocate(n, NULL);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_1d<T, A, P, R>::deallocate_(ss_typename_type_k fixed_array_1d<T, A, P, R>::pointer p, ss_typename_type_k fixed_array_1d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    ator.deallocate(p, n);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::pointer fixed_array_1d<T, A, P, R>::data_()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::index_type fixed_array_1d<T, A, P, R>::calc_index_(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0) const
{
    return i0;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_1d<T, A, P, R>::range_check_(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < m_d0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_1d<T, A, P, R>::allocator_type& fixed_array_1d<T, A, P, R>::get_allocator_()
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(T* src, ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type d0)
    : m_data(src)
    , m_d0(d0)
{
    STLSOFT_STATIC_ASSERT(!R);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type d0)
    : m_data(allocate_(d0))
    , m_d0(d0)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_1d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0))
    , m_d0(d0)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type d0, value_type const& t)
    : m_data(allocate_(d0))
    , m_d0(d0)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type d0, value_type const& t, ss_typename_type_k fixed_array_1d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0))
    , m_d0(d0)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

#ifdef STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(fixed_array_1d<T, A, P, R> const& rhs)
    : m_data(R ? allocate_(rhs.dimension0()) : rhs.m_data)
    , m_d0(rhs.dimension0())
{
    if(R)
    {
        array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
    }
}

#else /* ? STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::fixed_array_1d(fixed_array_1d<T, A, P, R> const& rhs)
    : m_data(allocate_(rhs.dimension0()))
    , m_d0(rhs.dimension0())
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
}

#endif /* STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_1d<T, A, P, R>::~fixed_array_1d() stlsoft_throw_0()
{
    if(R)
    {
        array_range_initialiser<T, A, P>::destroy(*this, data_(), size());
        deallocate_(m_data, size());
    }
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_1d<T, A, P, R>::allocator_type fixed_array_1d<T, A, P, R>::get_allocator() const
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_1d<T, A, P, R>::swap(ss_typename_type_k fixed_array_1d<T, A, P, R>::class_type& rhs) stlsoft_throw_0()
{
    // We don't need to do any construct and swap here, because all the
    // variables that are being swapped are simple types (integers and
    // pointers).

    std_swap(get_allocator_(), rhs.get_allocator_());
    std_swap(m_data, rhs.m_data);
    std_swap(m_d0, rhs.m_d0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reference fixed_array_1d<T, A, P, R>::at(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0)
{
    range_check_(i0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reference fixed_array_1d<T, A, P, R>::at(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0) const
{
    range_check_(i0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reference fixed_array_1d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return m_data[i0];
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reference fixed_array_1d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return m_data[i0];
}

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reference fixed_array_1d<T, A, P, R>::operator ()(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0)
{
    return at_unchecked(i0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reference fixed_array_1d<T, A, P, R>::operator ()(ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0) const
{
    return at_unchecked(i0);
}
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reference fixed_array_1d<T, A, P, R>::operator [](ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0)
{
    return at_unchecked(i0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reference fixed_array_1d<T, A, P, R>::operator [](ss_typename_type_k fixed_array_1d<T, A, P, R>::index_type i0) const
{
    return at_unchecked(i0);
}

#if 0
template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::pointer fixed_array_1d<T, A, P, R>::operator &()
{
    return &m_data[0];
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_pointer fixed_array_1d<T, A, P, R>::operator &() const
{
    return &m_data[0];
}
#endif /* 0 */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reference fixed_array_1d<T, A, P, R>::front()
{
    return at(0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reference fixed_array_1d<T, A, P, R>::back()
{
    return at(m_d0 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reference fixed_array_1d<T, A, P, R>::front() const
{
    return at(0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reference fixed_array_1d<T, A, P, R>::back() const
{
    return at(m_d0 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::index_type fixed_array_1d<T, A, P, R>::dimension0() const
{
    return m_d0;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::index_type fixed_array_1d<T, A, P, R>::size() const
{
    return m_d0;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::bool_type fixed_array_1d<T, A, P, R>::empty() const
{
    return 0 == size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline /* static */ ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::size_type fixed_array_1d<T, A, P, R>::max_size()
{
    return static_cast<size_type>(-1) / sizeof(T);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::iterator fixed_array_1d<T, A, P, R>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::iterator fixed_array_1d<T, A, P, R>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_iterator fixed_array_1d<T, A, P, R>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_iterator fixed_array_1d<T, A, P, R>::end() const
{
    return m_data + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reverse_iterator fixed_array_1d<T, A, P, R>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::reverse_iterator fixed_array_1d<T, A, P, R>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reverse_iterator fixed_array_1d<T, A, P, R>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_reverse_iterator fixed_array_1d<T, A, P, R>::rend() const
{
    return const_reverse_iterator(begin());
}

#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::pointer fixed_array_1d<T, A, P, R>::data()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_1d<T, A, P, R>::const_pointer fixed_array_1d<T, A, P, R>::data() const
{
    return m_data;
}


// fixed_array_2d

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::pointer fixed_array_2d<T, A, P, R>::allocate_(ss_typename_type_k fixed_array_2d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    return ator.allocate(n, NULL);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_2d<T, A, P, R>::deallocate_(ss_typename_type_k fixed_array_2d<T, A, P, R>::pointer p, ss_typename_type_k fixed_array_2d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    ator.deallocate(p, n);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::pointer fixed_array_2d<T, A, P, R>::data_()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::index_type fixed_array_2d<T, A, P, R>::calc_index_(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1) const
{
    return (i0 * m_d1) + i1;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_2d<T, A, P, R>::range_check_(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( !(i0 < m_d0) ||
        !(i1 < m_d1))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_2d<T, A, P, R>::allocator_type& fixed_array_2d<T, A, P, R>::get_allocator_()
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_2d<T, A, P, R>::range_check_(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < m_d0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(T* src, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d1)
    : m_data(src)
    , m_d0(d0)
    , m_d1(d1)
    , m_size(d0 * d1)
{
    STLSOFT_STATIC_ASSERT(!R);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d1)
    : m_data(allocate_(d0 * d1))
    , m_d0(d0)
    , m_d1(d1)
    , m_size(d0 * d1)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d1, ss_typename_type_k fixed_array_2d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0 * d1))
    , m_d0(d0)
    , m_d1(d1)
    , m_size(d0 * d1)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d1, value_type const& t)
    : m_data(allocate_(d0 * d1))
    , m_d0(d0)
    , m_d1(d1)
    , m_size(d0 * d1)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type d1, value_type const& t, ss_typename_type_k fixed_array_2d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0 * d1))
    , m_d0(d0)
    , m_d1(d1)
    , m_size(d0 * d1)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

#ifdef STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(fixed_array_2d<T, A, P, R> const& rhs)
    : m_data(R ? allocate_(rhs.dimension0() * rhs.dimension1()) : rhs.m_data)
    , m_d0(rhs.dimension0())
    , m_d1(rhs.dimension1())
    , m_size(rhs.dimension0() * rhs.dimension1())
{
    if(R)
    {
        array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
    }
}

#else /* ? STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::fixed_array_2d(fixed_array_2d<T, A, P, R> const& rhs)
    : m_data(allocate_(rhs.dimension0() * rhs.dimension1()))
    , m_d0(rhs.dimension0())
    , m_d1(rhs.dimension1())
    , m_size(rhs.dimension0() * rhs.dimension1())
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
}

#endif /* STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_2d<T, A, P, R>::~fixed_array_2d() stlsoft_throw_0()
{
    if(R)
    {
        array_range_initialiser<T, A, P>::destroy(*this, data_(), size());
        deallocate_(m_data, size());
    }
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_2d<T, A, P, R>::allocator_type fixed_array_2d<T, A, P, R>::get_allocator() const
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_2d<T, A, P, R>::swap(ss_typename_type_k fixed_array_2d<T, A, P, R>::class_type& rhs) stlsoft_throw_0()
{
    // We don't need to do any construct and swap here, because all the
    // variables that are being swapped are simple types (integers and
    // pointers).

    std_swap(get_allocator_(), rhs.get_allocator_());
    std_swap(m_data, rhs.m_data);
    std_swap(m_d0, rhs.m_d0);
    std_swap(m_d1, rhs.m_d1);
    std_swap(m_size, rhs.m_size);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reference fixed_array_2d<T, A, P, R>::at(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1)
{
    range_check_(i0, i1);

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reference fixed_array_2d<T, A, P, R>::at(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1) const
{
    range_check_(i0, i1);

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reference fixed_array_2d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1));

    return *(m_data + calc_index_(i0, i1));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reference fixed_array_2d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1));

    return *(m_data + calc_index_(i0, i1));
}

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reference fixed_array_2d<T, A, P, R>::operator ()(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1)
{
    return at_unchecked(i0, i1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reference fixed_array_2d<T, A, P, R>::operator ()(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i1) const
{
    return at_unchecked(i0, i1);
}
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::dimension_element_type fixed_array_2d<T, A, P, R>::at(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0)
{
    range_check_(i0);

    return dimension_element_type(m_data + i0 * m_d1, m_d1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_dimension_element_type fixed_array_2d<T, A, P, R>::at(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0) const
{
    range_check_(i0);

    return dimension_element_type(m_data + i0 * m_d1, m_d1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::dimension_element_type fixed_array_2d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1, m_d1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_dimension_element_type fixed_array_2d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1, m_d1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::dimension_element_type fixed_array_2d<T, A, P, R>::operator [](ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1, m_d1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_dimension_element_type fixed_array_2d<T, A, P, R>::operator [](ss_typename_type_k fixed_array_2d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1, m_d1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reference fixed_array_2d<T, A, P, R>::front()
{
    return at(0, 0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reference fixed_array_2d<T, A, P, R>::back()
{
    return at(m_d0 - 1, m_d1 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reference fixed_array_2d<T, A, P, R>::front() const
{
    return at(0, 0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reference fixed_array_2d<T, A, P, R>::back() const
{
    return at(m_d0 - 1, m_d1 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::index_type fixed_array_2d<T, A, P, R>::dimension0() const
{
    return m_d0;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::index_type fixed_array_2d<T, A, P, R>::dimension1() const
{
    return m_d1;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::index_type fixed_array_2d<T, A, P, R>::size() const
{
    STLSOFT_ASSERT(m_size == m_d0 * m_d1);

    return m_size;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::bool_type fixed_array_2d<T, A, P, R>::empty() const
{
    return 0 == size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline /* static */ ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::size_type fixed_array_2d<T, A, P, R>::max_size()
{
    return static_cast<size_type>(-1) / sizeof(T);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::iterator fixed_array_2d<T, A, P, R>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::iterator fixed_array_2d<T, A, P, R>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_iterator fixed_array_2d<T, A, P, R>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_iterator fixed_array_2d<T, A, P, R>::end() const
{
    return m_data + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reverse_iterator fixed_array_2d<T, A, P, R>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::reverse_iterator fixed_array_2d<T, A, P, R>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reverse_iterator fixed_array_2d<T, A, P, R>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_reverse_iterator fixed_array_2d<T, A, P, R>::rend() const
{
    return const_reverse_iterator(begin());
}

#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::pointer fixed_array_2d<T, A, P, R>::data()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_2d<T, A, P, R>::const_pointer fixed_array_2d<T, A, P, R>::data() const
{
    return m_data;
}

// fixed_array_3d

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::pointer fixed_array_3d<T, A, P, R>::allocate_(ss_typename_type_k fixed_array_3d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    return ator.allocate(n, NULL);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_3d<T, A, P, R>::deallocate_(ss_typename_type_k fixed_array_3d<T, A, P, R>::pointer p, ss_typename_type_k fixed_array_3d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    ator.deallocate(p, n);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::pointer fixed_array_3d<T, A, P, R>::data_()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::index_type fixed_array_3d<T, A, P, R>::calc_index_(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2) const
{
    return ((i0 * m_d1) + i1) * m_d2 + i2;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_3d<T, A, P, R>::range_check_(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( !(i0 < m_d0) ||
        !(i1 < m_d1) ||
        !(i2 < m_d2))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1 && i2 < m_d2));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_3d<T, A, P, R>::allocator_type& fixed_array_3d<T, A, P, R>::get_allocator_()
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_3d<T, A, P, R>::range_check_(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < m_d0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(ss_typename_type_k fixed_array_3d<T, A, P, R>::pointer src, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d2)
    : m_data(src)
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
{
    STLSOFT_STATIC_ASSERT(!R);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d2)
    : m_data(allocate_(d0 * d1 * d2))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d2, ss_typename_type_k fixed_array_3d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0 * d1 * d2))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d2, ss_typename_type_k fixed_array_3d<T, A, P, R>::value_type const& t)
    : m_data(allocate_(d0 * d1 * d2))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type d2, ss_typename_type_k fixed_array_3d<T, A, P, R>::value_type const& t, ss_typename_type_k fixed_array_3d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0 * d1 * d2))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

#ifdef STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(fixed_array_3d<T, A, P, R> const& rhs)
    : m_data(R ? allocate_(rhs.dimension0() * rhs.dimension1() * rhs.dimension2()) : rhs.m_data)
    , m_d0(rhs.dimension0())
    , m_d1(rhs.dimension1())
    , m_d2(rhs.dimension2())
{
    if(R)
    {
        array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
    }
}

#else /* ? STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::fixed_array_3d(fixed_array_3d<T, A, P, R> const& rhs)
    : m_data(allocate_(rhs.dimension0() * rhs.dimension1() * rhs.dimension2()))
    , m_d0(rhs.dimension0())
    , m_d1(rhs.dimension1())
    , m_d2(rhs.dimension2())
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
}

#endif /* STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_3d<T, A, P, R>::~fixed_array_3d() stlsoft_throw_0()
{
    if(R)
    {
        array_range_initialiser<T, A, P>::destroy(*this, data_(), size());
        deallocate_(m_data, size());
    }
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_3d<T, A, P, R>::allocator_type fixed_array_3d<T, A, P, R>::get_allocator() const
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_3d<T, A, P, R>::swap(ss_typename_type_k fixed_array_3d<T, A, P, R>::class_type& rhs) stlsoft_throw_0()
{
    // We don't need to do any construct and swap here, because all the
    // variables that are being swapped are simple types (integers and
    // pointers).

    std_swap(get_allocator_(), rhs.get_allocator_());
    std_swap(m_data, rhs.m_data);
    std_swap(m_d0, rhs.m_d0);
    std_swap(m_d1, rhs.m_d1);
    std_swap(m_d2, rhs.m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reference fixed_array_3d<T, A, P, R>::at(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2)
{
    range_check_(i0, i1, i2);

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reference fixed_array_3d<T, A, P, R>::at(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2) const
{
    range_check_(i0, i1, i2);

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reference fixed_array_3d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1 && i2 < m_d2));

    return *(m_data + calc_index_(i0, i1, i2));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reference fixed_array_3d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1 && i2 < m_d2));

    return *(m_data + calc_index_(i0, i1, i2));
}

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reference fixed_array_3d<T, A, P, R>::operator ()(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2)
{
    return at_unchecked(i0, i1, i2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reference fixed_array_3d<T, A, P, R>::operator ()(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i1, ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i2) const
{
    return at_unchecked(i0, i1, i2);
}
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::dimension_element_type fixed_array_3d<T, A, P, R>::at(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0)
{
    range_check_(i0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2, m_d1, m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_dimension_element_type fixed_array_3d<T, A, P, R>::at(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0) const
{
    range_check_(i0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2, m_d1, m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::dimension_element_type fixed_array_3d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2, m_d1, m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_dimension_element_type fixed_array_3d<T, A, P, R>::at_unchecked(ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2, m_d1, m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::dimension_element_type fixed_array_3d<T, A, P, R>::operator [](ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2, m_d1, m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_dimension_element_type fixed_array_3d<T, A, P, R>::operator [](ss_typename_type_k fixed_array_3d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2, m_d1, m_d2);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reference fixed_array_3d<T, A, P, R>::front()
{
    return at(0, 0, 0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reference fixed_array_3d<T, A, P, R>::back()
{
    return at(m_d0 - 1, m_d1 - 1, m_d2 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reference fixed_array_3d<T, A, P, R>::front() const
{
    return at(0, 0, 0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reference fixed_array_3d<T, A, P, R>::back() const
{
    return at(m_d0 - 1, m_d1 - 1, m_d2 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::index_type fixed_array_3d<T, A, P, R>::dimension0() const
{
    return m_d0;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::index_type fixed_array_3d<T, A, P, R>::dimension1() const
{
    return m_d1;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::index_type fixed_array_3d<T, A, P, R>::dimension2() const
{
    return m_d2;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::index_type fixed_array_3d<T, A, P, R>::size() const
{
    return m_d0 * m_d1 * m_d2;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::bool_type fixed_array_3d<T, A, P, R>::empty() const
{
    return 0 == size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline /* static */ ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::size_type fixed_array_3d<T, A, P, R>::max_size()
{
    return static_cast<size_type>(-1) / sizeof(T);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::iterator fixed_array_3d<T, A, P, R>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::iterator fixed_array_3d<T, A, P, R>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_iterator fixed_array_3d<T, A, P, R>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_iterator fixed_array_3d<T, A, P, R>::end() const
{
    return m_data + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reverse_iterator fixed_array_3d<T, A, P, R>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::reverse_iterator fixed_array_3d<T, A, P, R>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reverse_iterator fixed_array_3d<T, A, P, R>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_reverse_iterator fixed_array_3d<T, A, P, R>::rend() const
{
    return const_reverse_iterator(begin());
}

#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::pointer fixed_array_3d<T, A, P, R>::data()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_3d<T, A, P, R>::const_pointer fixed_array_3d<T, A, P, R>::data() const
{
    return m_data;
}

// fixed_array_4d

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::pointer fixed_array_4d<T, A, P, R>::allocate_(ss_typename_type_k fixed_array_4d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    return ator.allocate(n, NULL);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_4d<T, A, P, R>::deallocate_(ss_typename_type_k fixed_array_4d<T, A, P, R>::pointer p, ss_typename_type_k fixed_array_4d<T, A, P, R>::size_type n)
{
    allocator_type  &ator = *this;

    ator.deallocate(p, n);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::pointer fixed_array_4d<T, A, P, R>::data_()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::index_type fixed_array_4d<T, A, P, R>::calc_index_(ss_typename_type_k fixed_array_4d<T, A, P, R>::index_type i0, index_type i1, index_type i2, index_type i3) const
{
    return (((i0 * m_d1) + i1) * m_d2 + i2) * m_d3 + i3;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_4d<T, A, P, R>::range_check_(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( !(i0 < m_d0) ||
        !(i1 < m_d1) ||
        !(i2 < m_d2) ||
        !(i3 < m_d3))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1 && i2 < m_d2 && i3 < m_d3));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_4d<T, A, P, R>::allocator_type& fixed_array_4d<T, A, P, R>::get_allocator_()
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_4d<T, A, P, R>::range_check_(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0) const stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) )
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(i0 < m_d0))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("fixed array index out of range"));
    }
#else
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(T* src, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d3)
    : m_data(src)
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
    , m_d3(d3)
{
    STLSOFT_STATIC_ASSERT(!R);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d3)
    : m_data(allocate_(d0 * d1 * d2 * d3))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
    , m_d3(d3)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d3, ss_typename_param_k fixed_array_4d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0 * d1 * d2 * d3))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
    , m_d3(d3)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d3, ss_typename_param_k fixed_array_4d<T, A, P, R>::value_type const& t)
    : m_data(allocate_(d0 * d1 * d2 * d3))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
    , m_d3(d3)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type d3, ss_typename_param_k fixed_array_4d<T, A, P, R>::value_type const& t, ss_typename_param_k fixed_array_4d<T, A, P, R>::allocator_type const& ator)
    : allocator_type(ator)
    , m_data(allocate_(d0 * d1 * d2 * d3))
    , m_d0(d0)
    , m_d1(d1)
    , m_d2(d2)
    , m_d3(d3)
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::construct(*this, data_(), size(), t);
}

#ifdef STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(fixed_array_4d<T, A, P, R> const& rhs)
    : m_data(R ? allocate_(rhs.dimension0() * rhs.dimension1() * rhs.dimension2() * rhs.dimension3()) : rhs.m_data)
    , m_d0(rhs.dimension0())
    , m_d1(rhs.dimension1())
    , m_d2(rhs.dimension2())
    , m_d3(rhs.dimension3())
{
    if(R)
    {
        array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
    }
}

#else /* ? STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::fixed_array_4d(fixed_array_4d<T, A, P, R> const& rhs)
    : m_data(allocate_(rhs.dimension0() * rhs.dimension1() * rhs.dimension2() * rhs.dimension3()))
    , m_d0(rhs.dimension0())
    , m_d1(rhs.dimension1())
    , m_d2(rhs.dimension2())
    , m_d3(rhs.dimension3())
{
    STLSOFT_STATIC_ASSERT(R);
    array_range_initialiser<T, A, P>::copy_construct(*this, data_(), rhs.data(), size());
}

#endif /* STLSOFT_MULTIDIM_ARRAY_FEATURE_REQUIRES_COPY_CTOR_WITH_RVO */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline fixed_array_4d<T, A, P, R>::~fixed_array_4d() stlsoft_throw_0()
{
    if(R)
    {
        array_range_initialiser<T, A, P>::destroy(*this, data_(), size());
        deallocate_(m_data, size());
    }
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_4d<T, A, P, R>::allocator_type fixed_array_4d<T, A, P, R>::get_allocator() const
{
    return *this;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline void fixed_array_4d<T, A, P, R>::swap(ss_typename_type_k fixed_array_4d<T, A, P, R>::class_type& rhs) stlsoft_throw_0()
{
    // We don't need to do any construct and swap here, because all the
    // variables that are being swapped are simple types (integers and
    // pointers).

    std_swap(get_allocator_(), rhs.get_allocator_());
    std_swap(m_data, rhs.m_data);
    std_swap(m_d0, rhs.m_d0);
    std_swap(m_d1, rhs.m_d1);
    std_swap(m_d2, rhs.m_d2);
    std_swap(m_d3, rhs.m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reference fixed_array_4d<T, A, P, R>::at(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3)
{
    range_check_(i0, i1, i2, i3);

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reference fixed_array_4d<T, A, P, R>::at(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3) const
{
    range_check_(i0, i1, i2, i3);

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reference fixed_array_4d<T, A, P, R>::at_unchecked(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1 && i2 < m_d2 && i3 < m_d3));

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reference fixed_array_4d<T, A, P, R>::at_unchecked(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", (i0 < m_d0 && i1 < m_d1 && i2 < m_d2 && i3 < m_d3));

    return *(m_data + calc_index_(i0, i1, i2, i3));
}

#ifndef STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP
template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reference fixed_array_4d<T, A, P, R>::operator ()(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3)
{
    return at_unchecked(i0, i1, i2, i3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reference fixed_array_4d<T, A, P, R>::operator ()(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i1, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i2, ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i3) const
{
    return at_unchecked(i0, i1, i2, i3);
}
#endif /* !STLSOFT_FIXED_ARRAY_NO_FUNCTION_OP */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::dimension_element_type fixed_array_4d<T, A, P, R>::at(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0)
{
    range_check_(i0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2 * m_d3, m_d1, m_d2, m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_k fixed_array_4d<T, A, P, R>::const_dimension_element_type fixed_array_4d<T, A, P, R>::at(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0) const
{
    range_check_(i0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2 * m_d3, m_d1, m_d2, m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::dimension_element_type fixed_array_4d<T, A, P, R>::at_unchecked(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2 * m_d3, m_d1, m_d2, m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_dimension_element_type fixed_array_4d<T, A, P, R>::at_unchecked(ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2 * m_d3, m_d1, m_d2, m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::dimension_element_type fixed_array_4d<T, A, P, R>::operator [](ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0)
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2 * m_d3, m_d1, m_d2, m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_dimension_element_type fixed_array_4d<T, A, P, R>::operator [](ss_typename_param_k fixed_array_4d<T, A, P, R>::index_type i0) const
{
    STLSOFT_MESSAGE_ASSERT("fixed array index out of range", i0 < m_d0);

    return dimension_element_type(m_data + i0 * m_d1 * m_d2 * m_d3, m_d1, m_d2, m_d3);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reference fixed_array_4d<T, A, P, R>::front()
{
    return at(0, 0, 0, 0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reference fixed_array_4d<T, A, P, R>::back()
{
    return at(m_d0 - 1, m_d1 - 1, m_d2 - 1, m_d3 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reference fixed_array_4d<T, A, P, R>::front() const
{
    return at(0, 0, 0, 0);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reference fixed_array_4d<T, A, P, R>::back() const
{
    return at(m_d0 - 1, m_d1 - 1, m_d2 - 1, m_d3 - 1);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::index_type fixed_array_4d<T, A, P, R>::dimension0() const
{
    return m_d0;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::index_type fixed_array_4d<T, A, P, R>::dimension1() const
{
    return m_d1;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::index_type fixed_array_4d<T, A, P, R>::dimension2() const
{
    return m_d2;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::index_type fixed_array_4d<T, A, P, R>::dimension3() const
{
    return m_d3;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::index_type fixed_array_4d<T, A, P, R>::size() const
{
    return m_d0 * m_d1 * m_d2 * m_d3;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::bool_type fixed_array_4d<T, A, P, R>::empty() const
{
    return 0 == size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline /* static */ ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::size_type fixed_array_4d<T, A, P, R>::max_size()
{
    return static_cast<size_type>(-1) / sizeof(T);
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::iterator fixed_array_4d<T, A, P, R>::begin()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::iterator fixed_array_4d<T, A, P, R>::end()
{
    return m_data + size();
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_iterator fixed_array_4d<T, A, P, R>::begin() const
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_iterator fixed_array_4d<T, A, P, R>::end() const
{
    return m_data + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reverse_iterator fixed_array_4d<T, A, P, R>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::reverse_iterator fixed_array_4d<T, A, P, R>::rend()
{
    return reverse_iterator(begin());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reverse_iterator fixed_array_4d<T, A, P, R>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_reverse_iterator fixed_array_4d<T, A, P, R>::rend() const
{
    return const_reverse_iterator(begin());
}

#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::pointer fixed_array_4d<T, A, P, R>::data()
{
    return m_data;
}

template <ss_typename_param_k T, ss_typename_param_k A, ss_typename_param_k P, ss_bool_t R>
inline ss_typename_type_ret_k fixed_array_4d<T, A, P, R>::const_pointer fixed_array_4d<T, A, P, R>::data() const
{
    return m_data;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
inline ss_size_t array_size(fixed_array_1d<T, A, P, R> const& ar)
{
    return ar.size();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
inline ss_size_t array_size(fixed_array_2d<T, A, P, R> const& ar)
{
    return ar.size();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
inline ss_size_t array_size(fixed_array_3d<T, A, P, R> const& ar)
{
    return ar.size();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
inline ss_size_t array_size(fixed_array_4d<T, A, P, R> const& ar)
{
    return ar.size();
}

#if 0
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_typename_param_k P
        ,   ss_bool_t           R
        >
inline ss_size_t array_size(fixed_array_5d<T, A, P, R> const& ar)
{
    return ar.size();
}
#endif /* 0 */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_FIXED_ARRAY */

/* ///////////////////////////// end of file //////////////////////////// */
