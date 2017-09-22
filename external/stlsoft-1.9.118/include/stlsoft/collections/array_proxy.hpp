/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/collections/array_proxy.hpp
 *
 * Purpose:     Definition of the array_proxy template, which provides managed
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


/** \file stlsoft/collections/array_proxy.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::array_proxy
 *   class template
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY
#define STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY_MAJOR    4
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY_MINOR    0
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY_REVISION 4
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY_EDIT     57
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

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

/** \brief Acts as a proxy for built-in arrays, ensuring functions passed array proxies
 * have safe access to both array pointer and length
 *
 * \ingroup group__library__collections
 *
 * \note This is described in detail in section 14.4 of
 *  <a href = "http://www.imperfectcplusplus.com/">Imperfect C++</a>.
 */
template <ss_typename_param_k T>
class array_proxy
    : public stl_collection_tag
{
public:
    typedef T                   value_type;
    typedef array_proxy<T>      class_type;
    typedef value_type*         pointer;
    typedef value_type*         const_pointer;     // This is deliberately non-const
    typedef value_type*         iterator;
    typedef value_type*         const_iterator;    // This is deliberately non-const
    typedef value_type&         reference;
    typedef value_type&         const_reference;   // This is deliberately non-const
    typedef ss_size_t           size_type;

/// \name Construction
/// @{
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// Conversion constructor, facilitating conversion between polymorphic
    /// types of the same size
    ///
    /// \param d The array proxy of a derived type
    template <ss_typename_param_k D>
    array_proxy(array_proxy<D> &d)
        : m_begin(d.begin())
        , m_end(d.end())
    {
        // Ensures that D is a derived type of T. (Actually that is
        // handled in the initialiser list, but putting it here
        // makes the intent quite explicit.)
        stlsoft_constraint_must_have_base(D, T);

        // Ensures that D and T are the same size.
        stlsoft_constraint_must_be_same_size(D, T);
    }

# ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
    /// Constructs an instance of the array proxy from the given array
    ///
    /// \param d The array
    template <ss_typename_param_k D, ss_size_t N>
    ss_explicit_k array_proxy(D (&d)[N])
        : m_begin(&d[0])
        , m_end(&d[N])
    {
        // Ensures that D is a derived type of T. (Actually that is
        // handled in the initialiser list, but putting it here
        // makes the intent quite explicit.)
        stlsoft_constraint_must_have_base(D, T);

        // Ensures that D and T are the same size.
        stlsoft_constraint_must_be_same_size(D, T);
    }

    /// Constructs an instance of the array proxy from the given array
    ///
    /// \param t The array
#  if defined(STLSOFT_CF_NON_TEMPLATE_CTOR_REQUIRED_WITH_TEMPLATE_CTOR)
    template <ss_size_t N>
    ss_explicit_k array_proxy(T (&t)[N])
        : m_begin(&t[0])
        , m_end(&t[N])
    {}
#  endif /* STLSOFT_CF_NON_TEMPLATE_CTOR_REQUIRED_WITH_TEMPLATE_CTOR */
# endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

    /// Constructs an instance of the array proxy from the given range
    ///
    /// \param begin The start point of the range [begin, end)
    /// \param end The end point of the range [begin, end)
#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_NON_TEMPLATE_CTOR_REQUIRED_WITH_TEMPLATE_CTOR)
    array_proxy(pointer begin, pointer end)
        : m_begin(begin)
        , m_end(end)
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_NON_TEMPLATE_CTOR_REQUIRED_WITH_TEMPLATE_CTOR */

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// Constructs an instance of the array proxy from the given range
    ///
    /// \param b The start point of the range [begin, end)
    /// \param e The end point of the range [begin, end)
    template <ss_typename_param_k D>
    array_proxy(D *b, D *e)
        : m_begin(b)
        , m_end(e)
    {
        // Ensures that D is a derived type of T. (Actually that is
        // handled in the initialiser list, but putting it here
        // makes the intent quite explicit.)
        stlsoft_constraint_must_have_base(D, T);

        // Ensures that D and T are the same size.
        stlsoft_constraint_must_be_same_size(D, T);
   }
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_NON_TEMPLATE_CTOR_REQUIRED_WITH_TEMPLATE_CTOR)
    /// Constructs an instance of the array proxy from the given pointer
    ///
    /// \param p Pointer to the first element in the range
    /// \param n The number of elements in the range
    array_proxy(pointer p, ss_size_t n)
        : m_begin(p)
        , m_end(p + n)
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_NON_TEMPLATE_CTOR_REQUIRED_WITH_TEMPLATE_CTOR */

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// Constructs an instance of the array proxy from the given range
    ///
    /// \param p The start of the range
    /// \param n The number of elements in the range
    template <ss_typename_param_k D>
    array_proxy(D *p, ss_size_t n)
        : m_begin(p)
        , m_end(p + n)
    {
        // Ensures that D is a derived type of T. (Actually that is
        // handled in the initialiser list, but putting it here
        // makes the intent quite explicit.)
        stlsoft_constraint_must_have_base(D, T);

        // Ensures that D and T are the same size.
        stlsoft_constraint_must_be_same_size(D, T);
    }
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
///  @}

/// \name Attributes
/// @{
public:
    /// Returns the base of the array
    ///
    pointer base()
    {
        return m_begin;
    }
    /// Returns the base of the array
    ///
    pointer base() const
    {
        return m_begin;
    }
    /// Returns the number of elements in the sequence
    size_type size() const
    {
        return m_end - m_begin;
    }
    /// Indicates whether the search sequence is empty
    ss_bool_t empty() const
    {
        return m_begin == m_end;
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
    reference operator [](ss_size_t index)
    {
        STLSOFT_MESSAGE_ASSERT("index out of bounds, in array_proxy", !(size() < index));

        return m_begin[index];
    }
    /// Returns the element at the given index
    ///
    /// \param index The offset of the requested element
    /// \note No runtime checking of the validity of the index is provided in release builds, only a debug-time assert
    const_reference operator [](ss_size_t index) const
    {
        STLSOFT_MESSAGE_ASSERT("index out of bounds, in array_proxy", !(size() < index));

        return const_cast<pointer>(m_begin)[index];
    }
///  @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator begin()
    {
        return m_begin;
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator end()
    {
        return m_end;
    }
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator begin() const
    {
        return m_begin;
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator end() const
    {
        return m_end;
    }
///  @}

// Members
private:
    pointer const m_begin;
    pointer const m_end;

// Not to be implemented
private:
    array_proxy& operator =(array_proxy const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

#ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
template <ss_typename_param_k T, ss_size_t N>
inline array_proxy<T> make_array_proxy(T (&t)[N])
{
    return array_proxy<T>(&t[0], &t[N]);
//    return array_proxy<T>(t); // This one not used, because CodeWarrior gets confused
}
#endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

template <ss_typename_param_k T>
inline array_proxy<T> make_array_proxy(T *begin, T *end)
{
    return array_proxy<T>(begin, end);
}

template <ss_typename_param_k T>
inline array_proxy<const T> make_array_proxy(T const* begin, T const* end)
{
    return array_proxy<const T>(begin, end);
}

template <ss_typename_param_k T>
inline array_proxy<T> make_array_proxy(T *p, ss_size_t n)
{
    return array_proxy<T>(p, n);
}

#if 0
template <ss_typename_param_k T>
inline array_proxy<const T> make_array_proxy(T const* p, ss_size_t n)
{
    return array_proxy<const T>(p, n);
}
#endif /* 0 */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/array_proxy_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_ARRAY_PROXY */

/* ///////////////////////////// end of file //////////////////////////// */
