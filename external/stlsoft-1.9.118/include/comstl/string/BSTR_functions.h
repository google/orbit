/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/string/BSTR_functions.h (originally MOBStrFn.h, ::SynesisCom)
 *
 * Purpose:     Contains classes and functions for dealing with BSTR strings.
 *
 * Created:     24th June 2002
 * Updated:     4th March 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/string/BSTR_functions.h
 *
 * \brief [C, C++] Functions for manipulating BSTR strings
 *   (\ref group__library__string "String" Library).
 */

#ifndef COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS
#define COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_STRING_H_BSTR_FUNCTIONS_MAJOR    4
# define COMSTL_VER_COMSTL_STRING_H_BSTR_FUNCTIONS_MINOR    2
# define COMSTL_VER_COMSTL_STRING_H_BSTR_FUNCTIONS_REVISION 7
# define COMSTL_VER_COMSTL_STRING_H_BSTR_FUNCTIONS_EDIT     84
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */
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

/** \brief [C only] Creates a BSTR from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 *
 * \result The created BSTR
 */
STLSOFT_INLINE BSTR comstl__bstr_create_w(cs_char_w_t const* s)
{
    return STLSOFT_NS_GLOBAL(SysAllocString)(s);
}

/** \brief [C only] Creates a BSTR from a (part of a) Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 * \param len The number of characters of \c s to copy into the result
 *
 * \result The created BSTR
 */
STLSOFT_INLINE BSTR comstl__bstr_create_len_w(cs_char_w_t const* s, cs_size_t len)
{
    COMSTL_MESSAGE_ASSERT("Cannot pass size_t(-1) as length!", 0 != len + 1);

    COMSTL_ASSERT(NULL == s || !STLSOFT_NS_GLOBAL(IsBadStringPtrW(s, len)));

    return STLSOFT_NS_GLOBAL(SysAllocStringLen)(s, stlsoft_static_cast(UINT, len));
}

/** \brief [C only] Creates a BSTR from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 *
 * \result The created BSTR
 */
STLSOFT_INLINE BSTR comstl__bstr_create_a(cs_char_a_t const* s)
{
    BSTR bstr = NULL;

    if(s != NULL)
    {
        const cs_size_t len     =   STLSOFT_NS_GLOBAL(strlen)(s);
        cs_char_w_t     *pwsz   =   stlsoft_static_cast(cs_char_w_t*, STLSOFT_NS_GLOBAL(CoTaskMemAlloc)((1 + len) * sizeof(cs_char_w_t)));

        if(NULL != pwsz)
        {
            int n = STLSOFT_NS_GLOBAL(MultiByteToWideChar)(0, 0, s, -1, pwsz, stlsoft_static_cast(int, 1 + len));

            if(0 != n)
            {
                if(stlsoft_static_cast(cs_size_t, n) < len)
                {
                    pwsz[n] = L'\0';
                }

                bstr = comstl__bstr_create_w(pwsz);
            }

            STLSOFT_NS_GLOBAL(CoTaskMemFree)(pwsz);
        }
    }

    return bstr;
}

/** \brief [C only] Creates a BSTR from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 * \param len The number of characters of \c s to copy into the result
 *
 * \result The created BSTR
 */
STLSOFT_INLINE BSTR comstl__bstr_create_len_a(cs_char_a_t const* s, cs_size_t len)
{
    BSTR    bstr = NULL;

    COMSTL_MESSAGE_ASSERT("Cannot pass size_t(-1) as length!", 0 != len + 1);

    if(s != NULL)
    {
        cs_char_w_t* pwsz = stlsoft_static_cast(cs_char_w_t*, STLSOFT_NS_GLOBAL(CoTaskMemAlloc)((1 + len) * sizeof(cs_char_w_t)));

        if(NULL != pwsz)
        {
            int n = STLSOFT_NS_GLOBAL(MultiByteToWideChar)(0, 0, s, stlsoft_static_cast(int, len), pwsz, stlsoft_static_cast(int, 1 + len));

#ifdef _DEBUG
            if(0 == n)
            {
                STLSOFT_NS_GLOBAL(GetLastError)();
            }
#endif /* _DEBUG */
            if(0 != n)
            {
                bstr = comstl__bstr_create_len_w(pwsz, len);
            }

            STLSOFT_NS_GLOBAL(CoTaskMemFree)(pwsz);
        }
    }

    return bstr;
}

/** \brief [C only] Destroys a BSTR
 *
 * \ingroup group__library__string
 *
 * \param bstr The BSTR to destroy
 */
