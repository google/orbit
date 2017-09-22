/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/VARIANT_functions.h
 *
 * Purpose:     VARIANT helper functions.
 *
 * Created:     23rd August 2008
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2008-2009, Matthew Wilson and Synesis Software
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


/** \file comstl/util/VARIANT_functions.h
 *
 * \brief [C++ only; requires COM] VARIANT helper functions
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_H_VARIANT_FUNCTIONS
#define COMSTL_INCL_COMSTL_UTIL_H_VARIANT_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_H_VARIANT_FUNCTIONS_MAJOR       1
# define COMSTL_VER_COMSTL_UTIL_H_VARIANT_FUNCTIONS_MINOR       0
# define COMSTL_VER_COMSTL_UTIL_H_VARIANT_FUNCTIONS_REVISION    2
# define COMSTL_VER_COMSTL_UTIL_H_VARIANT_FUNCTIONS_EDIT        3
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS
# include <comstl/string/BSTR_functions.h>
#endif /* !COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS */
#ifndef COMSTL_INCL_COMSTL_UTIL_H_CY_FUNCTIONS
# include <comstl/util/CY_functions.h>
#endif /* !COMSTL_INCL_COMSTL_UTIL_H_CY_FUNCTIONS */
#ifndef COMSTL_INCL_COMSTL_UTIL_H_DECIMAL_FUNCTIONS
# include <comstl/util/DECIMAL_functions.h>
#endif /* !COMSTL_INCL_COMSTL_UTIL_H_DECIMAL_FUNCTIONS */
#ifndef COMSTL_INCL_COMSTL_UTIL_H_OBJECT_FUNCTIONS
# include <comstl/util/object_functions.h>
#endif /* !COMSTL_INCL_COMSTL_UTIL_H_OBJECT_FUNCTIONS */

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

/** \brief [C only] Indicates whether two VARIANT structures are equal
 *
 * \ingroup group__library__utility__com
 *
 * \param lhs Pointer to the left-hand instances to compare
 * \param lhs Pointer to the right-hand instances to compare
 * \param comparisonSucceeded Pointer to a result-code instance that will have
 *   an HRESULT value not equal to S_OK if the comparison cannot be made. May
 *   be NULL if the caller does not care
 *
 * \return a value indicating whether the values are equal
 * \retval 0 The structures are not equal
 * \retval >0 The structures are equal
 *
 * \pre \c lhs must not be NULL.
 * \pre \c rhs must not be NULL.
 */
STLSOFT_INLINE int comstl__VARIANT_equal(VARIANT const* lhs, VARIANT const* rhs, HRESULT* comparisonSucceeded)
{
    HRESULT comparisonSucceeded_;

    COMSTL_MESSAGE_ASSERT("Cannot pass NULL pointer(s) to VARIANT_compare()", (NULL != lhs && NULL != rhs));

    /* Use the Null Object (Variable) pattern to relieve rest of code
     * of burden of knowing whether value required by caller
     */
    if(NULL == comparisonSucceeded)
    {
        comparisonSucceeded = &comparisonSucceeded_;
    }

    *comparisonSucceeded = S_OK;

    if(COMSTL_ACCESS_VARIANT_vt_BYPTR(lhs) != COMSTL_ACCESS_VARIANT_vt_BYPTR(rhs))
    {
        return 0;
    }
    else
    {
        switch(COMSTL_ACCESS_VARIANT_vt_BYPTR(lhs))
        {
            case    VT_EMPTY:
            case    VT_NULL:
                return 1;
            case    VT_I1:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, cVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, cVal);
            case    VT_UI1:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, bVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, bVal);
            case    VT_I2:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, iVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, iVal);
            case    VT_UI2:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, uiVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, uiVal);
            case    VT_I4:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, lVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, lVal);
            case    VT_UI4:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, ulVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, ulVal);
            case    VT_INT:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, intVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, intVal);
            case    VT_UINT:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, uintVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, uintVal);
            case    VT_R4:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, fltVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, fltVal);
            case    VT_R8:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, dblVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, dblVal);
            case    VT_BOOL:
                return (VARIANT_FALSE != COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, boolVal)) == (VARIANT_FALSE != COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, boolVal));
            case    VT_BSTR:
                return 0 == comstl__bstr_compare(COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, bstrVal), COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, bstrVal));
            case    VT_ERROR:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, scode) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, scode);
            case    VT_DECIMAL:
                return 0 == comstl__DECIMAL_compare(&COMSTL_ACCESS_VARIANT_decVal_BYPTR(lhs), &COMSTL_ACCESS_VARIANT_decVal_BYPTR(rhs));
            case    VT_CY:
                return 0 == comstl__CY_compare(&COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, cyVal), &COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, cyVal));
            case    VT_UNKNOWN:
                return (COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, punkVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, punkVal)) || (S_OK == comstl__is_same_object(COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, punkVal), COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, punkVal)));
            case    VT_DISPATCH:
                return (COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, pdispVal) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, pdispVal)) || (S_OK == comstl__is_same_object(stlsoft_static_cast(LPUNKNOWN, COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, pdispVal)), stlsoft_static_cast(LPUNKNOWN, COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, pdispVal))));
            case    VT_DATE:
                return COMSTL_ACCESS_VARIANT_MEM_BYPTR(lhs, date) == COMSTL_ACCESS_VARIANT_MEM_BYPTR(rhs, date);
#if 0
            case    VT_VARIANT:
            case    VT_RECORD:
#endif /* 0 */
            default:
                *comparisonSucceeded = E_NOTIMPL;
                break;
        }
    }

    return 0;
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

/** \brief [C++ only] Indicates whether two VARIANT structures are equal
 *
 * \ingroup group__library__utility__com
 *
 * \param lhs Pointer to the left-hand instances to compare
 * \param lhs Pointer to the right-hand instances to compare
 * \param comparisonSucceeded Pointer to a result-code instance that will have
 *   an HRESULT value not equal to S_OK if the comparison cannot be made. May
 *   be NULL if the caller does not care
 *
 * \return a value indicating whether the values are equal
 * \retval 0 The structures are not equal
 * \retval >0 The structures are equal
 *
 * \pre \c lhs must not be NULL.
 * \pre \c rhs must not be NULL.
 */
inline bool VARIANT_equal(VARIANT const* lhs, VARIANT const* rhs, HRESULT* comparisonSucceeded)
{
    return 0 != comstl__VARIANT_equal(lhs, rhs, comparisonSucceeded);
}

/** \brief [C++ only] Indicates whether two VARIANT structures are equal
 *
 * \ingroup group__library__utility__com
 *
 * \param lhs Reference to the left-hand instances to compare
 * \param lhs Reference to the right-hand instances to compare
 * \param comparisonSucceeded Pointer to a result-code instance that will have
 *   an HRESULT value not equal to S_OK if the comparison cannot be made. May
 *   be NULL if the caller does not care
 *
 * \return a value indicating whether the values are equal
 * \retval 0 The structures are not equal
 * \retval >0 The structures are equal
 */
inline bool VARIANT_equal(VARIANT const& lhs, VARIANT const& rhs, HRESULT* comparisonSucceeded)
{
    return 0 != comstl__VARIANT_equal(&lhs, &rhs, comparisonSucceeded);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/VARIANT_functions_unittest_.h"
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

#endif /* !COMSTL_INCL_COMSTL_UTIL_H_VARIANT_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
