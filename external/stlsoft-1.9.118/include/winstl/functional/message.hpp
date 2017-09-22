/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/functional/message.hpp
 *
 * Purpose:     Window messaging function classes and predicates.
 *
 * Created:     19th January 2001
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2001-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/functional/message.hpp
 *
 * \brief [C++ only] Window messaging function classes and predicates
 *   (\ref group__library__functional "Functional" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FUNCTIONAL_HPP_MESSAGE
#define WINSTL_INCL_WINSTL_FUNCTIONAL_HPP_MESSAGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FUNCTIONAL_HPP_MESSAGE_MAJOR     4
# define WINSTL_VER_WINSTL_FUNCTIONAL_HPP_MESSAGE_MINOR     1
# define WINSTL_VER_WINSTL_FUNCTIONAL_HPP_MESSAGE_REVISION  1
# define WINSTL_VER_WINSTL_FUNCTIONAL_HPP_MESSAGE_EDIT      39
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
#ifndef _WINSTL_WINDOW_FUNCTIONALS_NO_STD
# include <functional>
#else /* ? _WINSTL_WINDOW_FUNCTIONALS_NO_STD */
# error Now need to write that std_binary_function stuff!!
#endif /* _WINSTL_WINDOW_FUNCTIONALS_NO_STD */

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
 * Functor classes
 */

/** \brief Functor used to send a message to windows
 *
 * \ingroup group__library__functional
 *
\code
\endcode
 */
// [[synesis:class:unary-functor: message_send]]
struct message_send
    : public std::unary_function<HWND, void>
{
public:
    /// This type
    typedef message_send        class_type;
public:
    message_send(   UINT    msg
                ,   WPARAM  wparam
                ,   LPARAM  lparam)
        : m_msg(msg)
        , m_wparam(wparam)
        , m_lparam(lparam)
    {}

public:
    void operator ()(HWND hwnd) const
    {
        send_(hwnd);
    }
    template <ss_typename_param_k W>
    void operator ()(W &wnd) const
    {
        send_(winstl_ns_qual(get_HWND)(wnd));
    }

private:
    void send_(HWND hwnd) const
    {
        ::SendMessage(hwnd, m_msg, m_wparam, m_lparam);
    }

private:
    const UINT      m_msg;
    const WPARAM    m_wparam;
    const LPARAM    m_lparam;

private:
    class_type& operator =(class_type const&);
};

/** \brief Functor used to post a message to windows
 *
 * \ingroup group__library__functional
 *
\code
\endcode
 */
// [[synesis:class:unary-functor: message_post]]
struct message_post
    : public std::unary_function<HWND, void>
{
public:
    /// This type
    typedef message_post        class_type;
public:
    message_post(   UINT    msg
                ,   WPARAM  wparam
                ,   LPARAM  lparam)
        : m_msg(msg)
        , m_wparam(wparam)
        , m_lparam(lparam)
    {}

public:
    void operator ()(HWND hwnd) const
    {
        post_(hwnd);
    }
    template <ss_typename_param_k W>
    void operator ()(W &wnd) const
    {
        post_(winstl_ns_qual(get_HWND)(wnd));
    }

private:
    void post_(HWND hwnd) const
    {
        ::PostMessage(hwnd, m_msg, m_wparam, m_lparam);
    }

private:
    const UINT      m_msg;
    const WPARAM    m_wparam;
    const LPARAM    m_lparam;

private:
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

#endif /* WINSTL_INCL_WINSTL_FUNCTIONAL_HPP_MESSAGE */

/* ///////////////////////////// end of file //////////////////////////// */
