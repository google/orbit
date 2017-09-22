/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/shims/access/string.hpp
 *
 * Purpose:     Contains classes and functions for dealing with OLE/COM strings.
 *
 * Created:     27th May 2002
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


/** \file atlstl/shims/access/string.hpp
 *
 * \brief [C++] Primary include file for string access shims representing
 *   ATL string types
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_HPP_STRING
#define ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_HPP_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_HPP_STRING_MAJOR      4
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_HPP_STRING_MINOR      0
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_HPP_STRING_REVISION   1
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_HPP_STRING_EDIT       96
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */
#ifndef ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CCOMBSTR
# include <atlstl/shims/access/string/ccombstr.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CCOMBSTR */
#ifdef __ATLWIN__
# ifndef ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW
#  include <atlstl/shims/access/string/cwindow.hpp>
# endif /* !ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW */
#endif /* __ATLWIN__ */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_HPP_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
