/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/error/exceptions.hpp
 *
 * Purpose:     Range exceptions.
 *
 * Created:     30th December 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file rangelib/error/exceptions.hpp
 *
 * \brief [C++ only] Definition of the rangelib::range_exception and
 *   rangelib::empty_range_exception exception classes.
 *
 *   (\ref group__library__error "Error" Library).
 */

#ifndef RANGELIB_INCL_RANGELIB_ERROR_HPP_EXCEPTIONS
#define RANGELIB_INCL_RANGELIB_ERROR_HPP_EXCEPTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_ERROR_HPP_EXCEPTIONS_MAJOR     2
# define RANGELIB_VER_RANGELIB_ERROR_HPP_EXCEPTIONS_MINOR     0
# define RANGELIB_VER_RANGELIB_ERROR_HPP_EXCEPTIONS_REVISION  2
# define RANGELIB_VER_RANGELIB_ERROR_HPP_EXCEPTIONS_EDIT      17
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
[DocumentationStatus:Ready]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGELIB
# include <rangelib/rangelib.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGELIB */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING
# include <stlsoft/util/exception_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_EXCEPTION_STRING */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifdef STLSOFT_UNITEST
# include <string.h>
#endif /* STLSOFT_UNITEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::rangelib */
namespace rangelib
{
# else
/* Define stlsoft::rangelib_project */

namespace stlsoft
{

namespace rangelib_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief General exception class for RangeLib failures.
 *
 * \ingroup group__library__error
 *
 */
class range_exception
#if defined(STLSOFT_COMPILER_IS_DMC)
    : public std::exception
#else /* ? compiler */
    : public stlsoft_ns_qual_std(exception)
#endif /* compiler */
{
/// \name Types
/// @{
private:
    typedef stlsoft_ns_qual(exception_string)   string_type;
public:
    typedef range_exception                     class_type;
#if defined(STLSOFT_COMPILER_IS_DMC)
    typedef std::exception                      parent_class_type;
#else /* ? compiler */
    typedef stlsoft_ns_qual_std(exception)      parent_class_type;
#endif /* compiler */
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k range_exception(char const* reason = NULL)
        : m_reason((NULL == reason) ? "" : reason)
    {}
    /// Destructor
    ///
    /// \note This does not do have any implementation, but is required to placate
    /// the Comeau and GCC compilers, which otherwise complain about mismatched
    /// exception specifications between this class and its parent
    virtual ~range_exception() stlsoft_throw_0()
    {}
/// @}

/// \name Accessors
/// @{
public:
    virtual char const* what() const stlsoft_throw_0()
    {
        return m_reason.empty() ? this->real_what_() : m_reason.c_str();
    }
/// @}

/// \name Implementation
/// @{
private:
    virtual char const* real_what_() const throw()
    {
        return "Range exception";
    }
/// @}

/// \name Members
/// @{
private:
    const string_type   m_reason;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/** \brief Indicates that an operation requiring non-empty range was invoked
 *    on an empty range.
 *
 * \ingroup group__library__error
 *
 */
class empty_range_exception
    : public range_exception
{
public:
    typedef range_exception         parent_class_type;
    typedef empty_range_exception  class_type;

/// \name Construction
/// @{
public:
    ss_explicit_k empty_range_exception(char const* reason = NULL)
        : parent_class_type(reason)
    {}
    /// Destructor
    ///
    /// \note This does not do have any implementation, but is required to placate
    /// the Comeau and GCC compilers, which otherwise complain about mismatched
    /// exception specifications between this class and its parent
    virtual ~empty_range_exception() stlsoft_throw_0()
    {}
/// @}

/// \name Implementation
/// @{
private:
    virtual char const* real_what_() const throw()
    {
        return "Range was empty";
    }
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/exceptions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace rangelib
# else
} // namespace rangelib_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !RANGELIB_INCL_RANGELIB_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
