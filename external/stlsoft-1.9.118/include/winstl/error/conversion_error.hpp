/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/error/conversion_error.hpp
 *
 * Purpose:     Definition of the winstl::conversion_error exception class.
 *
 * Created:     16th December 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/error/conversion_error.hpp
 *
 * \brief [C++ only] Definition of the winstl::conversion_error
 *   exception class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef WINSTL_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR
#define WINSTL_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR

#ifndef WINSTL_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_ERROR_HPP_CONVERSION_ERROR_MAJOR     1
# define WINSTL_VER_WINSTL_ERROR_HPP_CONVERSION_ERROR_MINOR     0
# define WINSTL_VER_WINSTL_ERROR_HPP_CONVERSION_ERROR_REVISION  1
# define WINSTL_VER_WINSTL_ERROR_HPP_CONVERSION_ERROR_EDIT      6
#endif /* !WINSTL_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_STLSOFT
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_CONVERSION_ERROR
# include <stlsoft/error/conversion_error.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_CONVERSION_ERROR */

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

/** Represents a failed conversion
 *
 * \ingroup group__library__error
 */
class conversion_error
    : public stlsoft_ns_qual(conversion_error_base)
{
/// \name Types
/// @{
public:
    /// The parent type
    typedef stlsoft_ns_qual(conversion_error_base)  parent_class_type;
    /// The type of the current instantiation
    typedef conversion_error                        class_type;
    /// The string argument type
    typedef stlsoft_ns_qual_std(string)             string_type;
    /// The error code type
    typedef ws_dword_t                              error_code_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs an instance from the given message and error code
    conversion_error(string_type const& message, error_code_type code)
        : parent_class_type(message)
        , m_code(code)
    {}
    /// Constructs an instance from the given message and error code
    conversion_error(char const* message, error_code_type code)
        : parent_class_type(message)
        , m_code(code)
    {}

#ifndef WINSTL_DOCUMENTATION_SKIP_SECTION
public:
    conversion_error(class_type const& rhs)
        : parent_class_type(rhs)
        , m_code(rhs.m_code)
    {}
    virtual ~conversion_error() stlsoft_throw_0()
    {}
private:
    class_type& operator =(class_type const&);
#endif /* !WINSTL_DOCUMENTATION_SKIP_SECTION */
/// @}

/// \name Accessors
/// @{
public:
    error_code_type get_error_code() const
    {
        return m_code;
    }

    error_code_type get_last_error() const
    {
        return get_error_code();
    }
/// @}

/// \name Members
/// @{
private:
    const error_code_type   m_code;
/// @}
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

#endif /* !STLSOFT_INCL_WINSTL_ERROR_HPP_CONVERSION_ERROR */

/* ///////////////////////////// end of file //////////////////////////// */
