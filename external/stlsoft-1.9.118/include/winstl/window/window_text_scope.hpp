/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/window/window_text_scope.hpp
 *
 * Purpose:     Window text scoping class.
 *
 * Created:     21st August 2003
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


/** \file winstl/window/window_text_scope.hpp
 *
 * \brief [C++ only] Definition of the winstl::window_text_scope class
 *   (\ref group__library__windows_window "Windows Window" Library).
 */

#ifndef WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE
#define WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE_MAJOR    4
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE_MINOR    1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE_REVISION 1
# define WINSTL_VER_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE_EDIT     39
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
#ifndef WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TRAITS
# include <winstl/window/window_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */

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

// window_text_scope
/** \brief Provides scoping of the text of a window.
 *
 * \ingroup group__library__windows_window
 *
 * This class provides scoping of the text of a window via the API
 * function SetWindowText().
 */
template <ss_typename_param_k S>
class window_text_scope
{
/// \name Member Types
/// @{
public:
    /// The string type
    typedef S                                           string_type;
    /// The type of the current instantiation
    typedef window_text_scope<S>                        class_type;
    /// The character type
    typedef ss_typename_type_k string_type::value_type  char_type;
    /// The traits type
    typedef window_traits<char_type>                    traits_type;
private:
    typedef stlsoft_ns_qual(auto_buffer_old)<   char_type
                                            ,   processheap_allocator<char_type>
                                            >           buffer_type_;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Toggles the window text
    ///
    /// Takes a HWND and changes sets its text, which is set back to
    /// the original in the destructor.
    ///
    /// \param wnd The window whose text is to be modified
    /// \param s The new, temporary, window text
    ss_explicit_k window_text_scope(HWND wnd, char_type const* s)
        : m_hwnd(wnd)
    {
        buffer_type_    buffer(1 + traits_type::get_window_text_length(m_hwnd));

        if(0 < buffer.size()) // Some allocators return NULL, rather than throw
        {
            buffer[buffer.size() - 1] = char_type(0);

            traits_type::get_window_text(m_hwnd, &buffer[0], buffer.size());

            m_oldText = buffer.data();

            traits_type::set_window_text(m_hwnd, s);
        }
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// \brief Toggles the window text
    ///
    /// Takes a HWND and changes sets its text, which is set back to
    /// the original in the destructor.
    ///
    /// \param wnd The window whose text is to be modified
    /// \param s The new, temporary, window text
#if 0
    template <ss_typename_param_k S>
    ss_explicit_k window_text_scope(HWND wnd, S const& s)
        : m_hwnd(wnd)
    {
        buffer_type_    buffer(1 + traits_type::get_window_text_length(m_hwnd));

        if(0 < buffer.size()) // Some allocators return NULL, rather than throw
        {
            buffer[buffer.size() - 1] = char_type(0);

            traits_type::get_window_text(m_hwnd, &buffer[0], buffer.size());

            m_oldText = buffer.data();

            traits_type::set_window_text(m_hwnd, s);
        }
    }
#endif /* 0 */

    /// \brief Toggles the window text
    ///
    /// Takes a HWND and changes sets its text, which is set back to
    /// the original in the destructor.
    ///
    /// \param wnd The window whose text is to be modified
    /// \param s The new, temporary, window text
    template<   ss_typename_param_k W
            ,   ss_typename_param_k S1
            >
    ss_explicit_k window_text_scope(W &wnd, S1 const& s)
        : m_hwnd(get_HWND(wnd))
    {
        buffer_type_    buffer(1 + traits_type::get_window_text_length(m_hwnd));

        if(0 < buffer.size()) // Some allocators return NULL, rather than throw
        {
            buffer[buffer.size() - 1] = char_type(0);

            traits_type::get_window_text(m_hwnd, &buffer[0], buffer.size());

            m_oldText = buffer.data();

            traits_type::set_window_text(m_hwnd, s);
        }
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

    /// Resets the window text
    ~window_text_scope() stlsoft_throw_0()
    {
        traits_type::set_window_text(m_hwnd, m_oldText);
    }
/// @}

/// \name Members
/// @{
private:
    HWND        m_hwnd;
    string_type m_oldText;
/// @}

/// \name Not to be implemented
/// @{
private:
    window_text_scope(class_type const& rhs);
    class_type const& operator =(class_type const& rhs);
/// @}
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

#endif /* WINSTL_INCL_WINSTL_WINDOW_HPP_WINDOW_TEXT_SCOPE */

/* ///////////////////////////// end of file //////////////////////////// */
