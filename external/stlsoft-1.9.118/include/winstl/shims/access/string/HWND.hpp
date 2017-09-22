/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/access/string/HWND.hpp
 *
 * Purpose:     Contains classes and functions for dealing with Win32 strings.
 *
 * Created:     24th May 2002
 * Updated:     22nd December 2009
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


/** \file winstl/shims/access/string/HWND.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>HWND</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND
#define WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND_MAJOR       4
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND_MINOR       1
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND_REVISION    1
# define WINSTL_VER_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND_EDIT        113
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
#ifndef WINSTL_INCL_WINSTL_WINDOW_H_FUNCTIONS
# include <winstl/window/functions.h>
#endif /* !WINSTL_INCL_WINSTL_WINDOW_H_FUNCTIONS */
#ifndef WINSTL_INCL_WINSTL_WINDOW_UTIL_HPP_IDENT_
# include <winstl/window/util/ident_.hpp>
#endif /* !WINSTL_INCL_WINSTL_WINDOW_UTIL_HPP_IDENT_ */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CSTRING_MAKER
# include <stlsoft/string/cstring_maker.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CSTRING_MAKER */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

#ifdef NOUSER
# error This file cannot be used when NOUSER is specified (to suppress the Windows User API)
#endif /* NOUSER */
#ifdef NOWINOFFSETS
# error This file cannot be used when NOWINOFFSETS is specified (to suppress GWL_*, GCL_*, associated routines)
#endif /* NOWINOFFSETS */

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
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ws_size_t GetWindowTextLength_T_(HWND hwnd, int (WINAPI *pfn)(HWND ))
{
    WINSTL_ASSERT(NULL != pfn);

    WindowIdent ident       =   GetWindowIdent(hwnd);
    int         sel;
# ifndef NOWINSTYLES
    const long  lbsStyle    =   LBS_MULTIPLESEL | LBS_EXTENDEDSEL;
# else /* ? NOWINSTYLES */
    const long  lbsStyle    =   0x0008L | 0x0800L;
# endif /* NOWINSTYLES */

    switch(ident)
    {
        case    ListBox:
            if(0 == (GetStyle(hwnd) & lbsStyle))
            {
                sel = static_cast<int>(::SendMessage(hwnd, LB_GETCURSEL, 0, 0l));

                if(LB_ERR != sel)
                {
                    return static_cast<ws_size_t>(::SendMessage(hwnd, LB_GETTEXTLEN, static_cast<WPARAM>(sel), 0L));
                }
                else
                {
                    return 0;
                }
            }
            break;
#if 0
        case    ListBox:
            if(1 == SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L))
            {
                sel =
            }
            break;
#endif /* 0 */
        case    Generic:
        case    ComboBox:
        case    ListView:
        default:
            break;
    }

    return static_cast<ws_size_t>(pfn(hwnd));
}

inline ws_size_t GetWindowTextLength_T_(HWND hwnd)
{
    return GetWindowTextLength_T_(hwnd, ::GetWindowTextLength);
}
inline ws_size_t GetWindowTextLength_A_(HWND hwnd)
{
    return GetWindowTextLength_T_(hwnd, ::GetWindowTextLengthA);
}
inline ws_size_t GetWindowTextLength_W_(HWND hwnd)
{
    return GetWindowTextLength_T_(hwnd, ::GetWindowTextLengthW);
}

