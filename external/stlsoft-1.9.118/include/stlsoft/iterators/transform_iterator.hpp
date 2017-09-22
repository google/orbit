/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/transform_iterator.hpp
 *
 * Purpose:     An iterator adaptor that uses a unary function object to
 *              transform the values from the iterator's underlying
 *              sequence.
 *
 * Created:     6th February 1999
 * Updated:     2nd September 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/iterators/transform_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::transform_iterator
 *   iterator adaptor class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR_MAJOR     2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR_MINOR     1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR_REVISION  1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR_EDIT      120
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
# error stlsoft/iterators/transform_iterator.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR
# include <stlsoft/smartptr/shared_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# include <functional>
# include <string>
# include <vector>
# include <stlsoft/conversion/integer_to_string.hpp>
#endif /* STLSOFT_UNITTEST */

// #define STLSOFT_TRANSFORM_ITERATOR_IS_COMPATIBLE_WITH_HETEROGENOUS_ITERATOR_ALGOS

/* /////////////////////////////////////////////////////////////////////////
 * Compiler & library compatibility
 */

#ifdef STLSOFT_ITER_TXFM_ITER_OLD_DW
# undef STLSOFT_ITER_TXFM_ITER_OLD_DW
#endif /* STLSOFT_ITER_TXFM_ITER_OLD_DW */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
# define STLSOFT_ITER_TXFM_ITER_OLD_DW
#endif /* library */

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

#if 0
struct null_function
{
public:
    typedef void*   result_type;
};
#endif /* 0 */


