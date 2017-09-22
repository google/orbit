/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/indirect_reverse_iterator.hpp
 *
 * Purpose:     indirect_reverse_iterator class template.
 *
 * Created:     7th June 2005
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


/** \file stlsoft/iterators/indirect_reverse_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::indirect_reverse_iterator
 *   iterator adaptor class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR_MAJOR      2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR_MINOR      2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR_REVISION   6
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR_EDIT       30
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
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_DESTRUCTOR
# include <stlsoft/memory/auto_destructor.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ITERATOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

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

/** \brief This class template provides the same services as
 * std::reverse_iterator, but uses pointers in order to facilitate reverse
 * adaptation of incompletely defined types, such as forward-declared member
 * classes.
 *
 * \ingroup group__library__iterators
 *
 * \param I The iterator to be adapted for reverse iteration
 * \param T The value type
 * \param R The reference type
 * \param P The pointer type
 * \param D The distance type
 */
template<   ss_typename_param_k I
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_BORLAND) && \
    (   !defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) || \
        STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION >= STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1)
        ,   ss_typename_param_k T = ss_typename_type_def_k stlsoft_ns_qual_std(iterator_traits)<I>::value_type
#else /* ? compiler */
        ,   ss_typename_param_k T
#endif /* compiler */
        ,   ss_typename_param_k R = T&
        ,   ss_typename_param_k P = T*
        ,   ss_typename_param_k D = ss_ptrdiff_t
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_BORLAND) && \
    (   !defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) || \
        STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION >= STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1)
        ,   ss_typename_param_k C = ss_typename_type_def_k stlsoft_ns_qual_std(iterator_traits)<I>::iterator_category
#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
        ,   ss_typename_param_k C = stlsoft_ns_qual_std(input_iterator_tag)
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
        >
// [[synesis:class:iterator: indirect_reverse_iterator<T<I>, T<T>, T<R>, T<P>, T<D>>]]
class indirect_reverse_iterator
//    : public reverse_iterator_base<I, T, R, P, D>
{
/// \name Members
/// @{
private:
    /// The iterator pointer type
    ///
    /// \note auto_desctructor is chosen for three reasons:
    /// - Is supplies RAII management of the iterator object
    /// - It supports only a one-to-one relationship between the mapped
    ///   iterator type and indirect_reverse_iterator
    /// - It has immutable RAII, requiring explicit writing of the
    ///   copy constructor and copy assignment operator
    typedef auto_destructor<I>                              iterator_ptr_type;
public:
    typedef indirect_reverse_iterator<I, T, R, P, D, C>     class_type;
    typedef I                                               iterator_type;
    typedef T                                               value_type;
    typedef R                                               reference;
    typedef P                                               pointer;
    typedef D                                               difference_type;
    typedef C                                               iterator_category;
/// @}

/// \name Construction
/// @{
public:
    indirect_reverse_iterator()
        : m_it(new iterator_type())
    {}
    explicit indirect_reverse_iterator(I it)
        : m_it(new iterator_type(it))
    {}
    indirect_reverse_iterator(class_type const& rhs)
        : m_it(new iterator_type(*rhs.m_it.get()))
    {}
/// @}

/// \name Iterator Operations
/// @{
public:
    /// \brief A copy of the base iterator
    iterator_type base() const
    {
        return *m_it.get();
    }
    reference operator *() const
    {
        return *(*m_it.get() - 1);
    }
    // Define STLSOFT_INDIRECT_REVERSE_ITERATOR_ALLOW_PTR2MEMBER to
    // force provision of operator ->() for compilers for which it
    // is not normally provided.
#if defined(STLSOFT_INDIRECT_REVERSE_ITERATOR_ALLOW_PTR2MEMBER) || \
    (   !defined(STLSOFT_COMPILER_IS_BORLAND) && \
        (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
            _MSC_VER > 1300))
    pointer operator ->() const
    {
        return &*m_it.get();
    }
#endif /* compiler */
    class_type& operator ++()
    {
        --(*m_it.get());

        return *this;
    }
    class_type operator ++(int)
    {
        class_type ret(*this);

        operator ++();

        return ret;
    }

    class_type& operator --()
    {
        ++(*m_it.get());

        return *this;
    }
    class_type operator --(int)
    {
        class_type ret(*this);

        operator --();

        return ret;
    }

    reference operator [](difference_type index) const
    {
        return (*(*this + index));
    }

    class_type& operator +=(difference_type index)
    {
        m_it -= index;

        return (*this);
    }
    class_type operator +(difference_type index) const
    {
        return (class_type(m_it - index));
    }
    class_type& operator -=(difference_type index)
    {
        m_it += index;

        return (*this);
    }
    class_type operator -(difference_type index) const
    {
        return (class_type(m_it + index));
    }
    difference_type operator -(class_type const& rhs) const
    {
        // NOTE: The operands are reversed (since it's a reverse range)
        return *rhs.m_it.get() - *m_it.get();
    }

    bool equal(class_type const& rhs) const
    {
        return *m_it.get() == *rhs.m_it.get();
    }

    int compare(class_type const& rhs) const
    {
        return (*m_it.get() < *rhs.m_it.get()) ? -1 : (*rhs.m_it.get() < *m_it.get()) ? +1 : 0;
    }
/// @}

/// \name Members
/// @{
private:
    iterator_ptr_type   m_it;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k D
        ,   ss_typename_param_k C
        >
inline bool operator ==(indirect_reverse_iterator<I, T, R, P, D, C> const& lhs, indirect_reverse_iterator<I, T, R, P, D, C> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k D
        ,   ss_typename_param_k C
        >
inline bool operator !=(indirect_reverse_iterator<I, T, R, P, D, C> const& lhs, indirect_reverse_iterator<I, T, R, P, D, C> const& rhs)
{
    return !lhs.equal(rhs);
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k D
        ,   ss_typename_param_k C
        >
inline bool operator <(indirect_reverse_iterator<I, T, R, P, D, C> const& lhs, indirect_reverse_iterator<I, T, R, P, D, C> const& rhs)
{
    return lhs.compare(rhs) < 0;
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k D
        ,   ss_typename_param_k C
        >
inline bool operator <=(indirect_reverse_iterator<I, T, R, P, D, C> const& lhs, indirect_reverse_iterator<I, T, R, P, D, C> const& rhs)
{
    return lhs.compare(rhs) <= 0;
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k D
        ,   ss_typename_param_k C
        >
inline bool operator >(indirect_reverse_iterator<I, T, R, P, D, C> const& lhs, indirect_reverse_iterator<I, T, R, P, D, C> const& rhs)
{
    return lhs.compare(rhs) > 0;
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k T
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k D
        ,   ss_typename_param_k C
        >
inline bool operator >=(indirect_reverse_iterator<I, T, R, P, D, C> const& lhs, indirect_reverse_iterator<I, T, R, P, D, C> const& rhs)
{
    return lhs.compare(rhs) >= 0;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/indirect_reverse_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_INDIRECT_REVERSE_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