template <ss_typename_param_k C>
struct WindowTextLength_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct WindowTextLength_traits<ws_char_a_t>
{
    static ws_size_t get_length(HWND hwnd)
    {
        return GetWindowTextLength_A_(hwnd);
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct WindowTextLength_traits<ws_char_w_t>
{
    static ws_size_t get_length(HWND hwnd)
    {
        return GetWindowTextLength_W_(hwnd);
    }
};

inline ws_size_t GetWindowText_A_(HWND hwnd, ws_char_a_t *buffer, ws_size_t cchBuffer)
{
    WindowIdent ident   =   GetWindowIdent(hwnd);
    int         sel;
    ws_size_t   cch;

    switch(ident)
    {
        case    ListBox:
            if(0 == (GetStyle(hwnd) & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)))
            {
                sel = static_cast<int>(::SendMessage(hwnd, LB_GETCURSEL, 0, 0l));

                if(LB_ERR != sel)
                {
                    cch =   static_cast<ws_size_t>(::SendMessage(hwnd, LB_GETTEXT, static_cast<WPARAM>(sel), reinterpret_cast<LPARAM>(buffer)));

                    // Some programs using list-boxes do not null-terminate - Visual
                    // SourceSafe Explorer, anyone? - so we must do so here.
                    buffer[cch] = '\0';
                }
                else
                {
                    buffer[0] = '\0';

                    cch = 0;
                }

                WINSTL_MESSAGE_ASSERT("Buffer overwrite", !(cchBuffer < cch));

                return cch;
            }
            break;
        case    Generic:
        case    ComboBox:
        case    ListView:
        default:
            break;
    }

    return static_cast<ws_size_t>(::GetWindowTextA(hwnd, buffer, static_cast<int>(cchBuffer)));
}

inline ws_size_t GetWindowText_W_(HWND hwnd, ws_char_w_t *buffer, ws_size_t cchBuffer)
{
    WindowIdent ident   =   GetWindowIdent(hwnd);
    int         sel;

    switch(ident)
    {
        case    ListBox:
            if(0 == (GetStyle(hwnd) & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)))
            {
                ws_size_t  cch;

                sel = static_cast<int>(::SendMessage(hwnd, LB_GETCURSEL, 0, 0l));

                if(LB_ERR != sel)
                {
                    cch =   static_cast<ws_size_t>(::SendMessage(hwnd, LB_GETTEXT, static_cast<WPARAM>(sel), reinterpret_cast<LPARAM>(buffer)));
                }
                else
                {
                    buffer[0] = '\0';

                    cch = 0;
                }

                WINSTL_MESSAGE_ASSERT("Buffer overwrite", !(cchBuffer < cch));

                return cch;
            }
            break;
        case    Generic:
        case    ComboBox:
        case    ListView:
        default:
            break;
    }

