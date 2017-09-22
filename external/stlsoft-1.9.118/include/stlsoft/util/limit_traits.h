/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/limit_traits.h (originally MLLimits.h; ::SynesisStd)
 *
 * Purpose:     limit_traits classes. Provides nothing that is not in
 *              std::numeric_limits, but uses minimum() and maximum() rather
 *              than min() and max(), since some compilers are not well-behaved
 *              in making these functions rather than macros.
 *
 * Created:     16th January 2002
 * Updated:     10th August 2009
 *
 * Thanks:      To Jonathan Wakely for help with Solaris compatibility.
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


/** \file stlsoft/util/limit_traits.h
 *
 * \brief [C, C++] Macros, constants and traits (stlsoft::limit_traits) for
 *  classes
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS
#define STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_H_LIMIT_TRAITS_MAJOR    4
# define STLSOFT_VER_STLSOFT_UTIL_H_LIMIT_TRAITS_MINOR    2
# define STLSOFT_VER_STLSOFT_UTIL_H_LIMIT_TRAITS_REVISION 6
# define STLSOFT_VER_STLSOFT_UTIL_H_LIMIT_TRAITS_EDIT     60
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#if defined(__cplusplus)
# ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_SIZE_TRAITS
#  include <stlsoft/util/size_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_SIZE_TRAITS */
#endif /* __cplusplus */

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

#define STLSOFT_GEN_SINT8_SUFFIX(i)             (i)
#define STLSOFT_GEN_UINT8_SUFFIX(i)             (i ## U)
#define STLSOFT_GEN_SINT16_SUFFIX(i)            (i)
#define STLSOFT_GEN_UINT16_SUFFIX(i)            (i ## U)
#if _STLSOFT_SIZEOF_LONG == 4
# define STLSOFT_GEN_SINT32_SUFFIX(i)           (i ## L)
# define STLSOFT_GEN_UINT32_SUFFIX(i)           (i ## UL)
#else /* ? _STLSOFT_SIZEOF_LONG */
# define STLSOFT_GEN_SINT32_SUFFIX(i)           (i)
# define STLSOFT_GEN_UINT32_SUFFIX(i)           (i ## U)
#endif /* _STLSOFT_SIZEOF_LONG */

#if (   (   defined(STLSOFT_COMPILER_IS_BORLAND) && \
            __BORLANDC__ >= 0x0582) || \
        defined(STLSOFT_COMPILER_IS_DMC) || \
        defined(STLSOFT_COMPILER_IS_COMO) || \
        defined(STLSOFT_COMPILER_IS_GCC) || \
        defined(STLSOFT_COMPILER_IS_MWERKS) || \
        defined(STLSOFT_COMPILER_IS_SUNPRO))
# define STLSOFT_GEN_SINT64_SUFFIX(i)           (i ## LL)
# define STLSOFT_GEN_UINT64_SUFFIX(i)           (i ## ULL)
#elif ( defined(STLSOFT_COMPILER_IS_BORLAND) || \
        defined(STLSOFT_COMPILER_IS_INTEL) || \
        defined(STLSOFT_COMPILER_IS_MSVC) || \
        defined(STLSOFT_COMPILER_IS_VECTORC) || \
        defined(STLSOFT_COMPILER_IS_WATCOM))
# define STLSOFT_GEN_SINT64_SUFFIX(i)           (i ## L)
# define STLSOFT_GEN_UINT64_SUFFIX(i)           (i ## UL)
#else
# error Compiler not discriminated
#endif /* compiler */


#define   STLSOFT_LIMIT_TRAITS__SINT8_MIN       (- STLSOFT_GEN_SINT8_SUFFIX(127) - 1)
#define   STLSOFT_LIMIT_TRAITS__SINT8_MAX       (+ STLSOFT_GEN_SINT8_SUFFIX(127))

#define   STLSOFT_LIMIT_TRAITS__UINT8_MIN       (  STLSOFT_GEN_UINT8_SUFFIX(0))
#define   STLSOFT_LIMIT_TRAITS__UINT8_MAX       (  STLSOFT_GEN_UINT8_SUFFIX(255))

#define   STLSOFT_LIMIT_TRAITS__SINT16_MIN      (- STLSOFT_GEN_SINT16_SUFFIX(32767) - 1)
#define   STLSOFT_LIMIT_TRAITS__SINT16_MAX      (+ STLSOFT_GEN_SINT16_SUFFIX(32767))

#define   STLSOFT_LIMIT_TRAITS__UINT16_MIN      (  STLSOFT_GEN_UINT16_SUFFIX(0))
#define   STLSOFT_LIMIT_TRAITS__UINT16_MAX      (  STLSOFT_GEN_UINT16_SUFFIX(65535))

#define   STLSOFT_LIMIT_TRAITS__SINT32_MIN      (- STLSOFT_GEN_SINT32_SUFFIX(2147483647) - 1)
#define   STLSOFT_LIMIT_TRAITS__SINT32_MAX      (+ STLSOFT_GEN_SINT32_SUFFIX(2147483647))

