/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/conversion/windows_type_conversions.hpp (originally MWWinCmn.h, ::SynesisWin)
 *
 * Purpose:     Type conversions for Windows.
 *
 * Created:     1st June 2003
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/conversion/windows_type_conversions.hpp
 *
 * \brief [C++ only] Definition of various type conversions common in
 *  Windows programming
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS
#define WINSTL_INCL_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS_MAJOR    4
# define WINSTL_VER_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS_MINOR    1
# define WINSTL_VER_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS_REVISION 1
# define WINSTL_VER_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS_EDIT     37
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_UNION_CAST
# include <stlsoft/conversion/union_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_UNION_CAST */

/* ////////////////////////////////////////////////////////////////////// */

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

/* ////////////////////////////////////////////////////////////////////// */

/** \brief Checked conversion from <code>char const*</code> to <code>LPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LPARAM, char const*, true>    StrA2LPARAM;
/** \brief Checked conversion from <code>wchar_t const*</code> to <code>LPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LPARAM, wchar_t const*, true> StrW2LPARAM;
/** \brief Checked conversion from <code>TCHAR const*</code> to <code>LPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LPARAM, TCHAR const*, true>   StrT2LPARAM;

/** \brief Checked conversion from <code>WPARAM</code> to <code>HWND</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<HWND, WPARAM, false>          WPARAM2HWND;
/** \brief Checked conversion from <code>LPARAM</code> to <code>HWND</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<HWND, LPARAM, false>          LPARAM2HWND;

/** \brief Checked conversion from <code>HWND</code> to <code>WPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<WPARAM, HWND, false>          HWND2WPARAM;
/** \brief Checked conversion from <code>HWND</code> to <code>LPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LPARAM, HWND, false>          HWND2LPARAM;
/** \brief Checked conversion from <code>HWND</code> to <code>UINT</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<UINT, HWND, false>            HWND2UINT;

/** \brief Checked conversion from <code>WPARAM</code> to <code>int</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<int, WPARAM, false>           WPARAM2int;
/** \brief Checked conversion from <code>int</code> to <code>WPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<WPARAM, int, false>           int2WPARAM;

#if defined(DragQueryFile) || \
    defined(HDROP_DEFINED) || \
    defined(WINSHELLAPI) || \
    defined(_INC_SHELLAPI)
/** \brief Checked conversion from <code>WPARAM</code> to <code>HDROP</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<HDROP, WPARAM, false>         WPARAM2HDROP;
#endif /* HDROP is defined */

/** \brief Checked conversion from <code>void*</code> to <code>WPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<WPARAM, void*, true>          Ptr2WPARAM;
/** \brief Checked conversion from <code>void*</code> to <code>LPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LPARAM, void*, true>          Ptr2LPARAM;
/** \brief Checked conversion from <code>void*</code> to <code>UINT</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<UINT, void*, true>            Ptr2UINT;

/** \brief Checked conversion from <code>int</code> to <code>char const*</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<char const*, int, true>      Id2StrA;
/** \brief Checked conversion from <code>int</code> to <code>wchar_t const*</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<wchar_t const*, int, true>   Id2StrW;
/** \brief Checked conversion from <code>int</code> to <code>TCHAR const*</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<TCHAR const*, int, true>     Id2StrT;

/** \brief Checked conversion from <code>HICON</code> to <code>LPARAM</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LPARAM, HICON, false>         HICON2LPARAM;
/** \brief Checked conversion from <code>LONG</code> to <code>HICON</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<HICON, LONG, false>           LONG2HICON;

/** \brief Checked conversion from <code>HICON</code> to <code>LRESULT</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<LRESULT, HICON, false>        HICON2LRESULT;
/** \brief Checked conversion from <code>LRESULT</code> to <code>HICON</code>.
 * \ingroup group__library__conversion
 */
typedef stlsoft_ns_qual(union_caster)<HICON, LRESULT, false>        LRESULT2HICON;

#if defined(_INC_COMMCTRL) || \
    defined(_COMMCTRL_H)
/** \brief Checked conversion from <code>LPARAM</code> to <code>LPNMHDR</code>.
 * \ingroup group__library__conversion
 */
 typedef union_caster<LPNMHDR, LPARAM, true>                        LPARAM2LPNMHDR;
/** \brief Checked conversion from <code>LONG</code> to <code>HTREEITEM</code>.
 * \ingroup group__library__conversion
 */
 typedef union_caster<HTREEITEM, LONG, false>                       LONG2HTREEITEM;
#endif /* _INC_COMMCTRL || _COMMCTRL_H */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_CONVERSION_HPP_WINDOWS_TYPE_CONVERSIONS */

/* ///////////////////////////// end of file //////////////////////////// */