    return static_cast<ws_size_t>(::GetWindowTextW(hwnd, buffer, static_cast<int>(cchBuffer)));
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/* HWND */
/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr_null() function, such that the window text of a given window may be
 * accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k C>
class c_str_ptr_null_HWND_proxy
{
    typedef cstring_maker<C>                            string_maker_type;
public:
    /// The character type
    typedef C                                           char_type;
    /// This type
    typedef c_str_ptr_null_HWND_proxy<C>                class_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given HWND
    ///
    /// \param h The HWND from which the text will be retrieved
    ss_explicit_k c_str_ptr_null_HWND_proxy(HWND h)
    {
        ws_size_t length  =   WindowTextLength_traits<C>::get_length(h);

        if(length == 0)
        {
            m_buffer = NULL;
        }
        else
        {
            m_buffer = string_maker_type::alloc(length);

            if(NULL != m_buffer)
            {
                get_window_text(h, m_buffer, length + 1);
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
    c_str_ptr_null_HWND_proxy(class_type& rhs)
        : m_buffer(rhs.m_buffer)
    {
        move_lhs_from_rhs(rhs).m_buffer = NULL;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_ptr_null_HWND_proxy(class_type const& rhs)
        : m_buffer(string_maker_type::dup_null(rhs.m_buffer))
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_ptr_null_HWND_proxy() stlsoft_throw_0()
    {
        string_maker_type::free(m_buffer);
    }

// Accessors
public:
    /// Returns a null-terminated string representing the window contents, or
    /// the empty string "" if the window contains no text.
    operator char_type const* () const
    {
        return m_buffer;
    }

// Implementation
private:
    ws_size_t get_window_text(HWND h, char_type* buffer, ws_size_t cchBuffer);

// Members
private:
    char_type   *m_buffer;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
inline ws_size_t c_str_ptr_null_HWND_proxy<ws_char_a_t>::get_window_text(HWND h, ws_char_a_t *buffer, ws_size_t cchBuffer)
{
    return GetWindowText_A_(h, buffer, cchBuffer);
}

STLSOFT_TEMPLATE_SPECIALISATION
inline ws_size_t c_str_ptr_null_HWND_proxy<ws_char_w_t>::get_window_text(HWND h, ws_char_w_t *buffer, ws_size_t cchBuffer)
{
    return GetWindowText_W_(h, buffer, cchBuffer);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr() function, such that the window text of a given window may be
 * accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k C>
class c_str_ptr_HWND_proxy
{
    typedef cstring_maker<C>                            string_maker_type;
public:
    /// The character type
    typedef C                                           char_type;
    /// This type
    typedef c_str_ptr_HWND_proxy<C>                     class_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given HWND
    ///
    /// \param h The HWND from which the text will be retrieved
    ss_explicit_k c_str_ptr_HWND_proxy(HWND h)
    {
        ws_size_t length  =   WindowTextLength_traits<C>::get_length(h);

        m_buffer = string_maker_type::alloc(length);

        if(NULL != m_buffer)
        {
            get_window_text(h, m_buffer, length + 1);
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
    c_str_ptr_HWND_proxy(class_type& rhs)
        : m_buffer(rhs.m_buffer)
    {
        move_lhs_from_rhs(rhs).m_buffer = NULL;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_ptr_HWND_proxy(class_type const& rhs)
        : m_buffer(string_maker_type::dup_null(rhs.m_buffer))
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_ptr_HWND_proxy() stlsoft_throw_0()
    {
        string_maker_type::free(m_buffer);
    }

// Accessors
public:
    /// Returns a null-terminated string representing the window contents, or
    /// the empty string "" if the window contains no text.
    operator char_type const* () const
    {
        static char_type    s_ch[1] = { '\0' };

        return (NULL == m_buffer) ? s_ch : m_buffer;
    }

// Implementation
private:
    ws_size_t get_window_text(HWND h, char_type* buffer, ws_size_t cchBuffer);

// Members
private:
    char_type   *m_buffer;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
inline ws_size_t c_str_ptr_HWND_proxy<ws_char_a_t>::get_window_text(HWND h, ws_char_a_t *buffer, ws_size_t cchBuffer)
{
    return GetWindowText_A_(h, buffer, cchBuffer);
}

STLSOFT_TEMPLATE_SPECIALISATION
inline ws_size_t c_str_ptr_HWND_proxy<ws_char_w_t>::get_window_text(HWND h, ws_char_w_t *buffer, ws_size_t cchBuffer)
{
    return GetWindowText_W_(h, buffer, cchBuffer);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/* /////////////////////////////////////////////////////////////////////////
 * IOStream compatibility
 */

template<   ss_typename_param_k C
        ,   ss_typename_param_k S
        >
inline S& operator <<(S& s, c_str_ptr_null_HWND_proxy<C> const& shim)
{
    s << static_cast<C const*>(shim);

    return s;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k S
        >
inline S& operator <<(S& s, c_str_ptr_HWND_proxy<C> const& shim)
{
    s << static_cast<C const*>(shim);

    return s;
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline c_str_ptr_HWND_proxy<ws_char_a_t> c_str_data_a(HWND h)
{
    return c_str_ptr_HWND_proxy<ws_char_a_t>(h);
}
inline c_str_ptr_HWND_proxy<ws_char_w_t> c_str_data_w(HWND h)
{
    return c_str_ptr_HWND_proxy<ws_char_w_t>(h);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for HWND
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline c_str_ptr_HWND_proxy<TCHAR> c_str_data(HWND h)
{
    return c_str_ptr_HWND_proxy<TCHAR>(h);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/* HWND */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ws_size_t c_str_len_a(HWND h)
{
    return GetWindowTextLength_A_(h);
}
inline ws_size_t c_str_len_w(HWND h)
{
    return GetWindowTextLength_W_(h);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for HWND
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ws_size_t c_str_len(HWND h)
{
    return GetWindowTextLength_T_(h);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

/* HWND */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline c_str_ptr_HWND_proxy<ws_char_a_t> c_str_ptr_a(HWND h)
{
    return c_str_ptr_HWND_proxy<ws_char_a_t>(h);
}
inline c_str_ptr_HWND_proxy<ws_char_w_t> c_str_ptr_w(HWND h)
{
    return c_str_ptr_HWND_proxy<ws_char_w_t>(h);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for HWND
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline c_str_ptr_HWND_proxy<TCHAR> c_str_ptr(HWND h)
{
    return c_str_ptr_HWND_proxy<TCHAR>(h);
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/* HWND */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline c_str_ptr_null_HWND_proxy<ws_char_a_t> c_str_ptr_null_a(HWND h)
{
    return c_str_ptr_null_HWND_proxy<ws_char_a_t>(h);
}
inline c_str_ptr_null_HWND_proxy<ws_char_w_t> c_str_ptr_null_w(HWND h)
{
    return c_str_ptr_null_HWND_proxy<ws_char_w_t>(h);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for HWND
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline c_str_ptr_null_HWND_proxy<TCHAR> c_str_ptr_null(HWND h)
{
    return c_str_ptr_null_HWND_proxy<TCHAR>(h);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/hwnd_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace stlsoft::winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _WINSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::winstl::c_str_data;
using ::winstl::c_str_data_a;
using ::winstl::c_str_data_w;

using ::winstl::c_str_len;
using ::winstl::c_str_len_a;
using ::winstl::c_str_len_w;

using ::winstl::c_str_ptr;
using ::winstl::c_str_ptr_a;
using ::winstl::c_str_ptr_w;

using ::winstl::c_str_ptr_null;
using ::winstl::c_str_ptr_null_a;
using ::winstl::c_str_ptr_null_w;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_STRING_HPP_HWND */

/* ///////////////////////////// end of file //////////////////////////// */
