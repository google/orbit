/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/filesystem/current_directory.hpp
 *
 * Purpose:     Platform header for the current_directory components.
 *
 * Created:     13th June 2005
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


/** \file platformstl/filesystem/current_directory.hpp
 *
 * \brief [C++ only] Definition of the platformstl::basic_current_directory
 *  pseudo type
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY
#define PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_MAJOR     2
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_MINOR     2
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_REVISION  1
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_EDIT      17
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY
#  include <unixstl/filesystem/current_directory.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY
#  include <winstl/filesystem/current_directory.hpp>
# endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY */
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

    /** \brief A facade for the platform's basic_current_directory functionality.
     *
     * \ingroup group__library__filesystem
     *
     * The notional class template platformstl::basic_current_directory
     * used to be a placeholder for the appropriate type for the given
     * platform, relying on
     * \ref section__principle__conformance__intersecting_conformance "Intersecting Conformance"
     * of the resolved platform-specific types.
     *
     * When compiling on UNIX platforms, the platformstl::basic_current_directory
     * type would have resolved to the unixstl::basic_current_directory
     * class. On Windows platforms it would have resolved to the
     * winstl::basic_current_directory class.
     *
     * However, with version 1.9, the current_directory family of types now
     * uses the
     * \ref group__pattern__special_string_instance "Special String Instance"
     * pattern, so there is no longer a basic_current_directory. Rather,
     * there are the
     *  unixstl::current_directory_a
     * and
     *  unixstl::current_directory,
     * and
     *  winstl::current_directory_a,
     *  winstl::current_directory_w,
     * and
     *  winstl::current_directory
     * specialisations. For platform independence, use
     *  platformstl::current_directory_a
     * and
     *  platformstl::current_directory.
     */
    template<   ss_typename_param_k C
            ,   ss_typename_param_k T = unixstl_ns_qual(filesystem_traits)<C>
            >
    class basic_current_directory
    {};

#elif defined(PLATFORMSTL_OS_IS_UNIX)

# ifdef _UNIXSTL_NO_NAMESPACE
   using ::current_directory_a;
   using ::current_directory;
# else /* ? _UNIXSTL_NO_NAMESPACE */
   using ::unixstl::current_directory_a;
   using ::unixstl::current_directory;
# endif /* _UNIXSTL_NO_NAMESPACE */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

# ifdef _WINSTL_NO_NAMESPACE
   using ::current_directory_a;
   using ::current_directory_w;
   using ::current_directory;
# else /* ? _WINSTL_NO_NAMESPACE */
   using ::winstl::current_directory_a;
   using ::winstl::current_directory_w;
   using ::winstl::current_directory;
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

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY */

/* ///////////////////////////// end of file //////////////////////////// */
