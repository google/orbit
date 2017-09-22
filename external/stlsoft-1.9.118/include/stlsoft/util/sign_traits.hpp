/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/sign_traits.hpp
 *
 * Purpose:     sign_traits classes.
 *
 * Created:     16th January 2002
 * Updated:     31st July 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2012, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/sign_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::sign_traits traits class
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_SIGN_TRAITS_MAJOR      4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_SIGN_TRAITS_MINOR      1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_SIGN_TRAITS_REVISION   3
# define STLSOFT_VER_STLSOFT_UTIL_HPP_SIGN_TRAITS_EDIT       47
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_SIZE_TRAITS
# include <stlsoft/util/size_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_SIZE_TRAITS */

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

// struct sign_traits

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \brief Traits for determining the signed, unsigned and alternate-signed type
 *
 * \ingroup group__library__utility
 *
 * sign_traits is a traits class for acquiring the corresponding signed,
 * unsigned, and alternate-signed type eg.
 *
\code
  assert(stlsoft::is_same_type<stlsoft::sign_traits<ss_sint16_t>::signed_type, ss_sint16_t>::value);
  assert(stlsoft::is_same_type<stlsoft::sign_traits<ss_sint16_t>::unsigned_type, ss_uint16_t>::value);
  assert(stlsoft::is_same_type<stlsoft::sign_traits<ss_sint16_t>::alt_sign_type, ss_uint16_t>::value);
\endcode
 *
 * \param T The char type
 *
 */
template <ss_typename_param_k T>
struct sign_traits
{
    enum
    {
        bytes   =   0                   //!< The type size, in bytes
    };
    enum
    {
        bits    =   0                   //!< The type size, in bits
    };

    typedef T           type;
    typedef signed T    signed_type;    //!< The signed type
    typedef unsigned T  unsigned_type;  //!< The unsigned type
    typedef unsigned T  alt_sign_type;  //!< The alternate-signed type
};

#else

template <ss_typename_param_k T>
struct sign_traits;

/* char */
STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_char_a_t>
{
    enum { bytes    =   1                       };
    enum { bits     =   8 * bytes               };

    typedef ss_char_a_t type;
    typedef ss_uint8_t  signed_type;
    typedef ss_uint8_t  unsigned_type;
//    typedef ss_uint8_t  alt_sign_type;
};

#ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_char_w_t>
{
    enum { bytes    =   int(sizeof(ss_char_w_t))     };
    enum { bits     =   8 * bytes               };

    typedef ss_char_w_t                             type;
    typedef int_size_traits<bytes>::signed_type     signed_type;
    typedef int_size_traits<bytes>::unsigned_type   unsigned_type;
};
#endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

/* s/uint8 */
STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_sint8_t>
{
    enum { bytes    =   1                       };
    enum { bits     =   8 * bytes               };

    typedef ss_sint8_t  type;
    typedef ss_sint8_t  signed_type;
    typedef ss_uint8_t  unsigned_type;
    typedef ss_uint8_t  alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_uint8_t>
{
    enum { bytes    =   1                       };
    enum { bits     =   8 * bytes               };

    typedef ss_uint8_t  type;
    typedef ss_sint8_t  signed_type;
    typedef ss_uint8_t  unsigned_type;
    typedef ss_sint8_t  alt_sign_type;
};

/* s/uint16 */
STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_sint16_t>
{
    enum { bytes    =   2                       };
    enum { bits     =   8 * bytes               };

    typedef ss_sint16_t type;
    typedef ss_sint16_t signed_type;
    typedef ss_uint16_t unsigned_type;
    typedef ss_uint16_t alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_uint16_t>
{
    enum { bytes    =   2                       };
    enum { bits     =   8 * bytes               };

    typedef ss_uint16_t type;
    typedef ss_sint16_t signed_type;
    typedef ss_uint16_t unsigned_type;
    typedef ss_sint16_t alt_sign_type;
};

/* s/uint32 */
STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_sint32_t>
{
    enum { bytes    =   4                       };
    enum { bits     =   8 * bytes               };

    typedef ss_sint32_t type;
    typedef ss_sint32_t signed_type;
    typedef ss_uint32_t unsigned_type;
    typedef ss_uint32_t alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_uint32_t>
{
    enum { bytes    =   4                       };
    enum { bits     =   8 * bytes               };

    typedef ss_uint32_t type;
    typedef ss_sint32_t signed_type;
    typedef ss_uint32_t unsigned_type;
    typedef ss_sint32_t alt_sign_type;
};

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
/* s/uint64 */
STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_sint64_t>
{
    enum { bytes    =   8                       };
    enum { bits     =   8 * bytes               };

    typedef ss_sint64_t type;
    typedef ss_sint64_t signed_type;
    typedef ss_uint64_t unsigned_type;
    typedef ss_uint64_t alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<ss_uint64_t>
{
    enum { bytes    =   8                       };
    enum { bits     =   8 * bytes               };

    typedef ss_uint64_t type;
    typedef ss_sint64_t signed_type;
    typedef ss_uint64_t unsigned_type;
    typedef ss_sint64_t alt_sign_type;
};
#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */

#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<signed short>
{
    enum { bytes    =   sizeof(signed short)    };
    enum { bits     =   8 * bytes               };

    typedef signed short    type;
    typedef signed short    signed_type;
    typedef unsigned short  unsigned_type;
    typedef unsigned short  alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<unsigned short>
{
    enum { bytes    =   sizeof(unsigned short)  };
    enum { bits     =   8 * bytes               };

    typedef unsigned short  type;
    typedef signed short    signed_type;
    typedef unsigned short  unsigned_type;
    typedef signed short    alt_sign_type;
};

#endif /* !STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<int>
{
    enum { bytes    =   sizeof(int)             };
    enum { bits     =   8 * bytes               };

    typedef int         type;
    typedef int         signed_type;
    typedef unsigned    unsigned_type;
    typedef unsigned    alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<unsigned>
{
    enum { bytes    =   sizeof(unsigned)        };
    enum { bits     =   8 * bytes               };

    typedef unsigned    type;
    typedef int         signed_type;
    typedef unsigned    unsigned_type;
    typedef int         alt_sign_type;
};

#endif /* !STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<long>
{
    enum { bytes    =   int(sizeof(long))            };
    enum { bits     =   8 * bytes               };

    typedef long            type;
    typedef long            signed_type;
    typedef unsigned long   unsigned_type;
    typedef unsigned long   alt_sign_type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct sign_traits<unsigned long>
{
    enum { bytes    =   int(sizeof(unsigned long))   };
    enum { bits     =   8 * bytes               };

    typedef unsigned long   type;
    typedef long            signed_type;
    typedef unsigned long   unsigned_type;
    typedef long            alt_sign_type;
};

#endif /* !STLSOFT_CF_LONG_DISTINCT_INT_TYPE */


#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
