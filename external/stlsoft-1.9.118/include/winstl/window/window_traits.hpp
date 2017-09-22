/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/window_traits.hpp
 *
 * Purpose:     Contains the window_traits template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     24th August 2003
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


/** \file winstl/window/window_traits.hpp
 *
 * \brief [C++ only] Definition of the winstl::window_traits traits class
 *   template, and ANSI and Unicode specialisations thereof
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TRAITS
#define WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TRAITS_MAJOR    4
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TRAITS_MINOR    1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TRAITS_REVISION 1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TRAITS_EDIT     31
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
# include <winstl/shims/access/string.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */
#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS
# include <winstl/system/system_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS */

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

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \brief Traits for accessing the correct registry functions for a given character type
 *
 * \ingroup group__library__windows_window
 *
 * window_traits is a traits class for determining the correct window manipulation
 * structures and functions for a given character type.
 */
template <ss_typename_param_k C>
struct window_traits
    : public system_traits<C>
{
/// \name Member Types
/// @{
public:
    /// The character type
    typedef C               char_type;
    /// The size type
    typedef ws_size_t       size_type;
    /// The difference type
    typedef ws_ptrdiff_t    difference_type;
/// @}

/// \name Window functions
/// @{
public:
    /// Gets the number of characters of text for the given window
    static ws_int_t     get_window_text_length(HWND hwnd);

    /// Retrieves the text for the given window
    static ws_int_t     get_window_text(HWND hwnd, char_type* buffer, ws_int_t cchBuff);

    /// Sets the text for the given window
    static ws_bool_t    set_window_text(HWND hwnd, char_type const* buffer);
/// @}
};

#else

template <ss_typename_param_k C>
struct window_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct window_traits<ws_char_a_t>
    : public system_traits<ws_char_a_t>
{
public:
    typedef window_traits<ws_char_a_t>  class_type;
    typedef ws_char_a_t                 char_type;
    typedef ws_size_t                   size_type;
    typedef ws_ptrdiff_t                difference_type;

public:
    static ws_int_t get_window_text_length(HWND hwnd)
    {
        return ::GetWindowTextLength(hwnd);
    }

    static ws_int_t get_window_text(HWND hwnd, char_type* buffer, ws_int_t cchBuff)
    {
        return ::GetWindowTextA(hwnd, buffer, cchBuff);
    }

    static ws_bool_t set_window_text(HWND hwnd, char_type const* s)
    {
        return 0 != ::SetWindowTextA(hwnd, s);
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    static ws_bool_t set_window_text(HWND hwnd, S const& s)
    {
        return class_type::set_window_text(hwnd, stlsoft_ns_qual(c_str_ptr_a)(s));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
};

STLSOFT_TEMPLATE_SPECIALISATION
struct window_traits<ws_char_w_t>
    : public system_traits<ws_char_w_t>
{
public:
    typedef window_traits<ws_char_w_t>  class_type;
    typedef ws_char_w_t                 char_type;
    typedef ws_size_t                   size_type;
    typedef ws_ptrdiff_t                difference_type;

public:
    static ws_int_t get_window_text_length(HWND hwnd)
    {
        return ::GetWindowTextLength(hwnd);
    }

    static ws_int_t get_window_text(HWND hwnd, char_type* buffer, ws_int_t cchBuff)
    {
        return ::GetWindowTextW(hwnd, buffer, cchBuff);
    }

    static ws_bool_t set_window_text(HWND hwnd, char_type const* s)
    {
        return 0 != ::SetWindowTextW(hwnd, s);
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    static ws_bool_t set_window_text(HWND hwnd, S const& s)
    {
        return class_type::set_window_text(hwnd, stlsoft_ns_qual(c_str_ptr_w)(s));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
};

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
