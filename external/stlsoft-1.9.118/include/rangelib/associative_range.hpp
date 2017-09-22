/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/associative_range.hpp
 *
 * Purpose:     Associative container range adaptor.
 *
 * Created:     1st October 2004
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


/** \file rangelib/associative_range.hpp Sequence container range adaptor */

#ifndef RANGELIB_INCL_RANGELIB_HPP_ASSOCIATIVE_RANGE
#define RANGELIB_INCL_RANGELIB_HPP_ASSOCIATIVE_RANGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_ASSOCIATIVE_RANGE_MAJOR       1
# define RANGELIB_VER_RANGELIB_HPP_ASSOCIATIVE_RANGE_MINOR       4
# define RANGELIB_VER_RANGELIB_HPP_ASSOCIATIVE_RANGE_REVISION    6
# define RANGELIB_VER_RANGELIB_HPP_ASSOCIATIVE_RANGE_EDIT        33
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
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
#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGE_CATEGORIES
# include <rangelib/range_categories.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGE_CATEGORIES */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL
# include <stlsoft/util/operator_bool.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE
# include <stlsoft/meta/is_const_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
 // This stuff's needed for type fixing 'referent_type' => 'mapped_type'
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR
#  include <stlsoft/collections/util/associative_mapped_type_detector.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR */
#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
# if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC)
#  if STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION <= STLSOFT_CF_DINKUMWARE_VC_VERSION_6_0
#   pragma message("associative_range<> assumes that the adapted type has a 'mapped_type' member type. std::map in the Dinkumware library that ships with Visual C++ 5 & 6 uses the non-standard 'referent_type', so adapting a parameterisation of it will not compile")
#  endif /* STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION <= STLSOFT_CF_DINKUMWARE_VC_VERSION_6_0 */
# endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# include <map>
# include <numeric>
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

#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)

/** \brief Traits class for determining the attributes of range-adapted associative container types
 *
 * \ingroup group__library__rangelib
 */
template<   ss_typename_param_k S
        ,   bool                B_CONST
        >
struct associative_range_traits
{
public:
    /// The associative type
    typedef S                                                       associative_type;
    /// The associative reference type
    typedef S&                                                      associative_reference_type;
    /// The key type
    typedef ss_typename_type_k associative_type::key_type           key_type;
    /// The mapped type
    typedef ss_typename_type_k associative_mapped_type_detector<S>::mapped_type mapped_type;
    /// The value type
    typedef ss_typename_type_k associative_type::value_type         value_type;
    /// The mutating (non-const) iterator type
    typedef ss_typename_type_k select_first_type_if<   ss_typename_type_k associative_type::const_iterator
                                                ,   ss_typename_type_k associative_type::iterator
                                                ,   B_CONST
                                                >::type             iterator;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k associative_type::const_iterator     const_iterator;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k select_first_type_if<   ss_typename_type_k associative_type::const_reference
                                                ,   ss_typename_type_k associative_type::reference
                                                ,   B_CONST
                                                >::type             reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k associative_type::const_reference    const_reference;
// TODO: Stick in the member-finder stuff here, so can assume ptrdiff_t if none found
    /// The difference type
    typedef ss_typename_type_k associative_type::difference_type    difference_type;
    /// The size type
    typedef ss_typename_type_k associative_type::size_type          size_type;
};

template<   ss_typename_param_k S
        >
struct associative_range_traits<S, true>
{
public:
    typedef S                                                       associative_type;
    typedef S&                                                      associative_reference_type;
    typedef ss_typename_type_k associative_type::key_type           key_type;
    typedef ss_typename_type_k associative_mapped_type_detector<S>::mapped_type mapped_type;
//    typedef ss_typename_type_k associative_type::referent_type      mapped_type;
    typedef ss_typename_type_k associative_type::value_type         value_type;
    typedef ss_typename_type_k associative_type::const_iterator     iterator;
    typedef ss_typename_type_k associative_type::const_iterator     const_iterator;
    typedef ss_typename_type_k associative_type::const_reference    reference;
    typedef ss_typename_type_k associative_type::const_reference    const_reference;
// TODO: Stick in the member-finder stuff here, so can assume ptrdiff_t if none found
    typedef ss_typename_type_k associative_type::difference_type    difference_type;
    typedef ss_typename_type_k associative_type::size_type          size_type;
};

#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

template<   ss_typename_param_k S
        >
