/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/performance/processtimes_counter.hpp
 *
 * Purpose:     Platform header for the processtimes_counter components.
 *
 * Created:     9th June 2006
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


#ifndef PLATFORMSTL_INCL_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER
#define PLATFORMSTL_INCL_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_MAJOR     1
# define PLATFORMSTL_VER_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_MINOR     1
# define PLATFORMSTL_VER_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_REVISION  1
# define PLATFORMSTL_VER_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_EDIT      7
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file platformstl/performance/processtimes_counter.hpp
 *
 * \brief [C++ only] Definition of the <code>platformstl::processtimes_counter</code>
 *  type
 *   (\ref group__library__performance "Performance" Library).
 *
 * When compiling on UNIX platforms, the platformstl::processtimes_counter
 * type resolves to the unixstl::processtimes_counter class. On Windows
 * platforms it resolves to the winstl::processtimes_counter class. It is
 * not defined for other platforms.
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER
#  include <unixstl/performance/processtimes_counter.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER
#  include <winstl/performance/processtimes_counter.hpp>
# endif /* !WINSTL_INCL_WINSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER */
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

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)

    /** \brief A performance counter class that measures kernel and user activity.
     *
     * The class is not actually defined in the
     * \link ::platformstl platformstl\endlink namespace. Rather, it
     * resolves to the appropriate type for the given platform, relying on
     * \ref section__principle__conformance__intersecting_conformance "Intersecting Conformance"
     * of the resolved platform-specific types.
     *
     * When compiling on UNIX platforms, the platformstl::processtimes_counter
     * type resolves to the unixstl::processtimes_counter class. On Windows
     * platforms it resolves to the winstl::processtimes_counter class. It
     * is not defined for other platforms.
     */
    class processtimes_counter
    {};

#elif defined(PLATFORMSTL_OS_IS_UNIX)

# ifdef _UNIXSTL_NO_NAMESPACE
    using ::processtimes_counter;
# else /* ? _UNIXSTL_NO_NAMESPACE */
    using ::unixstl::processtimes_counter;
# endif /* _UNIXSTL_NO_NAMESPACE */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

# ifdef _WINSTL_NO_NAMESPACE
    using ::processtimes_counter;
# else /* ? _WINSTL_NO_NAMESPACE */
    using ::winstl::processtimes_counter;
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

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER */

/* ///////////////////////////// end of file //////////////////////////// */
