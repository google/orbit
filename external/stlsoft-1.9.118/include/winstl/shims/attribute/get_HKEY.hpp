/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/attribute/get_HKEY.hpp
 *
 * Purpose:     get_HKEY attribute shim.
 *
 * Created:     1st June 2007
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


/** \file winstl/shims/attribute/get_HKEY.hpp
 *
 * \brief [C++ only] Definition of stlsoft::get_HKEY attribute shim
 *   functions for Win32 types
 *   (\ref group__library__shims__hkey_attribute "Window Attribute Shims").
 */

#ifndef WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HKEY
#define WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HKEY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_SHIMS_ATTRIBUTE_HPP_GET_HKEY_MAJOR     1
# define WINSTL_VER_SHIMS_ATTRIBUTE_HPP_GET_HKEY_MINOR     0
# define WINSTL_VER_SHIMS_ATTRIBUTE_HPP_GET_HKEY_REVISION  1
# define WINSTL_VER_SHIMS_ATTRIBUTE_HPP_GET_HKEY_EDIT      2
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
 * get_HKEY
 *
 * This can be applied to an expression, and the return value is the
 * corresponding HKEY.
 */

/* HKEY */

/** \brief Access the HKEY of the given HKEY
 *
 * \ingroup group__library__shims__hkey_attribute
 *
 * This access <a href = "http://stlsoft.org/white_papers.html#shims">shim</a>
 * retrieves the HKEY registry handle for the given HKEY handle.
 *
 * \param h A HKEY whose corresponding HKEY will be retrieved
 * \return The HKEY corresponding to the given HKEY \c h
 */
inline HKEY get_HKEY(HKEY h)
{
    return h;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/get_HKEY_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HKEY */

/* ///////////////////////////// end of file //////////////////////////// */
