/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/error/excepinfo_functions.h
 *
 * Purpose:     EXCEPINFO functions.
 *
 * Created:     11th August 2007
 * Updated:     27th April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2007-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/error/excepinfo_functions.h
 *
 * \brief [C, C++] Error info functions
 *   (\ref group__library__error "Error" Library).
 */

#ifndef COMSTL_INCL_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS
#define COMSTL_INCL_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS_MAJOR    1
# define COMSTL_VER_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS_MINOR    0
# define COMSTL_VER_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS_REVISION 3
# define COMSTL_VER_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS_EDIT     4
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

STLSOFT_INLINE void comstl_c_EXCEPINFO_free(EXCEPINFO *pxi)
{
    /* Precondition tests */
    COMSTL_ASSERT(NULL != pxi);

    comstl__bstr_destroy(pxi->bstrSource);
    pxi->bstrSource = NULL;
    comstl__bstr_destroy(pxi->bstrDescription);
    pxi->bstrDescription = NULL;
    comstl__bstr_destroy(pxi->bstrHelpFile);
    pxi->bstrHelpFile = NULL;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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
/** \brief Releases the resources associated with the EXCEPINFO instance.
 *
 * \ingroup group__library__error
 */
inline void EXCEPINFO_free(EXCEPINFO* pxi)
{
    comstl_c_EXCEPINFO_free(pxi);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/excepinfo_functions_unittest_.h"
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

#endif /* !COMSTL_INCL_COMSTL_ERROR_H_EXCEPINFO_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
