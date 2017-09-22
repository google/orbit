/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/sequence_range.hpp
 *
 * Purpose:     Sequence container range adaptor.
 *
 * Created:     4th November 2003
 * Updated:     10th August 2009
 *
 * Thanks:      To Luoyi (whom I could not thank by email), for pointing out
 *              some gaps with the sequence_range
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file rangelib/sequence_range.hpp Sequence container range adaptor */

#ifndef RANGELIB_INCL_RANGELIB_HPP_SEQUENCE_RANGE
#define RANGELIB_INCL_RANGELIB_HPP_SEQUENCE_RANGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_SEQUENCE_RANGE_MAJOR    2
# define RANGELIB_VER_RANGELIB_HPP_SEQUENCE_RANGE_MINOR    12
# define RANGELIB_VER_RANGELIB_HPP_SEQUENCE_RANGE_REVISION 2
# define RANGELIB_VER_RANGELIB_HPP_SEQUENCE_RANGE_EDIT     62
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_DMC:
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
#ifndef RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS
# include <rangelib/operator_adaptors.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE
# include <stlsoft/meta/is_const_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE */
#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_DIFFERENCE_TYPE
#  include <stlsoft/meta/typefixer/difference_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_DIFFERENCE_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE
#  include <stlsoft/meta/typefixer/reference.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_ITERATOR
#  include <stlsoft/meta/typefixer/iterator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_ITERATOR */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_POINTER
#  include <stlsoft/meta/typefixer/pointer.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_POINTER */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# include <deque>
# include <list>
# include <numeric>
# include <vector>
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

/** \brief Traits class for determining the attributes of range-adapted sequence container types
 *
 * \ingroup group__library__rangelib
 */
template<   ss_typename_param_k S
        ,   bool                B_CONST = false
        >
struct sequence_range_traits
{
private:
    /// Indicates whether the parameterising sequence type has a \c reference member
    ///
    /// \note We can't use the type fixer in a fully correct way here, because
    /// most compilers do not detect reference / const_reference members, so we
    /// take a guess and use the presence/absence of the iterator membertype as
    /// the sign of the presence of the reference member type.
#if defined(STLSOFT_COMPILER_IS_MWERKS)
    enum { HAS_MEMBER_REFERENCE         =   0 != member_traits<S>::has_member_reference         };
    enum { HAS_MEMBER_CONST_REFERENCE   =   0 != member_traits<S>::has_member_const_reference   };
#else /* ? compiler */
    enum { HAS_MEMBER_REFERENCE         =   0 != member_traits<S>::has_member_iterator          };
    enum { HAS_MEMBER_CONST_REFERENCE   =   0 != member_traits<S>::has_member_const_iterator    };
#endif /* compiler */
    enum { HAS_MEMBER_ITERATOR          =   0 != member_traits<S>::has_member_iterator          };  // Need to do these as member enums, otherwise GCC2.95 cries a river
    enum { HAS_MEMBER_CONST_ITERATOR    =   0 != member_traits<S>::has_member_const_iterator    };  // Need to do these as member enums, otherwise GCC2.95 cries a river
    enum { HAS_MEMBER_DIFFERENCE_TYPE   =   0 != member_traits<S>::has_member_difference_type   };  // Need to do these as member enums, otherwise GCC2.95 cries a river
    enum { HAS_MEMBER_POINTER           =   0 != member_traits<S>::has_member_pointer           };
    enum { HAS_MEMBER_CONST_POINTER     =   0 != member_traits<S>::has_member_const_pointer     };

public:
    /// The sequence type
    typedef S                                                       sequence_type;
    /// The sequence reference type
    typedef S&                                                      sequence_reference_type;
    /// The value type
    typedef ss_typename_type_k sequence_type::value_type            value_type;
    /// The mutating (non-const) iterator type
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k ::stlsoft::typefixer::fixer_iterator<S, HAS_MEMBER_ITERATOR>::iterator
                                                ,   ss_typename_type_k ::stlsoft::typefixer::fixer_const_iterator<S, HAS_MEMBER_CONST_ITERATOR>::const_iterator
                                                ,   HAS_MEMBER_ITERATOR
                                                >::type             iterator;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k ::stlsoft::typefixer::fixer_const_iterator<S, HAS_MEMBER_CONST_ITERATOR>::const_iterator
                                                                    const_iterator;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k ::stlsoft::typefixer::fixer_reference<sequence_type, HAS_MEMBER_REFERENCE>::reference
                                                ,   ss_typename_type_k ::stlsoft::typefixer::fixer_const_reference<sequence_type, HAS_MEMBER_CONST_REFERENCE>::const_reference
                                                ,   HAS_MEMBER_REFERENCE
                                                >::type             reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k ::stlsoft::typefixer::fixer_const_reference<S, HAS_MEMBER_CONST_REFERENCE>::const_reference
                                                                    const_reference;
    /// The mutating (non-const) pointer type
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k ::stlsoft::typefixer::fixer_pointer<sequence_type, HAS_MEMBER_POINTER>::pointer
                                                ,   ss_typename_type_k ::stlsoft::typefixer::fixer_const_pointer<sequence_type, HAS_MEMBER_CONST_POINTER>::const_pointer
                                                ,   HAS_MEMBER_POINTER
                                                >::type             pointer;
    /// The non-mutating (const) pointer type
    typedef ss_typename_type_k ::stlsoft::typefixer::fixer_const_pointer<S, HAS_MEMBER_CONST_POINTER>::const_pointer
                                                                    const_pointer;
    /// The difference type
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k ::stlsoft::typefixer::fixer_difference_type<S, HAS_MEMBER_DIFFERENCE_TYPE>::difference_type
                                                ,   ss_ptrdiff_t
                                                ,   HAS_MEMBER_DIFFERENCE_TYPE
                                                >::type             difference_type;
    /// The size type
    typedef ss_typename_type_k sequence_type::size_type             size_type;
};

