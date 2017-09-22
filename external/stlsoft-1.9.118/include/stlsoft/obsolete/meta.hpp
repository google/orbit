/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/meta.hpp (originally MTBase.h, ::SynesisStl)
 *
 * Purpose:     Meta programming primitives.
 *
 * Created:     19th November 1998
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/obsolete/meta.hpp
 *
 * \brief [C++ only; OBSOLETE] Contains obsolete Meta programming primitives
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_META
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_META

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_META_MAJOR    4
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_META_MINOR    0
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_META_REVISION 3
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_META_EDIT     133
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
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
# error stlsoft/obsolete/meta.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_
# include <stlsoft/meta/util/meta_.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_ */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_ARRAY_TYPE
# include <stlsoft/meta/is_array_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_ARRAY_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_BOOL_TYPE
# include <stlsoft/meta/is_bool_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_BOOL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE
# include <stlsoft/meta/is_character_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CLASS_TYPE
# include <stlsoft/meta/is_class_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CLASS_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_COMPOUND_TYPE
# include <stlsoft/meta/is_compound_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_COMPOUND_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE
# include <stlsoft/meta/is_const_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FLOATING_POINT_TYPE
# include <stlsoft/meta/is_floating_point_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FLOATING_POINT_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE
# include <stlsoft/meta/is_function_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE
# include <stlsoft/meta/is_fundamental_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
# include <stlsoft/meta/is_integral_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_NUMERIC_TYPE
# include <stlsoft/meta/is_numeric_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_NUMERIC_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
# include <stlsoft/meta/is_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE
# include <stlsoft/meta/is_same_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SIGNED_TYPE
# include <stlsoft/meta/is_signed_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SIGNED_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_VOID_TYPE
# include <stlsoft/meta/is_void_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_VOID_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_VOLATILE_TYPE
# include <stlsoft/meta/is_volatile_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_VOLATILE_TYPE */

#ifdef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_ITERATOR
#  include <stlsoft/meta/detector/has_const_iterator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_ITERATOR */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_POINTER
#  include <stlsoft/meta/detector/has_const_pointer.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_POINTER */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_REFERENCE
#  include <stlsoft/meta/detector/has_const_reference.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_CONST_REFERENCE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_DIFFERENCE_TYPE
#  include <stlsoft/meta/detector/has_difference_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_DIFFERENCE_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_DISTANCE_TYPE
#  include <stlsoft/meta/detector/has_distance_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_DISTANCE_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_ITERATOR
#  include <stlsoft/meta/detector/has_iterator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_ITERATOR */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_ITERATOR_CATEGORY
#  include <stlsoft/meta/detector/has_iterator_category.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_ITERATOR_CATEGORY */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_KEY_TYPE
#  include <stlsoft/meta/detector/has_key_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_KEY_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_MAPPED_TYPE
#  include <stlsoft/meta/detector/has_mapped_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_MAPPED_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_POINTER
#  include <stlsoft/meta/detector/has_pointer.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_POINTER */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_POINTER_TYPE
#  include <stlsoft/meta/detector/has_pointer_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_POINTER_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_REFERENCE
#  include <stlsoft/meta/detector/has_reference.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_REFERENCE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_REFERENCE_TYPE
#  include <stlsoft/meta/detector/has_reference_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_REFERENCE_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_REFERENT_TYPE
#  include <stlsoft/meta/detector/has_referent_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_REFERENT_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_VALUE_TYPE
#  include <stlsoft/meta/detector/has_value_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_DETECTOR_HPP_HAS_VALUE_TYPE */
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES
# include <stlsoft/meta/n_types.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF
# include <stlsoft/meta/select_first_type_if.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_SIZEOF
# include <stlsoft/meta/size_of.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SIZEOF */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Tests
 */

// is_convertible_to_pointer
//
/** \brief This test determines whether the given type is convertible to a pointer
 * type
 *
 * \ingroup group__library__meta
 *
 * \deprecated
 *
 */
template <ss_typename_param_k T>
struct is_convertible_to_pointer
{
};

// is_convertible_to_bool
//

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
struct convertible_index
{
//  typedef <size_type>     type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct convertible_index<int>
{
    typedef size_type<2>    type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct convertible_index<unsigned>
{
    typedef size_type<3>    type;
};

#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
STLSOFT_TEMPLATE_SPECIALISATION
struct convertible_index<bool>
{
    typedef size_type<4>    type;
};
#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */

STLSOFT_TEMPLATE_SPECIALISATION
struct convertible_index<void*>
{
    typedef size_type<5>    type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct convertible_index<long double>
{
    typedef size_type<6>    type;
};


size_type<1>                            convertible_index_function(...);
//convertible_index<int>::type          convertible_index_function(int );
//convertible_index<unsigned>::type     convertible_index_function(unsigned );
# ifdef STLSOFT_COMPILER_IS_MSVC
convertible_index<int>::type            convertible_index_function(int );
convertible_index<int>::type            convertible_index_function(unsigned int );
convertible_index<int>::type            convertible_index_function(long );
convertible_index<int>::type            convertible_index_function(unsigned long );
convertible_index<long double>::type    convertible_index_function(double );
convertible_index<long double>::type    convertible_index_function(long double );
# endif /* compiler */
# ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
convertible_index<bool>::type           convertible_index_function(bool );
# endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */
convertible_index<void*>::type          convertible_index_function(void const volatile* );

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief This test determines whether the given type is convertible to a pointer
 * type
 *
 * \ingroup group__library__meta
 *
 * \deprecated
 */
template <ss_typename_param_k T>
struct is_convertible_to_bool
{
    enum { value = sizeof(convertible_index_function(*static_cast<T*>(0))) == sizeof(convertible_index<bool>::type) };
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
struct is_convertible_to_bool<void>
{
    enum { value = 0 };
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief Obsolete form of stlsoft::is_const_type
 *
 * \ingroup group__library__meta
 *
 * \deprecated Use stlsoft::is_const_type instead.
 */
template <ss_typename_param_k T>
struct is_const
{
    enum { value = is_const_type<T>::value };
};


/** \brief Obsolete form of stlsoft::is_volatile_type
 *
 * \ingroup group__library__meta
 *
 * \deprecated Use stlsoft::is_volatile_type instead.
 */
template <ss_typename_param_k T>
struct is_volatile
{
    enum { value = is_volatile_type<T>::value };
};



/** \brief Obsolete form of \link stlsoft::is_void_type is_void_type\endlink.
 *
 * \ingroup group__library__meta
 *
 * \deprecated Use \link stlsoft::is_void_type is_void_type\endlink instead.
 */
template <ss_typename_param_k T>
struct is_void
{
    enum { value = is_void_type<T>::value };
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} /* namespace stlsoft */
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_META */

/* ///////////////////////////// end of file //////////////////////////// */
