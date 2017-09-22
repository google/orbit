/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/hdc_scope.hpp (originally MWGdi.h, ::SynesisWin)
 *
 * Purpose:     GDI scope class.
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


/** \file winstl/window/hdc_scope.hpp
 *
 * \brief [C++ only] Definition of the winstl::HDC_scope class
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_HPP_HDC_SCOPE
#define WINSTL_INCL_WINSTL_WINDOW_HPP_HDC_SCOPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_HPP_HDC_SCOPE_MAJOR      3
# define WINSTL_VER_WINSTL_WINDOW_HPP_HDC_SCOPE_MINOR      0
# define WINSTL_VER_WINSTL_WINDOW_HPP_HDC_SCOPE_REVISION   1
# define WINSTL_VER_WINSTL_WINDOW_HPP_HDC_SCOPE_EDIT       43
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
 * Classes
 */

/** \brief Scopes the device context of a window
 *
 * \ingroup group__library__windows_window
 */
class HDC_scope
{
/// \name Types
/// @{
public:
    typedef HDC_scope   class_type;
    typedef HDC (WINAPI *HDC_get_fn_type)(HWND);
/// @}

/// \name Construction
/// @{
public:
    /// Constructor
    ///
    /// \param hdc The device context
    /// \param hwnd The window associated with the device context
    HDC_scope(HDC hdc, HWND hwnd)
        : m_hdc(hdc)
        , m_hwnd(hwnd)
    {
        WINSTL_ASSERT(NULL != hwnd);
    }
    /// Constructor
    ///
    /// \param hwnd The window associated with the device context
    /// \param pfn The function that will retrive the device context from the given window
    HDC_scope(HWND hwnd, HDC_get_fn_type pfn)
        : m_hdc(pfn(hwnd))
        , m_hwnd(hwnd)
    {
        WINSTL_ASSERT(NULL != hwnd);
    }
    /// Releases the device context associated with the window
    ~HDC_scope() stlsoft_throw_0()
    {
        if(NULL != m_hdc)
        {
            ::ReleaseDC(m_hwnd, m_hdc);
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    /// The device context
    HDC get_hdc() const
    {
        return m_hdc;
    }
    /// The window handle
    HWND get_hwnd() const
    {
        return m_hwnd;
    }
/// @}

/// \name Members
/// @{
private:
    HDC     m_hdc;
    HWND    m_hwnd;
/// @}

// Not to be implemented
private:
    HDC_scope(class_type const&);
    class_type& operator =(class_type const&);
};

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

#endif /* WINSTL_INCL_WINSTL_WINDOW_HPP_HDC_SCOPE */

/* ///////////////////////////// end of file //////////////////////////// */
