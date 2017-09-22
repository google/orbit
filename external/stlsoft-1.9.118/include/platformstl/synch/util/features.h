/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/synch/util/features.h
 *
 * Purpose:     Discrimination of synchronisation features.
 *
 * Created:     15th January 2007
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2007-2009, Matthew Wilson and Synesis Software
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


/** \file platformstl/synch/util/features.h
 *
 * \brief [C, C++] Discrimination of synchronisation features
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_UTIL_H_FEATURES
#define PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_UTIL_H_FEATURES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_UTIL_H_FEATURES_MAJOR    1
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_UTIL_H_FEATURES_MINOR    1
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_UTIL_H_FEATURES_REVISION 2
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_UTIL_H_FEATURES_EDIT     5
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL
# include <platformstl/platformstl.h>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL */
#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES
#  include <unixstl/synch/util/features.h>
# endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* /////////////////////////////////////////////////////////////////////////
 * Features
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# ifdef PLATFORMSTL_HAS_ATOMIC_INTEGER_OPERATIONS
#  undef PLATFORMSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# endif /* PLATFORMSTL_HAS_ATOMIC_INTEGER_OPERATIONS */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifdef UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
#  define PLATFORMSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# endif /* UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# define PLATFORMSTL_HAS_ATOMIC_INTEGER_OPERATIONS  /* Windows always has it */
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_UTIL_H_FEATURES */

/* ///////////////////////////// end of file //////////////////////////// */