#define   STLSOFT_LIMIT_TRAITS__UINT32_MIN      (  STLSOFT_GEN_UINT32_SUFFIX(0))
#define   STLSOFT_LIMIT_TRAITS__UINT32_MAX      (  STLSOFT_GEN_UINT32_SUFFIX(4294967295))

#define   STLSOFT_LIMIT_TRAITS__SINT64_MIN      (- STLSOFT_GEN_SINT64_SUFFIX(9223372036854775807) - 1)
#define   STLSOFT_LIMIT_TRAITS__SINT64_MAX      (+ STLSOFT_GEN_SINT64_SUFFIX(9223372036854775807) )

#define   STLSOFT_LIMIT_TRAITS__UINT64_MIN      (  STLSOFT_GEN_UINT64_SUFFIX(0) )
#define   STLSOFT_LIMIT_TRAITS__UINT64_MAX      (  STLSOFT_GEN_UINT64_SUFFIX(18446744073709551615) )


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define __STLSOFT_GEN_SINT8_SUFFIX(i)          STLSOFT_GEN_SINT8_SUFFIX(i)
# define __STLSOFT_GEN_UINT8_SUFFIX(i)          STLSOFT_GEN_UINT8_SUFFIX(i)
# define __STLSOFT_GEN_SINT16_SUFFIX(i)         STLSOFT_GEN_SINT16_SUFFIX(i)
# define __STLSOFT_GEN_UINT16_SUFFIX(i)         STLSOFT_GEN_UINT16_SUFFIX(i)
# define __STLSOFT_GEN_SINT32_SUFFIX(i)         STLSOFT_GEN_SINT32_SUFFIX(i)
# define __STLSOFT_GEN_UINT32_SUFFIX(i)         STLSOFT_GEN_UINT32_SUFFIX(i)
# define __STLSOFT_GEN_SINT64_SUFFIX(i)         STLSOFT_GEN_SINT64_SUFFIX(i)
# define __STLSOFT_GEN_UINT64_SUFFIX(i)         STLSOFT_GEN_UINT64_SUFFIX(i)

# define __STLSOFT_LIMIT_TRAITS__SINT8_MIN      STLSOFT_LIMIT_TRAITS__SINT8_MIN
# define __STLSOFT_LIMIT_TRAITS__SINT8_MAX      STLSOFT_LIMIT_TRAITS__SINT8_MAX

# define __STLSOFT_LIMIT_TRAITS__UINT8_MIN      STLSOFT_LIMIT_TRAITS__UINT8_MIN
# define __STLSOFT_LIMIT_TRAITS__UINT8_MAX      STLSOFT_LIMIT_TRAITS__UINT8_MAX

# define __STLSOFT_LIMIT_TRAITS__SINT16_MIN     STLSOFT_LIMIT_TRAITS__SINT16_MIN
# define __STLSOFT_LIMIT_TRAITS__SINT16_MAX     STLSOFT_LIMIT_TRAITS__SINT16_MAX

# define __STLSOFT_LIMIT_TRAITS__UINT16_MIN     STLSOFT_LIMIT_TRAITS__UINT16_MIN
# define __STLSOFT_LIMIT_TRAITS__UINT16_MAX     STLSOFT_LIMIT_TRAITS__UINT16_MAX

# define __STLSOFT_LIMIT_TRAITS__SINT32_MIN     STLSOFT_LIMIT_TRAITS__SINT32_MIN
# define __STLSOFT_LIMIT_TRAITS__SINT32_MAX     STLSOFT_LIMIT_TRAITS__SINT32_MAX

# define __STLSOFT_LIMIT_TRAITS__UINT32_MIN     STLSOFT_LIMIT_TRAITS__UINT32_MIN
# define __STLSOFT_LIMIT_TRAITS__UINT32_MAX     STLSOFT_LIMIT_TRAITS__UINT32_MAX

# define __STLSOFT_LIMIT_TRAITS__SINT64_MIN     STLSOFT_LIMIT_TRAITS__SINT64_MIN
# define __STLSOFT_LIMIT_TRAITS__SINT64_MAX     STLSOFT_LIMIT_TRAITS__SINT64_MAX

# define __STLSOFT_LIMIT_TRAITS__UINT64_MIN     STLSOFT_LIMIT_TRAITS__UINT64_MIN
# define __STLSOFT_LIMIT_TRAITS__UINT64_MAX     STLSOFT_LIMIT_TRAITS__UINT64_MAX

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

#ifdef __cplusplus

# ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \brief Traits for determining the minimum and maximum values of types
 *
 * \ingroup group__library__utility
 *
 * limit_traits is a traits class for acquiring the minimum and maximum values
 * of types.
 *
\code
  assert(stlsoft::limit_traits<ss_sint16_t>::minimum() == -32768);
  assert(stlsoft::limit_traits<ss_sint16_t>::maximum() == 32767);
\endcode
 * \param T The type
 *
 * \note Provides nothing that is not in std::numeric_limits, but uses
 *   minimum() and maximum() rather than min() and max(), since some
 *   compilers are not well-behaved in making these functions rather than
 *   macros.
 *
 */
