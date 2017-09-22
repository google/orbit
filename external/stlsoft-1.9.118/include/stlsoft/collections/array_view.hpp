/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/collections/array_view.hpp (derived from array_proxy (stlsoft_array_proxy.h)
 *
 * Purpose:     Definition of the array_view template, which provides managed
 *              access to arrays, and can be used to avoid polymorphic array
 *              problems.
 *
 * Created:     11th November 2002
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


/** \file stlsoft/collections/array_view.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::array_view
 *   class template
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW
#define STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW_MAJOR       4
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW_MINOR       1
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW_REVISION    1
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW_EDIT        70
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
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# include <stdexcept>                       // for std::out_of_range
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Acts as a view for built-in arrays, ensuring functions passed
 *    array proxies have safe access to both array pointer and length, to
 *    avoid polymorphic array problems.
 *
 * \ingroup group__library__collections
 */
template <ss_typename_param_k T>
class array_view
    : public stl_collection_tag
{
/// \name Types
/// @{
public:
    typedef T                                               value_type;
    typedef array_view<T>                                   class_type;
    typedef value_type*                                     pointer;
    typedef value_type const*                               const_pointer;
    typedef value_type&                                     reference;
    typedef value_type const&                               const_reference;
    typedef ss_size_t                                       size_type;
    typedef ss_ptrdiff_t                                    difference_type;
#if !defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The iterator type
    typedef value_type*                                     iterator;
    /// The non-mutable (const) iterator type
    typedef value_type const*                               const_iterator;
#else /* ? !STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The iterator type
    typedef
# if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
# endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    /// The non-mutating (const) iterator type
    typedef
# if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
# endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

    /// The mutating (non-const) reverse iterator type
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    /// The non-mutating (const) reverse iterator type
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* !STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Construction
/// @{
public:
    /// Default constructor, creating a view of 0 size
    ///
    /// \note The base() method returns NULL
    array_view()
        : m_size(0)
        , m_base(NULL)
    {
        STLSOFT_ASSERT(is_valid());
    }

#ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
    /// Constructs an instance of the array view from the given array
    ///
    /// \param t The array
    template <size_type N>
    ss_explicit_k array_view(T (&t)[N])
        : m_size(N)
        , m_base(&t[0])
    {
        STLSOFT_ASSERT(is_valid());
    }
#endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

    /// Constructs an instance of the array view from the given range
    ///
    /// \param begin The start point of the range [begin, end)
    /// \param end The end point of the range [begin, end)
    array_view(pointer begin, pointer end)
        : m_size(stlsoft_ns_qual_std(distance)(begin, end))
        , m_base(begin)
    {
        STLSOFT_ASSERT(is_valid());
    }

    /// Constructs an instance of the array view from the given pointer
    ///
    /// \param p Pointer to the first element in the range
    /// \param n The number of elements in the range
    array_view(pointer p, size_type n)
        : m_size(n)
        , m_base(p)
    {
        STLSOFT_ASSERT(is_valid());
    }

    /// Swaps the contents between \c this and \c rhs
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        STLSOFT_ASSERT(is_valid());

        std_swap(m_size, rhs.m_size);
        std_swap(m_base, rhs.m_base);

        STLSOFT_ASSERT(is_valid());
    }

/// \name Attributes
/// @{
public:
    /// Returns the base of the array
    ///
    pointer base()
    {
        STLSOFT_ASSERT(is_valid());

        return m_base;
    }
    /// Returns the base of the array
    ///
    pointer base() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_base;
    }
    /// Returns the number of elements in the sequence
    size_type size() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_size;
    }
    /// Indicates whether the search sequence is empty
    ss_bool_t empty() const
    {
        STLSOFT_ASSERT(is_valid());

        return 0 == m_size;
    }
    /// Returns the maximum number of elements in the sequence
    static size_type max_size()
    {
        return static_cast<size_type>(-1) / sizeof(value_type);
    }
