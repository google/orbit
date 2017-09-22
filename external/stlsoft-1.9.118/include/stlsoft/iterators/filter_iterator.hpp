/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/filter_iterator.hpp
 *
 * Purpose:     An iterator adaptor that uses a predicate to filter desired
 *              values from the iterator's underlying sequence.
 *
 * Created:     9th July 2004
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
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/iterators/filter_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::filter_iterator
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR_MAJOR    4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR_MINOR    2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR_REVISION 4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR_EDIT     40
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
# error filter_iterator cannot be used with compilers that do not support partial template specialisation
#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
# ifndef STLSOFT_INCL_STLSOFT_ITERATOR_HPP_ADAPTED_ITERATOR_TRAITS
#  include <stlsoft/iterators/adapted_iterator_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ITERATOR_HPP_ADAPTED_ITERATOR_TRAITS */
#endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
# include <stlsoft/meta/is_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS
# include <stlsoft/util/std/iterator_category_limiters.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS */

/* /////////////////////////////////////////////////////////////////////////
 * Feature discrimination
 */

#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
# define STLSOFT_FILTER_ITERATOR_MUTABLE_OP_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
# if !defined(STLSOFT_COMPILER_IS_GCC) || \
     __GNUC__ > 4 || \
     (   __GNUC__ == 3 && \
         __GNUC_MINOR__ >= 4)
#   define STLSOFT_FILTER_ITERATOR_MEM_SEL_OP_SUPPORT
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

/** \brief An iterator adaptor that uses a predicate to filter desired
 *    values from the iterator's underlying sequence.
 *
 * The design of this component is described in detail in chapter 42 of
 * <a href="http://extendedstl.com/">Extended STL, volume 1</a>, which is
 * also available for
 * <a href="http://www.informit.com/content/images/9780321305503/samplechapter/0321305507_CH42.pdf">free download</a>.
 *
 * \ingroup group__library__iterators
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k P
        ,   ss_typename_param_k T = adapted_iterator_traits<I>
        >
// [[synesis:class:iterator: filter_iterator<T<I>, T<P>, T<T>>]]
class filter_iterator
{
/// \name Member Types
/// @{
public:
    typedef I                                                               base_iterator_type;
    typedef P                                                               filter_predicate_type;
    typedef T                                                               traits_type;
    typedef filter_iterator<I, P, T>                                        class_type;
#if 0
    typedef ss_typename_type_k traits_type::iterator_category               iterator_category;
#else /* ? 0 */
    typedef ss_typename_type_k min_iterator_category<   ss_typename_type_k traits_type::iterator_category
                                                    ,   std::bidirectional_iterator_tag
                                                    >::iterator_category    iterator_category;
#endif /* 0 */
    typedef ss_typename_type_k traits_type::value_type                      value_type;
    typedef ss_typename_type_k traits_type::difference_type                 difference_type;
    typedef ss_typename_type_k traits_type::pointer                         pointer;
    typedef ss_typename_type_k traits_type::const_pointer                   const_pointer;
    typedef ss_typename_type_k traits_type::reference                       reference;
    typedef ss_typename_type_k traits_type::const_reference                 const_reference;
    typedef ss_typename_type_k traits_type::effective_reference             effective_reference;
    typedef ss_typename_type_k traits_type::effective_const_reference       effective_const_reference;
    typedef ss_typename_type_k traits_type::effective_pointer               effective_pointer;
    typedef ss_typename_type_k traits_type::effective_const_pointer         effective_const_pointer;
/// @}

/// \name Construction
/// @{
public:
    filter_iterator(base_iterator_type begin, base_iterator_type end, filter_predicate_type pr)
        : m_it(begin)
        , m_begin(begin)
        , m_end(end)
        , m_predicate(pr)
    {
        for(; m_it != m_end; ++m_it)
        {
            if(m_predicate(*m_it))
            {
                break;
            }
        }
    }

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
        STLSOFT_MESSAGE_ASSERT("Attempting to increment an endpoint iterator", m_it != m_end);

        for(++m_it; m_it != m_end; ++m_it)
        {
            if(m_predicate(*m_it))
            {
                break;
            }
        }

        return *this;
    }
    class_type operator ++(int)
    {
        class_type  ret(*this);

        operator ++();

        return ret;
    }

    effective_reference operator *()
    {
        return *m_it;
    }
    effective_const_reference operator *() const
    {
        return *m_it;
    }

