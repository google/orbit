/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/object_functions.h (originally MOComFns.h)
 *
 * Purpose:     Reference-counting helper functions.
 *
 * Created:     2nd March 1996
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1996-2009, Matthew Wilson and Synesis Software
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


/** \file comstl/util/object_functions.h
 *
 * \brief [C, C++] Definition of several interface and identity functions
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_H_OBJECT_FUNCTIONS
#define COMSTL_INCL_COMSTL_UTIL_H_OBJECT_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_H_OBJECT_FUNCTIONS_MAJOR    3
# define COMSTL_VER_COMSTL_UTIL_H_OBJECT_FUNCTIONS_MINOR    1
# define COMSTL_VER_COMSTL_UTIL_H_OBJECT_FUNCTIONS_REVISION 2
# define COMSTL_VER_COMSTL_UTIL_H_OBJECT_FUNCTIONS_EDIT     66
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(_COMSTL_NO_NAMESPACE) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if defined(_STLSOFT_NO_NAMESPACE)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * C functions
 */

/** \brief [C only] Evaluates an object's identity
 *
 * \param p The pointer to the object whose identity will be evaluated
 * \param identity
 *
 * \return A status code indicating whether the identity was retrieved
 * \retval S_OK The pointers refer to the same object
 * \retval E_POINTER If p is null
 * \retval E_INVALIDARG If identity is null
 * \retval any-other-code Indicates an error from the call to QueryInterface()
 *
 * \pre \c p is a NULL pointer, or a valid pointer to a COM interface.
 *
 * \remarks If the function success, the caller must release the reference count
 *        associated with the interface pointer in \c *identity.
 *
 * \note Intended for use in C-compilation units only. For C++, use comstl::get_object_identity().
 */
STLSOFT_INLINE HRESULT comstl__get_object_identity(/* [in] */ LPUNKNOWN p, /* [in] */ LPUNKNOWN *identity)
{
    HRESULT hr;

    if(NULL == identity)
    {
        hr = E_INVALIDARG;
    }
    else if(NULL == p)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = COMSTL_ITF_CALL(p)->QueryInterface(COMSTL_ITF_THIS(p) COMSTL_IID_2_REF(IID_IUnknown), stlsoft_reinterpret_cast(void**, identity));
    }

    return hr;
}

/** \brief [C only] Determines whether two interfaces refer to the same object
 *
 * \return A status code indicating whether the two pointers refer to the same
 *          object
 * \retval S_OK The pointers refer to the same object
 * \retval S_FALSE The pointers refer to different objects
 * \retval E_POINTER If either/both pointers are null
 * \retval any-other-code Indicates an error from one of the calls to QueryInterface()
 *
 * \pre \c p1 and \c p2 are NULL pointers, or are valid pointers to COM interfaces.
 */
STLSOFT_INLINE HRESULT comstl__is_same_object(/* [in] */ LPUNKNOWN p1, /* [in] */ LPUNKNOWN p2)
{
    LPUNKNOWN   punk1   =   NULL;
    LPUNKNOWN   punk2   =   NULL;
    HRESULT     hr      =   S_OK;

    if( NULL == p1 ||
        NULL == p2)
    {
        hr = E_POINTER;
    }

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(p1)->QueryInterface(COMSTL_ITF_THIS(p1) COMSTL_IID_2_REF(IID_IUnknown), stlsoft_reinterpret_cast(void**, &punk1));
    }
    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(p2)->QueryInterface(COMSTL_ITF_THIS(p2) COMSTL_IID_2_REF(IID_IUnknown), stlsoft_reinterpret_cast(void**, &punk2));
    }

    if(SUCCEEDED(hr))
    {
        if( NULL == punk1 ||
            NULL == punk2 ||
            punk1 != punk2)
        {
            hr = S_FALSE;
        }
    }

    if(NULL != punk1)
    {
        COMSTL_ITF_CALL(punk1)->Release(COMSTL_ITF_THIS0(punk1));
    }
    if(NULL != punk2)
    {
        COMSTL_ITF_CALL(punk2)->Release(COMSTL_ITF_THIS0(punk2));
    }

    return hr;
}

/** \brief [C only] Determines whether an object implements a given interface without adding a reference count
 *
 * \return A status code indicating whether the given interface is implemented
 * \retval S_OK The interface is implemented and accessible
 * \retval E_NOINTERFACE The interface is not implemented
 * \retval any-other-code Indicates an error from the call to QueryInterface()
 *
 * \pre \c p is a non-NULL pointer to a COM interface.
 */
STLSOFT_INLINE HRESULT comstl__is_interface_implemented(/* [in] */ LPUNKNOWN p, /* [in] */ REFIID riid)
{
    LPUNKNOWN   punk    =   NULL;
    HRESULT     hr;

    COMSTL_MESSAGE_ASSERT("is_interface_implemented() does not accept a NULL pointer", NULL != p);

    hr = COMSTL_ITF_CALL(p)->QueryInterface(COMSTL_ITF_THIS(p) riid, stlsoft_reinterpret_cast(void**, &punk));

    if(NULL != punk)
    {
        COMSTL_ITF_CALL(punk)->Release(COMSTL_ITF_THIS0(punk));
    }

    return hr;
}

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace comstl
{
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * C++ functions
 */

#ifdef __cplusplus

/** \brief Evaluates an object's identity
 *
 * \param p The pointer to the object whose identity will be evaluated
 * \param identity
 *
 * \return A status code indicating whether the identity was retrieved
 * \retval S_OK The pointers refer to the same object
 * \retval E_POINTER If p is null
 * \retval E_INVALIDARG If identity is null
 * \retval any-other-code Indicates an error from the call to QueryInterface()
 *
 * \pre \c p is a NULL pointer, or a valid pointer to a COM interface.
 *
 * \remarks If the function success, the caller must release the reference count
 *        associated with the interface pointer in \c *identity.
 *
 * \note Implemented in terms of the C-compatible function comstl__get_object_identity().
 */
inline HRESULT get_object_identity(/* [in] */ LPUNKNOWN p, /* [in] */ LPUNKNOWN *identity)
{
    return comstl__get_object_identity(p, identity);
}

/** \brief Determines whether two interfaces refer to the same object
 *
 * \return A status code indicating whether the two pointers refer to the same
 *          object
 * \retval S_OK The pointers refer to the same object
 * \retval S_FALSE The pointers refer to different objects
 * \retval E_POINTER One or both pointers are NULL.
 * \retval any-other-code Indicates an error from one of the calls to QueryInterface()
 *
 * \pre \c p1 and \c p2 are NULL pointers, or are valid pointers to COM interfaces.
 */
inline HRESULT is_same_object(/* [in] */ LPUNKNOWN p1, /* [in] */ LPUNKNOWN p2)
{
    return comstl__is_same_object(p1, p2);
}

/** \brief Determines whether an object implements a given interface without adding a reference count
 *
 * \return A status code indicating whether the given interface is implemented
 * \retval S_OK The interface is implemented and accessible
 * \retval E_NOINTERFACE The interface is not implemented
 * \retval any-other-code Indicates an error from the call to QueryInterface()
 *
 * \pre \c p is a non-NULL pointer to a COM interface.
 */
inline HRESULT is_interface_implemented(/* [in] */ LPUNKNOWN p, /* [in] */ REFIID riid)
{
    return comstl__is_interface_implemented(p, riid);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/object_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace comstl */
# else
} /* namespace comstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_UTIL_H_OBJECT_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
