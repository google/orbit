/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/index_iterator.hpp
 *
 * Purpose:     index_iterator class template.
 *
 * Created:     5th April 2005
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


/** \file stlsoft/iterators/index_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::index_iterator iterator
 *   adaptor class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR_MAJOR     1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR_MINOR     3
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR_REVISION  5
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR_EDIT      25
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1310
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if !defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
# error index_iterator cannot be used with compilers that do not support partial template specialisation
#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
# ifndef STLSOFT_INCL_STLSOFT_ITERATOR_HPP_ADAPTED_ITERATOR_TRAITS
#  include <stlsoft/iterators/adapted_iterator_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ITERATOR_HPP_ADAPTED_ITERATOR_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
#  include <stlsoft/meta/is_pointer_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
#  include <stlsoft/meta/yesno.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Feature discrimination
 */

#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
# define STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
# if !defined(STLSOFT_COMPILER_IS_GCC) || \
     __GNUC__ > 4 || \
     (   __GNUC__ == 3 && \
         __GNUC_MINOR__ >= 4)
#   define STLSOFT_INDEX_ITERATOR_MEM_SEL_OP_SUPPORT
# endif /* compiler */
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

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

/** An indexed iterator class.
 *
 * The implementation of this class template is described in a bonus
 * chapter on the CD for
 * <a href="http://extendedstl.com/">Extended STL, volume 1</a>
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k T = adapted_iterator_traits<I>
        >
// [[synesis:class:iterator: index_iterator<T<I>>]]
class index_iterator
{
/// \name Member Types
/// @{
private:
    // There are several problems to solve here.
    //
    //  First, the issue of whether the library profers a meaningful definition of
    //  std::iterator_traits for this, or any, iterator type
    //
    //  Second, deducing whether the base iterator is const or not
    //
    //  Third, deducing the element reference category of base iterator type, and
    //  defining the member types accordingly
    //
    // Postscriptum:
    //
    // Thankfully we don't need to worry about that any more, because
    // adapted_iterator_traits handles it all for us. :-)

public:
    typedef I                                                           base_iterator_type;
    typedef T                                                           traits_type;
    typedef index_iterator<I>                                           class_type;
    typedef ss_typename_type_k traits_type::iterator_category           iterator_category;
    typedef ss_typename_type_k traits_type::value_type                  value_type;
    typedef ss_typename_type_k traits_type::pointer                     pointer;
    typedef ss_typename_type_k traits_type::reference                   reference;
    typedef ss_typename_type_k traits_type::difference_type             difference_type;
    typedef ss_typename_type_k traits_type::const_pointer               const_pointer;
    typedef ss_typename_type_k traits_type::const_reference             const_reference;
    typedef ss_typename_type_k traits_type::effective_reference         effective_reference;
    typedef ss_typename_type_k traits_type::effective_const_reference   effective_const_reference;
    typedef ss_ptrdiff_t                                                index_type;
    typedef ss_typename_type_k traits_type::effective_pointer           effective_pointer;
    typedef ss_typename_type_k traits_type::effective_const_pointer     effective_const_pointer;
/// @}

/// \name Construction
/// @{
public:
    index_iterator()
        : m_it(base_iterator_type())
        , m_index(0)
    {}
    ss_explicit_k index_iterator(base_iterator_type it, index_type index = 0)
        : m_it(it)
        , m_index(index)
    {}
    index_iterator(class_type const& rhs)
        : m_it(rhs.m_it)
        , m_index(rhs.m_index)
    {}

    /// \brief A copy of the base iterator
    base_iterator_type  base() const
    {
        return m_it;
    }
/// @}

/// \name Forward Iterator methods
/// @{
public:
    class_type& operator ++()
    {
        ++m_it;
        ++m_index;

        return *this;
    }

    class_type operator ++(int)
    {
        class_type  r(*this);

        operator ++();

        return r;
    }

#ifdef STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT
    effective_reference operator *()
    {
        return *m_it;
    }
#endif /* STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT */

    effective_const_reference operator *() const
    {
        return *m_it;
    }


#ifdef STLSOFT_INDEX_ITERATOR_MEM_SEL_OP_SUPPORT

#if 0
    effective_pointer operator ->()
    {
        return m_it.operator ->();
    }

    effective_const_pointer operator ->() const
    {
        return m_it.operator ->();
    }
#else /* ? 0 */
# ifdef STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT
    effective_pointer operator ->()
    {
        enum { is_iterator_pointer_type = is_pointer_type<base_iterator_type>::value };

        // This has to be a separate typedef, otherwise DMC++ has a fit
        typedef ss_typename_type_k value_to_yesno_type<is_iterator_pointer_type>::type  yesno_t;

        return invoke_member_selection_operator_(yesno_t());
    }
# endif /* STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT */

    effective_const_pointer operator ->() const
    {
        enum { is_iterator_pointer_type = is_pointer_type<base_iterator_type>::value };

        // This has to be a separate typedef, otherwise DMC++ has a fit
        typedef ss_typename_type_k value_to_yesno_type<is_iterator_pointer_type>::type  yesno_t;

        return invoke_member_selection_operator_(yesno_t());
    }
#endif /* 0 */

#endif /* STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT */
/// @}

/// \name Bidirectional Iterator methods
/// @{
public:
    class_type& operator --()
    {
        --m_it;
        --m_index;

        return *this;
    }

    class_type operator --(int)
    {
        class_type  r(*this);

        operator --();

        return r;
    }
/// @}

