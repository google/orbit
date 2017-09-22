/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/filtered_range.hpp
 *
 * Purpose:     Range filter adaptor.
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


/** \file rangelib/filtered_range.hpp Range filter adaptor */

#ifndef RANGELIB_INCL_RANGELIB_HPP_FILTERED_RANGE
#define RANGELIB_INCL_RANGELIB_HPP_FILTERED_RANGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_FILTERED_RANGE_MAJOR    2
# define RANGELIB_VER_RANGELIB_HPP_FILTERED_RANGE_MINOR    5
# define RANGELIB_VER_RANGELIB_HPP_FILTERED_RANGE_REVISION 2
# define RANGELIB_VER_RANGELIB_HPP_FILTERED_RANGE_EDIT     31
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_DMC:        __DMC__ < 0x0845
STLSOFT_COMPILER_IS_MSVC:       _MSC_VER < 1310
STLSOFT_COMPILER_IS_MWERKS:     (__MWERKS__ & 0xFF00) < 0x3000
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGELIB
# include <rangelib/rangelib.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGELIB */
#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGE_CATEGORIES
# include <rangelib/range_categories.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGE_CATEGORIES */
#ifndef RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS
# include <rangelib/operator_adaptors.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#if !defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
# error This file is not compatible with compilers that do not support member type detection
#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_DIFFERENCE_TYPE
#  include <stlsoft/meta/typefixer/difference_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_DIFFERENCE_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE
#  include <stlsoft/meta/typefixer/reference.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_ITERATOR
#  include <stlsoft/meta/typefixer/iterator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_ITERATOR */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
#ifndef STLSOFT_INCL_STLSOFT_HPP_FILTER_ITERATOR
# include <stlsoft/iterators/filter_iterator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_HPP_FILTER_ITERATOR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::rangelib */
namespace rangelib
{
# else
/* Define stlsoft::rangelib_project */

namespace stlsoft
{

namespace rangelib_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief This class adapts an STL sequence instance into a Range
 *
 * \ingroup group__library__rangelib
 *
 * \param R The range class
 * \param P The filter predicate
 * \param RC The range category tag type
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k P
        ,   ss_typename_param_k RC = ss_typename_type_k R::range_tag_type
        >
class filtered_range
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
    : public /* ss_typename_type_k */ operator_adaptor_selector<filtered_range<R, P, RC>, R>::type // This provides the operator forms of the methods. R can serve as its own traits here
#else /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
    : public non_mutating_operator_adaptor<filtered_range<R, P, RC>, R>
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
    , public RC
{
/// \name Member Types
/// @{
public:
    typedef R                                                                   filtered_range_type;
    typedef P                                                                   filter_predicate_type;
    typedef RC                                                                  range_tag_type;
    typedef filtered_range<R, P, RC>                                            class_type;
private:
#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
    /// Indicates whether the parameterising sequence type has a \c reference member
    ///
    /// \note We can't use the type fixer in a fully correct way here, because
    /// most compilers do not detect reference / const_reference members, so we
    /// take a guess and use the presence/absence of the iterator membertype as
    /// the sign of the presence of the reference member type.
    enum { HAS_MEMBER_ITERATOR          =   0 != member_traits<filtered_range_type>::has_member_iterator        };
    enum { HAS_MEMBER_CONST_ITERATOR    =   0 != member_traits<filtered_range_type>::has_member_const_iterator  };
# if defined(STLSOFT_COMPILER_IS_MWERKS)
    enum { HAS_MEMBER_REFERENCE         =   0 != member_traits<filtered_range_type>::has_member_reference       };
# else /* ? compiler */
    enum { HAS_MEMBER_REFERENCE         =   0 != HAS_MEMBER_ITERATOR                                            };
# endif /* compiler */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
private:
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k select_first_type_if<   ss_typename_type_k ::stlsoft::typefixer::fixer_const_iterator<filtered_range_type, HAS_MEMBER_CONST_ITERATOR>::const_iterator
                                                ,   void
                                                ,   HAS_MEMBER_CONST_ITERATOR
                                                >::type                         const_iterator_base_type;

    /// The mutating (non-const) iterator type
    typedef ss_typename_type_k select_first_type_if<   ss_typename_type_k ::stlsoft::typefixer::fixer_iterator<filtered_range_type, HAS_MEMBER_ITERATOR>::iterator
                                                ,   const_iterator_base_type
                                                ,   HAS_MEMBER_ITERATOR
                                                >::type                         iterator_base_type;
public:
    /// The mutating (non-const) iterator type
    typedef filter_iterator<iterator_base_type, filter_predicate_type>          iterator;
    /// The non-mutating (const) iterator type
    typedef filter_iterator<const_iterator_base_type, filter_predicate_type>    const_iterator;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k select_first_type_if<   ss_typename_type_k ::stlsoft::typefixer::fixer_reference<filtered_range_type, HAS_MEMBER_REFERENCE>::reference
                                                ,   ss_typename_type_k filtered_range_type::const_reference
                                                ,   HAS_MEMBER_REFERENCE
                                                >::type                         reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k filtered_range_type::const_reference             const_reference;
    /// The value type
    typedef ss_typename_type_k filtered_range_type::value_type                  value_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs from a range and a predicate
    ///
    /// \param r The range whose values will be filtered
    /// \param pr The predicate which will be used to filter the values of the range \c r
    ss_explicit_k filtered_range(filtered_range_type r, filter_predicate_type pr = filter_predicate_type())
        : m_range(r)
        , m_predicate(pr)
    {
        for(; m_range; ++m_range)
        {
            if(m_predicate(*m_range))
            {
                break;
            }
        }
    }
/// @}

/// \name Notional Range methods
/// @{
public:
    /// Indicates whether the range is open
    ss_bool_t is_open() const
    {
        return m_range.is_open();
    }
    /// Returns the current value in the range
    reference current()
    {
        STLSOFT_ASSERT(is_open());

        return m_range.current();
    }
    /// Returns the current value in the range
    const_reference current() const
    {
        STLSOFT_ASSERT(is_open());

        return m_range.current();
    }
    /// Advances the current position in the range
    class_type& advance()
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to increment the range past its end point", is_open());

        for(++m_range; m_range; ++m_range)
        {
            if(m_predicate(*m_range))
            {
                break;
            }
        }

        return *this;
    }
/// @}

/// \name Iterable Range methods
/// @{
public:
    /// Returns an iterator to the current position of the range
    iterator begin()
    {
        return iterator(m_range.begin(), m_range.end(), m_predicate);
    }
    /// Returns an iterator to the end of the range
    iterator end()
    {
        return iterator(m_range.end(), m_range.end(), m_predicate);
    }

    /// Returns an iterator to the current position of the range
    const_iterator begin() const
    {
        return const_iterator(m_range.begin(), m_range.end(), m_predicate);
    }
    /// Returns an iterator to the end of the range
    const_iterator end() const
    {
        return const_iterator(m_range.end(), m_range.end(), m_predicate);
    }
/// @}

/// \name Members
/// @{
private:
    filtered_range_type     m_range;
    filter_predicate_type   m_predicate;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

template<   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline filtered_range<R, P> make_filtered_range(R r, P pr)
{
    return filtered_range<R, P>(r, pr);
}

template<   ss_typename_param_k R
        ,   ss_typename_param_k P
        >
inline filtered_range<R, P> filter_range(R r, P pr)
{
    return filtered_range<R, P>(r, pr);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/filtered_range_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace rangelib
# else
} // namespace rangelib_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !RANGELIB_INCL_RANGELIB_HPP_FILTERED_RANGE */

/* ///////////////////////////// end of file //////////////////////////// */
