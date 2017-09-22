/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/error/errorinfo_functions.h
 *
 * Purpose:     Error info functions.
 *
 * Created:     5th Feburary 2004
 * Updated:     15th February 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/error/errorinfo_functions.h
 *
 * \brief [C, C++] Error info functions
 *   (\ref group__library__error "Error" Library).
 */

#ifndef COMSTL_INCL_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS
#define COMSTL_INCL_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS_MAJOR    4
# define COMSTL_VER_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS_MINOR    2
# define COMSTL_VER_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS_REVISION 4
# define COMSTL_VER_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS_EDIT     43
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef STLSOFT_INCL_H_OLEAUTO
# define STLSOFT_INCL_H_OLEAUTO
# include <oleauto.h>
#endif /* !STLSOFT_INCL_H_OLEAUTO */
#ifndef STLSOFT_INCL_H_OAIDL
# define STLSOFT_INCL_H_OAIDL
# include <oaidl.h>
#endif /* !STLSOFT_INCL_H_OAIDL */

#ifdef STLSOFT_UNITTEST
# include <wchar.h>
#endif /* STLSOFT_UNITTEST */

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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_INLINE HRESULT comstl__set_error_info_w_(   cs_char_w_t const*  description
                                                ,   cs_char_w_t const*  source
                                                ,   GUID const*         guid
                                                ,   cs_char_w_t const*  helpFile
                                                ,   cs_dword_t const*   helpContext)
{
    ICreateErrorInfo    *pCEI;
    HRESULT             hr;

    hr = STLSOFT_NS_GLOBAL(CreateErrorInfo)(&pCEI);

    if(SUCCEEDED(hr))
    {
        IErrorInfo  *pEI;

        /* Ask for IErrorInfo */
        hr = COMSTL_ITF_CALL(pCEI)->QueryInterface( COMSTL_ITF_THIS(pCEI)
                                                    COMSTL_IID_2_REF(IID_IErrorInfo)
                                                ,   stlsoft_reinterpret_cast(void**, &pEI));

        if(SUCCEEDED(hr))
        {
            if(NULL != description)
            {
                hr = COMSTL_ITF_CALL(pCEI)->SetDescription(COMSTL_ITF_THIS(pCEI) stlsoft_const_cast(LPOLESTR, description));
            }

            if(NULL != source)
            {
                hr = COMSTL_ITF_CALL(pCEI)->SetSource(COMSTL_ITF_THIS(pCEI) stlsoft_const_cast(LPOLESTR, source));
            }

            if(NULL != guid)
            {
                hr = COMSTL_ITF_CALL(pCEI)->SetGUID(COMSTL_ITF_THIS(pCEI) COMSTL_IID_2_REF(*guid));
            }

            if(NULL != helpFile)
            {
                hr = COMSTL_ITF_CALL(pCEI)->SetHelpFile(COMSTL_ITF_THIS(pCEI) stlsoft_const_cast(LPOLESTR, helpFile));
            }

            if(NULL != helpContext)
            {
                hr = COMSTL_ITF_CALL(pCEI)->SetHelpContext(COMSTL_ITF_THIS(pCEI) *helpContext);
            }

            if(SUCCEEDED(hr))
            {
                hr = STLSOFT_NS_GLOBAL(SetErrorInfo)(0, pEI);
            }

            COMSTL_ITF_CALL(pEI)->Release(COMSTL_ITF_THIS0(pEI));
        }

        COMSTL_ITF_CALL(pCEI)->Release(COMSTL_ITF_THIS0(pCEI));
    }

    return hr;
}

