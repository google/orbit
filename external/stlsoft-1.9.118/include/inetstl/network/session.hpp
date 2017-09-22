/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/network/session.hpp (originally MIntSess.h)
 *
 * Purpose:     Contains the basic_session class.
 *
 * Created:     30th April 1999
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


/** \file inetstl/network/session.hpp
 *
 * \brief [C++ only] Definition of the inetstl::basic_session class
 *   template
 *   (\ref group__library__network "Network" Library).
 */

#ifndef INETSTL_INCL_INETSTL_NETWORK_HPP_SESSION
#define INETSTL_INCL_INETSTL_NETWORK_HPP_SESSION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_NETWORK_HPP_SESSION_MAJOR      5
# define INETSTL_VER_INETSTL_NETWORK_HPP_SESSION_MINOR      1
# define INETSTL_VER_INETSTL_NETWORK_HPP_SESSION_REVISION   3
# define INETSTL_VER_INETSTL_NETWORK_HPP_SESSION_EDIT       61
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef INETSTL_INCL_INETSTL_H_INETSTL
# include <inetstl/inetstl.h>
#endif /* !INETSTL_INCL_INETSTL_H_INETSTL */
#ifndef INETSTL_OS_IS_WINDOWS
# error This file is currently compatible only with the Win32/Win64 API
#endif /* !INETSTL_OS_IS_WINDOWS */
#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <inetstl/filesystem/filesystem_traits.hpp>
#endif /* !INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS
#  include <inetstl/error/exceptions.hpp>           // for throw_internet_exception_policy
# endif /* !INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS */
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
# ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
#  include <stlsoft/error/exceptions.hpp>           // for stlsoft::null_exception_policy
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::inetstl */
namespace inetstl
{
# else
/* Define stlsoft::inetstl_project */

namespace stlsoft
{

namespace inetstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Represents an internet session
 *
 * \ingroup group__library__network
 *
 * \note A session is required for WinInet, as it represents an initialisation of the WinInet libraries
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ,   ss_typename_param_k X   =   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k X   =   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k T   =   filesystem_traits<C>
        >
class basic_session
{
public:
    typedef C                                                       char_type;
    typedef X                                                       exception_policy_type;
    typedef ss_typename_param_k exception_policy_type::thrown_type  thrown_type;
    typedef T                                                       traits_type;
    typedef basic_session<C, X, T>                                  class_type;

    typedef HINTERNET                                               resource_type;
// Construction
public:
    /// Default constructor
    ///
    /// \note A session is created with default arguments
    ///
    /// \note If the session fails to initialise, the exception_policy_type function-call operator is called. If the
    /// policy throws an exception, then that exception is thrown. If the policy is "null exception" policy type, then
    /// the session object will be constructed in a closed state.
    basic_session();
    /// Create a session with the given arguments
    ///
    /// \param pcszAgent The name of the user agent, e.g. "Acme Internet Browser"
    /// \param accessType The type of access required, e.g. INTERNET_OPEN_TYPE_DIRECT, INTERNET_OPEN_TYPE_PRECONFIG. Defaults to INTERNET_OPEN_TYPE_PRECONFIG
    /// \param pcszProxyName The name of the proxy server to use. Defaults to NULL
    /// \param pcszProxyBypass List of names not be routed by the proxy. Defaults to NULL
    /// \param flags Flags altering behaviour of the session initialisation.
    ///
    /// \note If the session fails to initialise, the exception_policy_type function-call operator is called. If the
    /// policy throws an exception, then that exception is thrown. If the policy is "null exception" policy type, then
    /// the session object will be constructed in a closed state.
    ss_explicit_k basic_session(    char_type const*    pcszAgent
                                ,   is_dword_t          accessType          =   INTERNET_OPEN_TYPE_PRECONFIG
                                ,   char_type const*    pcszProxyName      =   NULL
                                ,   char_type const*    pcszProxyBypass    =   NULL
                                ,   is_dword_t          flags               =   0);
    /// Closes the session, if open
    ~basic_session() stlsoft_throw_0();

// Operations
public:
    /// Open a session with the given arguments
    ///
    /// \param pcszAgent The name of the user agent, e.g. "Acme Internet Browser"
    /// \param accessType The type of access required, e.g. INTERNET_OPEN_TYPE_DIRECT, INTERNET_OPEN_TYPE_PRECONFIG. Defaults to INTERNET_OPEN_TYPE_PRECONFIG
    /// \param pcszProxyName The name of the proxy server to use. Defaults to NULL
    /// \param pcszProxyBypass List of names not be routed by the proxy. Defaults to NULL
    /// \param flags Flags altering behaviour of the session initialisation.
    ///
    /// \note If the session fails to initialise, the exception_policy_type function-call operator is called. If the
    /// policy throws an exception, then that exception is thrown. If the policy is "null exception" policy type, then
    /// the session object will be left in a closed state.
    is_bool_t    open(  char_type const*    pcszAgent
                    ,   is_dword_t          accessType          =   INTERNET_OPEN_TYPE_PRECONFIG
                    ,   char_type const*    pcszProxyName      =   NULL
                    ,   char_type const*    pcszProxyBypass    =   NULL
                    ,   is_dword_t          flags               =   0);
    /// Closes the session, if open
    void        close();
    /// Removes the session from this object, and returns the underlying WinInet handle to the
    /// caller, who must take responsibility for releasing it.
    HINTERNET   detach();

// State
public:
    /// Indicates whether the object is managing an open session
    is_bool_t   is_open() const;

