/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/filesystem/path_functions.hpp
 *
 * Purpose:     Path manipulation functions.
 *
 * Created:     12th June 2006
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


/** \file platformstl/filesystem/path_functions.hpp
 *
 * \brief [C++ only] Path manipulation functions
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS
#define PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_MAJOR    1
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_MINOR    1
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_REVISION 1
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_EDIT     8
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS
#  include <unixstl/filesystem/path_functions.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_PATH_FUNCTIONS
#  include <winstl/filesystem/path_functions.hpp>
# endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_PATH_FUNCTIONS */
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


template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline stlsoft_ns_qual(ss_size_t) path_squeeze(S const& path, C *buffer, stlsoft_ns_qual(ss_size_t) cchBuffer)
{
#if defined(PLATFORMSTL_OS_IS_UNIX)
    return unixstl_ns_qual(path_squeeze)(path, buffer, cchBuffer);
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
    return winstl_ns_qual(path_squeeze)(path, buffer, cchBuffer);
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */
}

/* ////////////////////////////////////////////////////////////////////// */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PATH_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
