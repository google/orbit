/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/conversion/to_uint64/std/fundamental.hpp
 *
 * Purpose:     Contains the to_uint64 access shim.
 *
 * Created:     4th July 2007
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2007-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/shims/conversion/to_uint64/std/fundamental.hpp
 *
 * \brief [C++] Integer conversion shims for built-in types
 *   (\ref group__concept__shim__integer_conversion__to_uint64 "to_uint64 Integer Attribute Shim").
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL
#define STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL_MAJOR       1
# define STLSOFT_VER_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL_MINOR       0
# define STLSOFT_VER_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL_REVISION    1
# define STLSOFT_VER_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL_EDIT        6
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_HPP_DEGENERATE
# include <stlsoft/shims/conversion/to_uint64/degenerate.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_HPP_DEGENERATE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * to_uint64 functions
 */

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER != 1200
/** <code>uint8_t</code> overload of the <b>stlsoft::to_uint64</b> shim
 * \ingroup group__concept__shim__integer_conversion__to_uint64
 */
inline ss_uint64_t to_uint64(ss_uint8_t i)
{
    return i;
}

/** <code>uint16_t</code> overload of the <b>stlsoft::to_uint64</b> shim
 * \ingroup group__concept__shim__integer_conversion__to_uint64
 */
inline ss_uint64_t to_uint64(ss_uint16_t i)
{
    return i;
}

/** <code>uint32_t</code> overload of the <b>stlsoft::to_uint64</b> shim
 * \ingroup group__concept__shim__integer_conversion__to_uint64
 */
inline ss_uint64_t to_uint64(ss_uint32_t i)
{
    return i;
}
#endif /* VC++ 6.0  */

/** Degenerate form of the <b>stlsoft::to_uint64</b> shim
 *
 * \ingroup group__concept__shim__integer_conversion__to_uint64
 */
inline ss_uint64_t to_uint64(ss_uint64_t i)
{
    return i;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/fundamental_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_STD_HPP_FUNDAMENTAL */

/* ///////////////////////////// end of file //////////////////////////// */