STLSOFT_INLINE void comstl__bstr_destroy(BSTR bstr)
{
    STLSOFT_NS_GLOBAL(SysFreeString)(bstr);
}

/** \brief [C only] Duplicates a BSTR
 *
 * \ingroup group__library__string
 *
 * \param bstr The BSTR to duplicate
 * \return The copied BSTR
 */
STLSOFT_INLINE BSTR comstl__bstr_dup(BSTR bstr)
{
    return comstl__bstr_create_w(bstr);
}

/** \brief [C only] Compares two BSTR strings
 *
 * \ingroup group__library__string
 *
 * \param s1 The first BSTR to compare. May be NULL
 * \param s2 The second BSTR to compare. May be NULL
 *
 * \return An integer value representing the relationship between the strings
 * \retval <0 s1 is lexicographically less than s2
 * \retval 0 s1 is lexicographically equal than s2
 * \retval >0 s1 is lexicographically greater than s2
 */
STLSOFT_INLINE int comstl__bstr_compare(BSTR s1, BSTR s2)
{
    if(NULL == s1)
    {
        if(NULL == s2)
        {
            return 0;   /* Two NULLs are equal */
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if(NULL == s2)
        {
            return +1;
        }
        else
        {
            return STLSOFT_NS_GLOBAL(wcscmp)(s1, s2);
        }
    }
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

/** \brief Creates a BSTR from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 *
 * \result The created BSTR
 */
inline BSTR bstr_create_w(cs_char_w_t const* s)
{
    return comstl__bstr_create_w(s);
}

/** \brief Creates a BSTR from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 * \param len The number of characters of \c s to copy into the result
 *
 * \result The created BSTR
 */
inline BSTR bstr_create_w(cs_char_w_t const* s, cs_size_t len)
{
    return comstl__bstr_create_len_w(s, len);
}

/** \brief Creates a BSTR from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 *
 * \result The created BSTR
 */
inline BSTR bstr_create_a(cs_char_a_t const* s)
{
    return comstl__bstr_create_a(s);
}

/** \brief Creates a BSTR from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 * \param len The number of characters of \c s to copy into the result
 *
 * \result The created BSTR
 */
inline BSTR bstr_create_a(cs_char_a_t const* s, cs_size_t len)
{
    return comstl__bstr_create_len_a(s, len);
}

/** \brief Creates a BSTR from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 *
 * \result The created BSTR
 */
inline BSTR bstr_create(cs_char_a_t const* s)
{
    return bstr_create_a(s);
}

/** \brief Creates a BSTR from an ANSI string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 * \param len The number of characters of \c s to copy into the result
 *
 * \result The created BSTR
 */
inline BSTR bstr_create(cs_char_a_t const* s, cs_size_t len)
{
    return bstr_create_a(s, len);
}

/** \brief Creates a BSTR from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 *
 * \result The created BSTR
 */
inline BSTR bstr_create(cs_char_w_t const* s)
{
    return bstr_create_w(s);
}

/** \brief Creates a BSTR from a Unicode string
 *
 * \ingroup group__library__string
 *
 * \param s The string from which to create the BSTR
 * \param len The number of characters of \c s to copy into the result
 *
 * \result The created BSTR
 */
inline BSTR bstr_create(cs_char_w_t const* s, cs_size_t len)
{
    return bstr_create_w(s, len);
}

/** \brief Destroys a BSTR
 *
 * \ingroup group__library__string
 *
 * \param bstr The BSTR to destroy
 */
inline void bstr_destroy(BSTR bstr)
{
    comstl__bstr_destroy(bstr);
}

/** \brief Duplicates a BSTR
 *
 * \ingroup group__library__string
 *
 * \param bstr The BSTR to duplicate
 * \return The copied BSTR
 */
inline BSTR bstr_dup(BSTR bstr)
{
    return comstl__bstr_dup(bstr);
}

/** \brief Compares two BSTR strings
 *
 * \ingroup group__library__string
 *
 * \param s1 The first BSTR to compare. May be NULL
 * \param s2 The second BSTR to compare. May be NULL
 *
 * \return An integer value representing the relationship between the strings
 * \retval <0 s1 is lexicographically less than s2
 * \retval 0 s1 is lexicographically equal than s2
 * \retval >0 s1 is lexicographically greater than s2
 */
inline int bstr_compare(BSTR s1, BSTR s2)
{
    return comstl__bstr_compare(s1, s2);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/BSTR_functions_unittest_.h"
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

#endif /* !COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
