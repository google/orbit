/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/error/exceptions.hpp
 *
 * Purpose:     unix_exception class, and its policy class
 *
 * Created:     19th June 2004
 * Updated:     11th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2010, Matthew Wilson and Synesis Software
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


/** \file unixstl/error/exceptions.hpp
 *
 * \brief [C++ only] Definition of the unixstl::unix_exception and exception
 *   class and the unixstl::unix_exception_policy exception policy class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_ERROR_HPP_EXCEPTIONS
#define UNIXSTL_INCL_UNIXSTL_ERROR_HPP_EXCEPTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_ERROR_HPP_EXCEPTIONS_MAJOR     4
# define UNIXSTL_VER_UNIXSTL_ERROR_HPP_EXCEPTIONS_MINOR     2
# define UNIXSTL_VER_UNIXSTL_ERROR_HPP_EXCEPTIONS_REVISION  5
# define UNIXSTL_VER_UNIXSTL_ERROR_HPP_EXCEPTIONS_EDIT      53
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

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
# include <stlsoft/error/exceptions.hpp>      // for null_exception_policy
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING
# include <stlsoft/util/exception_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */
#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief General exception class for UNIX operating system failures.
 *
 * \ingroup group__library__error
 *
 */
class unix_exception
    : public stlsoft_ns_qual(os_exception)
{
/// \name Types
/// @{
protected:
    typedef stlsoft_ns_qual(exception_string)   string_type;
public:
    typedef stlsoft_ns_qual(os_exception)       parent_class_type;
    typedef int                                 error_code_type;
    typedef unix_exception                      class_type;
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k unix_exception(error_code_type err)
        : m_reason()
        , m_errorCode(err)
    {}
    unix_exception(char const* reason, error_code_type err)
        : m_reason(class_type::create_reason_(reason, err))
        , m_errorCode(err)
    {}
protected:
    unix_exception(string_type const& reason, error_code_type err)
        : m_reason(reason)
        , m_errorCode(err)
    {}
public:
    /// Destructor
    ///
    /// \note This does not do have any implementation, but is required to placate
    /// the Comeau and GCC compilers, which otherwise complain about mismatched
    /// exception specifications between this class and its parent
    virtual ~unix_exception() stlsoft_throw_0()
    {}
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
            char const* s = this->strerror();

            UNIXSTL_ASSERT(NULL != s);

            return (*s != '\0') ? s : "UNIX system error";
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
    error_code_type get_errno() const
    {
        return get_error_code();
    }

    /// [DEPRECATED] String form of the contained error code
    ///
    /// \deprecated This method <em>will</em> be removed in a future version.
    char const* strerror() const
    {
        return strerror_(m_errorCode);
    }
/// @}

/// \name Implementation
/// @{
private:
    static char const* strerror_(int code)
    {
#if defined(STLSOFT_USING_SAFE_STR_FUNCTIONS) && \
    defined(STLSOFT_COMPILER_IS_MSVC)
# pragma warning(push)
# pragma warning(disable : 4996 )
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS && STLSOFT_COMPILER_IS_MSVC */

        return ::strerror(code);

#if defined(STLSOFT_USING_SAFE_STR_FUNCTIONS) && \
    defined(STLSOFT_COMPILER_IS_MSVC)
# pragma warning(pop)
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS && STLSOFT_COMPILER_IS_MSVC */
    }

    static string_type create_reason_(char const* reason, error_code_type err)
    {
        if( err == ENOMEM ||
            NULL == reason ||
            '\0' == reason[0])
        {
            return string_type();
        }
        else
        {
            strerror_(0);  // need to flush the errno

            string_type r(reason);
            char const* s = strerror_(err);

            UNIXSTL_ASSERT(NULL != s);

            if(*s != '\0')
            {
                return r + ": " + s;
            }

            return r;
        }
    }
/// @}

/// \name Members
/// @{
private:
    const string_type   m_reason;
    error_code_type     m_errorCode;
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

/** \brief The NULL exception type. It does not throw, and its throw type is empty.
 *
 * \ingroup group__library__error
 *
 */
// [[synesis:class:exception-policy: unix_exception_policy]]
struct unix_exception_policy
{
/// \name Member Types
/// @{
public:
    /// The thrown type
    typedef unix_exception   thrown_type;
    typedef int              error_code_type;
/// @}

/// \name Operators
/// @{
public:
    /// Function call operator, taking no parameters
    void operator ()() const
    {
        STLSOFT_THROW_X(thrown_type(errno));
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

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/exceptions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
