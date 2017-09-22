/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/colour_constants.h (originally MWWinDef.h, ::SynesisWin)
 *
 * Purpose:     Colour constants.
 *
 * Created:     20th February 1999
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/window/colour_constants.h
 *
 * \brief [C, C++] Colour constants
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_H_COLOUR_CONSTANTS
#define WINSTL_INCL_WINSTL_WINDOW_H_COLOUR_CONSTANTS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_H_COLOUR_CONSTANTS_MAJOR      4
# define WINSTL_VER_WINSTL_WINDOW_H_COLOUR_CONSTANTS_MINOR      0
# define WINSTL_VER_WINSTL_WINDOW_H_COLOUR_CONSTANTS_REVISION   2
# define WINSTL_VER_WINSTL_WINDOW_H_COLOUR_CONSTANTS_EDIT       26
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
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
 * Constants
 */

const COLORREF  white           =   RGB(255, 255, 255); /*!< Colour constant */
const COLORREF  black           =   RGB(  0,   0,   0); /*!< Colour constant */
const COLORREF  darkBlue        =   RGB(  0,   0, 128); /*!< Colour constant */
const COLORREF  darkGreen       =   RGB(  0, 128,   0); /*!< Colour constant */
const COLORREF  blueGreen       =   RGB(  0, 128, 128); /*!< Colour constant */
const COLORREF  brown           =   RGB(128,   0,   0); /*!< Colour constant */
const COLORREF  olive           =   RGB(128, 128,   0); /*!< Colour constant */
const COLORREF  purple          =   RGB(128,   0, 128); /*!< Colour constant */
const COLORREF  darkGrey        =   RGB(128, 128, 128); /*!< Colour constant */
const COLORREF  lightGrey       =   RGB(192, 192, 192); /*!< Colour constant */
const COLORREF  blue            =   RGB(  0,   0, 255); /*!< Colour constant */
const COLORREF  green           =   RGB(  0, 255,   0); /*!< Colour constant */
const COLORREF  cyan            =   RGB(  0, 255, 255); /*!< Colour constant */
const COLORREF  red             =   RGB(255,   0,   0); /*!< Colour constant */
const COLORREF  yellow          =   RGB(255, 255,   0); /*!< Colour constant */
const COLORREF  magenta         =   RGB(255,   0, 255); /*!< Colour constant */

const COLORREF  darkblue        =   RGB(  0,   0, 128); /*!< Colour constant */
const COLORREF  darkgreen       =   RGB(  0, 128,   0); /*!< Colour constant */
const COLORREF  bluegreen       =   RGB(  0, 128, 128); /*!< Colour constant */
const COLORREF  darkgrey        =   RGB(128, 128, 128); /*!< Colour constant */
const COLORREF  lightgrey       =   RGB(192, 192, 192); /*!< Colour constant */

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

#endif /* WINSTL_INCL_WINSTL_WINDOW_H_COLOUR_CONSTANTS */

/* ///////////////////////////// end of file //////////////////////////// */