STLSOFT_INLINE HRESULT comstl__set_error_info_a_helper_(cs_char_a_t const* s_a, cs_char_w_t **ps_w)
{
    COMSTL_ASSERT(NULL != ps_w);

    if(NULL == s_a)
    {
        return S_FALSE;
    }
    else
    {
        const cs_size_t len = STLSOFT_NS_GLOBAL(strlen)(s_a);

        if(NULL == (*ps_w = stlsoft_static_cast(cs_char_w_t*, STLSOFT_NS_GLOBAL(CoTaskMemAlloc)((1 + len) * sizeof(cs_char_w_t)))))
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            int n = STLSOFT_NS_GLOBAL(MultiByteToWideChar)(0, 0, s_a, -1, *ps_w, stlsoft_static_cast(int, 1 + len));

            if(0 == n)
            {
                return HRESULT_FROM_WIN32(STLSOFT_NS_GLOBAL(GetLastError)());
            }
            else
            {
                if(stlsoft_static_cast(cs_size_t, n) < len)
                {
                    (*ps_w)[n] = L'\0';
                }
            }
        }

        return S_OK;
    }
}

STLSOFT_INLINE HRESULT comstl__set_error_info_a_(   cs_char_a_t const*  description
                                                ,   cs_char_a_t const*  source
                                                ,   GUID const*         guid
                                                ,   cs_char_a_t const*  helpFile
                                                ,   cs_dword_t const*   helpContext)
{
    HRESULT         hr              =   S_OK;
    cs_char_w_t     *description_w  =   NULL;
    cs_char_w_t     *source_w       =   NULL;
    cs_char_w_t     *helpFile_w     =   NULL;

    if(SUCCEEDED(hr))
    {
        hr = comstl__set_error_info_a_helper_(description, &description_w);
    }
    if(SUCCEEDED(hr))
    {
        hr = comstl__set_error_info_a_helper_(source, &source_w);
    }
    if(SUCCEEDED(hr))
    {
        hr = comstl__set_error_info_a_helper_(helpFile, &helpFile_w);
    }

    if(SUCCEEDED(hr))
    {
        hr = comstl__set_error_info_w_(description_w, source_w, guid, helpFile_w, helpContext);
    }

    STLSOFT_NS_GLOBAL(CoTaskMemFree)(description_w);
    STLSOFT_NS_GLOBAL(CoTaskMemFree)(source_w);
    STLSOFT_NS_GLOBAL(CoTaskMemFree)(helpFile_w);

    return hr;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [C only] Sets the description of the current error object to the given Unicode string
 *
 * \ingroup group__library__error
 *
 * \see comstl::set_error_info_description
 */
STLSOFT_INLINE HRESULT comstl__set_error_info_description_w(cs_char_w_t const* description)
{
    COMSTL_MESSAGE_ASSERT("error info description string cannot be NULL", NULL != description);

    return comstl__set_error_info_w_(description, NULL, NULL, NULL, NULL);
}

/** \brief [C only] Sets the description of the current error object to the given ANSI string
 *
 * \ingroup group__library__error
 *
 * \see comstl::set_error_info_description
 */
STLSOFT_INLINE HRESULT comstl__set_error_info_description_a(cs_char_a_t const* description)
{
    COMSTL_MESSAGE_ASSERT("error info description string cannot be NULL", NULL != description);

    return comstl__set_error_info_a_(description, NULL, NULL, NULL, NULL);
}

/** \brief [C only] Sets the description and source of the current error object to the given Unicode strings
 *
 * \ingroup group__library__error
 *
 * \see comstl::set_error_info_description_and_source
 */
STLSOFT_INLINE HRESULT comstl__set_error_info_description_and_source_w(cs_char_w_t const* description, cs_char_w_t const* source)
{
    COMSTL_MESSAGE_ASSERT("error info description string cannot be NULL", NULL != description);
    COMSTL_MESSAGE_ASSERT("error info source string cannot be NULL", NULL != source);

    return comstl__set_error_info_w_(description, source, NULL, NULL, NULL);
}

/** \brief [C only] Sets the description and source of the current error object to the given ANSI strings
 *
 * \ingroup group__library__error
 *
 * \see comstl::set_error_info_description_and_source
 */
STLSOFT_INLINE HRESULT comstl__set_error_info_description_and_source_a(cs_char_a_t const* description, cs_char_a_t const* source)
{
    COMSTL_MESSAGE_ASSERT("error info description string cannot be NULL", NULL != description);
    COMSTL_MESSAGE_ASSERT("error info source string cannot be NULL", NULL != source);

    return comstl__set_error_info_a_(description, source, NULL, NULL, NULL);
}

/** \brief [C only] Sets the description, source, interface ID and help information of the current error object
 *
 * \ingroup group__library__error
 *
 * \see comstl::set_error_info
 */
STLSOFT_INLINE HRESULT comstl__set_error_info_w(cs_char_w_t const*  description
                                            ,   cs_char_w_t const*  source
                                            ,   REFGUID             guid
                                            ,   cs_char_w_t const*  helpFile
                                            ,   cs_dword_t          helpContext)
{
    COMSTL_MESSAGE_ASSERT("error info description string cannot be NULL", NULL != description);
    COMSTL_MESSAGE_ASSERT("error info source string cannot be NULL", NULL != source);
    COMSTL_MESSAGE_ASSERT("error info help file string cannot be NULL", NULL != helpFile);

    return comstl__set_error_info_w_(description, source, COMSTL_REF_2_PTR(guid), helpFile, &helpContext);
}

/** \brief [C only] Sets the description, source, interface ID and help information of the current error object
 *
 * \ingroup group__library__error
 *
 * \see comstl::set_error_info
 */
STLSOFT_INLINE HRESULT comstl__set_error_info_a(cs_char_a_t const*  description
                                            ,   cs_char_a_t const*  source
                                            ,   REFGUID             guid
                                            ,   cs_char_a_t const*  helpFile
                                            ,   cs_dword_t          helpContext)
{
    COMSTL_MESSAGE_ASSERT("error info description string cannot be NULL", NULL != description);
    COMSTL_MESSAGE_ASSERT("error info source string cannot be NULL", NULL != source);
    COMSTL_MESSAGE_ASSERT("error info help file string cannot be NULL", NULL != helpFile);

    return comstl__set_error_info_a_(description, source, COMSTL_REF_2_PTR(guid), helpFile, &helpContext);
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

/** \brief Sets the description of the current error object to the given ANSI string
 *
 * \ingroup group__library__error
 */
inline HRESULT set_error_info(cs_char_a_t const* description)
{
    return comstl__set_error_info_description_a(description);
}

/** \brief Sets the description of the current error object to the given Unicode string
 *
 * \ingroup group__library__error
 *
 * \param description The error description
 */
inline HRESULT set_error_info(cs_char_w_t const* description)
{
    return comstl__set_error_info_description_w(description);
}

/** \brief Sets the description and source of the current error object to the given ANSI string
 *
 * \ingroup group__library__error
 *
 * \param description The error description
 * \param source The error source
 */
inline HRESULT set_error_info(cs_char_a_t const* description, cs_char_a_t const* source)
{
    return comstl__set_error_info_description_and_source_a(description, source);
}

/** \brief Sets the description and source of the current error object to the given Unicode string
 *
 * \ingroup group__library__error
 *
 * \param description The error description
 * \param source The error source
 */
inline HRESULT set_error_info(cs_char_w_t const* description, cs_char_w_t const* source)
{
    return comstl__set_error_info_description_and_source_w(description, source);
}

/** \brief Sets the description, source and GUID of the current error object to the given ANSI string
 *
 * \ingroup group__library__error
 *
 * \param description The error description
 * \param source The error source
 * \param guid The GUID of the interface in error
 */
inline HRESULT set_error_info(cs_char_a_t const* description, cs_char_a_t const* source, REFGUID guid)
{
    return comstl__set_error_info_a_(description, source, &guid, NULL, NULL);
}

/** \brief Sets the description, source and GUID of the current error object to the given Unicode string
 *
 * \ingroup group__library__error
 *
 * \param description The error description
 * \param source The error source
 * \param guid The GUID of the interface in error
 */
inline HRESULT set_error_info(cs_char_w_t const* description, cs_char_w_t const* source, REFGUID guid)
{
    return comstl__set_error_info_w_(description, source, &guid, NULL, NULL);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/errorinfo_functions_unittest_.h"
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

#endif /* !COMSTL_INCL_COMSTL_ERROR_H_ERRORINFO_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
