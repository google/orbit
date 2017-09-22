/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/CY_functions.h
 *
 * Purpose:     CY helper functions.
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


/** \file comstl/util/CY_functions.h
 *
 * \brief [C++ only; requires COM] CY helper functions
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_H_CY_FUNCTIONS
#define COMSTL_INCL_COMSTL_UTIL_H_CY_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_H_CY_FUNCTIONS_MAJOR       1
# define COMSTL_VER_COMSTL_UTIL_H_CY_FUNCTIONS_MINOR       0
# define COMSTL_VER_COMSTL_UTIL_H_CY_FUNCTIONS_REVISION    1
# define COMSTL_VER_COMSTL_UTIL_H_CY_FUNCTIONS_EDIT        2
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

/** \brief [C only] Compares two CY structures
 *
 * \ingroup group__library__utility__com
 *
 * \param lhs Pointer to the left-hand instances to compare
 * \param lhs Pointer to the right-hand instances to compare
 *
 * \pre \c lhs must not be NULL.
 * \pre \c rhs must not be NULL.
 */
STLSOFT_INLINE int comstl__CY_compare(CY const* lhs, CY const* rhs)
{
    COMSTL_MESSAGE_ASSERT("Cannot pass NULL pointer(s) to CY_compare()", (NULL != lhs && NULL != rhs));

    if(lhs->Hi != rhs->Hi)
    {
        if(lhs->Hi < rhs->Hi)
        {
            return -1;
        }
        else
        {
            return +1;
        }
    }
    else
    {
        if(lhs->Lo < rhs->Lo)
        {
            return -1;
        }
        else
        {
            return +1;
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

/** \brief [C++ only] Compares two CY structures
 *
 * \ingroup group__library__utility__com
 *
 * \param lhs Pointer to the left-hand instances to compare
 * \param lhs Pointer to the right-hand instances to compare
 *
 * \pre \c lhs must not be NULL.
 * \pre \c rhs must not be NULL.
 */
inline int CY_compare(CY const* lhs, CY const* rhs)
{
    return comstl__CY_compare(lhs, rhs);
}

/** \brief [C++ only] Compares two CY structures
 *
 * \ingroup group__library__utility__com
 *
 * \param lhs Reference to the left-hand instances to compare
 * \param lhs Reference to the right-hand instances to compare
 */
inline int CY_compare(CY const& lhs, CY const& rhs)
{
    return comstl__CY_compare(&lhs, &rhs);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/CY_functions_unittest_.h"
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

#endif /* !COMSTL_INCL_COMSTL_UTIL_H_CY_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