template <ss_typename_param_k T>
struct limit_traits
{
public:
    /** The value type */
    typedef T   value_type;

public:
    /** Returns the minimum value for the type */
    static value_type       minimum();
    /** Returns the maximum value for the type */
    static value_type       maximum();

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = ???;
    static const value_type maximum_value = ???;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

# else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* class limit_traits */

template <ss_typename_param_k T>
struct limit_traits;

#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_bool_t>
{
public:
    typedef ss_bool_t  value_type;

public:
    static value_type       minimum() { return false; }
    static value_type       maximum() { return true; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = false;
    static const value_type maximum_value = true;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */


# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
struct limit_traits_fixed;

/* s/uint8 */
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_sint8_t>
{
public:
    typedef ss_sint8_t  value_type;

public:
    static value_type       minimum() { return STLSOFT_LIMIT_TRAITS__SINT8_MIN; }
    static value_type       maximum() { return STLSOFT_LIMIT_TRAITS__SINT8_MAX; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__SINT8_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__SINT8_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_uint8_t>
{
public:
    typedef ss_uint8_t  value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__UINT8_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__UINT8_MAX; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__UINT8_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__UINT8_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

/* s/uint16 */
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_sint16_t>
{
public:
    typedef ss_sint16_t value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__SINT16_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__SINT16_MAX; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__SINT16_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__SINT16_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_uint16_t>
{
public:
    typedef ss_uint16_t value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__UINT16_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__UINT16_MAX; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__UINT16_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__UINT16_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

/* s/uint32 */
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_sint32_t>
{
public:
    typedef ss_sint32_t value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__SINT32_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__SINT32_MAX; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__SINT32_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__SINT32_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_uint32_t>
{
public:
    typedef ss_uint32_t value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__UINT32_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__UINT32_MAX; }

#  ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__UINT32_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__UINT32_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT

/* s/uint64 */
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_sint64_t>
{
public:
    typedef ss_sint64_t value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__SINT64_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__SINT64_MAX; }

#  if defined(STLSOFT_CF_MEMBER_CONSTANT_SUPPORT) && \
      !defined(STLSOFT_COMPILER_IS_BORLAND)
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__SINT64_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__SINT64_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits_fixed<ss_uint64_t>
{
public:
    typedef ss_uint64_t value_type;

public:
    static value_type   minimum() { return STLSOFT_LIMIT_TRAITS__UINT64_MIN; }
    static value_type   maximum() { return STLSOFT_LIMIT_TRAITS__UINT64_MAX; }

#  if defined(STLSOFT_CF_MEMBER_CONSTANT_SUPPORT) && \
      !defined(STLSOFT_COMPILER_IS_BORLAND)
    static const value_type minimum_value = STLSOFT_LIMIT_TRAITS__UINT64_MIN;
    static const value_type maximum_value = STLSOFT_LIMIT_TRAITS__UINT64_MAX;
#  endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */
};
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */


# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


#ifdef STLSOFT_CF_CHAR_DISTINCT_INT_TYPE

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<signed char>
    : limit_traits_fixed<ss_sint8_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<unsigned char>
    : limit_traits_fixed<ss_uint8_t>
{};

#endif /* STLSOFT_CF_CHAR_DISTINCT_INT_TYPE */


STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<char>
#ifdef STLSOFT_CF_CHAR_IS_UNSIGNED
    : limit_traits_fixed<ss_uint8_t>
#else /* ? STLSOFT_CF_CHAR_IS_UNSIGNED */
    : limit_traits_fixed<ss_sint8_t>
#endif /* STLSOFT_CF_CHAR_IS_UNSIGNED */
{};




STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_sint8_t>
    : limit_traits_fixed<ss_sint8_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_uint8_t>
    : limit_traits_fixed<ss_uint8_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_sint16_t>
    : limit_traits_fixed<ss_sint16_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_uint16_t>
    : limit_traits_fixed<ss_uint16_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_sint32_t>
    : limit_traits_fixed<ss_sint32_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_uint32_t>
    : limit_traits_fixed<ss_uint32_t>
{};

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_sint64_t>
    : limit_traits_fixed<ss_sint64_t>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<ss_uint64_t>
    : limit_traits_fixed<ss_uint64_t>
{};
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */


#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<short>
    : limit_traits_fixed<int_size_traits<sizeof(short)>::signed_type>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<unsigned short>
    : limit_traits_fixed<int_size_traits<sizeof(unsigned short)>::unsigned_type>
{};
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<int>
    : limit_traits_fixed<int_size_traits<sizeof(int)>::signed_type>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<unsigned int>
    : limit_traits_fixed<int_size_traits<sizeof(unsigned int)>::unsigned_type>
{};
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<long>
    : limit_traits_fixed<int_size_traits<sizeof(long)>::signed_type>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct limit_traits<unsigned long>
    : limit_traits_fixed<int_size_traits<sizeof(unsigned long)>::unsigned_type>
{};
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/limit_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} /* namespace stlsoft */
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