/** \brief Partial specialisation for constant sequences
 *
 * \ingroup group__library__rangelib
 */
template<   ss_typename_param_k S
        >
struct sequence_range_traits<S, true>
{
public:
    /// The sequence type
    typedef S                                                       sequence_type;
    /// The sequence reference type
    typedef S&                                                      sequence_reference_type;
    /// The value type
    typedef ss_typename_type_k sequence_type::value_type            value_type;
    /// The mutating (non-const) iterator type
    typedef ss_typename_type_k sequence_type::const_iterator        iterator;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k sequence_type::const_iterator        const_iterator;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k sequence_type::const_reference       reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k sequence_type::const_reference       const_reference;
// TODO: Stick in the member-finder stuff here, so can assume ptrdiff_t if none found
    /// The difference type
    typedef ss_typename_type_k sequence_type::difference_type       difference_type;
    /// The size type
    typedef ss_typename_type_k sequence_type::size_type             size_type;
};

#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

template<   ss_typename_param_k S
        >
struct sequence_range_traits
{
public:
    typedef S                                                       sequence_type;
    typedef S&                                                      sequence_reference_type;
    typedef ss_typename_type_k sequence_type::value_type            value_type;
    typedef ss_typename_type_k sequence_type::iterator              iterator;
    typedef ss_typename_type_k sequence_type::const_iterator        const_iterator;
    typedef ss_typename_type_k sequence_type::reference             reference;
    typedef ss_typename_type_k sequence_type::const_reference       const_reference;
    typedef ss_typename_type_k sequence_type::difference_type       difference_type;
    typedef ss_typename_type_k sequence_type::size_type             size_type;
};

template<   ss_typename_param_k S
        >
struct const_sequence_range_traits
{
public:
    typedef S                                                       sequence_type;
    typedef S const&                                                sequence_reference_type;
    typedef ss_typename_type_k sequence_type::value_type            value_type;
    typedef ss_typename_type_k sequence_type::const_iterator        iterator;
    typedef ss_typename_type_k sequence_type::const_iterator        const_iterator;
    typedef ss_typename_type_k sequence_type::const_reference       reference;
    typedef ss_typename_type_k sequence_type::const_reference       const_reference;
    typedef ss_typename_type_k sequence_type::difference_type       difference_type;
    typedef ss_typename_type_k sequence_type::size_type             size_type;
};

template<   ss_typename_param_k S
        >
struct non_const_sequence_range_traits
{
public:
    typedef S                                                       sequence_type;
    typedef S const&                                                sequence_reference_type;
    typedef ss_typename_type_k sequence_type::value_type            value_type;
    typedef ss_typename_type_k sequence_type::iterator              iterator;
    typedef ss_typename_type_k sequence_type::iterator              const_iterator;
    typedef ss_typename_type_k sequence_type::reference             reference;
    typedef ss_typename_type_k sequence_type::reference             const_reference;
    typedef ss_typename_type_k sequence_type::difference_type       difference_type;
    typedef ss_typename_type_k sequence_type::size_type             size_type;
};
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