/** \brief An iterator adaptor that uses a predicate to filter desired
 *    values from the iterator's underlying sequence.
 *
 * \ingroup group__library__iterators
 *
 * I The iterator to transform
 * F The unary function that will be used to transform the values
 *
 * \note The iterator provides the same iterator category as the base iterator type I
 *  (which is mapped to the member type \c iterator_type), but it always provides
 *  By-Value Temporary (BVT) element references, so its pointer and reference member types
 *  are always void.
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
// [[synesis:class:iterator: transform_iterator<T<I>, T<F>>]]
class transform_iterator
#if 0
    : public stlsoft_ns_qual(iterator_base)<ss_typename_type_k stlsoft_ns_qual_std(iterator_traits)<I>::iterator_category
                                        ,   ss_typename_type_k transform_function_type::result_type
                                        ,   ????
                                        ,   void
                                        ,   void
                                        >
#endif /* 0 */
{
/// \name Member Types
/// @{
public:
    /// The iterator to transform
    typedef I                                                       iterator_type;
    /// The unary function that will be used to transform the values
    typedef F                                                       transform_function_type;
    /// The value type
    typedef ss_typename_type_k transform_function_type::result_type value_type;
#ifdef STLSOFT_ITER_TXFM_ITER_OLD_DW
    /// The iterator category
    typedef stlsoft_ns_qual_std(input_iterator_tag)                 iterator_category;
    /// The difference type
    typedef void                                                    difference_type;
#else /* ? STLSOFT_ITER_TXFM_ITER_OLD_DW */
    typedef stlsoft_ns_qual_std(iterator_traits)<I>                 traits_type;
    /// The iterator category
    typedef ss_typename_type_k traits_type::iterator_category       iterator_category;
    /// The difference type
    typedef ss_typename_type_k traits_type::difference_type         difference_type;
#endif /* STLSOFT_ITER_TXFM_ITER_OLD_DW */
    /// The parameterisation of the type
    typedef transform_iterator<I, F>                                class_type;
    /// The mutating (non-const) pointer type
    ///
    /// \note Since a transforming iterator <i>must</i> have the By-Value Temporary element
    /// reference category, the \c pointer member must be \c void.
    typedef void                                                    pointer;
    /// The mutating (non-const) reference type
    ///
    /// \note Since a transforming iterator <i>must</i> have the By-Value Temporary element
    /// reference category, the \c reference member must be \c void.
    typedef void                                                    reference;
    /// The effective mutating (non-const) reference type
    ///
    /// \note Since a transforming iterator <i>must</i> have the By-Value Temporary element
    /// reference category, this member must be \c value_type.
    typedef value_type                                              effective_reference;
    /// The effective mutating (non-const) reference type
    ///
    /// \note Since a transforming iterator <i>must</i> have the By-Value Temporary element
    /// reference category, this member must be \c const value_type.
    typedef const value_type                                        effective_const_reference;
/// @}

/// \name Construction
/// @{
public:
    transform_iterator(iterator_type it, transform_function_type pr)
        : m_it(it)
        , m_transformer(pr)
        , m_current()
    {}
    transform_iterator()
        : m_it()
        , m_transformer()
        , m_current()
    {}

    /// \brief A copy of the base iterator
    iterator_type base() const
    {
        return m_it;
    }
/// @}

/// \name Conversion
/// @{
public:
#if 0
# if 1
    transform_iterator(class_type const& rhs)
        : m_it(rhs.m_it)
        , m_transformer(rhs.m_transformer)
    {}
# endif /* 0 */

    template <ss_typename_param_k F2>
    transform_iterator(transform_iterator<I, F2> const& rhs)
        : m_it(rhs.m_it)
        , m_transformer()
    {}

# if 0
    template <ss_typename_param_k F2>
    operator transform_iterator<I, F2>() const
    {
        return transform_iterator<I, F2>(m_it, F2());
    }
# endif /* 0 */
#endif /* 0 */
/// @}

/// \name Accessors
/// @{
public:
    effective_reference         operator *()
    {
        get_current_();

        return *m_current;
    }
    effective_const_reference   operator *() const
    {
        get_current_();

        return *m_current;
    }
/// @}

/// \name Forward Iterator methods
/// @{
public:
    class_type& operator ++()
    {
        ++m_it;
        invalidate_current_();

        return *this;
    }
    class_type operator ++(int)
    {
        class_type ret(*this);

        operator ++();

        return ret;
    }
/// @}

/// \name Bidirectional Iterator methods
/// @{
public:
    class_type& operator --()
    {
        --m_it;
        invalidate_current_();

        return *this;
    }
    class_type operator --(int)
    {
        class_type ret(*this);

        operator --();

        return ret;
    }
/// @}

#ifndef STLSOFT_ITER_TXFM_ITER_OLD_DW
/// \name Random Access Iterator methods
/// @{
public:
    class_type& operator +=(difference_type d)
    {
        m_it += d;

        return *this;
    }
    class_type& operator -=(difference_type d)
    {
        m_it -= d;

        return *this;
    }

    // NOTE: Can't be reference, since what would it reference??
    effective_reference         operator [](difference_type index)
    {
        return m_transformer(m_it[index]);
    }
    // NOTE: Can't be reference, since what would it reference??
    effective_const_reference   operator [](difference_type index) const
    {
        return m_transformer(m_it[index]);
    }
/// @}
#endif /* !STLSOFT_ITER_TXFM_ITER_OLD_DW */

/// \name Comparison
/// @{
public:
    /// Evaluates whether \c this and \c rhs are equivalent
    ss_bool_t equal(class_type const& rhs) const
    {
        return m_it == rhs.m_it;
    }
#if 0
    ss_bool_t equal(iterator_type rhs) const
    {
        return m_it == rhs;
    }
#endif /* 0 */
    /// Compares \c this with the given string
    ss_sint_t compare(class_type const& rhs) const
    {
        return (m_it < rhs.m_it) ? -1 : (rhs.m_it < m_it) ? +1 : 0;
    }
#ifndef STLSOFT_ITER_TXFM_ITER_OLD_DW
    /// Calculate the distance between \c this and \c rhs
    difference_type distance(class_type const& rhs) const
    {
        return m_it - rhs.m_it;
    }
#endif /* !STLSOFT_ITER_TXFM_ITER_OLD_DW */
/// @}

/// \name Implementation
/// @{
private:
    void get_current_()
    {
        if(NULL == m_current.get())
        {
            m_current = value_type_ptr_type_(new value_type(m_transformer(*m_it)));
        }
    }
    void invalidate_current_()
    {
        m_current.close();
    }
/// @}

/// \name Members
/// @{
private:
    typedef shared_ptr<value_type>    value_type_ptr_type_;

    iterator_type           m_it;
    transform_function_type m_transformer;
    value_type_ptr_type_    m_current;
/// @}

/// \name Not to be implemented
/// @{
private:
    struct transform_iterator_is_BVT_so_no_member_selection_operators
    {};

    transform_iterator_is_BVT_so_no_member_selection_operators *operator ->() const;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

/** \brief Creator function for transform_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param it The iterator to transform
 * \param fn The function object used to effect the transformation
 *
 * \return An instance of the specialisation transform_iterator&lt;T, F&gt;
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline transform_iterator<I, F> make_transform_iterator(I it, F fn)
{
    return transform_iterator<I, F>(it, fn);
}

/** \brief Creator function for transform_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param it The iterator to transform
 * \param fn The function object used to effect the transformation
 *
 * \return An instance of the specialisation transform_iterator&lt;T, F&gt;
 *
 * \note Short-hand for make_transform_iterator()
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline transform_iterator<I, F> transformer(I it, F fn)
{
    return make_transform_iterator(it, fn);
}

#if 0
template<   ss_typename_param_k I
        >
inline transform_iterator<I, null_function> transformer(I it)
{
    return transform_iterator<I, null_function>(it, null_function());
}
#endif

#if 0
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline transform_iterator<I, F> transform(I it, F fn)
{
    return transform_iterator<I, F>(it, fn);
}
#endif /* 0 */

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

// operator ==

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator ==(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return lhs.equal(rhs);
}

// operator !=

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator !=(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return !lhs.equal(rhs);
}

#if 0
template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator !=(transform_iterator<I, F> const& lhs, I rhs)
{
    return !lhs.equal(rhs);
}
#endif /* 0 */

#ifndef STLSOFT_ITER_TXFM_ITER_OLD_DW
// operator +

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline transform_iterator<I, F> operator +(transform_iterator<I, F> const& lhs, ss_typename_type_k transform_iterator<I, F>::difference_type rhs)
{
    return transform_iterator<I, F>(lhs) += rhs;
}

// operator -

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
#if 0
inline transform_iterator<I, F> operator -(transform_iterator<I, F> const& lhs, ss_typename_type_k transform_iterator<I, F>::difference_type rhs)
#else /* ? 0 */
inline transform_iterator<I, F> operator -(transform_iterator<I, F> const& lhs, ss_ptrdiff_t rhs)
#endif /* 0 */
{
    return transform_iterator<I, F>(lhs) -= rhs;
}

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_typename_type_ret_k transform_iterator<I, F>::difference_type operator -(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return lhs.distance(rhs);
}

// operator <

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator <(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return lhs.compare(rhs) < 0;
}

// operator <=

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator <=(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return lhs.compare(rhs) <= 0;
}

// operator >

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator >(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return lhs.compare(rhs) > 0;
}

// operator >=

template<   ss_typename_param_k I
        ,   ss_typename_param_k F
        >
inline ss_bool_t operator >=(transform_iterator<I, F> const& lhs, transform_iterator<I, F> const& rhs)
{
    return lhs.compare(rhs) >= 0;
}

#endif /* !STLSOFT_ITER_TXFM_ITER_OLD_DW */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/transform_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
