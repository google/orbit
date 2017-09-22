/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/error/exceptions.hpp
 *
 * Purpose:     windows_exception class, and its policy class
 *
 * Created:     19th June 2004
 * Updated:     23rd February 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2011, Matthew Wilson and Synesis Software
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


/** \file winstl/error/exceptions.hpp
 *
 * \brief [C++ only] Definition of the winstl::windows_exception and
 *   winstl::resource_exception exception classes, and the
 *   winstl::windows_exception_policy and winstl::resource_exception_policy
 *   exception policy classes
 *   (\ref group__library__error "Error" Library).
 */

#ifndef WINSTL_INCL_WINSTL_ERROR_HPP_EXCEPTIONS
#define WINSTL_INCL_WINSTL_ERROR_HPP_EXCEPTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_ERROR_HPP_EXCEPTIONS_MAJOR       4
# define WINSTL_VER_WINSTL_ERROR_HPP_EXCEPTIONS_MINOR       5
# define WINSTL_VER_WINSTL_ERROR_HPP_EXCEPTIONS_REVISION    1
# define WINSTL_VER_WINSTL_ERROR_HPP_EXCEPTIONS_EDIT        63
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[DocumentationStatus:Ready]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
# include <stlsoft/error/exceptions.hpp>      // for null_exception_policy
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#ifndef WINSTL_INCL_WINSTL_ERROR_H_ERROR_FUNCTIONS
# include <winstl/error/error_functions.h>
#endif /* !WINSTL_INCL_WINSTL_ERROR_H_ERROR_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING
# include <stlsoft/util/exception_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE
# include <stlsoft/smartptr/scoped_handle.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */

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

/** \brief General exception class for Windows operating system failures.
 *
 * \ingroup group__library__error
 *
 */
