/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/window_icon_scope.hpp (originally MWIcnScp.h; ::SynesisWin)
 *
 * Purpose:     Window icon scoping class.
 *
 * Created:     30th June 1999
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


/** \file winstl/window/window_icon_scope.hpp
 *
 * \brief [C++ only] Definition of the winstl::window_icon_scope class
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE
#define WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE_MAJOR      4
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE_MINOR      1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE_REVISION   1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE_EDIT       50
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HWND
# include <winstl/shims/attribute/get_HWND.hpp>
#endif /* !WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HWND */
#ifndef WINSTL_INCL_WINSTL_WINDOW_H_FUNCTIONS
# include <winstl/window/functions.h>
#endif /* !WINSTL_INCL_WINSTL_WINDOW_H_FUNCTIONS */

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

// window_icon_scope
/** \brief Provides scoping of the icon(s) of a window.
 *
 * \ingroup group__library__windows_window
 *
 * This class provides scoping of the icon(s) of a window via the WM_SETICON
 * message.
 */
class window_icon_scope
{
public:
    /// This type
    typedef window_icon_scope class_type;

// Construction
public:
    /// \brief Toggles the icon of a window
    ///
    /// Takes a HWND and sets its icon, which is then reset to the original in
    /// the destructor.
    ///
    /// \param wnd The window whose enable state is to be controlled
    /// \param iconType The identifier of the window's icon property to be changed. Should be either \c ICON_BIG or \c ICON_SMALL
    /// \param hicon The handle of the icon to associate with the given window's identified icon property
    window_icon_scope(HWND wnd, int iconType, HICON hicon)
        : m_hwnd(wnd)
        , m_iconId(iconType)
        , m_hicon(set_window_icon(m_hwnd, iconType, hicon))
    {}
    /// \brief Toggles the icon of a window
    ///
    /// Takes a HWND and sets its icon, which is then reset to the original in
    /// the destructor.
    ///
    /// \param wnd The window whose enable state is to be controlled
    /// \param iconType The identifier of the window's icon property to be changed. Should be either \c ICON_BIG or \c ICON_SMALL
    /// \param hinst The module from which the named icon is to be loaded
    /// \param iconName The name of the icon to associate with the given window's identified icon property
    window_icon_scope(HWND wnd, int iconType, HINSTANCE hinst, LPCTSTR iconName)
        : m_hwnd(wnd)
        , m_iconId(iconType)
        , m_hicon(set_window_icon(m_hwnd, iconType, hinst, iconName))
    {}
    /// \brief Toggles the icon of a window
    ///
    /// Takes a HWND and sets its icon, which is then reset to the original in
    /// the destructor.
    ///
    /// \param wnd The window whose enable state is to be controlled
    /// \param iconType The identifier of the window's icon property to be changed. Should be either \c ICON_BIG or \c ICON_SMALL
    /// \param hinst The module from which the named icon is to be loaded
    /// \param iconId The resource identifier of the icon to associate with the given window's identified icon property
    window_icon_scope(HWND wnd, int iconType, HINSTANCE hinst, int iconId)
        : m_hwnd(wnd)
        , m_iconId(iconType)
        , m_hicon(set_window_icon(m_hwnd, iconType, hinst, iconId))
    {}

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// \brief Toggles the window enable state
    ///
    /// Takes a window and sets its icon, which is then reset to the original in
    /// the destructor.
    ///
    /// \param wnd The window whose enable state is to be controlled
    /// \param iconType The identifier of the window's icon property to be changed. Should be either \c ICON_BIG or \c ICON_SMALL
    /// \param hicon The handle of the icon to associate with the given window's identified icon property
    template <ss_typename_param_k W>
    window_icon_scope(W &wnd, int iconType, HICON hicon)
        : m_hwnd(get_HWND(wnd))
        , m_iconId(iconType)
        , m_hicon(set_window_icon(m_hwnd, iconType, hicon))
    {}
    /// \brief Toggles the window enable state
    ///
    /// Takes a window and sets its icon, which is then reset to the original in
    /// the destructor.
    ///
    /// \param wnd The window whose enable state is to be controlled
    /// \param iconType The identifier of the window's icon property to be changed. Should be either \c ICON_BIG or \c ICON_SMALL
    /// \param hinst The module from which the named icon is to be loaded
    /// \param iconName The name of the icon to associate with the given window's identified icon property
    template <ss_typename_param_k W>
    window_icon_scope(W &wnd, int iconType, HINSTANCE hinst, LPCTSTR iconName)
        : m_hwnd(get_HWND(wnd))
        , m_iconId(iconType)
        , m_hicon(set_window_icon(m_hwnd, iconType, hinst, iconName))
    {}
    /// \brief Toggles the window enable state
    ///
    /// Takes a window and sets its icon, which is then reset to the original in
    /// the destructor.
    ///
    /// \param wnd The window whose enable state is to be controlled
    /// \param iconType The identifier of the window's icon property to be changed. Should be either \c ICON_BIG or \c ICON_SMALL
    /// \param hinst The module from which the named icon is to be loaded
    /// \param iconId The resource identifier of the icon to associate with the given window's identified icon property
    template <ss_typename_param_k W>
    window_icon_scope(W &wnd, int iconType, HINSTANCE hinst, int iconId)
        : m_hwnd(get_HWND(wnd))
        , m_iconId(iconType)
        , m_hicon(set_window_icon(m_hwnd, iconType, hinst, iconId))
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

    /// Resets the window's original icon
    ~window_icon_scope() stlsoft_throw_0()
    {
#ifdef STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT
        WINSTL_STATIC_ASSERT(STLSOFT_RAW_OFFSETOF(class_type, m_hwnd) < STLSOFT_RAW_OFFSETOF(class_type, m_hicon));
#endif /* STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT */

        set_window_icon(m_hwnd, m_iconId, m_hicon);
    }

// Members
private:
    HWND    m_hwnd;
    int     m_iconId;
    HICON   m_hicon;

// Not to be implemented
private:
    window_icon_scope(class_type const& rhs);
    class_type const& operator =(class_type const& rhs);
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

#endif /* WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_ICON_SCOPE */

/* ///////////////////////////// end of file //////////////////////////// */
