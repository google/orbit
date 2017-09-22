/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/shims/access/string/CException.hpp
 *
 * Purpose:     Contains classes and functions for dealing with MFC
 *              exceptions.
 *
 * Created:     15th September 2010
 * Updated:     15th September 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2010, Matthew Wilson and Synesis Software
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


/** \file mfcstl/shims/access/string/CException.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>CException</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef MFCSTL_INCL_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION
#define MFCSTL_INCL_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION_MAJOR    1
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION_MINOR    0
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION_REVISION 1
# define MFCSTL_VER_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION_EDIT     90
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD
# include <stlsoft/shims/access/string/fwd.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */

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
 * Constants
 */

ms_size_t const MFCSTL_EXCEPTION_SAS_CCH    =   128;

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct ximpl_CException_sas_util
{
    typedef stlsoft_ns_qual(auto_buffer)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH>       buffer_t;
    typedef stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> shim_string_t;

    static shim_string_t create_(CException const& x)
    {
        buffer_t buff(buffer_t::internal_size());

        for(; !buff.empty(); buff.resize(1u + 2* buff.size()))
        {
            buff[buff.size() - 1] = '~';

            if(!const_cast<CException&>(x).GetErrorMessage(&buff[0], buff.size(), NULL))
            {
            }
            else if('\0' != buff[buff.size() - 1])
            {
                break;
            }
        }

        return shim_string_t(buff.data());
    }
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* CException */
/** \brief \ref group__concept__shim__string_access__c_str_data for CException
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_data(CException const& x)
{
    return ximpl_CException_sas_util::create_(x);
}

#if defined(UNICODE)
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_data_w(CException const& x)
#else /* ? UNICODE */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_data_a(CException const& x)
#endif /* UNICODE */
{
    return c_str_data(x);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/* CException */
/** \brief \ref group__concept__shim__string_access__c_str_len for CException
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_len(CException const& x)
{
    return ximpl_CException_sas_util::create_(x);
}

#if defined(UNICODE)
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_len_w(CException const& x)
#else /* ? UNICODE */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_len_a(CException const& x)
#endif /* UNICODE */
{
    return c_str_len(x);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* CException */
/** \brief \ref group__concept__shim__string_access__c_str_ptr for CException
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_ptr(CException const& x)
{
    return ximpl_CException_sas_util::create_(x);
}

#if defined(UNICODE)
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_ptr_w(CException const& x)
#else /* ? UNICODE */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, MFCSTL_EXCEPTION_SAS_CCH> c_str_ptr_a(CException const& x)
#endif /* UNICODE */
{
    return c_str_ptr(x);
}

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


# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !MFCSTL_INCL_MFCSTL_SHIMS_ACCESS_STRING_HPP_CEXCEPTION */

/* ///////////////////////////// end of file //////////////////////////// */
