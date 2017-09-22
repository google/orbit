/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/operator_adaptors.hpp
 *
 * Purpose:     Definition of the mutating_operator_adaptor and
 *              non_mutating_operator_adaptor classes.
 *
 * Created:     4th November 2003
 * Updated:     10th August 2009
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


/** \file rangelib/operator_adaptors.hpp
 *
 * \brief [C++ only] Definition of the rangelib::mutating_operator_adaptor
 *   and rangelib::non_mutating_operator_adaptor classes, which are used to
 *   bolt in Range operators to classes that implement the range methods
 *   (\ref group__library__smart_pointers "Smart Pointers" Library).
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS
#define RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_OPERATOR_ADAPTORS_MAJOR       1
# define RANGELIB_VER_RANGELIB_HPP_OPERATOR_ADAPTORS_MINOR       5
# define RANGELIB_VER_RANGELIB_HPP_OPERATOR_ADAPTORS_REVISION    4
# define RANGELIB_VER_RANGELIB_HPP_OPERATOR_ADAPTORS_EDIT        30
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
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
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF
# include <stlsoft/meta/select_first_type_if.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF */
#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL
# include <stlsoft/util/operator_bool.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_INERT
# include <stlsoft/util/inert.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_INERT */

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

/** \brief This class facilitates specific definition of the \c const_iterator and
 * \c iterator member types
 *
 * \ingroup group__library__rangelib
 */
template<   ss_typename_param_k CR
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1300
        ,   ss_typename_param_k R   =   inert
#else /* ? compiler */
        ,   ss_typename_param_k R   =   void
#endif /* compiler */
        >
struct operator_adaptor_specific_traits
{
public:
    /// The non-mutating (const) reference type
    typedef CR              const_reference;
    /// The mutating (non-const) reference type
    typedef R               reference;
};


/** \brief This class is a reverse bolt-in, which provides mutating and
 * non-mutating Range operators based on the method forms of its
 * parameterising (and deriving) class
 *
 * \ingroup group__library__rangelib
 *
 * \note Because this is a reverse bolt-in, \c R is an incomplete type at the
 * time of the template parsing. Hence, we cannot deduce \c reference and
 * \c const_reference from \c R. This is why the traits type is required in
 * the form of the \c T parameter.
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k T
        >
class mutating_operator_adaptor
{
public:
    /// The range type
    typedef R                                               adapted_range_type;
    /// The traits type
    typedef T                                               traits_type;
    /// The type of this instantiation
    typedef mutating_operator_adaptor<R, T>                 class_type;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k traits_type::reference       reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k traits_type::const_reference const_reference;

protected:
    /// \brief Default constructor
    ///
    /// This is defined protected to ensure that the adaptor may only be used
    /// as a base class, and not instantiated directly
    mutating_operator_adaptor()
    {}

/// \name Range operators
/// @{
private:
    STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(class_type, boolean_generator_type, boolean_type);
public:
    /// Indicates whether the range is open
    operator boolean_type() const
    {
        return boolean_generator_type::translate(static_cast<adapted_range_type const*>(this)->is_open());
    }
    /// Returns the current value in the range
    reference operator *()
    {
        return static_cast<adapted_range_type*>(this)->current();
    }
    /// Returns the current value in the range
    const_reference operator *() const
    {
        return static_cast<adapted_range_type const*>(this)->current();
    }
    /// Advances the current position in the range
    adapted_range_type& operator ++()
    {
        return static_cast<adapted_range_type*>(this)->advance();
    }
    /// Advances the current position in the range, returning a copy of the
    /// range prior to its being advanced
    adapted_range_type operator ++(int)
    {
        adapted_range_type  ret(*static_cast<adapted_range_type const*>(this));

        operator ++();

        return ret;
    }
/// @}
};

/** \brief This class is a reverse bolt-in, which provides non-mutating Range
 * operators based on the method forms of its parameterising (and deriving)
 * class
 *
 * \ingroup group__library__rangelib
 *
 * \note Because this is a reverse bolt-in, \c R is an incomplete type at the
 * time of the template parsing. Hence, we cannot deduce \c reference and
 * \c const_reference from \c R. This is why the traits type is required in
 * the form of the \c T parameter.
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k T
        >
class non_mutating_operator_adaptor
{
public:
    /// The range type
    typedef R                                               adapted_range_type;
    /// The traits type
    typedef T                                               traits_type;
    /// The type of this instantiation
    typedef non_mutating_operator_adaptor<R, T>             class_type;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k traits_type::const_reference const_reference;

protected:
    /// \brief Default constructor
    ///
    /// This is defined protected to ensure that the adaptor may only be used
    /// as a base class, and not instantiated directly
    non_mutating_operator_adaptor()
    {}

/// \name Range operators
/// @{
private:
    STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(class_type, boolean_generator_type, boolean_type);
public:
    /// Indicates whether the range is open
    operator boolean_type() const
    {
        return boolean_generator_type::translate(static_cast<adapted_range_type const*>(this)->is_open());
    }
    /// Returns the current value in the range
    const_reference operator *() const
    {
        return static_cast<adapted_range_type const*>(this)->current();
    }
    /// Advances the current position in the range
    adapted_range_type& operator ++()
    {
        return static_cast<adapted_range_type*>(this)->advance();
    }
    /// Advances the current position in the range, returning a copy of the
    /// range prior to its being advanced
    adapted_range_type operator ++(int)
    {
        adapted_range_type  ret(*static_cast<adapted_range_type const*>(this));

        operator ++();

        return ret;
    }
/// @}
};

#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)

template<   ss_typename_param_k R
        ,   ss_typename_param_k T
        ,   ss_bool_t
        >
struct fixer_mutating_operator_adaptor
{
    typedef mutating_operator_adaptor<R, T>     type;
};

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// When not present it uses void as a placeholder
template<   ss_typename_param_k R
        ,   ss_typename_param_k T
        >
struct fixer_mutating_operator_adaptor<R, T, false>
{
    typedef void                                type;
};

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief This type is used to select the mutating or non-mutating form of the
 * operator adaptor
 *
 * \ingroup group__library__rangelib
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k T
        >
struct operator_adaptor_selector
{
private:
    enum { HAS_MEMBER_ITERATOR = 0 != member_traits<R>::has_member_iterator };

public:
    /// The mutating (non-const) operator adaptor type
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k fixer_mutating_operator_adaptor<R, T, HAS_MEMBER_ITERATOR>::type
                                                ,   non_mutating_operator_adaptor<R, T>
                                                ,   HAS_MEMBER_ITERATOR
                                                >::type             type;
};

#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/operator_adaptors_unittest_.h"
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

#endif /* !RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS */

/* ///////////////////////////// end of file //////////////////////////// */
