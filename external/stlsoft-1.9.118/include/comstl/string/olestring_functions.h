/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/string/olestring_functions.h (originally MOOleStr.h, ::SynesisCom)
 *
 * Purpose:     Contains classes and functions for dealing with OLE/COM strings.
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


/** \file comstl/string/olestring_functions.h
 *
 * \brief [C, C++] Functions for manipulating OLE/COM strings
 *   (\ref group__library__string "String" Library).
 */

#ifndef COMSTL_INCL_COMSTL_STRING_H_OLESTRING_FUNCTIONS
#define COMSTL_INCL_COMSTL_STRING_H_OLESTRING_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_STRING_H_OLESTRING_FUNCTIONS_MAJOR       5
# define COMSTL_VER_COMSTL_STRING_H_OLESTRING_FUNCTIONS_MINOR       1
# define COMSTL_VER_COMSTL_STRING_H_OLESTRING_FUNCTIONS_REVISION    4
# define COMSTL_VER_COMSTL_STRING_H_OLESTRING_FUNCTIONS_EDIT        158
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_MEMORY_H_MEMORY_FUNCTIONS
# include <comstl/memory/functions.h>
#endif /* !COMSTL_INCL_COMSTL_MEMORY_H_MEMORY_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */

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

/** \brief [C only] Creates an OLE string from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the OLE string
 * \result The created OLE string
 */
STLSOFT_INLINE LPOLESTR comstl__olestring_create_a(cs_char_a_t const* s)
{
    LPOLESTR    posz;

    if(NULL == s)
    {
        posz = NULL;
    }
    else
    {
        /* NOTE: All this posz2 hoop jumping is needed because BC++ gets confused. Ahh, bless it ... */

        cs_size_t       cch     =   stlsoft_static_cast(cs_size_t, STLSOFT_NS_GLOBAL(MultiByteToWideChar)(0, 0, s, -1, NULL, 0));
        const LPOLESTR  posz2   =   stlsoft_static_cast(LPOLESTR, STLSOFT_NS_GLOBAL(CoTaskMemAlloc)(sizeof(OLECHAR) * (cch + 1)));

        posz = posz2;

        if(NULL != posz)
        {
            cs_size_t n = stlsoft_static_cast(cs_size_t, STLSOFT_NS_GLOBAL(MultiByteToWideChar)(0, 0, s, -1, posz2, stlsoft_static_cast(int, cch + 1)));

            if(n < cch)
            {
                posz[n] = L'\0';
            }
        }
    }

    return posz;
}

/** \brief [C only] Creates an OLE string from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the OLE string
 * \result The created OLE string
 */
STLSOFT_INLINE LPOLESTR comstl__olestring_create_w(cs_char_w_t const* s)
{
    LPOLESTR posz;

    if(NULL == s)
    {
        posz = NULL;
    }
    else
    {
        cs_size_t cch = STLSOFT_NS_GLOBAL(wcslen)(s);

        posz = stlsoft_static_cast(LPOLESTR, STLSOFT_NS_GLOBAL(CoTaskMemAlloc)(sizeof(OLECHAR) * (cch + 1)));

        if(NULL != posz)
        {
#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS
            STLSOFT_NS_GLOBAL(wcscpy_s)(posz, 1 + cch, s);
#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */
            STLSOFT_NS_GLOBAL(wcscpy)(posz, s);
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */
        }
    }

    return posz;
}

/** \brief [C only] Destroys an OLE string
 *
 * \ingroup group__library__string
 *
 * \param posz The OLE string to destroy
 */
STLSOFT_INLINE void comstl__olestring_destroy(LPOLESTR posz)
{
    COMSTL_MESSAGE_ASSERT("Invalid request to destroy non-COM string", 0 != comstl__CoTaskMemDidAlloc(posz));

    STLSOFT_NS_GLOBAL(CoTaskMemFree)(stlsoft_static_cast(LPVOID, posz));
}

/** \brief [C only] Duplicates an OLE string
 *
 * \ingroup group__library__string
 *
 * \param posz The OLE string to duplicate
 * \return The copied OLE string
 */
STLSOFT_INLINE LPOLESTR comstl__olestring_dup(LPCOLESTR posz)
{
    return comstl__olestring_create_w(posz);
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

/** \brief Creates an OLE string from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the OLE string
 * \result The created OLE string
 */
inline LPOLESTR olestring_create_a(cs_char_a_t const* s)
{
    return comstl__olestring_create_a(s);
}

/** \brief Creates an OLE string from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the OLE string
 * \result The created OLE string
 */
inline LPOLESTR olestring_create_w(cs_char_w_t const* s)
{
    return comstl__olestring_create_w(s);
}

/** \brief Creates an OLE string from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the OLE string
 * \result The created OLE string
 */
inline LPOLESTR olestring_create(cs_char_a_t const* s)
{
    return olestring_create_a(s);
}

/** \brief Creates an OLE string from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the OLE string
 * \result The created OLE string
 */
inline LPOLESTR olestring_create(cs_char_w_t const* s)
{
    return olestring_create_w(s);
}

/** \brief Destroys an OLE string
 *
 * \ingroup group__library__string
 *
 * \param posz The OLE string to destroy
 */
inline void olestring_destroy(LPOLESTR posz)
{
    comstl__olestring_destroy(posz);
}

/** \brief Duplicates an OLE string
 *
 * \ingroup group__library__string
 *
 * \param posz The OLE string to duplicate
 * \return The copied OLE string
 */
inline LPOLESTR olestring_dup(LPCOLESTR posz)
{
    return olestring_create(posz);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/olestring_functions_unittest_.h"
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

#endif /* !COMSTL_INCL_COMSTL_STRING_H_OLESTRING_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
