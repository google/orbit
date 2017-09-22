/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/printf_traits.hpp
 *
 * Purpose:     printf_traits classes.
 *
 * Created:     16th January 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/printf_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::printf_traits class
 *   template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_PRINTF_TRAITS
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_PRINTF_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_PRINTF_TRAITS_MAJOR    5
# define STLSOFT_VER_STLSOFT_UTIL_HPP_PRINTF_TRAITS_MINOR    0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_PRINTF_TRAITS_REVISION 1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_PRINTF_TRAITS_EDIT     60
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS
# include <stlsoft/util/integral_printf_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Constants & definitions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define STLSOFT_PRINTF_TRAITS_SINT8_MIN         STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MIN
#define STLSOFT_PRINTF_TRAITS_SINT8_MAX         STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MAX

#define STLSOFT_PRINTF_TRAITS_UINT8_MIN         STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MIN
#define STLSOFT_PRINTF_TRAITS_UINT8_MAX         STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MAX

#define STLSOFT_PRINTF_TRAITS_SINT16_MIN        STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN
#define STLSOFT_PRINTF_TRAITS_SINT16_MAX        STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX

#define STLSOFT_PRINTF_TRAITS_UINT16_MIN        STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN
#define STLSOFT_PRINTF_TRAITS_UINT16_MAX        STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX

#define STLSOFT_PRINTF_TRAITS_SINT32_MIN        STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MIN
#define STLSOFT_PRINTF_TRAITS_SINT32_MAX        STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MAX

#define STLSOFT_PRINTF_TRAITS_UINT32_MIN        STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MIN
#define STLSOFT_PRINTF_TRAITS_UINT32_MAX        STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MAX

#define STLSOFT_PRINTF_TRAITS_SINT64_MIN        STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN
#define STLSOFT_PRINTF_TRAITS_SINT64_MAX        STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX

#define STLSOFT_PRINTF_TRAITS_UINT64_MIN        STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN
#define STLSOFT_PRINTF_TRAITS_UINT64_MAX        STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define __STLSOFT_PRINTF_TRAITS__SINT8_MIN     STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MIN
# define __STLSOFT_PRINTF_TRAITS__SINT8_MAX     STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MAX

# define __STLSOFT_PRINTF_TRAITS__UINT8_MIN     STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MIN
# define __STLSOFT_PRINTF_TRAITS__UINT8_MAX     STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MAX

# define __STLSOFT_PRINTF_TRAITS__SINT16_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN
# define __STLSOFT_PRINTF_TRAITS__SINT16_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX

# define __STLSOFT_PRINTF_TRAITS__UINT16_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN
# define __STLSOFT_PRINTF_TRAITS__UINT16_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX

# define __STLSOFT_PRINTF_TRAITS__SINT32_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MIN
# define __STLSOFT_PRINTF_TRAITS__SINT32_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MAX

# define __STLSOFT_PRINTF_TRAITS__UINT32_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MIN
# define __STLSOFT_PRINTF_TRAITS__UINT32_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MAX

# define __STLSOFT_PRINTF_TRAITS__SINT64_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN
# define __STLSOFT_PRINTF_TRAITS__SINT64_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX

# define __STLSOFT_PRINTF_TRAITS__UINT64_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN
# define __STLSOFT_PRINTF_TRAITS__UINT64_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// struct printf_traits

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \brief Traits for determining the size, in printf-ed form, of the minimum and
 * maximum values of types
 *
 * \ingroup group__library__utility
 *
 * printf_traits is a traits class for acquiring enum values representing the
 * lengths, when expressed in string form, of the minimum and maximum values of
 * the type, and the maximum of the two. The lengths are inclusive of the
 * null terminator.
 *
\code
  assert(stlsoft::printf_traits<ss_sint16_t>::size_min == 6);
  assert(stlsoft::printf_traits<ss_sint16_t>::size_max == 7);
  assert(stlsoft::printf_traits<ss_sint16_t>::size == 7);
\endcode
 *
 * \param T The type
 *
 */
template <ss_typename_param_k T>
struct printf_traits
{
    enum
    {
            size_min    //!< The number of characters (& null) in the minimum value
        ,   size_max    //!< The number of characters (& null) in the maximum value
        ,   size        //!< The maximum of \c size_min and \c size_max
    };

    /// Returns the appropriate printf format for the type
    static ss_char_a_t const* format_a();
    /// Returns the appropriate wprintf format for the type
    static ss_char_w_t const* format_w();
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k T>
struct printf_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_sint8_t>
    : integral_printf_traits<ss_sint8_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_uint8_t>
    : integral_printf_traits<ss_uint8_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_sint16_t>
    : integral_printf_traits<ss_sint16_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_uint16_t>
    : integral_printf_traits<ss_uint16_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_sint32_t>
    : integral_printf_traits<ss_sint32_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_uint32_t>
    : integral_printf_traits<ss_uint32_t>
{};

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_sint64_t>
    : integral_printf_traits<ss_sint64_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<ss_uint64_t>
    : integral_printf_traits<ss_uint64_t>
{};
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */


#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<short>
    : integral_printf_traits<short>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<unsigned short>
    : integral_printf_traits<unsigned short>
{};
#endif // STLSOFT_CF_SHORT_DISTINCT_INT_TYPE

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<int>
    : integral_printf_traits<int>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<unsigned int>
    : integral_printf_traits<unsigned int>
{};
#endif // STLSOFT_CF_INT_DISTINCT_INT_TYPE

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<long>
    : integral_printf_traits<long>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct printf_traits<unsigned long>
    : integral_printf_traits<unsigned long>
{};
#endif // STLSOFT_CF_LONG_DISTINCT_INT_TYPE

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/printf_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_PRINTF_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