/// \name Random-Access Iterator methods
/// @{
public:
    /// Moves the iterator forward
    ///
    /// \param n The amount by which to increment the iterator's current position
    class_type& operator +=(difference_type n)
    {
        m_it    +=  n;
        m_index +=  n;

        return *this;
    }

    /// Moves the iterator backward
    ///
    /// \param n The amount by which to decrement the iterator's current position
    class_type& operator -=(difference_type n)
    {
        m_it    -=  n;
        m_index -=  n;

        return *this;
    }

    /// Access the element at the given index
    ///
    /// \param index The required offset from the iterator's position

#ifdef STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT
    effective_reference         operator [](difference_type index)
    {
        return m_it[index];
    }
#endif /* STLSOFT_INDEX_ITERATOR_MUTABLE_OP_SUPPORT */

    /// Access the element at the given index
    ///
    /// \param index The required offset from the iterator's position
    effective_const_reference   operator [](difference_type index) const
    {
        return m_it[index];
    }

    /// Calculate the distance between \c this and \c rhs
    difference_type distance(class_type const& rhs) const
    {
        return m_it - rhs.m_it;
    }
/// @}

/// \name Index Iterator methods
/// @{
public:
    index_type index() const
    {
        return m_index;
    }
/// @}

/// \name Comparison
/// @{
public:
    ss_bool_t equal(class_type const& rhs) const
    {
        return m_it == rhs.m_it;
    }

    int compare(class_type const& rhs) const
    {
        return (m_it < rhs.m_it) ? -1 : (rhs.m_it < m_it) ? +1 : 0;
    }
/// @}

/// \name Implementation
/// @{
private:
#ifdef STLSOFT_INDEX_ITERATOR_MEM_SEL_OP_SUPPORT
    effective_pointer invoke_member_selection_operator_(yes_type)
    {
        return m_it;
    }
    effective_pointer invoke_member_selection_operator_(no_type)
    {
        return m_it.operator ->();
    }

    effective_const_pointer invoke_member_selection_operator_(yes_type) const
    {
        return m_it;
    }
    effective_const_pointer invoke_member_selection_operator_(no_type) const
    {
        return m_it.operator ->();
    }
#endif /* STLSOFT_INDEX_ITERATOR_MEM_SEL_OP_SUPPORT */
/// @}

/// \name Members
/// @{
private:
    base_iterator_type  m_it;
    index_type          m_index;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

/** \brief Creator function for index_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param it The iterator to index
 * \param index The initial index of the iterator. Defaults to 0
 *
 * \return An instance of the specialisation index_iterator&lt;I&gt;
 */
template<   ss_typename_param_k I
        >
inline index_iterator<I> make_index_iterator(I it, ss_ptrdiff_t index = 0)
{
    return index_iterator<I>(it, index);
}

/** \brief Creator function for index_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param it The iterator to index
 * \param index The initial index of the iterator. Defaults to 0
 *
 * \return An instance of the specialisation index_iterator&lt;T&gt;
 *
 * \note Short-hand for make_index_iterator()
 */
template<   ss_typename_param_k I
        >
inline index_iterator<I> indexer(I it, ss_ptrdiff_t index = 0)
{
    return make_index_iterator(it, index);
}

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

// operator ==

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator ==(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return lhs.equal(rhs);
}

// operator !=

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator !=(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return !lhs.equal(rhs);
}

// operator +

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline index_iterator<I, T> operator +(index_iterator<I, T> const& lhs, ss_typename_type_k index_iterator<I, T>::difference_type rhs)
{
    return index_iterator<I, T>(lhs) += rhs;
}

// operator -

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
#if 0
inline index_iterator<I, T> operator -(index_iterator<I, T> const& lhs, ss_typename_type_k index_iterator<I, T>::difference_type rhs)
#else /* ? 0 */
inline index_iterator<I, T> operator -(index_iterator<I, T> const& lhs, ss_ptrdiff_t rhs)
#endif /* 0 */
{
    return index_iterator<I, T>(lhs) -= rhs;
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k index_iterator<I, T>::difference_type operator -(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return lhs.distance(rhs);
}

// operator <

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator <(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return lhs.compare(rhs) < 0;
}

// operator <=

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator <=(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return lhs.compare(rhs) <= 0;
}

// operator >

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator >(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return lhs.compare(rhs) > 0;
}

// operator >=

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator >=(index_iterator<I, T> const& lhs, index_iterator<I, T> const& rhs)
{
    return lhs.compare(rhs) >= 0;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/index_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

namespace std
{
    template<   ss_typename_param_k I
            ,   ss_typename_param_k T
            >
    inline ss_typename_type_ret_k stlsoft_ns_qual(index_iterator)<I, T>::iterator_category _Iter_cat(stlsoft_ns_qual(index_iterator)<I, T> const&)
    {
        return ss_typename_type_k stlsoft_ns_qual(index_iterator)<I, T>::iterator_category();
    }
    template<   ss_typename_param_k I
            ,   ss_typename_param_k T
            >
    inline ss_typename_type_ret_k stlsoft_ns_qual(index_iterator)<I, T>::value_type *_Val_type(stlsoft_ns_qual(index_iterator)<I, T> const&)
    {
        return static_cast</* ss_typename_type_k  */stlsoft_ns_qual(index_iterator)<I, T>::value_type*>(0);
    }

# if STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0
    template<   ss_typename_param_k I
            ,   ss_typename_param_k T
            >
    inline ss_typename_type_ret_k stlsoft_ns_qual(index_iterator)<I, T>::difference_type *_Dist_type(stlsoft_ns_qual(index_iterator)<I, T> const&)
    {
        return static_cast</* ss_typename_type_k  */stlsoft_ns_qual(index_iterator)<I, T>::difference_type*>(0);
    }
# elif STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
# else
#  error Error in discrimination
# endif

} // namespace std


#endif /* old-dinkumware */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_INDEX_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
