/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/network/connection.hpp (originally MIntConn.h)
 *
 * Purpose:     Contains the basic_connection class.
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


/** \file inetstl/network/connection.hpp
 *
 * \brief [C++ only] Definition of the inetstl::basic_connection class
 *   template
 *   (\ref group__library__network "Network" Library).
 */

#ifndef INETSTL_INCL_INETSTL_NETWORK_HPP_CONNECTION
#define INETSTL_INCL_INETSTL_NETWORK_HPP_CONNECTION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_NETWORK_HPP_CONNECTION_MAJOR       5
# define INETSTL_VER_INETSTL_NETWORK_HPP_CONNECTION_MINOR       1
# define INETSTL_VER_INETSTL_NETWORK_HPP_CONNECTION_REVISION    3
# define INETSTL_VER_INETSTL_NETWORK_HPP_CONNECTION_EDIT        67
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
#ifndef INETSTL_INCL_INETSTL_NETWORK_HPP_SESSION
# include <inetstl/network/session.hpp>
#endif /* !INETSTL_INCL_INETSTL_NETWORK_HPP_SESSION */
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

/** \brief Represents an internet connection
 *
 * \ingroup group__library__network
 *
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ,   ss_typename_param_k X   =   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k X   =   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k T   =   filesystem_traits<C>
        >
class basic_connection
{
public:
    typedef C                                                       char_type;
    typedef X                                                       exception_policy_type;
    typedef ss_typename_param_k exception_policy_type::thrown_type  thrown_type;
    typedef T                                                       traits_type;
    typedef basic_connection<C, X, T>                               class_type;

    typedef HINTERNET                                               resource_type;
// Construction
public:
    /// Default constructor
    ///
    /// \note No connection exists. Use connect() to open a connection
    basic_connection();

    /// Create a connection with the given arguments
    ///
    /// \param hsess A WinInet handle to the current session, obtained via \c InternetOpen() or the basic_session class
    /// \param server The name of the server to which to connect. May not be NULL
    /// \param port The number of the port to connect on
    /// \param userName The username for the connection. May be NULL
    /// \param password The password for the connection. May be NULL
    /// \param service The service to access. Either INTERNET_SERVICE_FTP or INTERNET_SERVICE_HTTP
    /// \param flags Service specific flags, e.g. INTERNET_FLAG_PASSIVE
    /// \param context Context used for callback functions
    ///
    /// \note If the connection fails to open, the exception_policy_type function-call operator is called. If the
    /// policy throws an exception, then that exception is thrown. If the policy is "null exception" policy type, then
    /// the connection object will be constructed in a closed state.
    ss_explicit_k basic_connection( HINTERNET           hsess
                                ,   char_type const*    server
                                ,   INTERNET_PORT       port
                                ,   char_type const*    userName
                                ,   char_type const*    password
                                ,   is_dword_t          service
                                ,   is_dword_t          flags
                                ,   is_dword_t          context     =   0);
    /// Closes the connection, if open
    ~basic_connection() stlsoft_throw_0();

// Operations
public:
    /// Opens a connection with the given arguments
    ///
    /// \param hsess A WinInet handle to the current session, obtained via \c InternetOpen() or the basic_session class
    /// \param server The name of the server to which to connect. May not be NULL
    /// \param port The number of the port to connect on
    /// \param userName The username for the connection. May be NULL
    /// \param password The password for the connection. May be NULL
    /// \param service The service to access. Either INTERNET_SERVICE_FTP or INTERNET_SERVICE_HTTP
    /// \param flags Service specific flags, e.g. INTERNET_FLAG_PASSIVE
    /// \param context Context used for callback functions
    ///
    /// \note If the connection fails to open, the exception_policy_type function-call operator is called. If the
    /// policy throws an exception, then that exception is thrown. If the policy is "null exception" policy type, then
    /// the connection object will be left in a closed state.
    is_bool_t    connect(   HINTERNET           hsess
                        ,   char_type const*    server
                        ,   INTERNET_PORT       port
                        ,   char_type const*    userName
                        ,   char_type const*    password
                        ,   is_dword_t          service
                        ,   is_dword_t          flags
                        ,   is_dword_t          context =   0);
    /// Closes the connection, if open
    void        close();
    /// Removes the connection from this object, and returns the underlying WinInet handle to the
    /// caller, who must take responsibility for releasing it.
    HINTERNET   detach();

// State
public:
    /// Indicates whether the object is managing an open connection
    is_bool_t   is_connected() const;
    /// The last Win32/WinInet error associated with this connection object
    is_dword_t  last_error() const;

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
    is_dword_t  m_lastError;
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_connection template for the ANSI character type \c char
 *
 * \ingroup group__library__network
 *
 */