struct associative_range_traits
{
public:
    typedef S                                                       associative_type;
    typedef S&                                                      associative_reference_type;
    typedef ss_typename_type_k associative_type::key_type           key_type;
    typedef ss_typename_type_k associative_type::mapped_type        mapped_type;
    typedef ss_typename_type_k associative_type::value_type         value_type;
    typedef ss_typename_type_k associative_type::iterator           iterator;
    typedef ss_typename_type_k associative_type::const_iterator     const_iterator;
    typedef ss_typename_type_k associative_type::reference          reference;
    typedef ss_typename_type_k associative_type::const_reference    const_reference;
    typedef ss_typename_type_k associative_type::difference_type    difference_type;
    typedef ss_typename_type_k associative_type::size_type          size_type;
};

template<   ss_typename_param_k S
        >
struct associative_range_traits_dinkumware_early
{
public:
    typedef S                                                       associative_type;
    typedef S&                                                      associative_reference_type;
    typedef ss_typename_type_k associative_type::key_type           key_type;
    typedef ss_typename_type_k associative_type::referent_type      mapped_type;
    typedef ss_typename_type_k associative_type::value_type         value_type;
    typedef ss_typename_type_k associative_type::iterator           iterator;
    typedef ss_typename_type_k associative_type::const_iterator     const_iterator;
    typedef ss_typename_type_k associative_type::reference          reference;
    typedef ss_typename_type_k associative_type::const_reference    const_reference;
    typedef ss_typename_type_k associative_type::difference_type    difference_type;
    typedef ss_typename_type_k associative_type::size_type          size_type;
};

template<   ss_typename_param_k S
        >
struct const_associative_range_traits
{
public:
    typedef S                                                       associative_type;
    typedef S const&                                                associative_reference_type;
    typedef ss_typename_type_k associative_type::key_type           key_type;
    typedef ss_typename_type_k associative_type::mapped_type        mapped_type;
    typedef ss_typename_type_k associative_type::value_type         value_type;
    typedef ss_typename_type_k associative_type::const_iterator     iterator;
    typedef ss_typename_type_k associative_type::const_iterator     const_iterator;
    typedef ss_typename_type_k associative_type::const_reference    reference;
    typedef ss_typename_type_k associative_type::const_reference    const_reference;
    typedef ss_typename_type_k associative_type::difference_type    difference_type;
    typedef ss_typename_type_k associative_type::size_type          size_type;
};

template<   ss_typename_param_k S
        >
struct const_associative_range_traits_dinkumware_early
{
public:
    typedef S                                                       associative_type;
    typedef S const&                                                associative_reference_type;
    typedef ss_typename_type_k associative_type::key_type           key_type;
    typedef ss_typename_type_k associative_type::referent_type      mapped_type;
    typedef ss_typename_type_k associative_type::value_type         value_type;
    typedef ss_typename_type_k associative_type::const_iterator     iterator;
    typedef ss_typename_type_k associative_type::const_iterator     const_iterator;
    typedef ss_typename_type_k associative_type::const_reference    reference;
    typedef ss_typename_type_k associative_type::const_reference    const_reference;
    typedef ss_typename_type_k associative_type::difference_type    difference_type;
    typedef ss_typename_type_k associative_type::size_type          size_type;
};
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

/** \brief This class adapts an STL associative instance into a Range
 *
 * \ingroup group__library__rangelib
 *
 * \param S The associative class
 * \param T The associative range traits, used to deduce the Range's iterator, const_iterator, reference, const_reference and value_type
 *
 * It is categoried as an Iterable Range
 *
 * It could be used as follows:
\code
void dump_elements(std::vector<int> const& numbers)
{
  for(associative_range<std::vector<int> > r(numbers); r; ++r)
  {
    std::cout << &r; // Dump the current value to stdout
  }
}
\endcode
 */
template<   ss_typename_param_k S
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
        ,   ss_typename_param_k T = associative_range_traits<S, is_const_type<S>::value>    // Determines whether the associative is const
#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
        ,   ss_typename_param_k T = const_associative_range_traits<S>                       // Determines whether the associative is const
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
        >
