/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/contiguous_diluter_iterator.hpp
 *
 * Purpose:     Enhanced ostream iterator.
 *
 * Created:     16th December 2005
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


/** \file stlsoft/iterators/contiguous_diluter_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::contiguous_diluter_iterator
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR_MAJOR    1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR_MINOR    0
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR_REVISION 4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR_EDIT     12
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
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
 * Classes
 */

/** \brief An iterator adaptor class for use with Contiguous Iterators, to dilute the
 * iterator category to BiDirectional or lesser.
 *
 * \ingroup group__library__iterators
 *
 * \param T The value type of the iterator
 * \param C The iterator category. Defaults to std::bidirectional_iterator_tag
 * \param D The difference type of the iterator. Defaults to ptrdiff_t
 * \param P The pointer type of the iterator. Defaults to T*
 * \param R The reference type of the iterator. Defaults to T&
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C   =   stlsoft_ns_qual_std(bidirectional_iterator_tag)
        ,   ss_typename_param_k D   =   ss_ptrdiff_t
        ,   ss_typename_param_k P   =   T *
        ,   ss_typename_param_k R   =   T &
        >
// [[synesis:class:iterator: contiguous_diluter_iterator<T<T>, T<C>, T<D>, T<P>, T<R>>]]
class contiguous_diluter_iterator
    : public iterator_base<C, T, D, P, R>
{
/// \name Member Types
/// @{
private:
    typedef iterator_base<C, T, D, P, R>                    parent_class_type;
public:
    typedef contiguous_diluter_iterator<T, C, D, P, R>      class_type;
    typedef ss_typename_type_k parent_class_type::pointer   pointer;
    typedef ss_typename_type_k parent_class_type::reference reference;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs from an instance of the iterator to be diluted
    contiguous_diluter_iterator(pointer p)
        : m_p(p)
    {}
/// @}

/// \name Forward Iteration Methods
/// @{
public:
    /// \brief Dereference operator
    reference operator *()
    {
        return *m_p;
    }
    /// \brief Pre-increment operator
    class_type& operator ++()
    {
        ++m_p;

        return *this;
    }
    /// \brief Post-increment operator
    class_type& operator ++(int)
    {
        class_type r(*this);

        operator ++();

        return r;
    }

    ss_bool_t   equal(class_type const& rhs) const
    {
        return m_p == rhs.m_p;
    }
    ss_bool_t operator ==(class_type const& rhs) const
    {
        return equal(rhs);
    }
    ss_bool_t operator !=(class_type const& rhs) const
    {
        return !equal(rhs);
    }
/// @}

/// \name Bidirectional Iteration Methods
/// @{
public:
    /// \brief Pre-decrement operator
    class_type& operator --()
    {
        --m_p;

        return *this;
    }
    /// \brief Post-decrement operator
    class_type& operator --(int)
    {
        class_type r(*this);

        operator --();

        return r;
    }
/// @}

/// \name Members
/// @{
private:
    pointer m_p;
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/contiguous_diluter_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_CONTIGUOUS_DILUTER_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
