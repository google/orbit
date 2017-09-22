/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/conversion/to_SYSTEMTIME/UDATE.hpp
 *
 * Purpose:     winstl::to_SYSTEMTIME(UDATE const&) overload.
 *
 * Created:     15th January 2007
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


/** \file winstl/shims/conversion/to_SYSTEMTIME/UDATE.hpp
 *
 * \brief [C++] Definition of the winstl::to_SYSTEMTIME(UDATE const&)
 *   overload
 *   (\ref group__concept__shim__time_conversion__to_SYSTEMTIME "winstl::to_SYSTEMTIME" Time Conversion Shim).
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE
#define WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE_MAJOR       1
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE_MINOR       0
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE_REVISION    3
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE_EDIT        7
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_UDATE_DEFINED
# error UDATE is not defined. If you are certain that your compiler's Windows header files define this type, #define the symbol WINSTL_FORCE_UDATE
#endif /* !WINSTL_UDATE_DEFINED */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef WINSTL_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR
#  include <winstl/error/conversion_error.hpp>
# endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR */
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
# include <winstl/util/struct_initialisers.hpp>
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifdef WINSTL_UDATE_DEFINED
# include <objbase.h>
# include <oleauto.h>
#endif /* WINSTL_UDATE_DEFINED */

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
 * \param rhs An instance of type <code>UDATE</code>
 * \return An instance of type <code>SYSTEMTIME</code>
 *
 * \exception - No error checking is performed, and no exception
 *   can be thrown.
 */
inline SYSTEMTIME const& to_SYSTEMTIME(UDATE const& rhs)
{
    return rhs.st;
}

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

#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE */

/* ///////////////////////////// end of file //////////////////////////// */
