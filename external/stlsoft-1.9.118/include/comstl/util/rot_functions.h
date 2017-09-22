/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/rot_functions.h (originally MORotFns.h, ::SynesisCom)
 *
 * Purpose:     COM ROT (Running Object Table) functions.
 *
 * Created:     21st October 1998
 * Updated:     29th July 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/util/rot_functions.h
 *
 * \brief [C++ only; requires COM] COM ROT (Running Object Table) functions
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_H_ROT_FUNCTIONS
#define COMSTL_INCL_COMSTL_UTIL_H_ROT_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_H_ROT_FUNCTIONS_MAJOR       5
# define COMSTL_VER_COMSTL_UTIL_H_ROT_FUNCTIONS_MINOR       1
# define COMSTL_VER_COMSTL_UTIL_H_ROT_FUNCTIONS_REVISION    3
# define COMSTL_VER_COMSTL_UTIL_H_ROT_FUNCTIONS_EDIT        65
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


/** \brief [C only] Registers an object in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Registers an object and its identifying moniker in the Running Object Table (ROT).
 *
 * \param grfFlags Registration options
 * \param punkObject Pointer to the object being registered
 * \param pmkObjectName Pointer to the moniker of the object being registered
 * \param pdwRegister Pointer to the value identifying the registration
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_Register(
    DWORD       grfFlags
,   LPUNKNOWN   punkObject
,   LPMONIKER   pmkObjectName
,   DWORD*      pdwRegister
)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->Register(COMSTL_ITF_THIS(prot) grfFlags, punkObject, pmkObjectName, pdwRegister);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/** \brief [C only] Removes an object from the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Removes from the Running Object Table (ROT) an entry that was previously
 * registered by a call to Rot_Register().
 *
 * \param dwRegister Value identifying registration to be revoked
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_Revoke(DWORD dwRegister)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->Revoke(COMSTL_ITF_THIS(prot) dwRegister);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/** \brief [C only] Determines if object current in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Determines whether the object identified by the specified moniker is
 * currently running. This method looks for the moniker in the Running Object
 * Table (ROT).
 *
 * \param pmkObjectName Pointer to the moniker of the object whose status is desired
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_IsRunning(LPMONIKER pmkObjectName)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->IsRunning(COMSTL_ITF_THIS(prot) pmkObjectName);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/** \brief [C only] Retrieves the object from the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Determines whether the object identified by the specified moniker is running,
 * and if it is, retrieves a pointer to that object. This method looks for the
 * moniker in the Running Object Table (ROT), and retrieves the pointer
 * registered there.
 *
 * \param pmkObjectName Pointer to the moniker of the object
 * \param ppunkObject Address of output variable that receives the IUnknown interface pointer
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_GetObject(
    LPMONIKER   pmkObjectName
,   LPUNKNOWN*  ppunkObject
)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->GetObject(COMSTL_ITF_THIS(prot) pmkObjectName, ppunkObject);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/** \brief [C only] Retrieves the last modification time of a running object in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Records the time that a running object was last modified. The object must
 * have previously been registered with the Running Object Table (ROT). This
 * method stores the time of last change in the ROT.
 *
 * \param dwRegister Value identifying registration being updated
 * \param lpfiletime Pointer to structure containing object's last change time
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_NoteChangeTime(
    DWORD       dwRegister
,   FILETIME*   lpfiletime
)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->NoteChangeTime(COMSTL_ITF_THIS(prot) dwRegister, lpfiletime);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/** \brief [C only] Retrieves the last modification time of an object in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Returns the time that an object was last modified. The object must have
 * previously been registered with the Running Object Table (ROT). This method
 * looks for the last change time recorded in the ROT.
 *
 * \param pmkObjectName Pointer to moniker on the object whose status is desired
 * \param lpfiletime Pointer to structure containing object's last change time
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_GetTimeOfLastChange(
    LPMONIKER   pmkObjectName
,   FILETIME*   lpfiletime
)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->GetTimeOfLastChange(COMSTL_ITF_THIS(prot) pmkObjectName, lpfiletime);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/** \brief [C only] Queries the current set of objects in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Creates and returns a pointer to an enumerator that can list the monikers of
 * all the objects currently registered in the Running Object Table (ROT).
 *
 * \param ppenumMoniker Address of output variable that receives the IEnumMoniker interface pointer
 * \return An HRESULT indicating success or failure
 */
STLSOFT_INLINE HRESULT comstl_C_Rot_EnumRunning(IEnumMoniker** ppenumMoniker)
{
    LPRUNNINGOBJECTTABLE    prot;
    HRESULT                 hr = STLSOFT_NS_GLOBAL(GetRunningObjectTable)(0, &prot);

    if(SUCCEEDED(hr))
    {
        hr = COMSTL_ITF_CALL(prot)->EnumRunning(COMSTL_ITF_THIS(prot) ppenumMoniker);

        COMSTL_ITF_CALL(prot)->Release(COMSTL_ITF_THIS0(prot));
    }

    return hr;
}

/* /////////////////////////////////////////////////////////////////////////
 * Obsolete symbols
 *
 * NOTE: these are only defined if:
 *
 * - we're generating documentation, or
 * - STLSOFT_OBSOLETE is specified, or
 * - it's STLSoft 1.9 (or earlier)
 */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
    defined(STLSOFT_OBSOLETE) || \
    _STLSOFT_VER < 0x010a0000

