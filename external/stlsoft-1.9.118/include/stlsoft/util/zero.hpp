/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/zero.hpp
 *
 * Purpose:     ZERO_v template class.
 *
 * Created:     29th July 2003
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


/** \file stlsoft/util/zero.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::ZERO_v class
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_ZERO
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_ZERO

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_ZERO_MAJOR       4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_ZERO_MINOR       1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_ZERO_REVISION    1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_ZERO_EDIT        45
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

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

/** \brief Represents a type that can be an active replacement for ZERO
 *
 * \ingroup group__library__utility
 *
 * This class can act as a replacement for the ZERO macro, by being validly
 * assigned to or equated with pointer types only, as in
 *
 *   int   i = ZERO; // error
 *   int   *p = ZERO; // OK
 *
 *   if(i == ZERO) {} // error
 *   if(ZERO == i) {} // error
 *
 *   if(p == ZERO) {} // OK
 *   if(ZERO == p) {} // OK
 *
 *
 * When used via inclusion of the file stlsoft_zerodef.h, the macro ZERO is
 * redefined as ZERO_v(), such that expressions containing ZERO will be valid
 * against pointers only.
 */
struct ZERO_v
{
// Construction
public:
    /// Default constructor
    ZERO_v()
    {}

/** \brief Static creation
 *
 * \ingroup group__library__utility
 */
public:
    static ZERO_v create()
    {
        return ZERO_v();
    }

// Conversion
public:
    /// A zero (of type ss_sint8_t)
    operator ss_sint8_t () const
    {
        return 0;
    }
    /// A zero (of type ss_uint8_t)
    operator ss_uint8_t () const
    {
        return 0;
    }
    /// A zero (of type ss_sint16_t)
    operator ss_sint16_t () const
    {
        return 0;
    }
    /// A zero (of type ss_uint16_t)
    operator ss_uint16_t () const
    {
        return 0;
    }
    /// A zero (of type ss_sint32_t)
    operator ss_sint32_t () const
    {
        return 0;
    }
    /// A zero (of type ss_uint32_t)
    operator ss_uint32_t () const
    {
        return 0;
    }
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
    /// A zero (of type ss_sint64_t)
    operator ss_sint64_t () const
    {
        return 0;
    }
    /// A zero (of type ss_uint64_t)
    operator ss_uint64_t () const
    {
        return 0;
    }
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
    /// A zero (of type signed int)
    operator signed int () const
    {
        return 0;
    }
    /// A zero (of type unsigned int)
    operator unsigned int () const
    {
        return 0;
    }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
    /// A zero (of type signed long)
    operator signed long () const
    {
        return 0;
    }
    /// A zero (of type unsigned long)
    operator unsigned long () const
    {
        return 0;
    }
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

    /// A zero (of type float)
    operator float () const
    {
        return 0.0f;
    }
    /// A zero (of type double)
    operator double () const
    {
        return 0.0;
    }
    /// A zero (of type long double)
    operator long double () const
    {
        return 0.0;
    }

// Not to be implemented
private:
    void operator &() const;

    ZERO_v(ZERO_v const&);
    ZERO_v const& operator =(ZERO_v const&);
};

/** \brief operator == for ZERO_v and integral types
 *
 * \ingroup group__library__utility
 */
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_sint8_t i)         { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_uint8_t i)         { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_sint16_t i)        { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_uint16_t i)        { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_sint32_t i)        { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_uint32_t i)        { return i == 0; }
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_sint64_t const& i) { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, ss_uint64_t const& i) { return i == 0; }
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, signed int i)         { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, unsigned int i)       { return i == 0; }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, signed long i)        { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, unsigned long i)      { return i == 0; }
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, float const& i)       { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, double const& i)      { return i == 0; }
inline ss_bool_t operator ==(ZERO_v const& /* lhs */, long double const& i) { return i == 0; }

/** \brief operator == for an integral type and ZERO_v
 *
 * \ingroup group__library__utility
 */
