/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/console_functions.h
 *
 * Purpose:     Windows console functions.
 *
 * Created:     3rd December 2005
 * Updated:     29th January 2013
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2013, Matthew Wilson and Synesis Software
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


/** \file winstl/system/console_functions.h
 *
 * \brief [C, C++] Windows console functions.
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS
#define WINSTL_INCL_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS_MAJOR     2
# define WINSTL_VER_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS_MINOR     2
# define WINSTL_VER_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS_REVISION  2
# define WINSTL_VER_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS_EDIT      20
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

#if defined(STLSOFT_UNITTEST) && \
    (   !defined(STLSOFT_COMPILER_IS_COMO) && \
        !defined(STLSOFT_COMPILER_IS_WATCOM))
# include <winstl/dl/dl_call.hpp>
#endif /* STLSOFT_UNITTEST */

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
 * Functions
 */

/** \brief Evalutes the current width of the console.
 *
 * \ingroup group__library__system
 */
STLSOFT_INLINE ws_size_t winstl_C_get_console_width(void)
{
    HANDLE hStdOut = STLSOFT_NS_GLOBAL(GetStdHandle)(STD_OUTPUT_HANDLE);

    if(INVALID_HANDLE_VALUE != hStdOut)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if(STLSOFT_NS_GLOBAL(GetConsoleScreenBufferInfo)(hStdOut, &csbi))
        {
            return csbi.dwMaximumWindowSize.X;
        }
    }

#ifdef _DEBUG
    STLSOFT_NS_GLOBAL(GetLastError)();
#endif /* _DEBUG */

    return ~stlsoft_static_cast(ws_size_t, 0);
}

#if !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) && \
    (   !defined(_WIN32_WINNT) || \
        _WIN32_WINNT < 0x0500 || \
        (   defined(STLSOFT_COMPILER_IS_BORLAND) && \
            !defined(CONSOLE_NO_SELECTION)))

STLSOFT_INLINE HWND GetConsoleWindow()
{
    typedef HWND (WINAPI *GCW_t)();

    HMODULE Kernel32    =   STLSOFT_NS_GLOBAL(LoadLibraryA)("KERNEL32");
    GCW_t   pfn         =   stlsoft_reinterpret_cast(GCW_t, STLSOFT_NS_GLOBAL(GetProcAddress)(Kernel32, "GetConsoleWindow"));

    if(NULL == pfn)
    {
        return NULL;
    }
    else
    {
        HWND hwnd = (*pfn)();

        STLSOFT_NS_GLOBAL(FreeLibrary)(Kernel32);

        return hwnd;
    }
}

#else /* ? _WIN32_WINNT */

#endif /* _WIN32_WINNT */

/** Returns the window handle of the current console, or NULL if it cannot
 *    be found
 *
 * \ingroup group__library__system
 *
 * \warning This only works on Windows 2000, or later, operating systems. It
 *    will return NULL on other operating systems.
 */
STLSOFT_INLINE HWND winstl_C_get_console_window(void)
{
    return GetConsoleWindow();
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

/** \def winstl__get_console_window
 *
 * \deprecated Use winstl_C_get_console_window
 */
# define winstl__get_console_window         winstl_C_get_console_window
/** \def winstl__get_console_width
 *
 * \deprecated Use winstl_C_get_console_width
 */
# define winstl__get_console_width          winstl_C_get_console_width

#endif /* obsolete || 1.9 */

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

#if defined(__cplusplus)

/** \brief Evalutes the current width of the console.
 *
 * \ingroup group__library__system
 */
inline ws_size_t get_console_width()
{
    return winstl_C_get_console_width();
}

/** Returns the window handle of the current console, or NULL if it cannot
 *    be found
 *
 * \ingroup group__library__system
 *
 * \warning This only works on Windows 2000, or later, operating systems. It
 *    will return NULL on other operating systems.
 */
inline HWND get_console_window(void)
{
    return winstl_C_get_console_window();
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/console_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* WINSTL_INCL_WINSTL_SYSTEM_H_CONSOLE_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
