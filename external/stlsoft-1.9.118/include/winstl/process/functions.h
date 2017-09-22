/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/process/functions.h
 *
 * Purpose:     Process functions.
 *
 * Created:     12th March 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/process/functions.h
 *
 * \brief [C, C++] Process control functions
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_PROCESS_H_FUNCTIONS
#define WINSTL_INCL_WINSTL_PROCESS_H_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_PROCESS_H_FUNCTIONS_MAJOR    1
# define WINSTL_VER_WINSTL_PROCESS_H_FUNCTIONS_MINOR    0
# define WINSTL_VER_WINSTL_PROCESS_H_FUNCTIONS_REVISION 5
# define WINSTL_VER_WINSTL_PROCESS_H_FUNCTIONS_EDIT     18
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(_WINSTL_NO_NAMESPACE) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if defined(_STLSOFT_NO_NAMESPACE)
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
 * C functions
 */

/**
 *
 * \ingroup group__library__system
 */
STLSOFT_INLINE BOOL winstl__CreateProcessFEA(ws_char_a_t const* cmdLine, DWORD flags, void const* envBlock)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    BOOL                b;

    STLSOFT_NS_GLOBAL(memset)(&si, 0, sizeof(si));

    b = STLSOFT_NS_GLOBAL(CreateProcessA)(NULL, stlsoft_const_cast(ws_char_a_t*, cmdLine), NULL, NULL, FALSE, flags, stlsoft_const_cast(void*, envBlock), NULL, &si, &pi);

    if(b)
    {
        STLSOFT_NS_GLOBAL(CloseHandle)(pi.hProcess);
        STLSOFT_NS_GLOBAL(CloseHandle)(pi.hThread);
    }

    return b;
}

/**
 *
 * \ingroup group__library__system
 */
STLSOFT_INLINE BOOL winstl__CreateProcessEA(ws_char_a_t const* cmdLine, void const* envBlock)
{
    return winstl__CreateProcessFEA(cmdLine, 0, envBlock);
}

/**
 *
 * \ingroup group__library__system
 */
STLSOFT_INLINE BOOL winstl__CreateProcess0A(ws_char_a_t const* cmdLine)
{
    return winstl__CreateProcessEA(cmdLine, NULL);
}

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace winstl
{
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * C++ functions
 */

#ifdef __cplusplus

/**
 *
 * \ingroup group__library__system
 */
inline BOOL CreateProcess(ws_char_a_t const* cmdLine, DWORD flags, void const* envBlock)
{
    return winstl__CreateProcessFEA(cmdLine, flags, envBlock);
}

/**
 *
 * \ingroup group__library__system
 */
inline BOOL CreateProcess(ws_char_a_t const* cmdLine, void const* envBlock)
{
    return winstl__CreateProcessEA(cmdLine, envBlock);
}

/**
 *
 * \ingroup group__library__system
 */
inline BOOL CreateProcess(ws_char_a_t const* cmdLine)
{
    return winstl__CreateProcess0A(cmdLine);
}

#endif /* __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace winstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_PROCESS_H_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