/** \def comstl__Rot_Register
 *
 * \deprecated Use comstl_C_Rot_Register
 */
# define comstl__Rot_Register               comstl_C_Rot_Register
/** \def comstl__Rot_Revoke
 *
 * \deprecated Use comstl_C_Rot_Revoke
 */
# define comstl__Rot_Revoke                 comstl_C_Rot_Revoke
/** \def comstl__Rot_IsRunning
 *
 * \deprecated Use comstl_C_Rot_IsRunning
 */
# define comstl__Rot_IsRunning              comstl_C_Rot_IsRunning
/** \def comstl__Rot_GetObject
 *
 * \deprecated Use comstl_C_Rot_GetObject
 */
# define comstl__Rot_GetObject              comstl_C_Rot_GetObject
/** \def comstl__Rot_NoteChangeTime
 *
 * \deprecated Use comstl_C_Rot_NoteChangeTime
 */
# define comstl__Rot_NoteChangeTime         comstl_C_Rot_NoteChangeTime
/** \def comstl__Rot_GetTimeOfLastChange
 *
 * \deprecated Use comstl_C_Rot_GetTimeOfLastChange
 */
# define comstl__Rot_GetTimeOfLastChange    comstl_C_Rot_GetTimeOfLastChange
/** \def comstl__Rot_EnumRunning
 *
 * \deprecated Use comstl_C_Rot_EnumRunning
 */
# define comstl__Rot_EnumRunning            comstl_C_Rot_EnumRunning

#endif /* obsolete || 1.9 */

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

/** \brief Registers an object in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Registers an object and its identifying moniker in the Running Object Table (ROT).
 *
 * \param grfFlags Registration options
 * \param punkObject Pointer to the object being registered
 * \param pmkObjectName Pointer to the moniker of the object being registered
 * \param pdwRegister Pointer to the value identifying the registration
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_Register(
    DWORD       grfFlags
,   LPUNKNOWN   punkObject
,   LPMONIKER   pmkObjectName
,   DWORD*      pdwRegister
)
{
    return comstl__Rot_Register(grfFlags, punkObject, pmkObjectName, pdwRegister);
}

/** \brief Removes an object from the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Removes from the Running Object Table (ROT) an entry that was previously
 * registered by a call to Rot_Register().
 *
 * \param dwRegister Value identifying registration to be revoked
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_Revoke(DWORD dwRegister)
{
    return comstl__Rot_Revoke(dwRegister);
}

/** \brief Determines if object current in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Determines whether the object identified by the specified moniker is
 * currently running. This method looks for the moniker in the Running Object
 * Table (ROT).
 *
 * \param pmkObjectName Pointer to the moniker of the object whose status is desired
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_IsRunning(LPMONIKER pmkObjectName)
{
    return comstl__Rot_IsRunning(pmkObjectName);
}

/** \brief Retrieves the object from the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Determines whether the object identified by the specified moniker is running,
 * and if it is, retrieves a pointer to that object. This method looks for the
 * moniker in the Running Object Table (ROT), and retrieves the pointer
 * registered there.
 *
 * \param pmkObjectName Pointer to the moniker of the object
 * \param ppunkObject Address of output variable that receives the IUnknown interface pointer
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_GetObject(
    LPMONIKER   pmkObjectName
,   LPUNKNOWN*  ppunkObject
)
{
    return comstl__Rot_GetObject(pmkObjectName, ppunkObject);
}

/** \brief Retrieves the last modification time of a running object in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Records the time that a running object was last modified. The object must
 * have previously been registered with the Running Object Table (ROT). This
 * method stores the time of last change in the ROT.
 *
 * \param dwRegister Value identifying registration being updated
 * \param lpfiletime Pointer to structure containing object's last change time
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_NoteChangeTime(
    DWORD       dwRegister
,   FILETIME*   lpfiletime
)
{
    return comstl__Rot_NoteChangeTime(dwRegister, lpfiletime);
}

/** \brief Retrieves the last modification time of an object in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Returns the time that an object was last modified. The object must have
 * previously been registered with the Running Object Table (ROT). This method
 * looks for the last change time recorded in the ROT.
 *
 * \param pmkObjectName Pointer to moniker on the object whose status is desired
 * \param lpfiletime Pointer to structure containing object's last change time
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_GetTimeOfLastChange(
    LPMONIKER   pmkObjectName
,   FILETIME*   lpfiletime
)
{
    return comstl__Rot_GetTimeOfLastChange(pmkObjectName, lpfiletime);
}

/** \brief Queries the current set of objects in the Running Object Table
 *
 * \ingroup group__library__utility__com
 *
 * Creates and returns a pointer to an enumerator that can list the monikers of
 * all the objects currently registered in the Running Object Table (ROT).
 *
 * \param ppenumMoniker Address of output variable that receives the IEnumMoniker interface pointer
 * \return An HRESULT indicating success or failure
 */
inline HRESULT Rot_EnumRunning(IEnumMoniker** ppenumMoniker)
{
    return comstl__Rot_EnumRunning(ppenumMoniker);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/rot_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace comstl */
# else
} /* namespace stlsoft::comstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_UTIL_H_ROT_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
