/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/window/about_dialog.hpp
 *
 * Purpose:     Simple 'about' dialog, that shell-executes hyperlinks.
 *
 * Created:     30th January 2000
 * Updated:     9th September 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2000-2010, Matthew Wilson and Synesis Software
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


/** \file atlstl/window/about_dialog.hpp
 *
 * \brief [C++ only; requires ATL library] Definition of the
 *   atlstl::AboutDialog and atlstl::AboutDialogId dialog implementation
 *   class templates
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_WINDOW_HPP_ABOUT_DIALOG
#define ATLSTL_INCL_ATLSTL_WINDOW_HPP_ABOUT_DIALOG

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_WINDOW_HPP_ABOUT_DIALOG_MAJOR      4
# define ATLSTL_VER_ATLSTL_WINDOW_HPP_ABOUT_DIALOG_MINOR      0
# define ATLSTL_VER_ATLSTL_WINDOW_HPP_ABOUT_DIALOG_REVISION   3
# define ATLSTL_VER_ATLSTL_WINDOW_HPP_ABOUT_DIALOG_EDIT       54
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_MALLOC_ALLOCATOR
# include <stlsoft/memory/malloc_allocator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_MALLOC_ALLOCATOR */

#ifndef STLSOFT_INCL_SYS_H_ATLWIN
# define STLSOFT_INCL_SYS_H_ATLWIN
# include <atlwin.h>                         // for CDialogImplBase
#endif /* !STLSOFT_INCL_SYS_H_ATLWIN */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::atlstl */
namespace atlstl
{
# else
/* Define stlsoft::atlstl_project */

namespace stlsoft
{

namespace atlstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Turns an integral value into a type
 *
 * \ingroup group__library__windows_window
 *
 */
template <int N>
struct IDD_to_type
{
    enum { IDD = N };
};

/** \brief About dialog class based on a type that specifies an Id, and a parent window
 * type
 *
 * \ingroup group__library__windows_window
 */
// [[synesis:class:ui-window: atlstl::AboutDialog<T<T>, T<B>>]]
template<
    class   T
,   class   B = CWindow
>
class AboutDialog
    : public CDialogImplBaseT<B>
{
public:
    typedef CDialogImplBaseT<B>         parent_class_type;
    typedef AboutDialog<T, B>           class_type;

public:
    INT_PTR DoModal()
    {
        return this->DoModal(::GetActiveWindow());
    }

    INT_PTR DoModal(HWND hWndParent)
    {
        ATLASSERT(m_hWnd == NULL);

        // Borrow thunking logic from ATL's own CSimpleDialog
        parent_class_type* pThis = this;
        _Module.AddCreateWndData(&m_thunk.cd, pThis);

        int nRet = ::DialogBox(_Module.GetResourceInstance(), MAKEINTRESOURCE(T::IDD), hWndParent, (DLGPROC)StartDialogProc);

        m_hWnd = NULL;

        return nRet;
    }

    BEGIN_MSG_MAP(class_type)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    END_MSG_MAP()

protected:
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);

        return 0;
    }

    LRESULT OnCommand(UINT /* uMsg */, WPARAM /* wParam */, LPARAM lParam, BOOL &bHandled)
    {
        bHandled = false;

        HWND hwndCtrl = (HWND)lParam;

        // 1. Must be a child control
        if(NULL != hwndCtrl)
        {
            //  2. Must be a button
            LRESULT ctrlCode = ::SendMessage(hwndCtrl, WM_GETDLGCODE, 0, 0L);

            if(DLGC_BUTTON & ctrlCode)
            {
                typedef ::stlsoft::auto_buffer_old<
                    TCHAR
                ,   ::stlsoft::malloc_allocator<TCHAR>
                ,   512
                >                               buffer_t;

                // 3. Get text
                //
                // Note that this uses buffer.size(), so that it does not matter, if the buffer
                // allocation fails, whether allocator throws exceptions or returns NULL.
                buffer_t    buffer(1 + ::GetWindowTextLength(hwndCtrl));
                const int   len = ::GetWindowText(hwndCtrl, &buffer[0], buffer.size());

                buffer[len] = '\0';

                // 4. Check whether contains a '.'
                if( 0 < len &&
                    NULL != _tcschr(buffer.data(), '.'))
                {
                    SHELLEXECUTEINFO    sei;

                    sei.cbSize          =   sizeof(sei);
                    sei.fMask           =   SEE_MASK_NOCLOSEPROCESS;
                    sei.hwnd            =   *this;
                    sei.lpVerb          =   _T("open");
                    sei.lpFile          =   buffer.data();
                    sei.lpParameters    =   NULL;
                    sei.lpDirectory     =   NULL;
                    sei.nShow           =   SW_SHOWNORMAL;
                    sei.hInstApp        =   NULL;
                    sei.lpIDList        =   NULL;
                    sei.lpClass         =   NULL;
                    sei.hkeyClass       =   NULL;
                    sei.dwHotKey        =   NULL;
                    sei.hIcon           =   NULL;
                    sei.hProcess        =   NULL;

                    // Execute - ignore failures
                    (void)::ShellExecuteEx(&sei);

                    bHandled = true;
                }
            }
        }

        return 0;
    }
};

/** \brief About dialog class based from an Id
 *
 * \ingroup group__library__windows_window
 *
 */
// [[synesis:class:ui-window: atlstl::AboutDialogId<UINT, T<B>>]]
template<
    UINT    ID
,   class   B = CWindow
>
class AboutDialogId
    : public AboutDialog<IDD_to_type<ID>, B>
{
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace atlstl
# else
} // namespace atlstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* ATLSTL_INCL_ATLSTL_WINDOW_HPP_ABOUT_DIALOG */

/* ///////////////////////////// end of file //////////////////////////// */
