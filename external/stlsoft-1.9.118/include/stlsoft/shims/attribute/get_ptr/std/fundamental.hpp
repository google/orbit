/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/attribute/get_ptr/std/fundamental.hpp
 *
 * Purpose:     Contains the get_ptr access shim.
 *
 * Created:     10th January 2002
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


/** \file stlsoft/shims/attribute/get_ptr/std/fundamental.hpp
 *
 * \brief [C++] Pointer attribute shims for built-in types
 *   (\ref group__concept__shim__pointer_attribute__get_ptr "get_ptr Pointer Attribute Shim").
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL
#define STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL_MAJOR      4
# define STLSOFT_VER_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL_MINOR      0
# define STLSOFT_VER_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL_REVISION   2
# define STLSOFT_VER_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL_EDIT       46
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * get_ptr functions
 */

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER != 1200
/* pointer to const. */
/** \brief
 * \ingroup group__concept__shim__pointer_attribute__get_ptr
 */
template <ss_typename_param_k T>
inline T const* get_ptr(T const* p)
{
    return p;
}

/* pointer to const volatile. */
/** \brief
 * \ingroup group__concept__shim__pointer_attribute__get_ptr
 */
template <ss_typename_param_k T>
inline T const volatile* get_ptr(T const volatile* p)
{
    return p;
}

/* pointer to volatile. */
/** \brief
 * \ingroup group__concept__shim__pointer_attribute__get_ptr
 */
template <ss_typename_param_k T>
inline T volatile* get_ptr(T volatile* p)
{
    return p;
}
#endif /* VC++ 6.0  */

/* pointer to non-const. */
/** \brief
 * \ingroup group__concept__shim__pointer_attribute__get_ptr
 */
template <ss_typename_param_k T>
inline T* get_ptr(T* p)
{
    return p;
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

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_GET_PTR_STD_HPP_FUNDAMENTAL */

/* ///////////////////////////// end of file //////////////////////////// */