#ifdef STLSOFT_FILTER_ITERATOR_MEM_SEL_OP_SUPPORT
# ifdef STLSOFT_FILTER_ITERATOR_MUTABLE_OP_SUPPORT
    effective_pointer operator ->()
    {
        enum { is_iterator_pointer_type = is_pointer_type<base_iterator_type>::value };

        // This has to be a separate typedef, otherwise DMC++ has a fit
        typedef ss_typename_type_k value_to_yesno_type<is_iterator_pointer_type>::type  yesno_t;

        return invoke_member_selection_operator_(yesno_t());
    }
# endif /* STLSOFT_FILTER_ITERATOR_MUTABLE_OP_SUPPORT */

    effective_const_pointer operator ->() const
    {
        enum { is_iterator_pointer_type = is_pointer_type<base_iterator_type>::value };

        // This has to be a separate typedef, otherwise DMC++ has a fit
        typedef ss_typename_type_k value_to_yesno_type<is_iterator_pointer_type>::type  yesno_t;

        return invoke_member_selection_operator_(yesno_t());
    }
#endif /* STLSOFT_FILTER_ITERATOR_MUTABLE_OP_SUPPORT */
/// @}

/// \name Bidirectional Iterator methods
/// @{
public:
    class_type& operator --()
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to increment a start-of-range iterator", m_it != m_begin);

        for(--m_it; m_it != m_begin; --m_it)
        {
            if(m_predicate(*m_it))
            {
                break;
            }
        }

        return *this;
    }
    class_type operator --(int)
    {
        class_type  ret(*this);

        operator --();

        return ret;
    }
/// @}

/// \name Comparison
/// @{
public:
    ss_bool_t equal(class_type const& rhs) const
    {
        STLSOFT_MESSAGE_ASSERT("Comparing iterators from different sequences", m_end == rhs.m_end);

        return m_it == rhs.m_it;
    }
    /// \deprecated
    ss_bool_t equals(class_type const& rhs) const
    {
        return equal(rhs);
    }
/// @}

/// \name Attributes
/// @{
public:
    difference_type distance() const
    {
        return m_end - m_it;
    }
/// @}

/// \name Implementation
/// @{
private:
#ifdef STLSOFT_FILTER_ITERATOR_MEM_SEL_OP_SUPPORT
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
#endif /* STLSOFT_FILTER_ITERATOR_MEM_SEL_OP_SUPPORT */
/// @}

/// \name Members
/// @{
private:
    base_iterator_type      m_it;
    base_iterator_type      m_begin;
    base_iterator_type      m_end;
    filter_predicate_type   m_predicate;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator function
 */

/** \brief Creator function for filter_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param from The iterator marking the start of the range to filter
 * \param to The iterator marking (one past) the end of the range to filter
 * \param pr The predicate used to filter the underlying range
 *
 * \return An instance of the specialisation filter_iterator&lt;T, P&gt;
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k P
        >
inline filter_iterator<I, P> make_filter_iterator(I from, I to, P pr)
{
    return filter_iterator<I, P>(from, to, pr);
}

/** \brief Creator function for filter_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param from The iterator marking the start of the range to filter
 * \param to The iterator marking (one past) the end of the range to filter
 * \param pr The predicate used to filter the underlying range
 *
 * \return An instance of the specialisation filter_iterator&lt;T, P&gt;
 *
 * \note Short-hand for make_filter_iterator()
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k P
        >
inline filter_iterator<I, P> filter(I from, I to, P pr)
{
    return make_filter_iterator(from, to, pr);
}

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

// operator ==

template<   ss_typename_param_k I
        ,   ss_typename_param_k P
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator ==(filter_iterator<I, P, T> const& lhs, filter_iterator<I, P, T> const& rhs)
{
    return lhs.equal(rhs);
}

// operator !=

template<   ss_typename_param_k I
        ,   ss_typename_param_k P
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator !=(filter_iterator<I, P, T> const& lhs, filter_iterator<I, P, T> const& rhs)
{
    return !lhs.equal(rhs);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/filter_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_FILTER_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
