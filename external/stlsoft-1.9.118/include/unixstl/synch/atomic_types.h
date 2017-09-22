/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/synch/atomic_types.h
 *
 * Purpose:     UNIXSTL atomic functions.
 *
 * Created:     23rd October 1997
 * Updated:     29th April 2010
 *
 * Thanks:      To Brad Cox, for helping out in testing and fixing the
 *              implementation for MAC OSX (Intel).
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


/** \file unixstl/synch/atomic_types.h
 *
 * \brief [C++ only] Definition of atomic types
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_TYPES
#define UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_TYPES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_TYPES_MAJOR     7
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_TYPES_MINOR     0
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_TYPES_REVISION  1
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_TYPES_EDIT      201
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES
# include <unixstl/synch/util/features.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES */

#if defined(UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS)
 /* Nothing to include here; UNIXSTL_FORCED_ATOMIC_INTEGER_IMPLEMENTATIONS will be included inside unixstl namespace */
#elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_GCC_BUILTINS)
 /* Nothing to include, since using built-ins */
#elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX)
# include <libkern/OSAtomic.h>
#elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED)
# include <windows.h>
#else
# error Atomic integer operations not supported: see unixstl/synch/util/features.h for details
#endif /* ? */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 */

#if defined(UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS)
# ifndef UNIXSTL_FORCED_ATOMIC_INT_T
#  error If you are forcing atomic integer support (by defining UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS) you must also define UNIXSTL_FORCED_ATOMIC_INT_T
# endif /* UNIXSTL_FORCED_ATOMIC_INT_T */
typedef UNIXSTL_FORCED_ATOMIC_INT_T     atomic_int_t;
#elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_GCC_BUILTINS)
typedef volatile int                    atomic_int_t;
# elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED)
typedef us_sint32_t                     atomic_int_t;
# elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX)
typedef STLSOFT_NS_GLOBAL(int32_t)      atomic_int_t;
#else
# error Atomic integer operations not supported: see unixstl/synch/util/features.h for details
#endif

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace unixstl */
# else
} /* namespace unixstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
