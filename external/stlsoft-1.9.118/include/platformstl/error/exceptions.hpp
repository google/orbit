/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/error/exceptions.hpp
 *
 * Purpose:     Platform header for the platform_exceptions components.
 *
 * Created:     20th March 2005
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


#ifndef PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_EXCEPTIONS
#define PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_EXCEPTIONS

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_EXCEPTIONS_MAJOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_EXCEPTIONS_MINOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_EXCEPTIONS_REVISION   1
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_EXCEPTIONS_EDIT       19
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file platformstl/error/exceptions.hpp
 *
 * \brief [C++ only] Definition of the platformstl::platform_exception
 *   class
 *   (\ref group__library__error "Error" Library).
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_HPP_ERROR_UNIX_EXCEPTIONS
#  include <unixstl/error/exceptions.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_ERROR_HPP_UNIX_EXCEPTIONS */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
#  include <winstl/error/exceptions.hpp>
# endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::platformstl */
namespace platformstl
{
#else
/* Define stlsoft::platformstl_project */

namespace stlsoft
{

namespace platformstl_project
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(PLATFORMSTL_OS_IS_UNIX)

# ifdef _UNIXSTL_NO_NAMESPACE
    typedef ::unix_exception                        platform_exception;
    typedef ::unix_exception_policy                 platform_exception_policy;
# else /* ? _UNIXSTL_NO_NAMESPACE */
    typedef ::unixstl::unix_exception               platform_exception;
    typedef ::unixstl::unix_exception_policy        platform_exception_policy;
# endif /* _UNIXSTL_NO_NAMESPACE */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

# ifdef _WINSTL_NO_NAMESPACE
    typedef ::windows_exception                     platform_exception;
    typedef ::windows_exception_policy              platform_exception_policy;
# else /* ? _WINSTL_NO_NAMESPACE */
    typedef ::winstl::windows_exception             platform_exception;
    typedef ::winstl::windows_exception_policy      platform_exception_policy;
# endif /* _WINSTL_NO_NAMESPACE */

#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
