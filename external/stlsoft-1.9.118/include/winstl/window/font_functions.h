/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/font_functions.h (originally MWGdi.h / MWWinCmn, ::SynesisWin)
 *
 * Purpose:     Error functions.
 *
 * Created:     20th October 1994
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1994-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/window/font_functions.h
 *
 * \brief [C, C++] Error functions
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_H_FONT_FUNCTIONS
#define WINSTL_INCL_WINSTL_WINDOW_H_FONT_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_H_FONT_FUNCTIONS_MAJOR       4
# define WINSTL_VER_WINSTL_WINDOW_H_FONT_FUNCTIONS_MINOR       0
# define WINSTL_VER_WINSTL_WINDOW_H_FONT_FUNCTIONS_REVISION    1
# define WINSTL_VER_WINSTL_WINDOW_H_FONT_FUNCTIONS_EDIT        137
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_WINDOW_H_MESSAGE_FUNCTIONS
# include <winstl/window/message_functions.h>
#endif /* !WINSTL_INCL_WINSTL_WINDOW_H_MESSAGE_FUNCTIONS */

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

/** \brief Gets the font associated with the window.
 *
 * \ingroup group__library__windows_window
 */
STLSOFT_INLINE HFONT winstl__get_window_font(HWND hwnd)
{
    return stlsoft_reinterpret_cast(HFONT, winstl__SendMessage(hwnd, WM_GETFONT, 0, 0L));
}

/** \brief Sets the window's font.
 *
 * \ingroup group__library__windows_window
 */
STLSOFT_INLINE void winstl__set_window_font(HWND hwnd, HFONT hfont, ws_int_t bRedraw)
{
    stlsoft_static_cast(void, winstl__SendMessage(hwnd, WM_SETFONT, stlsoft_reinterpret_cast(WPARAM, hfont), bRedraw));
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

#if defined(__cplusplus)

/** \brief Gets the font associated with the window.
 *
 * \ingroup group__library__windows_window
 */
inline HFONT get_window_font(HWND hwnd)
{
    return winstl__get_window_font(hwnd);
}

/** \brief Sets the window's font.
 *
 * \ingroup group__library__windows_window
 */
inline void set_window_font(HWND hwnd, HFONT hfont, ws_bool_t bRedraw = true)
{
    winstl__set_window_font(hwnd, hfont, bRedraw);
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

#endif /* WINSTL_INCL_WINSTL_WINDOW_H_FONT_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