typedef basic_connection<is_char_a_t>   connection_a;
/** \brief Specialisation of the basic_connection template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__network
 *
 */
typedef basic_connection<is_char_w_t>   connection_w;
/** \brief Specialisation of the basic_connection template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__network
 *
 */
typedef basic_connection<TCHAR>         connection;

/* ////////////////////////////////////////////////////////////////////// */

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline /* static */ ss_typename_type_ret_k basic_connection<C, X, T>::char_type const* basic_connection<C, X, T>::null_string_()
{
    static char_type    s_null[1] = { 0 };

    return s_null;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_connection<C, X, T>::basic_connection()
    : m_hConn(NULL)
    , m_lastError(ERROR_SUCCESS)
{}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_connection<C, X, T>::basic_connection( HINTERNET       hsess
                                                ,   char_type const* server
                                                ,   INTERNET_PORT   port
                                                ,   char_type const* userName
                                                ,   char_type const* password
                                                ,   is_dword_t      service
                                                ,   is_dword_t      flags
                                                ,   is_dword_t      context /* = 0 */)
    : m_hConn(traits_type::internet_connect(hsess, server, port, userName, password, service, flags, context))
    , m_lastError(::GetLastError())
{
    if(NULL == m_hConn)
    {
        exception_policy_type()("Failed to open connection", m_lastError);
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_connection<C, X, T>::~basic_connection() stlsoft_throw_0()
{
    if(NULL != m_hConn)
    {
        traits_type::close_connection(m_hConn);
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline is_bool_t basic_connection<C, X, T>::connect(HINTERNET       hsess
                                                ,   char_type const* server
                                                ,   INTERNET_PORT   port
                                                ,   char_type const* userName
                                                ,   char_type const* password
                                                ,   is_dword_t      service
                                                ,   is_dword_t      flags
                                                ,   is_dword_t      context /* = 0 */)
{
    is_bool_t    bRet;

    if(is_connected())
    {
        bRet = false;
    }
    else
    {
        m_hConn     =   traits_type::internet_connect(hsess, server, port, userName, password, service, flags, context);
        m_lastError =   ::GetLastError();

        if(NULL == m_hConn)
        {
            exception_policy_type()("Failed to open connection", m_lastError);

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
inline void basic_connection<C, X, T>::close()
{
    if(NULL != m_hConn)
    {
        traits_type::close_connection(m_hConn);

        m_hConn = NULL;
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline HINTERNET basic_connection<C, X, T>::detach()
{
    INETSTL_MESSAGE_ASSERT("Attempting to detach from an empty connection", is_connected());

    HINTERNET   hConn   =   m_hConn;

    m_hConn = NULL;

    return hConn;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline is_bool_t basic_connection<C, X, T>::is_connected() const
{
    return NULL != m_hConn;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline is_dword_t basic_connection<C, X, T>::last_error() const
{
    return m_lastError;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline HINTERNET basic_connection<C, X, T>::get() const
{
    return m_hConn;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline basic_connection<C, X, T>::operator HINTERNET ()
{
    return m_hConn;
}

/* ////////////////////////////////////////////////////////////////////// */

template<   ss_typename_param_k C
        ,   ss_typename_param_k X
        ,   ss_typename_param_k T
        >
inline HINTERNET get_handle(basic_connection<C, X, T> &s)
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

#endif /* INETSTL_INCL_INETSTL_NETWORK_HPP_CONNECTION */

/* ///////////////////////////// end of file //////////////////////////// */
