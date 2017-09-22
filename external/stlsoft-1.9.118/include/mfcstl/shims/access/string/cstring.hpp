/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/shims/access/string/cstring.hpp
 *
 * Purpose:     Contains classes and functions for dealing with MFC strings.
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


/** \file mfcstl/shims/access/string/cstring.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>CString</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef MFCSTL_INCL_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING
#define MFCSTL_INCL_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING_MAJOR    4
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING_MINOR    0
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING_REVISION 1
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING_EDIT     89
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
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

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* CString */
/** \brief \ref group__concept__shim__string_access__c_str_data for CString
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline LPCTSTR c_str_data(CString const& s)
{
    /* CString always points to valid memory, whether its own
     * CStringData or afxEmptyString.m_pchData
     */
    return s;
}
#if defined(UNICODE)
inline LPCTSTR c_str_data_w(CString const& s)
#else /* ? UNICODE */
inline LPCTSTR c_str_data_a(CString const& s)
#endif /* UNICODE */
{
    return c_str_data(s);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/* CString */
/** \brief \ref group__concept__shim__string_access__c_str_len for CString
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ms_size_t c_str_len(CString const& s)
{
    return s.GetLength();
}

#if defined(UNICODE)
inline ms_size_t c_str_len_w(CString const& s)
#else /* ? UNICODE */
inline ms_size_t c_str_len_a(CString const& s)
#endif /* UNICODE */
{
    return c_str_len(s);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* CString */
/** \brief \ref group__concept__shim__string_access__c_str_ptr for CString
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline LPCTSTR c_str_ptr(CString const& s)
{
    /* CString always points to valid memory, whether its own
     * CStringData or afxEmptyString.m_pchData
     */
    return s;
}
#if defined(UNICODE)
inline LPCTSTR c_str_ptr_w(CString const& s)
#else /* ? UNICODE */
inline LPCTSTR c_str_ptr_a(CString const& s)
#endif /* UNICODE */
{
    return c_str_ptr(s);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/* CString */
/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for CString
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline LPCTSTR c_str_ptr_null(CString const& s)
{
    /* CString always points to valid memory, whether its own
     * CStringData or afxEmptyString.m_pchData
     */
    return s.IsEmpty() ? NULL : (LPCTSTR)s;
}
#if defined(UNICODE)
inline LPCTSTR c_str_ptr_null_w(CString const& s)
#else /* ? UNICODE */
inline LPCTSTR c_str_ptr_null_a(CString const& s)
#endif /* UNICODE */
{
    return c_str_ptr_null(s);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/cstring_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace stlsoft::mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::mfcstl::c_str_data;
#if defined(UNICODE)
using ::mfcstl::c_str_data_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_data_a;
#endif /* UNICODE */

using ::mfcstl::c_str_len;
#if defined(UNICODE)
using ::mfcstl::c_str_len_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_len_a;
#endif /* UNICODE */

using ::mfcstl::c_str_ptr;
#if defined(UNICODE)
using ::mfcstl::c_str_ptr_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_ptr_a;
#endif /* UNICODE */

using ::mfcstl::c_str_ptr_null;
#if defined(UNICODE)
using ::mfcstl::c_str_ptr_null_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_ptr_null_a;
#endif /* UNICODE */


# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Global namespace shims
 */

/* This defines a stream inserter shim function template for CString for use
 * with the Visual C++ <7.1 standard library.
 *
 * It cannot be defined as a template because that conflicts with operator << (CArchive &, CString const&)
 */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

# include <iosfwd>

/* Has to be like this otherwise it conflicts with operator <<(class CArchive &,const class CString &) (at least with VC++ <7.1). */
inline mfcstl_ns_qual_std(basic_ostream)<TCHAR>& operator <<(mfcstl_ns_qual_std(basic_ostream)<TCHAR> &stm, CString const& shim)
{
    return stm << static_cast<LPCTSTR>(shim);
}

#endif /* library */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !MFCSTL_INCL_MFCSTL_SHIMS_ACCESS_STRING_HPP_CSTRING */

/* ///////////////////////////// end of file //////////////////////////// */