class windows_exception
    : public stlsoft_ns_qual(os_exception)
{
/// \name Member Types
/// @{
protected:
    typedef stlsoft_ns_qual(exception_string)   string_type;
public:
    /// The parent class type
    typedef stlsoft_ns_qual(os_exception)       parent_class_type;
    /// The error code type
    typedef ws_dword_t                          error_code_type;
    /// The class type
    typedef windows_exception                   class_type;
    /// The size type
    typedef ws_size_t                           size_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance from the given error code
    ss_explicit_k windows_exception(error_code_type err)
        : m_reason()
        , m_strerror(NULL)
        , m_errorCode(err)
    {}
    /// \brief Copy constructor
    windows_exception(class_type const& rhs)
        : m_reason(rhs.m_reason)
        , m_strerror(NULL)
        , m_errorCode(rhs.m_errorCode)
    {}
    /// \brief Constructs an instance from the given message and error code
    windows_exception(char const* reason, error_code_type err)
        : m_reason(class_type::create_reason_(reason, err))
        , m_strerror(NULL)
        , m_errorCode(err)
    {}
    /// \brief Constructs an instance from the given message and error code
    windows_exception(char const* reason)
        : m_reason(reason)
        , m_strerror(NULL)
        , m_errorCode(ERROR_SUCCESS)
    {}
protected:
    /// \brief
    windows_exception(string_type const& reason, error_code_type err)
        : m_reason(reason)
        , m_strerror(NULL)
        , m_errorCode(err)
    {}
public:
    virtual ~windows_exception() stlsoft_throw_0()
    {
        if( NULL != m_strerror &&
            m_reason.c_str() != m_strerror)
        {
            format_message_free_buff(m_strerror);
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    virtual char const* what() const stlsoft_throw_0()
    {
        if(!m_reason.empty())
        {
            return m_reason.c_str();
        }
        else
        {
            return this->strerror();
        }
    }

    /// The error code associated with the exception
    error_code_type get_error_code() const
    {
        return m_errorCode;
    }

    /// [DEPRECATED] The error code associated with the exception
    ///
    /// \deprecated Use get_error_code() instead.
    error_code_type last_error() const
    {
        return get_error_code();
    }

    char const* strerror() const
    {
        if(NULL == m_strerror)
        {
            if(is_memory_error_(m_errorCode))
            {
                return "Out of memory";
            }
            else
            {
                char*& s = stlsoft_ns_qual(remove_const)(this->m_strerror);

                if(0 == format_message(FORMAT_MESSAGE_IGNORE_INSERTS, NULL, m_errorCode, &s))
                {
                    return "Windows system error";
                }
            }
        }

        return m_strerror;
    }
/// @}

/// \name Implementation
/// @{
private:
    static bool is_memory_error_(error_code_type code)
    {
        switch(code)
        {
            default:
                return false;
#ifdef _HRESULT_DEFINED
            case    static_cast<error_code_type>(E_OUTOFMEMORY):
#else /* ? _HRESULT_DEFINED */
            case    static_cast<error_code_type>(0x8007000EL):
#endif /* _HRESULT_DEFINED */
            case    static_cast<error_code_type>(ERROR_OUTOFMEMORY):
                return true;
        }
    }

    static string_type create_reason_(char const* reason, error_code_type err)
    {
        if( is_memory_error_(err) ||
            NULL == reason ||
            '\0' == reason[0])
        {
            return string_type();
        }
        else
        {
#if 0
            size_type const                             len = ::strlen(reason);
            stlsoft_ns_qual(exception_string_creator)   creator(len + 100u);

            creator.append(reason);

            char* s;

            if(0 != format_message(FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, &s))
            {
                stlsoft_ns_qual(scoped_handle)<char*> scoper(s, format_message_free_buff);

                creator.append(": ").append(s);
            }

            return creator.create();
#else /* ? 0 */
            string_type r(reason);
            char*       s;

            if(0 != format_message(FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, &s))
            {
#if 0
                stlsoft_ns_qual(scoped_handle)<char*> scoper(s, format_message_free_buff);
#else /* ? 0 */
                // Workaround for Intel compile error
                void (*pfn)(ws_char_a_t*) = format_message_free_buff;

                stlsoft_ns_qual(scoped_handle)<char*> scoper(s, pfn);
#endif /* 0 */

                return r + ": " + s;
            }
            else
            {
                return r;
            }
#endif /* 0 */
        }
    }
/// @}

/// \name Member Variables
/// @{
private:
    const string_type       m_reason;
    char*                   m_strerror;
    const error_code_type   m_errorCode;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/** \brief Indicates that a resource could not be located.
 *
 * \ingroup group__library__error
 *
 * \see winstl::applet_module |
 *      winstl::basic_resource_string
 */
class resource_exception
    : public windows_exception
{
/// \name Member Types
/// @{
public:
    typedef windows_exception   parent_class_type;
    typedef resource_exception  class_type;
/// @}

/// \name Construction
/// @{
public:
    resource_exception( char const*         reason
                    ,   error_code_type     err
                    ,   LPCTSTR             resourceId      =   NULL
                    ,   LPCTSTR             resourceType    =   NULL)
        : parent_class_type(reason, err)
        , m_resourceId(resourceId)
        , m_resourceType(resourceType)
    {}
/// @}

/// \name Members
/// @{
public:
    LPCTSTR get_resource_id() const
    {
        return m_resourceId;
    }
    LPCTSTR get_resource_type() const
    {
        return m_resourceType;
    }
/// @}

/// \name Members
/// @{
private:
    const LPCTSTR   m_resourceId;
    const LPCTSTR   m_resourceType;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/** \brief Indicates that a access condition was encountered.
 *
 * \ingroup group__library__error
 */
class access_exception
    : public windows_exception
{
/// \name Member Types
/// @{
public:
    typedef windows_exception                   parent_class_type;
    typedef access_exception                    class_type;
    typedef parent_class_type::error_code_type  error_code_type;
/// @}

/// \name Construction
/// @{
public:
    access_exception(   char const*         reason
                    ,   error_code_type     err)
        : parent_class_type(reason, err)
    {}
    access_exception(error_code_type err)
        : parent_class_type(err)
    {}
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Policies
 */

/** \brief A policy class that throws a windows_exception class.
 *
 * \ingroup group__library__error
 *
 */
// [[synesis:class:exception-policy: windows_exception_policy]]
struct windows_exception_policy
{
/// \name Member Types
/// @{
public:
    /// The thrown type
    typedef windows_exception   thrown_type;
    typedef ws_dword_t          error_code_type;
/// @}

/// \name Operators
/// @{
public:
    /// Function call operator, taking no parameters
    void operator ()() const
    {
        STLSOFT_THROW_X(thrown_type(::GetLastError()));
    }
    /// Function call operator, taking one parameter
    void operator ()(error_code_type err) const
    {
        STLSOFT_THROW_X(thrown_type(err));
    }
    /// Function call operator, taking two parameters
    void operator ()(char const* reason, error_code_type err) const
    {
        STLSOFT_THROW_X(thrown_type(reason, err));
    }
/// @}
};

/** \brief A policy class that throws a resource_exception class.
 *
 * \ingroup group__library__error
 *
 */
// [[synesis:class:exception-policy: resource_exception_policy]]
struct resource_exception_policy
{
/// \name Member Types
/// @{
public:
    /// The thrown type
    typedef resource_exception  thrown_type;
    typedef ws_dword_t          error_code_type;
/// @}

/// \name Operators
/// @{
public:
    /// Function call operator, taking two parameters
    void operator ()(char const* reason, error_code_type err) const
    {
        STLSOFT_THROW_X(thrown_type(reason, err));
    }
    /// Function call operator, taking three parameters
    void operator ()(char const* reason, error_code_type err, LPCTSTR resourceId) const
    {
        STLSOFT_THROW_X(thrown_type(reason, err, resourceId));
    }
    /// Function call operator, taking four parameters
    void operator ()(char const* reason, error_code_type err, LPCTSTR resourceId, LPCTSTR resourceType) const
    {
        STLSOFT_THROW_X(thrown_type(reason, err, resourceId, resourceType));
    }
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/exceptions_unittest_.h"
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

#endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
