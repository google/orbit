/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/error/exceptions.hpp
 *
 * Purpose:     Contains the internet_exception class.
 *
 * Created:     25th April 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file inetstl/error/exceptions.hpp
 *
 * \brief [C++ only] Definition of the inetstl::internet_exception and
 *   exception class and the inetstl::throw_internet_exception_policy
 *   exception policy class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS
#define INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_ERROR_HPP_EXCEPTIONS_MAJOR     4
# define INETSTL_VER_INETSTL_ERROR_HPP_EXCEPTIONS_MINOR     2
# define INETSTL_VER_INETSTL_ERROR_HPP_EXCEPTIONS_REVISION  1
# define INETSTL_VER_INETSTL_ERROR_HPP_EXCEPTIONS_EDIT      42
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

#ifndef INETSTL_INCL_INETSTL_H_INETSTL
# include <inetstl/inetstl.h>
#endif /* !INETSTL_INCL_INETSTL_H_INETSTL */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
# include <stlsoft/error/exceptions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING
# include <stlsoft/util/exception_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING */

#ifndef INETSTL_OS_IS_WINDOWS
# include <errno.h>
#endif /* !INETSTL_OS_IS_WINDOWS */

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
# error This file cannot be included when exception-handling is not supported
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

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

/** \brief General exception class for internet-related failures.
 *
 * \ingroup group__library__error
 *
 */
class internet_exception
    : public os_exception
{
/// \name Types
/// @{
protected:
    typedef stlsoft_ns_qual(exception_string)   string_type;
public:
    typedef os_exception                        parent_class_type;
#ifdef INETSTL_OS_IS_WINDOWS
    typedef is_dword_t                          error_code_type;
#else /* ? INETSTL_OS_IS_WINDOWS */
    typedef int                                 error_code_type;
#endif /* INETSTL_OS_IS_WINDOWS */
    typedef internet_exception                  class_type;
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k internet_exception(error_code_type err)
        : m_reason()
        , m_errorCode(err)
    {}
    internet_exception(class_type const& rhs)
        : m_reason(rhs.m_reason)
        , m_errorCode(rhs.m_errorCode)
    {}
    internet_exception(char const* reason, error_code_type err)
        : m_reason(class_type::create_reason_(reason, err))
        , m_errorCode(err)
    {}
protected:
    internet_exception(string_type const& reason, error_code_type err)
        : m_reason(reason)
        , m_errorCode(err)
    {}
public:
    /// Destructor
    ///
    /// \note This does not do have any implementation, but is required to placate
    /// the Comeau and GCC compilers, which otherwise complain about mismatched
    /// exception specifications between this class and its parent
    virtual ~internet_exception() stlsoft_throw_0()
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
            return "internet failure";
        }
    }

    /// The error code associated with the exception
    error_code_type get_error_code() const stlsoft_throw_0()
    {
        return m_errorCode;
    }
    /// [DEPRECATED] The error code associated with the exception
    ///
    /// \deprecated Use get_error_code() instead.
    error_code_type last_error() const stlsoft_throw_0()
    {
        return get_error_code();
    }
    /// [DEPRECATED] The error code associated with the exception
    ///
    /// \deprecated Use get_error_code() instead.
    error_code_type error() const stlsoft_throw_0()
    {
        return get_error_code();
    }
/// @}

/// \name Implementation
/// @{
private:
    static string_type create_reason_(char const* reason, error_code_type err)
    {
#ifdef INETSTL_OS_IS_WINDOWS
        if( err == static_cast<error_code_type>(E_OUTOFMEMORY) ||
            err == static_cast<error_code_type>(ERROR_OUTOFMEMORY) ||
            NULL == reason ||
            '\0' == reason[0])
        {
            return string_type();
        }
        else
#endif /* INETSTL_OS_IS_WINDOWS */
        {
            return string_type(reason);
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

/** \brief The policy class, which throws a internet_exception class.
 *
 * \ingroup group__library__error
 *
 */
// [[synesis:class:exception-policy: throw_internet_exception_policy]]
struct throw_internet_exception_policy
{
/// \name Member Types
/// @{
public:
    /// The thrown type
    typedef internet_exception                  thrown_type;
    typedef internet_exception::error_code_type error_code_type;
/// @}

/// \name Operators
/// @{
public:
    /// Function call operator, taking no parameters
    void operator ()() const
    {
#ifdef INETSTL_OS_IS_WINDOWS
        STLSOFT_THROW_X(thrown_type(::GetLastError()));
#else /* ? INETSTL_OS_IS_WINDOWS */
        STLSOFT_THROW_X(thrown_type(errno));
#endif /* INETSTL_OS_IS_WINDOWS */
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

#endif /* INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