    /// The underlying WinInet handle
    HINTERNET get() const;

    /// Implicit conversion operator to a WinInet handle
    ///
    /// \deprecated This will be removed in a future release
    operator HINTERNET ();

// Implementation
private:
    static char_type const  *null_string_();

// Members
private:
    HINTERNET   m_hConn;

// Not to be implemented
private:
    basic_session(class_type const&);
    class_type& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_session template for the ANSI character type \c char
 *
 * \ingroup group__library__network
 *
 */
typedef basic_session<is_char_a_t>  session_a;
/** \brief Specialisation of the basic_session template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__network
 *
 */
typedef basic_session<is_char_w_t>  session_w;
/** \brief Specialisation of the basic_session template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__network
 *
 */
typedef basic_session<TCHAR>        session;

/* ////////////////////////////////////////////////////////////////////// */

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline /* static */ ss_typename_type_ret_k basic_session<C, X, T>::char_type const* basic_session<C, X, T>::null_string_()
{
    static char_type    s_null[1] = { 0 };

    return s_null;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_session<C, X, T>::basic_session()
    : m_hConn(traits_type::internet_open(null_string_(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0))
{
    if(NULL == m_hConn)
    {
        exception_policy_type()("Failed to create session", ::GetLastError());
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_session<C, X, T>::basic_session(   char_type const*    pcszAgent
                                            ,   is_dword_t          accessType
                                            ,   char_type const*    pcszProxyName
                                            ,   char_type const*    pcszProxyBypass
                                            ,   is_dword_t          flags)
    : m_hConn(traits_type::internet_open(pcszAgent, accessType, pcszProxyName, pcszProxyBypass, flags))
{
    if(NULL == m_hConn)
    {
        exception_policy_type()("Failed to create session", ::GetLastError());
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_session<C, X, T>::~basic_session() stlsoft_throw_0()
{
    if(NULL != m_hConn)
    {
        ::InternetCloseHandle(m_hConn);
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline is_bool_t basic_session<C, X, T>::open(  char_type const*    pcszAgent
                                            ,   is_dword_t          accessType
                                            ,   char_type const*    pcszProxyName
                                            ,   char_type const*    pcszProxyBypass
                                            ,   is_dword_t          flags)
{
    is_bool_t    bRet;

    if(is_open())
    {
        bRet = false;
    }
    else
    {
        m_hConn = traits_type::internet_open(pcszAgent, accessType, pcszProxyName, pcszProxyBypass, flags);

        if(NULL == m_hConn)
        {
            exception_policy_type()("Failed to create session", ::GetLastError());

            bRet = false;
        }
        else
        {
            bRet = true;
        }
    }

    return bRet;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline void basic_session<C, X, T>::close()
{
    if(NULL != m_hConn)
    {
        ::InternetCloseHandle(m_hConn);

        m_hConn = NULL;
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline HINTERNET basic_session<C, X, T>::detach()
{
    INETSTL_MESSAGE_ASSERT("Attempting to detach from an empty session", is_open());

    HINTERNET   hConn   =   m_hConn;

    m_hConn = NULL;

    return hConn;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline is_bool_t basic_session<C, X, T>::is_open() const
{
    return NULL != m_hConn;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline HINTERNET basic_session<C, X, T>::get() const
{
    return m_hConn;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_session<C, X, T>::operator HINTERNET ()
{
    return m_hConn;
}

/* ////////////////////////////////////////////////////////////////////// */

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline HINTERNET get_handle(basic_session<C, X, T> &s)
{
    return s;
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace inetstl
# else
} // namespace inetstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* INETSTL_INCL_INETSTL_NETWORK_HPP_SESSION */

/* ///////////////////////////// end of file //////////////////////////// */
