/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/synch/util/features.h
 *
 * Purpose:     Discrimination of synchronisation features.
 *
 * Created:     23rd October 1997
 * Updated:     4th April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1997-2010, Matthew Wilson and Synesis Software
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


/** \file unixstl/synch/util/features.h
 *
 * \brief [C++ only] Discrimination of synchronisation features
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES
#define UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYNCH_UTIL_H_FEATURES_MAJOR    1
# define UNIXSTL_VER_UNIXSTL_SYNCH_UTIL_H_FEATURES_MINOR    1
# define UNIXSTL_VER_UNIXSTL_SYNCH_UTIL_H_FEATURES_REVISION 2
# define UNIXSTL_VER_UNIXSTL_SYNCH_UTIL_H_FEATURES_EDIT     7
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */

#ifndef STLSOFT_INCL_H_UNISTD
# define STLSOFT_INCL_H_UNISTD
# include <unistd.h> /* Required for definition of _POSIX_THREADS on some impls. */
#endif /* !STLSOFT_INCL_H_UNISTD */

/* /////////////////////////////////////////////////////////////////////////
 * Features
 *
 * 1. UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
 * 2. UNIXSTL_USING_PTHREADS
 */

/* 1. UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \def UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
 *
 * \brief If defined, indicates that the operating system supports atomic
 *   integer operations, and that they're safe for use outside the kernel.
 *
 * Atomic integer operations are assumed on the following "UNIX" platforms:
 *
 * - Mac OS-X (via OSMemoryBarrier(), etc; &lt;libkern/OSAtomic.h>)
 * - Win32 (via InterlockedIncrement(), etc; &lt;windows.h>)
 *
 * or:
 *
 * - the symbol UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS is defined. In
 *   this case, you must also define UNIXSTL_FORCED_ATOMIC_INT_T (as the
 *   atomic integer type, e.g. int) and
 *   UNIXSTL_FORCED_ATOMIC_INTEGER_IMPLEMENTATIONS (as the file to be
 *   included _inside_ the unixstl namespace in
 *   unixstl/synch/atomic_functions.h)
 */
# define UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

# ifdef UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
#  undef UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# endif /* UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS */

# ifdef UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_GCC_BUILTINS
#  undef UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_GCC_BUILTINS
# endif /* UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_GCC_BUILTINS */

# ifdef UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX
#  undef UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX
# endif /* UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX */

# ifdef UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED
#  undef UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED
# endif /* UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED */


# if \
    defined(UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS)
#  define UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# elif \
    (   defined(_WIN32) || \
        defined(_WIN64)) && \
    !defined(UNIXSTL_NO_ATOMIC_INTEGER_OPERATIONS_ON_WINDOWS)
#  define UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED
#  define UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# elif \
    defined(UNIXSTL_OS_IS_MACOSX)
#  define UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX
#  define UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# endif

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */


/* 1. UNIXSTL_USING_PTHREADS */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \def UNIXSTL_USING_PTHREADS
 *
 * \brief If defined, indicates that PThreads is selected.
 *
 * PThreads is selected if:
 *
 * - _POSIX_THREADS is defined and _REENTRANT is defined, or
 * - UNIXSTL_FORCE_PTHREADS is defined, which causes &lt;pthread.h> to be
 *    included, and the symbol _REENTRANT (and _POSIX_THREADS on Windows)
 *    to be defined if they are not already.
 */
# define UNIXSTL_USING_PTHREADS

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

# ifdef UNIXSTL_USING_PTHREADS
#  undef UNIXSTL_USING_PTHREADS
# endif /* UNIXSTL_USING_PTHREADS */

# if defined(UNIXSTL_FORCE_PTHREADS)
#  if defined(_STLSOFT_FORCE_ANY_COMPILER) && \
      (   defined(_WIN32) || \
          defined(_WIN64))
    /* Emulating UNIX on Win32 */
#   if !defined(__MT__) && \
       !defined(_MT)
#    error Cannot force PThreads on Windows unless multi-threaded compilation is enabled
#   else /* ? MT */
#    ifndef _POSIX_THREADS
#     define _POSIX_THREADS
#    endif /* !_POSIX_THREADS */
#    ifndef _REENTRANT
#     define _REENTRANT
#    endif /* !_REENTRANT */
#   endif /* MT */
#  else /* ? compiler / OS */
#   ifndef _POSIX_THREADS
#    error Cannot force PThreads on a system on which _POSIX_THREADS is not defined
#   endif /* !_POSIX_THREADS */
#   ifndef _REENTRANT
#    define _REENTRANT
#   endif /* !_REENTRANT */
#  endif /* compiler / OS */
# endif /* UNIXSTL_FORCE_PTHREADS */

# if defined(_REENTRANT)
#  define UNIXSTL_USING_PTHREADS
# endif /* _REENTRANT */

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES */

/* ///////////////////////////// end of file //////////////////////////// */
