/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/error/contract_violation.hpp
 *
 * Purpose:     Definition of the \c contract_violation exception class.
 *
 * Created:     17th October 2004
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


/** \file stlsoft/error/contract_violation.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::contract_violation
 *   exception class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION
#define STLSOFT_INCL_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION_MAJOR     2
# define STLSOFT_VER_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION_MINOR     0
# define STLSOFT_VER_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION_REVISION  2
# define STLSOFT_VER_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION_EDIT      20
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_UNRECOVERABLE
# include <stlsoft/error/unrecoverable.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_UNRECOVERABLE */

#ifndef STLSOFT_INCL_STRING
# define STLSOFT_INCL_STRING
# include <string>
#endif /* !STLSOFT_INCL_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Thrown when a contract has been violated
 *
 * \ingroup group__library__error
 *
 * Exceptions deriving from this class may be caught, but they result in
 * process termination at the end of the catch clause, or if they're not caught.
 */
class contract_violation
    : public unrecoverable
{
/// \name Types
/// @{
private:
#ifdef STLSOFT_CF_std_NAMESPACE
    typedef stlsoft_ns_qual_std(string) string_type;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# if defined(STLSOFT_COMPILER_IS_WATCOM)
    class string_type
        : public stlsoft_ns_qual_std(string)
    {
    private:
        typedef stlsoft_ns_qual_std(string) parent_class_type;
    public:
        string_type(char const* s) : parent_class_type(s) {}
        string_type() : parent_class_type() {}
        string_type(string_type const& rhs) : parent_class_type(rhs) {}
    public:
        char const* c_str() const { return *this; }
        bool empty() const { return (NULL == this->c_str() || '\0' == *this->c_str()); }
    };
# else /* ? compiler */
#  error No other non-std compiler supported
# endif /* compiler */
#endif /* STLSOFT_CF_std_NAMESPACE */
public:
    /// The parent type
    typedef unrecoverable               parent_class_type;
    /// The type of the current instantiation
    typedef contract_violation          class_type;
/// @}

/// \name Construction
/// @{
public:
    /// Default constructor
    contract_violation()
        : parent_class_type(NULL)
        , m_error()
    {}
    /// Creates an instance which will call the given function when termination
    /// is effected
    ///
    /// \note Only when the last copy of this instance is destroyed will the
    /// given function by executed
    ss_explicit_k contract_violation(   void        (*pfnHandler)())
        : parent_class_type(pfnHandler)
        , m_error()
    {}
    ss_explicit_k contract_violation(   char const  *error
                                    ,   void        (*pfnHandler)() =   NULL)
        : parent_class_type(pfnHandler)
        , m_error(error)
    {}
    ss_explicit_k contract_violation(   string_type const   &error
                                    ,   void                (*pfnHandler)() = NULL)
        : parent_class_type(pfnHandler)
        , m_error(error)
    {}
    /// Copy constructor
    ///
    /// \note Only when the last copy is destroyed will the termination be effected
    contract_violation(class_type const& rhs)
        : parent_class_type(rhs)
        , m_error(rhs.m_error)
    {}
    virtual ~contract_violation() stlsoft_throw_0()
    {}
/// @}

/// \name Accessors
/// @{
public:
    /// Returns a human-readable string describing the exception condition
    virtual char const* what() const throw()
    {
        return m_error.empty() ? default_message_() : m_error.c_str();
    }
/// @}

/// \name Implementation
/// @{
private:
    static char const   *default_message_()
    {
        return "contract violation";
    }
/// @}

/// \name Members
/// @{
private:
    const string_type   m_error;
/// @}

// Not to be implemented
private:
    class_type& operator =(class_type const&);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_CONTRACT_VIOLATION */

/* ///////////////////////////// end of file //////////////////////////// */