inline ss_bool_t operator ==(ss_sint8_t i, ZERO_v const& /* lhs */)         { return i == 0; }
inline ss_bool_t operator ==(ss_uint8_t i, ZERO_v const& /* lhs */)         { return i == 0; }
inline ss_bool_t operator ==(ss_sint16_t i, ZERO_v const& /* lhs */)        { return i == 0; }
inline ss_bool_t operator ==(ss_uint16_t i, ZERO_v const& /* lhs */)        { return i == 0; }
inline ss_bool_t operator ==(ss_sint32_t i, ZERO_v const& /* lhs */)        { return i == 0; }
inline ss_bool_t operator ==(ss_uint32_t i, ZERO_v const& /* lhs */)        { return i == 0; }
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
inline ss_bool_t operator ==(ss_sint64_t const& i, ZERO_v const& /* lhs */) { return i == 0; }
inline ss_bool_t operator ==(ss_uint64_t const& i, ZERO_v const& /* lhs */) { return i == 0; }
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
inline ss_bool_t operator ==(signed int i, ZERO_v const& /* lhs */)         { return i == 0; }
inline ss_bool_t operator ==(unsigned int i, ZERO_v const& /* lhs */)       { return i == 0; }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
inline ss_bool_t operator ==(signed long i, ZERO_v const& /* lhs */)        { return i == 0; }
inline ss_bool_t operator ==(unsigned long i,ZERO_v const& /* lhs */)       { return i == 0; }
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */
inline ss_bool_t operator ==(float const& i, ZERO_v const& /* lhs */)       { return i == 0; }
inline ss_bool_t operator ==(double const& i, ZERO_v const& /* lhs */)      { return i == 0; }
inline ss_bool_t operator ==(long double const& i, ZERO_v const& /* lhs */) { return i == 0; }

/** \brief operator != for ZERO_v and an integral type
 *
 * \ingroup group__library__utility
 */
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_sint8_t i)         { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_uint8_t i)         { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_sint16_t i)        { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_uint16_t i)        { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_sint32_t i)        { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_uint32_t i)        { return i != 0; }
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_sint64_t const& i) { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, ss_uint64_t const& i) { return i != 0; }
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, signed int i)         { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, unsigned int i)       { return i != 0; }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, signed long i)        { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, unsigned long i)      { return i != 0; }
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, float const& i)       { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, double const& i)      { return i != 0; }
inline ss_bool_t operator !=(ZERO_v const& /* lhs */, long double const& i) { return i != 0; }

/** \brief operator != for an integral type and ZERO_v
 *
 * \ingroup group__library__utility
 */
inline ss_bool_t operator !=(ss_sint8_t i, ZERO_v const& /* lhs */)         { return i != 0; }
inline ss_bool_t operator !=(ss_uint8_t i, ZERO_v const& /* lhs */)         { return i != 0; }
inline ss_bool_t operator !=(ss_sint16_t i, ZERO_v const& /* lhs */)        { return i != 0; }
inline ss_bool_t operator !=(ss_uint16_t i, ZERO_v const& /* lhs */)        { return i != 0; }
inline ss_bool_t operator !=(ss_sint32_t i, ZERO_v const& /* lhs */)        { return i != 0; }
inline ss_bool_t operator !=(ss_uint32_t i, ZERO_v const& /* lhs */)        { return i != 0; }
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
inline ss_bool_t operator !=(ss_sint64_t const& i, ZERO_v const& /* lhs */) { return i != 0; }
inline ss_bool_t operator !=(ss_uint64_t const& i, ZERO_v const& /* lhs */) { return i != 0; }
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
inline ss_bool_t operator !=(signed int i, ZERO_v const& /* lhs */)         { return i != 0; }
inline ss_bool_t operator !=(unsigned int i, ZERO_v const& /* lhs */)       { return i != 0; }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
inline ss_bool_t operator !=(signed long i, ZERO_v const& /* lhs */)        { return i != 0; }
inline ss_bool_t operator !=(unsigned long i,ZERO_v const& /* lhs */)       { return i != 0; }
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */
inline ss_bool_t operator !=(float const& i, ZERO_v const& /* lhs */)       { return i != 0; }
inline ss_bool_t operator !=(double const& i, ZERO_v const& /* lhs */)      { return i != 0; }
inline ss_bool_t operator !=(long double const& i, ZERO_v const& /* lhs */) { return i != 0; }

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/zero_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_ZERO */

/* ///////////////////////////// end of file //////////////////////////// */
