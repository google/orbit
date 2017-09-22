/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/associative_select_iterator.hpp
 *
 * Purpose:     An iterator adaptor that uses a selection function on its
 *              underlying iterator in order to obtain the key/mapped type.
 *
 * Created:     28th January 2005
 * Updated:     13th October 2010
 *
 * Thanks:      To Manfred Ehrhart, for detecting issues with iterator
 *              member types and select_second().
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/iterators/associative_select_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::contiguous_diluter_iterator
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR_MAJOR    2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR_MINOR    2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR_REVISION 2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR_EDIT     28
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

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_BORLAND)
# ifndef STLSOFT_INCL_STLSOFT_ITERATOR_HPP_ADAPTED_ITERATOR_TRAITS
#  include <stlsoft/iterators/adapted_iterator_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ITERATOR_HPP_ADAPTED_ITERATOR_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
#  include <stlsoft/meta/is_pointer_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
#  include <stlsoft/meta/yesno.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
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

/** \brief Selects the \c first member of the type to which it is applied
 *
 * \ingroup group__library__iterators
 */
// [[synesis:class:function-class:unary-function: select_first<T<P>>]]
template <class P>
struct select_first
{
public:
    typedef ss_typename_type_k P::first_type    value_type;
    typedef value_type&                         reference;
    typedef value_type const&                   const_reference;

public:
    reference operator ()(P& p)
    {
        return p.first;
    }
    const_reference operator ()(P const& p) const
    {
        return p.first;
    }
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <class P>
struct select_first_const
{
public:
    typedef const ss_typename_type_k P::first_type  value_type;
    typedef value_type&                             reference;
    typedef value_type&                             const_reference;

public:
    const_reference operator ()(P const& p) const
    {
        return p.first;
    }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Selects the \c first second of the type to which it is applied
 *
 * \ingroup group__library__iterators
 */
// [[synesis:class:function-class:unary-function: select_second<T<P>>]]
template <class P>
struct select_second
{
public:
    typedef ss_typename_type_k P::second_type   value_type;
    typedef value_type&                         reference;
    typedef value_type const&                   const_reference;

public:
    reference operator ()(P& p)
    {
        return p.second;
    }
    const_reference operator ()(P const& p) const
    {
        return p.second;
    }
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <class P>
struct select_second_const
{
public:
    typedef const ss_typename_type_k P::second_type value_type;
    typedef value_type&                             reference;
    typedef value_type&                             const_reference;

public:
    const_reference operator ()(P const& p) const
    {
        return p.second;
    }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An iterator adaptor that uses a select functor on its base iterator
 *    in order to obtain the value_type.
 *
 * \ingroup group__library__iterators
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
class associative_select_iterator
{
public:
    /// The type of the adapted iterator
    typedef I                                       adapted_iterator_type;
    /// The type of the selecting function
    typedef F                                       selecting_function_type;
    /// The iterator category type
    typedef ss_typename_type_k I::iterator_category iterator_category;
    /// The value type
    typedef ss_typename_type_k F::value_type        value_type;
    /// The difference type
    typedef ss_ptrdiff_t                            difference_type;
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    typedef difference_type                         distance_type;
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
    /// This type
    typedef associative_select_iterator<I, F>       class_type;
    /// The mutating (non-const) reference type
    typedef value_type&                             reference;
    /// The non-mutating (const) reference type
    typedef value_type const&                       const_reference;
    /// The mutating (non-const) pointer type
    typedef value_type*                             pointer;
    /// The non-mutating (const) pointer type
    typedef value_type const*                       const_pointer;

public:
    /// Constructs an instance of the iterator from the adapted iterator
    /// and, optionally, an instance of the selecting function type
    ///
    /// \param i The iterator to be adapted
    /// \param f The selecting function
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1400
    associative_select_iterator(
        adapted_iterator_type   i
    ,   selecting_function_type f = selecting_function_type()
    )
#else /* ? compiler */
    template <ss_typename_param_k I2>
    associative_select_iterator(
        I2                      i
    ,   selecting_function_type f = selecting_function_type()
    )
#endif /* compiler */
        : m_i(i)
        , m_f(f)
    {}

public:
    ss_bool_t equal(class_type const& rhs) const
    {
        return m_i == rhs.m_i;
    }

public:
    class_type& operator ++()
    {
        ++m_i;

        return *this;
    }
    class_type operator ++(int)
    {
        class_type  ret(*this);

        operator ++();

        return ret;
    }

#if defined(STLSOFT_COMPILER_IS_BORLAND)
    const_reference operator *() const
    {
        return m_f(*m_i);
    }
#else
    reference operator *()
    {
        return m_f(*m_i);
    }
#endif

private:
    adapted_iterator_type   m_i;
    selecting_function_type m_f;
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

/** Creates an instance of associative_select_iterator.
 *
 * \param i The iterator to be adapted
 * \param f The function to select \c first or \c second member
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline associative_select_iterator<I, F> make_associative_select_iterator(I i, F f)
{
    return associative_select_iterator<I, F>(i, f);
}

/** Creates an instance of associative_select_iterator.
 *
 * \param i The iterator to be adapted
 * \param f The function to select \c first or \c second member
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline associative_select_iterator<I, F> associative_select(I i, F f)
{
    return make_associative_select_iterator(i, f);
}

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_BORLAND)

/** Creates an instance of associative_select_iterator that selects the
 * \c first member.
 *
 * \param i The iterator to be adapted
 */
template <ss_typename_param_k I>
inline associative_select_iterator<
    I
,   select_first<ss_typename_type_k adapted_iterator_traits<I>::value_type>
> assoc_select_first(I i)
{
    return make_associative_select_iterator(i, select_first<ss_typename_type_k adapted_iterator_traits<I>::value_type>());
}

/** Creates an instance of associative_select_iterator that selects the
 * \c second member.
 *
 * \param i The iterator to be adapted
 */
template <ss_typename_param_k I>
inline associative_select_iterator<
    I
,   select_second<ss_typename_type_k adapted_iterator_traits<I>::value_type>
> assoc_select_second(I i)
{
    return make_associative_select_iterator(i, select_second<ss_typename_type_k adapted_iterator_traits<I>::value_type>());
}

#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator ==(associative_select_iterator<I, F> const& lhs, associative_select_iterator<I, F> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator !=(associative_select_iterator<I, F> const& lhs, associative_select_iterator<I, F> const& rhs)
{
    return !lhs.equal(rhs);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_ASSOCIATIVE_SELECT_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
