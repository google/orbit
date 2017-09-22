/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/shims/logical/is_empty/cstring.hpp
 *
 * Purpose:     Helper functions for CString class.
 *
 * Created:     18th December 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file mfcstl/shims/logical/is_empty/cstring.hpp
 *
 * \brief [C++] Primary include file for is_empty attribute shims
 *   for <code>CString</code>
 *   (\ref group__concept__shim__collection_logical__is_empty "is_empty Collection Logical Shim").
 */

#ifndef MFCSTL_INCL_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING
#define MFCSTL_INCL_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING_MAJOR     2
# define MFCSTL_VER_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING_MINOR     0
# define MFCSTL_VER_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING_REVISION  2
# define MFCSTL_VER_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING_EDIT      8
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */

#ifndef STLSOFT_INCL_H_AFX
# define STLSOFT_INCL_H_AFX
# include <afx.h>
#endif /* !STLSOFT_INCL_H_AFX */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPMFC
# if defined(_STLSOFT_NO_NAMESPMFC) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
/* Define stlsoft::mfcstl_project */

namespace stlsoft
{

namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPMFC */
#endif /* !_MFCSTL_NO_NAMESPMFC */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** \brief Indicates whether the string is empty
 *
 * \ingroup group__concept__shim__collection_logical__is_empty
 *
 */
inline BOOL is_empty(CString const& s)
{
    return s.IsEmpty();
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/cstring_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPMFC
# if defined(_STLSOFT_NO_NAMESPMFC) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPMFC */

namespace stlsoft
{

    using ::mfcstl::is_empty;

} // namespace stlsoft

#endif /* !_MFCSTL_NO_NAMESPMFC */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* MFCSTL_INCL_MFCSTL_SHIMS_LOGICAL_IS_EMPTY_HPP_CSTRING */

/* ///////////////////////////// end of file //////////////////////////// */