/** \brief This class adapts an STL sequence instance into a Range
 *
 * \ingroup group__library__rangelib
 *
 * \param S The sequence class
 * \param T The sequence range traits, used to deduce the Range's iterator, const_iterator, reference, const_reference and value_type
 *
 * It is categoried as an Iterable Range
 *
 * It could be used as follows
\code
  void dump_elements(std::vector<int> const& numbers)
  {
    for(sequence_range<std::vector<int> > r(numbers); r; ++r)
    {
      std::cout << &r; // Dump the current value to stdout
    }
  }
\endcode
 */
template<   ss_typename_param_k S
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
        ,   ss_typename_param_k T = sequence_range_traits<S, is_const_type<S>::value>   // Determines whether the sequence is const
#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
        ,   ss_typename_param_k T = const_sequence_range_traits<S>                      // Defaults to a const sequence
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
        >
class sequence_range
    : public mutating_operator_adaptor<sequence_range<S, T>, T> // This provides the operator forms of the methods
    , public iterable_range_tag
{
public:
    /// The sequence type
    typedef S                                                       sequence_type;
    /// The traits type
    typedef T                                                       traits_type;
    /// The range category tag type
    typedef iterable_range_tag                                      range_tag_type;
    /// The current instantiation of the type
    typedef sequence_range<S, T>                                    class_type;
    /// The sequence reference type
    typedef ss_typename_type_k traits_type::sequence_reference_type sequence_reference_type;
    /// The value type
    typedef ss_typename_type_k traits_type::value_type              value_type;
    /// The mutating (non-const) iterator type
    typedef ss_typename_type_k traits_type::iterator                iterator;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k traits_type::const_iterator          const_iterator;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k traits_type::reference               reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k traits_type::const_reference         const_reference;
    /// The difference type
    typedef ss_typename_type_k traits_type::difference_type         difference_type;
    /// The size type
    typedef ss_typename_type_k traits_type::size_type               size_type;

public:

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER == 1200)
    /// Constructor
    ///
    /// \param rhs The sequence which will be adapted to a range
    sequence_range(sequence_reference_type rhs) // The constness of this will require some thinking about. Maybe need sequence_range and const_sequence_range??
        : m_position(rhs.begin())
        , m_last(rhs.end())
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER != 1200)
    /// Constructor
    ///
    /// \param rhs The sequence which will be adapted to a range
    template <ss_typename_param_k S1>
    sequence_range(S1& rhs)
        : m_position(rhs.begin())
        , m_last(rhs.end())
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

#ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
    /// Constructs a range over an array
    template <ss_size_t N>
    sequence_range(S (&ar)[N])
        : m_position(&ar[0])
        , m_last(&ar[N])
    {}
#endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

    /// Constructs from an iterator
    ///
    /// \param position The start position of the range
    /// \param last The end position of the range
    sequence_range(iterator position, iterator last)
        : m_position(position)
        , m_last(last)
    {}

    /// Copy constructor
    ///
    /// \note This has to be provided, to avoid precipitating C4217 with Visual C++
    sequence_range(class_type const& rhs)
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
public:
    /// Indicates whether the range is open
    ss_bool_t is_open() const
    {
        return m_position != m_last;
    }
    /// Returns the current value in the range
    reference current()
    {
        STLSOFT_ASSERT(is_open());

        return *m_position;
    }
    /// Returns the current value in the range
    const_reference current() const
    {
        STLSOFT_ASSERT(is_open());

        return *m_position;
    }
    /// Advances the current position in the range
    class_type& advance()
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to increment the range past its end point", is_open());

        ++m_position;

        return *this;
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
// Functions

template<ss_typename_param_k S>
#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
inline sequence_range<S> make_sequence_range(S &s)
#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
inline sequence_range<S> make_sequence_range(S const& s)
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
{
    return sequence_range<S>(s);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/sequence_range_unittest_.h"
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

#endif /* !RANGELIB_INCL_RANGELIB_HPP_SEQUENCE_RANGE */

/* ///////////////////////////// end of file //////////////////////////// */
