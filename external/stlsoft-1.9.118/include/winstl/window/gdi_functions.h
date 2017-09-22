/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/gdi_functions.h (originally MWGdi.h / MWWinCmn, ::SynesisWin)
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


/** \file winstl/window/gdi_functions.h
 *
 * \brief [C, C++] GDI functions
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_H_GDI_FUNCTIONS
#define WINSTL_INCL_WINSTL_WINDOW_H_GDI_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_H_GDI_FUNCTIONS_MAJOR    4
# define WINSTL_VER_WINSTL_WINDOW_H_GDI_FUNCTIONS_MINOR    0
# define WINSTL_VER_WINSTL_WINDOW_H_GDI_FUNCTIONS_REVISION 2
# define WINSTL_VER_WINSTL_WINDOW_H_GDI_FUNCTIONS_EDIT     136
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

STLSOFT_INLINE ws_size_t winstl__get_BITMAP(HBITMAP hbmp, BITMAP *pbmp)
{
    return stlsoft_static_cast(ws_size_t, STLSOFT_NS_GLOBAL(GetObject)(hbmp, sizeof(BITMAP), pbmp));
}

STLSOFT_INLINE ws_size_t winstl__get_DIBSECTION(HBITMAP hbmp, DIBSECTION *pds)
{
    return stlsoft_static_cast(ws_size_t, STLSOFT_NS_GLOBAL(GetObject)(hbmp, sizeof(DIBSECTION), pds));
}

STLSOFT_INLINE ws_size_t winstl__get_LOGPEN(HPEN hpen, LOGPEN *plp)
{
    return stlsoft_static_cast(ws_size_t, STLSOFT_NS_GLOBAL(GetObject)(hpen, sizeof(LOGPEN), plp));
}

STLSOFT_INLINE ws_size_t winstl__get_EXTLOGPEN(HPEN hpen, EXTLOGPEN *pxlp)
{
    return stlsoft_static_cast(ws_size_t, STLSOFT_NS_GLOBAL(GetObject)(hpen, sizeof(EXTLOGPEN), pxlp));
}

STLSOFT_INLINE ws_size_t winstl__get_LOGBRUSH(HBRUSH hpal, LOGBRUSH *pbr)
{
    return stlsoft_static_cast(ws_size_t, STLSOFT_NS_GLOBAL(GetObject)(hpal, sizeof(LOGBRUSH), pbr));
}

STLSOFT_INLINE ws_size_t winstl__get_LOGFONT(HFONT hpal, LOGFONT *pfnt)
{
    return stlsoft_static_cast(ws_size_t, STLSOFT_NS_GLOBAL(GetObject)(hpal, sizeof(LOGFONT), pfnt));
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

inline ws_size_t get_object(HBITMAP h, BITMAP &desc)
{
    return winstl__get_BITMAP(h, &desc);
}

inline ws_size_t get_object(HBITMAP h, DIBSECTION &desc)
{
    return winstl__get_DIBSECTION(h, &desc);
}

inline ws_size_t get_object(HPEN h, EXTLOGPEN &desc)
{
    return winstl__get_EXTLOGPEN(h, &desc);
}

inline ws_size_t get_object(HPEN h, LOGPEN &desc)
{
    return winstl__get_LOGPEN(h, &desc);
}

inline ws_size_t get_object(HBRUSH h, LOGBRUSH &desc)
{
    return winstl__get_LOGBRUSH(h, &desc);
}

inline ws_size_t get_object(HFONT h, LOGFONT &desc)
{
    return winstl__get_LOGFONT(h, &desc);
}

template<   ss_typename_param_k D
        ,   ss_typename_param_k H
        >
inline D get_object_helper(H h)
{
    D   d;

    return get_object(h, d) ? d : D();
}

inline BITMAP get_BITMAP(HBITMAP h)
{
    return get_object_helper<BITMAP>(h);
}

inline DIBSECTION get_DIBSECTION(HBITMAP h)
{
    return get_object_helper<DIBSECTION>(h);
}

inline EXTLOGPEN get_EXTLOGPEN(HPEN h)
{
    return get_object_helper<EXTLOGPEN>(h);
}

inline LOGPEN get_LOGPEN(HPEN h)
{
    return get_object_helper<LOGPEN>(h);
}

inline LOGBRUSH get_LOGBRUSH(HBRUSH h)
{
    return get_object_helper<LOGBRUSH>(h);
}

inline LOGFONT get_LOGFONT(HFONT h)
{
    return get_object_helper<LOGFONT>(h);
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

#endif /* WINSTL_INCL_WINSTL_WINDOW_H_GDI_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
