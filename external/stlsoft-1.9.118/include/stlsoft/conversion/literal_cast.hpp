/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/literal_cast.hpp
 *
 * Purpose:     Compile-time checking for literal conversions.
 *
 * Created:     5th September 2003
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


/** \file stlsoft/conversion/literal_cast.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::literal_cast cast class
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_LITERAL_CAST
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_LITERAL_CAST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_LITERAL_CAST_MAJOR      4
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_LITERAL_CAST_MINOR      0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_LITERAL_CAST_REVISION   4
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_LITERAL_CAST_EDIT       44
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS
# include <stlsoft/util/limit_traits.h>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef ss_sint64_t         literal_cast_int_t;
typedef ss_uint64_t         invalid_int_t;
# else /* ? STLSOFT_CF_64BIT_INT_SUPPORT */
typedef ss_sint32_t         literal_cast_int_t;
typedef ss_uint32_t         invalid_int_t;
# endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#if 0
template<   ss_typename_param_k T
        ,   literal_cast_int_t  V
        >
inline T literal_cast()
{
#if defined(STLSOFT_CF_MEMBER_CONSTANT_SUPPORT)
    /* If member constants are supported, we can do compile-time assertions */
    const int __literal_cast_value_too_large__ = V <= limit_traits<T>::maximum_value;
    const int __literal_cast_value_too_small__ = V >= limit_traits<T>::minimum_value;

    STLSOFT_STATIC_ASSERT(__literal_cast_value_too_large__);
    STLSOFT_STATIC_ASSERT(__literal_cast_value_too_small__);
#else /* ? STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
    /* If not, then we are restricted to runtime assertions */
    const int __literal_cast_value_too_large__ = V <= limit_traits<T>::maximum();
    const int __literal_cast_value_too_small__ = V >= limit_traits<T>::minimum();

    STLSOFT_MESSAGE_ASSERT("Literal cast value is too large", __literal_cast_value_too_large__);
    STLSOFT_MESSAGE_ASSERT("Literal cast value is too small", __literal_cast_value_too_small__);
#endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */

    return T(V);
}
#else /* ? 0 */

/** \brief Provides compile-time validation for casts of literal values.
 *
 * \ingroup group__library__conversion
 *
 * \param T The type to cast to
 * \param V The literal value to be cast
 *
\code
short s1 = stlsoft::literal_cast<short, 10>();          // Ok: 10 fits into short
short s2 = stlsoft::literal_cast<short, 1000000000>();  // Compile error: value too large for short
\endcode
 *
 * \note Thanks to Kevlin Henney (http://www.curbralan.com/) for insights
 *  into the solution.
 */
template<   ss_typename_param_k T
        ,   literal_cast_int_t  V
        >
class literal_cast
{
public:
    operator T () const
    {
#if defined(STLSOFT_CF_MEMBER_CONSTANT_SUPPORT)
        /* If member constants are supported, we can do compile-time assertions */
        const int __literal_cast_value_too_large__ = V <= limit_traits<T>::maximum_value;
        const int __literal_cast_value_too_small__ = V >= limit_traits<T>::minimum_value;

        STLSOFT_STATIC_ASSERT(__literal_cast_value_too_large__);
        STLSOFT_STATIC_ASSERT(__literal_cast_value_too_small__);
#else /* ? STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
        /* If not, then we are restricted to runtime assertions */
        const int __literal_cast_value_too_large__ = V <= limit_traits<T>::maximum();
        const int __literal_cast_value_too_small__ = V >= limit_traits<T>::minimum();

        STLSOFT_MESSAGE_ASSERT("Literal cast value is too large", __literal_cast_value_too_large__);
        STLSOFT_MESSAGE_ASSERT("Literal cast value is too small", __literal_cast_value_too_small__);
#endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */

        return T(V);
    }
};


#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
/** Partial specialisation to 
 */
template<literal_cast_int_t V>
class literal_cast<invalid_int_t, V>
{
public:
# if 0
    literal_cast()
    {
        const int STLSOFT_LITERAL_CAST_INVALID_TYPE_WARNING =   0;

        STLSOFT_STATIC_ASSERT(STLSOFT_LITERAL_CAST_INVALID_TYPE_WARNING);
    }
# endif /* 0 */

private:
    operator invalid_int_t () const
    {
        const int __literal_cast_cannot_cast_to_largest_unsigned_integer_type__ =   0;

        STLSOFT_STATIC_ASSERT(__literal_cast_cannot_cast_to_largest_unsigned_integer_type__);

        return 0;
    }

# if defined(STLSOFT_COMPILER_IS_GCC)
public: static void f() {}
# endif /* compiler */
};
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#endif /* 0 */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/literal_cast_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_LITERAL_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
