/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/proxy_iterator.hpp
 *
 * Purpose:     proxy_iterator template class.
 *
 * Created:     28th June 2004
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


/// \file stlsoft/obsolete/proxy_iterator.hpp
///
/// proxy_iterator template class.

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR_MAJOR      3
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR_MINOR      0
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR_REVISION   3
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR_EDIT       53
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

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Warnings
 */

#if defined(STLSOFT_COMPILER_IS_MSVC)
# if _MSC_VER >= 1200
#  pragma warning(push)
# endif /* compiler */
# pragma warning(disable : 4355)
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Provides translation between the elements in a range and a different value type
 *
 * \ingroup group__library__iterators
 *
 * \param R The element type. This type is the element in the underlying sequence
 * \param V The value type. This is the type to which the element type is translated
 * \param T The traits_type. This type provides a static method \c make_value(), which
 * converts the element type to the value type
 * \param C The iterator category
 * \param R The reference type
 * \param P The pointer type
 *
 * \remarks Iterator for proxy_sequence
 *
 * \deprecated This is maintained for backwards compatibility with the
 *   recls/COM library. New users should instead use
 *   stlsoft::transform_iterator.
 */
template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R = T&
        ,   ss_typename_param_k P = T*
        >
class proxy_iterator
    : public iterator_base<C, V, ss_ptrdiff_t, P, R>
{
/// \name Types
/// @{
private:
    typedef iterator_base<C, V, ss_ptrdiff_t, P, R>                 parent_class_type;
    typedef ss_typename_type_k parent_class_type::value_type        raw_value_type;
public:
    typedef E                                                       element_type;
    typedef raw_value_type                                          value_type;
    typedef ss_typename_type_k parent_class_type::reference         reference;
    typedef value_type const&                                       const_reference;
    typedef T                                                       traits_type;
    typedef ss_typename_type_k parent_class_type::iterator_category iterator_category;
    typedef ss_typename_type_k parent_class_type::pointer           pointer;
    typedef proxy_iterator<E, V, T, C, R, P>                        class_type;
    typedef ss_size_t                                               size_type;
    typedef ss_typename_type_k parent_class_type::difference_type   difference_type;
/// @}

/// \name Construction
/// @{
public:
    /// Default constructor
    proxy_iterator()
        : m_begin(NULL)
        , m_end(NULL)
        , m_value(value_type())
        , m_modified(true)
    {}

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    proxy_iterator(element_type *from, element_type *to)
        : m_begin(from)
        , m_end(to)
        , m_value(value_type())
        , m_modified(true)
    {}
    proxy_iterator(element_type *p, size_type n)
        : m_begin(p)
        , m_end(p + n)
        , m_value(value_type())
        , m_modified(true)
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k P2>
    proxy_iterator(P2 *from, P2 *to)
        : m_begin(from)
        , m_end(to)
        , m_value(value_type())
        , m_modified(true)
    {}
    template <ss_typename_param_k P2>
    proxy_iterator(P2 *p, size_type n)
        : m_begin(p)
        , m_end(p + n)
        , m_value(value_type())
        , m_modified(true)
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

    proxy_iterator(class_type const& rhs)
        : m_begin(rhs.begin())
        , m_end(rhs.end())
        , m_value(value_type())
        , m_modified(true)
    {}

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER >= 1310)
    template <ss_typename_param_k I>
    /* ss_explicit_k */ proxy_iterator(I &i)
        : m_begin(i.begin())
        , m_end(i.end())
        , m_value(value_type())
        , m_modified(true)
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT && compiler */

    class_type& operator =(class_type const& rhs)
    {
        m_begin     =   rhs.begin();
        m_end       =   rhs.end();
        m_modified  =   true;

        return *this;
    }

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT)
    template <ss_typename_param_k I>
    class_type& operator =(I& rhs)
    {
        m_begin     =   rhs.begin();
        m_end       =   rhs.end();
        m_modified  =   true;

        return *this;
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
/// @}

/// \name Helper functions
/// @{
public:
    /// The proxy iterator begin-point
    ///
    /// \return A pointer to the current position of the proxy iterator
    element_type    *begin()
    {
        return m_begin;
    }
    /// The proxy iterator endpoint
    ///
    /// \return A pointer to the end point of the proxy iterator
    element_type    *end()
    {
        return m_end;
    }
    /// The proxy iterator begin-point
    ///
    /// \return A pointer to the current position of the proxy iterator
    element_type const  *begin() const
    {
        return m_begin;
    }
    /// The proxy iterator endpoint
    ///
    /// \return A pointer to the end point of the proxy iterator
    element_type const  *end() const
    {
        return m_end;
    }
/// @}

/// \name Iterator methods
/// @{
public:
    /// Pre-increment operator
    class_type& operator ++()
    {
        STLSOFT_MESSAGE_ASSERT("Incrementing invalid iterator", m_begin != m_end);

        ++m_begin;

        m_modified = true;

        return *this;
    }
    /// Post-increment operator
    class_type operator ++(int)
    {
        class_type  r(*this);

        operator ++();

        return r;
    }
    /// Pre-decrement operator
    class_type& operator --()
    {
        --m_begin;

        m_modified = true;

        return *this;
    }
    /// Post-decrement operator
    class_type operator --(int)
    {
        class_type  r(*this);

        operator --();

        return r;
    }
    /// Dereference to return a value at the current position of type V
    const_reference operator *() const
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to dereference an invalid iterator", m_begin != m_end);

        if(m_modified)
        {
            // Can't make m_value mutable, as it confuses too many compilers
            remove_const(m_value) = value_type(traits_type::make_value(*m_begin));

            m_modified = false;
        }

        return m_value;
    }

    /// Evaluates whether \c this and \c rhs are equivalent
    ss_bool_t equal(class_type const& rhs) const
    {
        ss_bool_t   bEqual;

        if(m_end == rhs.m_end)
        {
            // It's a copy of the same iterator, so it's only equal if the m_begin's are the tame
            bEqual = m_begin == rhs.m_begin;
        }
        else
        {
            // It's sourced from a different iterator, so they're only the same if they're both closed
            bEqual = (m_begin == m_end) == (rhs.m_begin == rhs.m_end);
        }

        return bEqual;
    }

    ss_ptrdiff_t compare(class_type const& rhs) const
    {
        return m_begin - rhs.m_begin;
    }
