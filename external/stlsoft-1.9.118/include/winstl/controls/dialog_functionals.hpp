/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/controls/dialog_functionals.hpp
 *
 * Purpose:     Functionals for application to controls.
 *
 * Created:     2nd August 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/controls/dialog_functionals.hpp
 *
 * \brief [C++] Functionals for application to dialog controls
 *   (\ref group__library__windows_controls "Windows Controls" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS
#define WINSTL_INCL_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS_MAJOR     1
# define WINSTL_VER_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS_MINOR     0
# define WINSTL_VER_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS_REVISION  2
# define WINSTL_VER_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS_EDIT      8
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:  __GNUC__<3
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1100
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

#if defined(STLSOFT_COMPILER_IS_GCC) && \
    __GNUC__ < 3
# error winstl/controls/dialog_functionals.hpp is not compatible with GNU C++ prior to 3.0
#endif /* compiler */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1100
# error winstl/controls/dialog_functionals.hpp is not compatible with Visual C++ 4.2 or earlier
#endif /* compiler */

#ifndef WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HWND
# include <winstl/shims/attribute/get_HWND.hpp>
#endif /* !WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HWND */
#ifndef _WINSTL_CONTROL_FUNCTIONALS_NO_STD
# include <functional>
#else /* ? _WINSTL_CONTROL_FUNCTIONALS_NO_STD */
# error Now need to write that std_binary_function stuff!!
#endif /* _WINSTL_CONTROL_FUNCTIONALS_NO_STD */

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

/** \brief Functor used to (un)check buttons
 *
 * \ingroup group__library__windows_controls
 */
// [[synesis:class:unary-functor: dialog_button_id_check]]
class dialog_button_id_check
    : public stlsoft_ns_qual_std(unary_function)<HWND, void>
{
public:
    /// This type
    typedef dialog_button_id_check    class_type;
public:
    ss_explicit_k dialog_button_id_check(HWND hwndDlg, UINT nCheck = BST_CHECKED)
        : m_hwndDlg(hwndDlg)
        , m_nCheck(nCheck)
    {}
    dialog_button_id_check(dialog_button_id_check const& rhs)
        : m_hwndDlg(rhs.m_hwndDlg)
        , m_nCheck(rhs.m_nCheck)
    {}

public:
    void operator ()(int id) const
    {
        check_(id, m_nCheck);
    }
private:
    void check_(int id, UINT nCheck) const
    {
        ::CheckDlgButton(m_hwndDlg, id, nCheck);
    }

private:
    HWND        m_hwndDlg;
    const UINT  m_nCheck;

private:
    class_type& operator =(class_type const&);
};


////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/dialog_functionals_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* WINSTL_INCL_WINSTL_CONTROL_HPP_DIALOG_FUNCTIONALS */

/* ///////////////////////////// end of file //////////////////////////// */
