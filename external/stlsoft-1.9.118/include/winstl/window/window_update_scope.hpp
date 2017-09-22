/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/window_update_scope.hpp
 *
 * Purpose:     Window update-state scoping class.
 *
 * Created:     5th January 1996
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1996-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/window/window_update_scope.hpp
 *
 * \brief [C++ only] Definition of the winstl::window_update_scope class
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE
#define WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE_MAJOR      5
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE_MINOR      1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE_REVISION   1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE_EDIT       76
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

// window_update_scope
/** \brief Provides scoping of the update-status of a window.
 *
 * \ingroup group__library__windows_window
 *
 * This class provides scoping of the update-status of a window via the Win32
 * function LockWindowUpdate().
 */
class window_update_scope
{
public:
    /// This type
    typedef window_update_scope class_type;

// Construction
public:
    /// Attempts to lock the given window from updates.
    ///
    /// Takes a HWND and changes it's current update-status, which is set back to
    /// the original state in the destructor.
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k W>
    ss_explicit_k window_update_scope(W &wnd)
    {
        HWND hwnd = get_HWND(wnd);

        m_bIsLocked = (hwnd != NULL) && ::LockWindowUpdate(hwnd);
    }
#else
    ss_explicit_k window_update_scope(HWND wnd)
        : m_bIsLocked(wnd ? (::LockWindowUpdate(wnd) ? true : false) : false)
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

    /// Releases the lock, if aquired in the constructor
    ~window_update_scope() stlsoft_throw_0()
    {
        if(m_bIsLocked)
        {
            // Lock was successful, so undo here
            ::LockWindowUpdate(NULL);
        }
    }

// Members
private:
    ws_bool_t   m_bIsLocked;

// Not to be implemented
private:
    window_update_scope(class_type const& rhs);
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

#endif /* WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_UPDATE_SCOPE */

/* ///////////////////////////// end of file //////////////////////////// */
