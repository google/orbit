/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/error/os_exception.hpp
 *
 * Purpose:     Basic exception classes.
 *
 * Created:     19th January 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/error/os_exception.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::os_exception exception
 *   class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_OS_EXCEPTION
#define STLSOFT_INCL_STLSOFT_ERROR_HPP_OS_EXCEPTION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ERROR_HPP_OS_EXCEPTION_MAJOR       5
# define STLSOFT_VER_STLSOFT_ERROR_HPP_OS_EXCEPTION_MINOR       0
# define STLSOFT_VER_STLSOFT_ERROR_HPP_OS_EXCEPTION_REVISION    1
# define STLSOFT_VER_STLSOFT_ERROR_HPP_OS_EXCEPTION_EDIT        50
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

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_PROJECT_EXCEPTION
# include <stlsoft/error/project_exception.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_PROJECT_EXCEPTION */

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

/** \brief Root exception class for sub-project platform-specific
 *    exceptions.
 *
 * \ingroup group__library__error
 *
 */
class os_exception
    : public project_exception
{
/// \name Types
/// @{
public:
    /// The type of the current instantiation
    typedef os_exception                    class_type;
    /// The parent type
    typedef project_exception               parent_class_type;
/// @}

/// \name Construction
/// @{
public:
    /// Default constructor
    os_exception()
    {}
/// @}

/// \name Accessors
/// @{
public:
    /// Returns a human-readable string describing the exception condition
    virtual char const* what() const stlsoft_throw_0() = 0;
/// @}
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_OS_EXCEPTION */

/* ///////////////////////////// end of file //////////////////////////// */
