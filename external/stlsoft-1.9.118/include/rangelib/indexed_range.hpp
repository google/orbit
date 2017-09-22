/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/indexed_range.hpp
 *
 * Purpose:     Indexed range adaptor class.
 *
 * Created:     11th October 2004
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


/** \file rangelib/indexed_range.hpp Integral range class */

#ifndef RANGELIB_INCL_RANGELIB_HPP_INDEXED_RANGE
#define RANGELIB_INCL_RANGELIB_HPP_INDEXED_RANGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_INDEXED_RANGE_MAJOR       2
# define RANGELIB_VER_RANGELIB_HPP_INDEXED_RANGE_MINOR       4
# define RANGELIB_VER_RANGELIB_HPP_INDEXED_RANGE_REVISION    2
# define RANGELIB_VER_RANGELIB_HPP_INDEXED_RANGE_EDIT        33
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_MSVC:     _MSC_VER < 1200
STLSOFT_COMPILER_IS_MWERKS:   (__MWERKS__ & 0xFF00) < 0x3000
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGELIB
# include <rangelib/rangelib.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGELIB */

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    (   defined(STLSOFT_COMPILER_IS_MWERKS) && \
        ((__MWERKS__ & 0xFF00) < 0x3000))
# error This file is not compatible with the current compiler
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE
#  include <stlsoft/meta/typefixer/reference.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
#ifndef RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS
# include <rangelib/operator_adaptors.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS */
#ifdef STLSOFT_UNITTEST
# ifndef RANGELIB_INCL_RANGELIB_HPP_INTEGRAL_RANGE
#  include <rangelib/integral_range.hpp>
# endif /* !RANGELIB_INCL_RANGELIB_HPP_INTEGRAL_RANGE */
#endif /* STLSOFT_UNITTEST */

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

/** \brief Adapts a range and provides an integral count alongside the range
 *
 * \ingroup group__library__rangelib
 *
 * \param R The adapted range type
 * \param I The integer type
 */
template<   class               R
        ,   ss_typename_param_k I   =   int
        >
class indexed_range
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
    : public /* ss_typename_type_k */ operator_adaptor_selector<indexed_range<R, I>, R>::type   // This provides the operator forms of the methods. R can serve as its own traits here
#else /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
    : public non_mutating_operator_adaptor< indexed_range<R, I>, R>
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
    , public R::range_tag_type
{
/// \name Member Types
/// @{
private:
#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
    /// Indicates whether the parameterising sequence type has a \c reference member
    ///
    /// \note We can't use the type fixer in a fully correct way here, because
    /// most compilers do not detect reference / const_reference members, so we
    /// take a guess and use the presence/absence of the iterator membertype as
    /// the sign of the presence of the reference member type.
# if defined(STLSOFT_COMPILER_IS_MWERKS)
    enum { HAS_MEMBER_REFERENCE = 0 != member_traits<R>::has_member_reference };
# else /* ? compiler */
    enum { HAS_MEMBER_REFERENCE = 0 != member_traits<R>::has_member_iterator };
# endif /* compiler */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

public:
    /// The of the adapted range
    typedef R                                                                       adapted_range_type;
    /// The index type
    typedef I                                                                       index_type;
    /// The range tag type
#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER > 1200
    typedef ss_typename_type_k R::range_tag_type                                    range_tag_type;
#endif /* compiler */
    /// The value type
    typedef ss_typename_type_k R::value_type                                        value_type;
    /// The non-mutating (const) type
    typedef ss_typename_type_k R::const_reference                                   const_reference;
#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
    /// The mutating (non-const) type
    typedef ss_typename_type_k select_first_type_if<   ss_typename_type_k ::stlsoft::typefixer::fixer_reference<R, HAS_MEMBER_REFERENCE>::reference
                                                ,   const_reference
                                                ,   HAS_MEMBER_REFERENCE
                                                >::type                             reference;
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
    /// The current instantiation of the type
    typedef indexed_range<R, I>                                                     class_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructor
    ///
    /// \param r The range
    /// \param index The initial index of the range
    ss_explicit_k indexed_range(R r, index_type index = 0)
        : m_range(r)
        , m_index(index)
    {}
/// @}

/// \name Notional Range methods
/// @{
public:
    /// Indicates whether the range is open
    ss_bool_t is_open() const
    {
        return m_range.is_open();
    }
#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
    /// Returns the current value in the range
    reference current()
    {
        STLSOFT_ASSERT(is_open());

        return m_range.current();
    }
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
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

        ++m_range;
        ++m_index;

        return *this;
    }
/// @}

#if 0
/// \name Iterable Range methods
/// @{
public:
    /// Returns an iterator to the current position of the range
    iterator begin()
    {
        return m_range.begin();
    }
    /// Returns an iterator to the end of the range
    iterator end()
    {
        return m_range.end();
    }

    /// Returns an iterator to the current position of the range
    const_iterator begin() const
    {
        return m_range.begin();
    }
    /// Returns an iterator to the end of the range
    const_iterator end() const
    {
        return m_range.end();
    }
/// @}
#endif /* 0 */

/// \name Iterable Range methods
/// @{
public:
    index_type  index() const
    {
        return m_index;
    }
/// @}

// Members
private:
    adapted_range_type  m_range;
    index_type          m_index;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/indexed_range_unittest_.h"
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

#endif /* !RANGELIB_INCL_RANGELIB_HPP_INDEXED_RANGE */

/* ///////////////////////////// end of file //////////////////////////// */
