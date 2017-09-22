/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/shims/access/string/cwindow.hpp
 *
 * Purpose:     Contains classes and functions for dealing with OLE/COM strings.
 *
 * Created:     27th May 2002
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


/** \file atlstl/shims/access/string/cwindow.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>CWindow</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW
#define ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW_MAJOR    4
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW_MINOR    0
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW_REVISION 3
# define ATLSTL_VER_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW_EDIT     102
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CSTRING_MAKER
# include <stlsoft/string/cstring_maker.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CSTRING_MAKER */

#ifdef STLSOFT_UNITTEST
# include <wchar.h>
#endif /* STLSOFT_UNITTEST */
#ifndef STLSOFT_INCL_SYS_H_ATLWIN
# define STLSOFT_INCL_SYS_H_ATLWIN
# include <atlwin.h>
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

/* CWindow */

/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr_null() function, such that the window text of a given window
 * may be accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
struct c_str_ptr_null_CWindow_proxy
{
private:
    typedef cstring_maker<TCHAR>            string_maker_type;
    typedef c_str_ptr_null_CWindow_proxy    class_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given CWindow instance
    ///
    /// \param w The CWindow instance from which the text will be retrieved
    ss_explicit_k c_str_ptr_null_CWindow_proxy(CWindow const& w)
    {
        int length  =   (NULL == w.m_hWnd) ? 0 : w.GetWindowTextLength();

        if(length == 0)
        {
            m_buffer = NULL;
        }
        else
        {
            m_buffer = string_maker_type::alloc(length);

            if(NULL != m_buffer)
            {
                w.GetWindowText(m_buffer, length + 1);
            }
        }
    }

#ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
    /// Move constructor
    ///
    /// This <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">move constructor</a>
    /// is for circumstances when the compiler does not, or cannot, apply the
    /// return value optimisation. It causes the contents of \c rhs to be
    /// transferred into the constructing instance. This is completely safe
    /// because the \c rhs instance will never be accessed in its own right, so
    /// does not need to maintain ownership of its contents.
    c_str_ptr_null_CWindow_proxy(class_type& rhs)
        : m_buffer(rhs.m_buffer)
    {
        rhs.m_buffer = 0;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_ptr_null_CWindow_proxy(class_type const& rhs)
        : m_buffer(string_maker_type::dup_null(rhs.m_buffer))
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_ptr_null_CWindow_proxy() stlsoft_throw_0()
    {
        string_maker_type::free(m_buffer);
    }

// Accessors
public:
    /// Returns a null-terminated string representing the window contents, or
    /// NULL if the window contains no text.
    operator LPCTSTR () const
    {
        return m_buffer;
    }

// Members
private:
    LPTSTR  m_buffer;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr() function, such that the window text of a given window may be
 * accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
struct c_str_ptr_CWindow_proxy
{
private:
    typedef cstring_maker<TCHAR>        string_maker_type;
    typedef c_str_ptr_CWindow_proxy     class_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given CWindow instance
    ///
    /// \param w The CWindow instance from which the text will be retrieved
    ss_explicit_k c_str_ptr_CWindow_proxy(CWindow const& w)
    {
        int length  =   (NULL == w.m_hWnd) ? 0 : w.GetWindowTextLength();

        if(length == 0)
        {
            m_buffer = string_maker_type::dup("");
        }
        else
        {
            m_buffer = string_maker_type::alloc(length);

            if(NULL != m_buffer)
            {
                w.GetWindowText(m_buffer, length + 1);
            }
        }
    }

#ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
    /// Move constructor
    ///
    /// This <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">move constructor</a>
    /// is for circumstances when the compiler does not, or cannot, apply the
    /// return value optimisation. It causes the contents of \c rhs to be
    /// transferred into the constructing instance. This is completely safe
    /// because the \c rhs instance will never be accessed in its own right, so
    /// does not need to maintain ownership of its contents.
    c_str_ptr_CWindow_proxy(class_type& rhs)
        : m_buffer(rhs.m_buffer)
    {
        rhs.m_buffer = 0;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_ptr_CWindow_proxy(class_type const& rhs)
        : m_buffer(string_maker_type::dup_null(rhs.m_buffer))
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_ptr_CWindow_proxy() stlsoft_throw_0()
    {
        string_maker_type::free(m_buffer);
    }

// Accessors
public:
    /// Returns a null-terminated string representing the window contents, or
    /// the empty string "" if the window contains no text.
    operator LPCTSTR () const
    {
        return (NULL == m_buffer) ? "" : m_buffer;
    }

// Members
private:
    LPTSTR  m_buffer;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

/* /////////////////////////////////////////////////////////////////////////
 * Equivalence testing
 */

inline as_bool_t operator ==(LPCTSTR lhs, c_str_ptr_null_CWindow_proxy const& rhs)
{
    return lhs == static_cast<LPCTSTR>(rhs);
}

inline as_bool_t operator ==(c_str_ptr_null_CWindow_proxy const& lhs, LPCTSTR rhs)
{
    return static_cast<LPCTSTR>(lhs) == rhs;
}

inline as_bool_t operator !=(LPCTSTR lhs, c_str_ptr_null_CWindow_proxy const& rhs)
{
    return lhs != static_cast<LPCTSTR>(rhs);
}

inline as_bool_t operator !=(c_str_ptr_null_CWindow_proxy const& lhs, LPCTSTR rhs)
{
    return static_cast<LPCTSTR>(lhs) != rhs;
}

/* /////////////////////////////////////////////////////////////////////////
 * IOStream compatibility
 */

template<ss_typename_param_k S>
inline S& operator <<(S& s, c_str_ptr_null_CWindow_proxy const& shim)
{
    s << static_cast<LPCTSTR>(shim);

    return s;
}

template <ss_typename_param_k S>
inline S& operator <<(S& s, c_str_ptr_CWindow_proxy const& shim)
{
    s << static_cast<LPCTSTR>(shim);

    return s;
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* CWindow */

/** \brief \ref group__concept__shim__string_access__c_str_data for CWindow
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline c_str_ptr_CWindow_proxy c_str_data(atlstl_ns_qual_atl(CWindow) const& w)
{
    return c_str_ptr_CWindow_proxy(w);
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifdef UNICODE
inline c_str_ptr_CWindow_proxy c_str_data_w(atlstl_ns_qual_atl(CWindow) const& w)
# else /* ? UNICODE */
inline c_str_ptr_CWindow_proxy c_str_data_a(atlstl_ns_qual_atl(CWindow) const& w)
# endif /* UNICODE */
{
    return c_str_data(w);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number
 * of characters in the character string in the expression.
 */

/* CWindow */

/** \brief \ref group__concept__shim__string_access__c_str_len for CWindow
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline as_size_t c_str_len(atlstl_ns_qual_atl(CWindow) const& w)
{
    return static_cast<as_size_t>(w.GetWindowTextLength());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifdef UNICODE
inline as_size_t c_str_len_w(atlstl_ns_qual_atl(CWindow) const& w)
# else /* ? UNICODE */
inline as_size_t c_str_len_a(atlstl_ns_qual_atl(CWindow) const& w)
# endif /* UNICODE */
{
    return c_str_len(w);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* CWindow */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for CWindow
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline c_str_ptr_CWindow_proxy c_str_ptr(atlstl_ns_qual_atl(CWindow) const& w)
{
    return c_str_ptr_CWindow_proxy(w);
}

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
#  ifdef UNICODE
inline c_str_ptr_CWindow_proxy c_str_ptr_w(atlstl_ns_qual_atl(CWindow) const& w)
#  else /* ? UNICODE */
inline c_str_ptr_CWindow_proxy c_str_ptr_a(atlstl_ns_qual_atl(CWindow) const& w)
#  endif /* UNICODE */
{
    return c_str_ptr(w);
}
# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/* CWindow */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for CWindow
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline c_str_ptr_null_CWindow_proxy c_str_ptr_null(atlstl_ns_qual_atl(CWindow) const& w)
{
    return c_str_ptr_null_CWindow_proxy(w);
}

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
#  ifdef UNICODE
inline c_str_ptr_null_CWindow_proxy c_str_ptr_null_w(atlstl_ns_qual_atl(CWindow) const& w)
#  else /* ? UNICODE */
inline c_str_ptr_null_CWindow_proxy c_str_ptr_null_a(atlstl_ns_qual_atl(CWindow) const& w)
#  endif /* UNICODE */
{
    return c_str_ptr_null(w);
}
# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/cwindow_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace atlstl
# else
} // namespace stlsoft::atlstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _ATLSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::atlstl::c_str_data;
#if defined(UNICODE)
using ::atlstl::c_str_data_w;
#else /* ? UNICODE*/
using ::atlstl::c_str_data_a;
#endif /* UNICODE*/

using ::atlstl::c_str_len;
#if defined(UNICODE)
using ::atlstl::c_str_len_w;
#else /* ? UNICODE*/
using ::atlstl::c_str_len_a;
#endif /* UNICODE*/

using ::atlstl::c_str_ptr;
#if defined(UNICODE)
using ::atlstl::c_str_ptr_w;
#else /* ? UNICODE*/
using ::atlstl::c_str_ptr_a;
#endif /* UNICODE*/

using ::atlstl::c_str_ptr_null;
#if defined(UNICODE)
using ::atlstl::c_str_ptr_null_w;
#else /* ? UNICODE*/
using ::atlstl::c_str_ptr_null_a;
#endif /* UNICODE*/

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !ATLSTL_INCL_ATLSTL_SHIMS_ACCESS_STRING_HPP_CWINDOW */

/* ///////////////////////////// end of file //////////////////////////// */
