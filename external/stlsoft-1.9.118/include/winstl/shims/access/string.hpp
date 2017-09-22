/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/access/string.hpp
 *
 * Purpose:     Contains classes and functions for dealing with Win32 strings.
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


/** \file winstl/shims/access/string.hpp
 *
 * \brief [C++] Primary include file for string access shims representing
 *   Windows string types
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
#define WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_HPP_STRING_MAJOR    4
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_HPP_STRING_MINOR    2
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_HPP_STRING_REVISION 1
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_HPP_STRING_EDIT     109
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#if !defined(NOUSER) && \
    !defined(NOWINOFFSETS)
# ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND
#  include <winstl/shims/access/string/hwnd.hpp>
# endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND */
#endif /* !NOUSER && !NOWINOFFSETS */
#ifdef _NTSECAPI_
# ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_LSA_UNICODE_STRING
#  include <winstl/shims/access/string/lsa_unicode_string.hpp>
# endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_LSA_UNICODE_STRING */
#endif /* _NTSECAPI_ */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME
# include <winstl/shims/access/string/time.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_TIME */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