///  @}

/// \name Subscripting
/// @{
public:
    /// Returns the element at the given index
    ///
    /// \param index The offset of the requested element
    /// \note No runtime checking of the validity of the index is provided in release builds, only a debug-time assert
    reference operator [](size_type index)
    {
        STLSOFT_MESSAGE_ASSERT("index out of bounds, in array_view", !(size() < index));

        STLSOFT_ASSERT(is_valid());

        return m_base[index];
    }
    /// Returns the element at the given index
    ///
    /// \param index The offset of the requested element
    /// \note No runtime checking of the validity of the index is provided in release builds, only a debug-time assert
    const_reference operator [](size_type index) const
    {
        STLSOFT_MESSAGE_ASSERT("index out of bounds, in array_view", !(size() < index));

        STLSOFT_ASSERT(is_valid());

        return const_cast<pointer>(m_base)[index];
    }

# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    /// Returns the element at the given index
    ///
    /// \param index The offset of the requested element
    ///
    /// \note Throws an instance of std::out_of_range if the index is not < size()
    reference at(size_type index)
    {
        STLSOFT_ASSERT(is_valid());

        range_check_(index);

        return m_base[index];
    }
    /// Returns the element at the given index
    ///
    /// \param index The offset of the requested element
    ///
    /// \note Throws an instance of std::out_of_range if the index is not < size()
    const_reference at(size_type index) const
    {
        STLSOFT_ASSERT(is_valid());

        range_check_(index);

        return const_cast<pointer>(m_base)[index];
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
///  @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator begin()
    {
        STLSOFT_ASSERT(is_valid());

        return m_base;
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator end()
    {
        STLSOFT_ASSERT(is_valid());

        return begin() + size();
    }
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator begin() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_base;
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator end() const
    {
        STLSOFT_ASSERT(is_valid());

        return begin() + size();
    }

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    reverse_iterator  rbegin()
    {
        return reverse_iterator(end());
    }
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    reverse_iterator  rend()
    {
        return reverse_iterator(begin());
    }
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
///  @}

/// \name Invariant
/// @{
#ifdef STLSOFT_UNITTEST
public:
#else /* ? STLSOFT_UNITTEST */
private:
#endif /* STLSOFT_UNITTEST */
    ss_bool_t is_valid() const
    {
        if( 0 != m_size &&
            NULL == m_base)
        {
#ifdef STLSOFT_UNITTEST
            fprintf(err, "Cannot have non-empty array view with NULL base pointer\n");
#endif /* STLSOFT_UNITTEST */

            return false;
        }

        return true;
    }
///  @}

/// \name Implementation
/// @{
private:
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    void range_check_(size_type index) const
    {
        if(!(index < size()))
        {
            STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("array view index out of range"));
        }
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
///  @}

/// \name Members
/// @{
private:
    size_type   m_size;
    pointer     m_base;
///  @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

#ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
template <ss_typename_param_k T, ss_size_t N>
inline array_view<T> make_array_view(T (&t)[N])
{
    return array_view<T>(&t[0], &t[N]);
//    return array_view<T>(t); // This one not used, because CodeWarrior gets confused
}
#endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

template <ss_typename_param_k T>
inline array_view<T> make_array_view(T *begin, T *end)
{
    return array_view<T>(begin, end);
}

template <ss_typename_param_k T>
inline array_view<const T> make_array_view(T const* begin, T const* end)
{
    return array_view<const T>(begin, end);
}

template <ss_typename_param_k T>
inline array_view<T> make_array_view(T *p, ss_size_t n)
{
    return array_view<T>(p, n);
}

#if 0
template <ss_typename_param_k T>
inline array_view<const T> make_array_view(T const* p, ss_size_t n)
{
    return array_view<const T>(p, n);
}
#endif /* 0 */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/array_view_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_ARRAY_VIEW */

/* ///////////////////////////// end of file //////////////////////////// */