class associative_range
    : public iterable_range_tag
{
public:
    /// The associative type
    typedef S                                                           associative_type;
    /// The traits type
    typedef T                                                           traits_type;
    /// The range category tag type
    typedef iterable_range_tag                                          range_tag_type;
    /// The current instantiation of the type
    typedef associative_range<S, T>                                     class_type;
    /// The associative reference type
    typedef ss_typename_type_k traits_type::associative_reference_type  associative_reference_type;
    /// The key type
    typedef ss_typename_type_k traits_type::key_type                    key_type;
    /// The referent type
    typedef ss_typename_type_k traits_type::mapped_type                 mapped_type;
    /// The value type
    typedef ss_typename_type_k traits_type::value_type                  value_type;
    /// The mutating (non-const) iterator type
    typedef ss_typename_type_k traits_type::iterator                    iterator;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k traits_type::const_iterator              const_iterator;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k traits_type::reference                   reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k traits_type::const_reference             const_reference;
    /// The difference type
    typedef ss_typename_type_k traits_type::difference_type             difference_type;
    /// The size type
    typedef ss_typename_type_k traits_type::size_type                   size_type;

public:

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER == 1200)
    /// Constructor
    ///
    /// \param seq The associative which will be adapted to a range
    associative_range(associative_reference_type seq) // The constness of this will require some thinking about. Maybe need associative_range and const_associative_range??
        : m_position(seq.begin())
        , m_last(seq.end())
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER != 1200)
    /// Constructor
    ///
    /// \param seq The associative which will be adapted to a range
    template <ss_typename_param_k S2>
    associative_range(S2 &seq)
        : m_position(seq.begin())
        , m_last(seq.end())
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

    /// Copy constructor
    ///
    /// \note This has to be provided, to avoid precipitating C4217 with Visual C++
    associative_range(class_type const& rhs)
        : m_position(rhs.m_position)
        , m_last(rhs.m_last)
    {}

    /// Copy assignment operator
    ///
    /// \note This has to be provided, to avoid precipitating C4217 with Visual C++
    class_type& operator =(class_type const& rhs)
    {
        m_position  =   rhs.m_position;
        m_last      =   rhs.m_last;

        return *this;
    }

/// \name Notional Range methods
/// @{
private:
    STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(class_type, boolean_generator_type, boolean_type);
public:
    /// Indicates whether the range is open
    ss_bool_t is_open() const
    {
        return m_position != m_last;
    }
    /// Returns the current key+value pair in the range
    reference current()
    {
        STLSOFT_ASSERT(is_open());

        return *m_position;
    }
    /// Returns the current key+value pair in the range
    const_reference current() const
    {
        STLSOFT_ASSERT(is_open());

        return *m_position;
    }
    /// Returns the key of the current item in the range
    key_type current_key()
    {
        return current().first;
    }
    /// Returns the value of the current item in the range
    key_type current_key() const
    {
        return current().first;
    }
    /// Returns the value of the current item in the range
    mapped_type current_value()
    {
        return current().second;
    }
    /// Returns the value of the current item in the range
    mapped_type current_value() const
    {
        return current().second;
    }
    /// Advances the current position in the range
    class_type& advance()
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to increment the range past its end point", is_open());

        ++m_position;

        return *this;
    }

    /// Indicates whether the range is open
    operator boolean_type() const
    {
        return boolean_generator_type::translate(is_open());
    }
    /// Returns the current key+value pair in the range
    reference operator *()
    {
        return current();
    }
    /// Returns the current key+value pair in the range
    const_reference operator *() const
    {
        return current();
    }
    /// Advances the current position in the range
    class_type& operator ++()
    {
        return advance();
    }
    /// Advances the current position in the range, returning a copy of the
    /// range prior to its being advanced
    class_type operator ++(int)
    {
        class_type  ret(*this);

        operator ++();

        return ret;
    }
/// @}

/// \name Iterable Range methods
/// @{
public:
    /// Returns an iterator to the current position of the range
    iterator begin()
    {
        return m_position;
    }
    /// Returns an iterator to the end of the range
    iterator end()
    {
        return m_last;
    }

    /// Returns an iterator to the current position of the range
    const_iterator begin() const
    {
        return m_position;
    }
    /// Returns an iterator to the end of the range
    const_iterator end() const
    {
        return m_last;
    }
/// @}

// Members
private:
    iterator    m_position;
    iterator    m_last;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/associative_range_unittest_.h"
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

#endif /* !RANGELIB_INCL_RANGELIB_HPP_ASSOCIATIVE_RANGE */

/* ///////////////////////////// end of file //////////////////////////// */
