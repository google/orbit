/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/conversion/to_SYSTEMTIME/SYSTEMTIME.hpp
 *
 * Purpose:     winstl::to_SYSTEMTIME(SYSTEMTIME const&) overload.
 *
 * Created:     24th May 2002
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


/** \file winstl/shims/conversion/to_SYSTEMTIME/SYSTEMTIME.hpp
 *
 * \brief [C++] Definition of the winstl::to_SYSTEMTIME(SYSTEMTIME const&)
 *   overload
 *   (\ref group__concept__shim__time_conversion__to_SYSTEMTIME "winstl::to_SYSTEMTIME" Time Conversion Shim).
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME
#define WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME_MAJOR      1
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME_MINOR      0
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME_REVISION   1
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME_EDIT       7
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** \brief Degenerate case of the \ref group__concept__shim__time_conversion__to_SYSTEMTIME.
 *
 * \ingroup group__concept__shim__time_conversion__to_SYSTEMTIME
 *
 * \param rhs An instance of type <code>SYSTEMTIME</code>
 * \return An instance of type <code>SYSTEMTIME</code>
 *
 * \exception - No error checking is performed, and no exception
 *   can be thrown.
 */
inline SYSTEMTIME const& to_SYSTEMTIME(SYSTEMTIME const& rhs)
{
    return rhs;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/SYSTEMTIME_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace stlsoft::winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME */

/* ///////////////////////////// end of file //////////////////////////// */
