/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/synch/semaphore.hpp
 *
 * Purpose:     Platform header for directory manipulation functions.
 *
 * Created:     30th May 2006
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


/** \file platformstl/synch/semaphore.hpp
 *
 * \brief [C++ only] Definition of the platformstl::semaphore type
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_HPP_SEMAPHORE
#define PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_HPP_SEMAPHORE

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_SEMAPHORE_MAJOR      1
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_SEMAPHORE_MINOR      1
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_SEMAPHORE_REVISION   1
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_SEMAPHORE_EDIT       10
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL
# include <platformstl/platformstl.h>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SEMAPHORE
#  include <unixstl/synch/semaphore.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SEMAPHORE */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_SEMAPHORE
#  include <winstl/synch/semaphore.hpp>
# endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_SEMAPHORE */
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(__cplusplus)
 /* Nothing defined in C */
#elif defined(_STLSOFT_NO_NAMESPACE) || \
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

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)

    /** \brief A semaphore class
     *
     * The class is not actually defined in the
     * \link ::platformstl platformstl\endlink namespace. Rather, it
     * resolves to the appropriate type for the given platform, relying on
     * \ref section__principle__conformance__intersecting_conformance "Intersecting Conformance"
     * of the resolved platform-specific types.
     *
     * When compiling on UNIX platforms, the platformstl::semaphore
     * type resolves to the unixstl::semaphore class. On Windows platforms
     * it resolves to the winstl::semaphore class. It is not defined
     * for other platforms.
     */
    class semaphore
    {};

#elif defined(PLATFORMSTL_OS_IS_UNIX)

# ifdef _UNIXSTL_NO_NAMESPACE
    using ::semaphore;
# else /* ? _UNIXSTL_NO_NAMESPACE */
    using ::unixstl::semaphore;
# endif /* _UNIXSTL_NO_NAMESPACE */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

# ifdef _WINSTL_NO_NAMESPACE
    using ::semaphore;
# else /* ? _WINSTL_NO_NAMESPACE */
    using ::winstl::semaphore;
# endif /* _WINSTL_NO_NAMESPACE */

#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* ////////////////////////////////////////////////////////////////////// */

#if !defined(__cplusplus)
 /* Nothing defined in C */
#elif defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_HPP_SEMAPHORE */

/* ///////////////////////////// end of file //////////////////////////// */
