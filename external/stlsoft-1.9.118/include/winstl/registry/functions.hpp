/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/functions.hpp
 *
 * Purpose:     Registry functions.
 *
 * Created:     20th November 1995
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1995-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/registry/functions.hpp
 *
 * \brief [C++ only] Simple and discrete registry functions, used by
 *   the \ref group__library__windows_registry "Windows Registry" Library.
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_FUNCTIONS
#define WINSTL_INCL_WINSTL_REGISTRY_HPP_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_HPP_FUNCTIONS_MAJOR     3
# define WINSTL_VER_WINSTL_REGISTRY_HPP_FUNCTIONS_MINOR     1
# define WINSTL_VER_WINSTL_REGISTRY_HPP_FUNCTIONS_REVISION  4
# define WINSTL_VER_WINSTL_REGISTRY_HPP_FUNCTIONS_EDIT      49
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS
# include <winstl/registry/util/defs.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS
# include <winstl/registry/reg_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS */

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

/** \brief Gets the contents of a registry key's string value into a
 *    caller-supplied buffer.
 * \ingroup group__library__windows_registry
 *
 * \param hkey <code class="inout">[in]</code> Handle of the key whose values will be retrieved
 * \param name <code class="inout">[in]</code> The name of the value. May be <code>NULL</code> or the empty
 *   string to access the key's default value
 * \param buffer <code class="inout">[in]</code> Pointer to the caller-allocated buffer into which the
 *   value's string result will be written.
 * \param cchBuffer <code class="inout">[inout]</code> Specifies the size of the <code>buffer</code>
 *   parameter and receives the number of bytes required for the whole value (including the
 *   string's nul-terminating character).
 *
 * \return A Registry API status code indicating success or failure
 * \retval "ERROR_SUCCESS (==0)" The function completed successfully
 * \retval "any other value" The function failed, and the error code indicates why
 */
template <ss_typename_param_k C>
inline LONG reg_get_string_value(HKEY hkey, C const* name, C *buffer, ws_size_t &cchBuffer)
{
    DWORD       type;
    ws_size_t   cbData  =   sizeof(C) * cchBuffer;
    LONG        res     =   reg_traits<C>::reg_query_value(hkey, name, type, buffer, cbData);

    if(ERROR_SUCCESS == res)
    {
        cchBuffer = cbData / sizeof(C);
    }

    return res;
}

/** \brief Gets the contents of a registry key's DWORD value into a
 *    caller-supplied variable.
 * \ingroup group__library__windows_registry
 *
 * \param hkey <code class="inout">[in]</code> Handle of the key whose values will be retrieved
 * \param name <code class="inout">[in]</code> The name of the value. May be <code>NULL</code> or the empty
 *   string to access the key's default value
 * \param value <code class="inout">[out]</code> The value's value.
 *
 * \return A Registry API status code indicating success or failure
 * \retval "ERROR_SUCCESS (==0)" The function completed successfully
 * \retval "any other value" The function failed, and the error code indicates why
 *
 * \remarks If the function fails, the value of <code>value</code> is unchanged.
 */
template <ss_typename_param_k C>
inline LONG reg_get_dword_value(HKEY hkey, C const* name, DWORD& value)
{
    DWORD       type;
    ws_size_t   cbData  =   sizeof(value);
    LONG        res     =   reg_traits<C>::reg_query_value(hkey, name, type, &value, cbData);

    return res;
}

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

#endif /* WINSTL_INCL_WINSTL_REGISTRY_HPP_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
