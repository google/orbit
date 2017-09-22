/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/synch/atomic_types.h
 *
 * Purpose:     Definition of the atomic types.
 *
 * Created:     22nd March 2005
 * Updated:     7th June 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file platformstl/synch/atomic_types.h
 *
 * \brief [C, C++] Definition of the atomic types
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES
#define PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES_MAJOR     3
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES_MINOR     0
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES_REVISION	2
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES_EDIT      28
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[Incompatibilies-start]
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL
# include <platformstl/platformstl.h>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_TYPES
#  include <unixstl/synch/atomic_types.h>
# endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_TYPES */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_TYPES
#  include <winstl/synch/atomic_types.h>
# endif /* !WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_TYPES */
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

#if defined(__cplusplus)

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    namespace implementation
    {
# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
# if defined(PLATFORMSTL_OS_IS_UNIX)

#  ifndef PLATFORSL_DEFINED_platform_stl_
#   define PLATFORSL_DEFINED_platform_stl_
        namespace platform_stl_ =   ::unixstl;
#  endif /* !PLATFORSL_DEFINED_platform_stl_ */

# elif defined(PLATFORMSTL_OS_IS_WINDOWS)

#  ifndef PLATFORSL_DEFINED_platform_stl_
#   define PLATFORSL_DEFINED_platform_stl_
        namespace platform_stl_ =   ::winstl;
#  endif /* !PLATFORSL_DEFINED_platform_stl_ */

# else /* ? operating system */
#  error Operating system not discriminated
# endif /* operating system */
# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    }
# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

# if (   defined(PLATFORMSTL_OS_IS_UNIX) && \
         defined(_UNIXSTL_NO_NAMESPACE)) || \
     (   defined(PLATFORMSTL_OS_IS_WINDOWS) && \
         defined(_WINSTL_NO_NAMESPACE))
 /* Source atomic functions are defined within a namespace, either unixstl or winstl. */

    using atomic_int_t;

# else /* ? global */
 /* Source atomic functions are defined within the global namespace. */

    using implementation::platform_stl_::atomic_int_t;

# endif /* global */
#endif /* __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#if !defined(__cplusplus)
 /* Nothing defined in C */
#elif defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace platformstl */
#else
} /* namespace platformstl_project */
} /* namespace stlsoft */
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES */

/* ///////////////////////////// end of file //////////////////////////// */