/// @}

private:
    element_type            *m_begin;
    element_type            *m_end;
    value_type              m_value;    // Can't make this mutable, as it confuses too many compilers
    ss_mutable_k ss_bool_t  m_modified;
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_bool_t operator ==(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_bool_t operator !=(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return !lhs.equal(rhs);
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline proxy_iterator<E, V, T, C, R, P> operator +(proxy_iterator<E, V, T, C, R, P> const& lhs, ss_ptrdiff_t d)
{
    return proxy_iterator<E, V, T, C, R, P>(lhs.begin() + d, lhs.end());
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline proxy_iterator<E, V, T, C, R, P> operator +(ss_ptrdiff_t d, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return proxy_iterator<E, V, T, C, R, P>(rhs.begin() + d, rhs.end());
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline proxy_iterator<E, V, T, C, R, P> operator -(proxy_iterator<E, V, T, C, R, P> const& lhs, ss_ptrdiff_t d)
{
    return proxy_iterator<E, V, T, C, R, P>(lhs.begin() - d, lhs.end());
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_ptrdiff_t operator -(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return rhs.begin() - lhs.begin();
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_ptrdiff_t operator <(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return lhs.compare(rhs) < 0;
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_ptrdiff_t operator <=(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return lhs.compare(rhs) <= 0;
}

template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_ptrdiff_t operator >(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return lhs.compare(rhs) > 0;
}


template<   ss_typename_param_k E
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k C
        ,   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline ss_ptrdiff_t operator >=(proxy_iterator<E, V, T, C, R, P> const& lhs, proxy_iterator<E, V, T, C, R, P> const& rhs)
{
    return lhs.compare(rhs) >= 0;
}

/* /////////////////////////////////////////////////////////////////////////
 * Warnings
 */

#if defined(STLSOFT_COMPILER_IS_MSVC)
# if _MSC_VER >= 1200
#  pragma warning(pop)
# else /* ? compiler */
#  pragma warning(default: 4355)
# endif /* _MSC_VER */
#endif /* compiler */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} /* namespace stlsoft */
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_PROXY_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
