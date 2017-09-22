/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/synch/atomic_functions.h
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


/** \file unixstl/synch/atomic_functions.h
 *
 * \brief [C++ only] Definition of the atomic functions
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS
#define UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS_MAJOR     6
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS_MINOR     1
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS_REVISION  1
# define UNIXSTL_VER_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS_EDIT      201
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
#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_TYPES
# include <unixstl/synch/atomic_types.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_TYPES */

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
 * Features
 */

#ifdef UNIXSTL_HAS_ATOMIC_PREINCREMENT
# undef UNIXSTL_HAS_ATOMIC_PREINCREMENT
#endif /* UNIXSTL_HAS_ATOMIC_PREINCREMENT */
#ifdef UNIXSTL_HAS_ATOMIC_PREDECREMENT
# undef UNIXSTL_HAS_ATOMIC_PREDECREMENT
#endif /* UNIXSTL_HAS_ATOMIC_PREDECREMENT */
#ifdef UNIXSTL_HAS_ATOMIC_POSTINCREMENT
# undef UNIXSTL_HAS_ATOMIC_POSTINCREMENT
#endif /* UNIXSTL_HAS_ATOMIC_POSTINCREMENT */
#ifdef UNIXSTL_HAS_ATOMIC_POSTDECREMENT
# undef UNIXSTL_HAS_ATOMIC_POSTDECREMENT
#endif /* UNIXSTL_HAS_ATOMIC_POSTDECREMENT */
#ifdef UNIXSTL_HAS_ATOMIC_INCREMENT
# undef UNIXSTL_HAS_ATOMIC_INCREMENT
#endif /* UNIXSTL_HAS_ATOMIC_INCREMENT */
#ifdef UNIXSTL_HAS_ATOMIC_DECREMENT
# undef UNIXSTL_HAS_ATOMIC_DECREMENT
#endif /* UNIXSTL_HAS_ATOMIC_DECREMENT */
#ifdef UNIXSTL_HAS_ATOMIC_READ
# undef UNIXSTL_HAS_ATOMIC_READ
#endif /* UNIXSTL_HAS_ATOMIC_READ */
#ifdef UNIXSTL_HAS_ATOMIC_WRITE
# undef UNIXSTL_HAS_ATOMIC_WRITE
#endif /* UNIXSTL_HAS_ATOMIC_WRITE */
#ifdef UNIXSTL_HAS_ATOMIC_PREADD
# undef UNIXSTL_HAS_ATOMIC_PREADD
#endif /* UNIXSTL_HAS_ATOMIC_PREADD */
#ifdef UNIXSTL_HAS_ATOMIC_POSTADD
# undef UNIXSTL_HAS_ATOMIC_POSTADD
#endif /* UNIXSTL_HAS_ATOMIC_POSTADD */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)

/** \brief Indicates whether the atomic_preincrement function is defined
 *    for the current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_PREINCREMENT

/** \brief Indicates whether the atomic_predecrement function is defined
 *    for the current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_PREDECREMENT

/** \brief Indicates whether the atomic_postincrement function is defined
 *    for the current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_POSTINCREMENT

/** \brief Indicates whether the atomic_postdecrement function is defined
 *    for the current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_POSTDECREMENT

/** \brief Indicates whether the atomic_increment function is defined for
 *    the current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_INCREMENT

/** \brief Indicates whether the atomic_decrement function is defined for
 *    the current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_DECREMENT

/** \brief Indicates whether the atomic_read function is defined for the
 *    current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_READ

/** \brief Indicates whether the atomic_write function is defined for the
 *    current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_WRITE

/** \brief Indicates whether the atomic_preadd function is defined for the
 *    current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_PREADD

/** \brief Indicates whether the atomic_postadd function is defined for the
 *    current compiler/operating-system/architecture
 */
# define UNIXSTL_HAS_ATOMIC_POSTADD

#elif defined(UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS)

# if defined(UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS)


 /* ************************************
  * Forced
  */

#  ifndef UNIXSTL_FORCED_ATOMIC_INTEGER_IMPLEMENTATIONS
#   error If you are forcing atomic integer support (by defining UNIXSTL_FORCE_ATOMIC_INTEGER_OPERATIONS) you must also define UNIXSTL_FORCED_ATOMIC_INTEGER_IMPLEMENTATIONS as the header containing the atomic integer operations, which will be included
#  endif /* UNIXSTL_FORCED_ATOMIC_INTEGER_IMPLEMENTATIONS */
#  include UNIXSTL_FORCED_ATOMIC_INTEGER_IMPLEMENTATIONS


# elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_GCC_BUILTINS)


 /* ************************************
  * GCC builtins
  */

#  error This feature is not yet supported, and you should not be seeing this compilation path unless unixstl/synch/util/features.h is out of synch with this file; contact Synesis Software


# elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_WINDOWS_INTERLOCKED)


 /* ************************************
  * Windows Interlocked
  */

#  if !defined(UNIXSTL_NO_WIN32_NATIVE_ATOMIC_FUNCTIONS)

STLSOFT_INLINE atomic_int_t atomic_preincrement(atomic_int_t volatile* pl)
{
    return STLSOFT_NS_GLOBAL(InterlockedIncrement)((LPLONG)pl);
}
#   define UNIXSTL_HAS_ATOMIC_PREINCREMENT

STLSOFT_INLINE atomic_int_t atomic_predecrement(atomic_int_t volatile* pl)
{
    return STLSOFT_NS_GLOBAL(InterlockedDecrement)((LPLONG)pl);
}
#   define UNIXSTL_HAS_ATOMIC_PREDECREMENT

STLSOFT_INLINE atomic_int_t atomic_postincrement(atomic_int_t volatile* pl)
{
    atomic_int_t pre = *pl;

    STLSOFT_NS_GLOBAL(InterlockedIncrement)((LPLONG)pl);

    return pre;
}
#   define UNIXSTL_HAS_ATOMIC_POSTINCREMENT

STLSOFT_INLINE atomic_int_t atomic_postdecrement(atomic_int_t volatile* pl)
{
    atomic_int_t pre = *pl;

    STLSOFT_NS_GLOBAL(InterlockedDecrement)((LPLONG)pl);

    return pre;
}
#   define UNIXSTL_HAS_ATOMIC_POSTDECREMENT

STLSOFT_INLINE void atomic_increment(atomic_int_t volatile* pl)
{
    STLSOFT_NS_GLOBAL(InterlockedIncrement)((LPLONG)pl);
}
#   define UNIXSTL_HAS_ATOMIC_INCREMENT

STLSOFT_INLINE void atomic_decrement(atomic_int_t volatile* pl)
{
    STLSOFT_NS_GLOBAL(InterlockedDecrement)((LPLONG)pl);
}
#   define UNIXSTL_HAS_ATOMIC_DECREMENT

#  endif /* !UNIXSTL_NO_WIN32_NATIVE_ATOMIC_FUNCTIONS */

/* NOTE: We allow atomic_write(), since on almost all platforms this'll be fine */

STLSOFT_INLINE atomic_int_t atomic_write(atomic_int_t volatile* pv, atomic_int_t n)
{
    return stlsoft_static_cast(atomic_int_t, STLSOFT_NS_GLOBAL(InterlockedExchange)(stlsoft_c_cast(LPLONG, pv), n));
}
#  define UNIXSTL_HAS_ATOMIC_WRITE

#  if !defined(UNIXSTL_NO_WIN32_NATIVE_ATOMIC_FUNCTIONS)

STLSOFT_INLINE atomic_int_t atomic_read(atomic_int_t volatile* pv)
{
    return *pv;
}
#   define UNIXSTL_HAS_ATOMIC_READ

/* STLSOFT_INLINE */ atomic_int_t atomic_preadd(atomic_int_t volatile* pl, atomic_int_t n);

STLSOFT_INLINE atomic_int_t atomic_postadd(atomic_int_t volatile* pl, atomic_int_t n)
{
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd)((LPLONG)pl, n);
}
#   define UNIXSTL_HAS_ATOMIC_POSTADD

#  endif /* !UNIXSTL_NO_WIN32_NATIVE_ATOMIC_FUNCTIONS */


# elif defined(UNIXSTL_ATOMIC_INTEGER_OPERATIONS_VIA_MACOSX)


 /* ************************************
  * Mac OS-X
  */

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_preincrement(atomic_int_t volatile* pl)
{
    return STLSOFT_NS_GLOBAL(OSAtomicIncrement32Barrier)(stlsoft_const_cast(atomic_int_t*, pl));
}
#  define UNIXSTL_HAS_ATOMIC_PREINCREMENT

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_predecrement(atomic_int_t volatile* pl)
{
    return STLSOFT_NS_GLOBAL(OSAtomicDecrement32Barrier)(stlsoft_const_cast(atomic_int_t*, pl));
}
#  define UNIXSTL_HAS_ATOMIC_PREDECREMENT

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_postincrement(atomic_int_t volatile* pl)
{
    return STLSOFT_NS_GLOBAL(OSAtomicIncrement32Barrier)(stlsoft_const_cast(atomic_int_t*, pl)) - 1;
}
#  define UNIXSTL_HAS_ATOMIC_POSTINCREMENT

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_postdecrement(atomic_int_t volatile* pl)
{
    return STLSOFT_NS_GLOBAL(OSAtomicDecrement32Barrier)(stlsoft_const_cast(atomic_int_t*, pl)) + 1;
}
#  define UNIXSTL_HAS_ATOMIC_POSTDECREMENT

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE void atomic_increment(atomic_int_t volatile* pl)
{
    STLSOFT_NS_GLOBAL(OSAtomicIncrement32Barrier)(stlsoft_const_cast(atomic_int_t*, pl));
}
#  define UNIXSTL_HAS_ATOMIC_INCREMENT

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE void atomic_decrement(atomic_int_t volatile* pl)
{
    STLSOFT_NS_GLOBAL(OSAtomicDecrement32Barrier)(stlsoft_const_cast(atomic_int_t*, pl));
}
#  define UNIXSTL_HAS_ATOMIC_DECREMENT

/** \brief
 *
 * \ingroup group__library__synch
 */
/** \brief Note: atomic_write() for PowerPC is not yet defined. If you wish to suggest an
 * implementation, it will be most welcome.
 *
 * \ingroup group__library__synch
 */
/* STLSOFT_INLINE */ atomic_int_t atomic_write(atomic_int_t volatile* pv, atomic_int_t n);

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_read(atomic_int_t volatile* pv)
{
    STLSOFT_NS_GLOBAL(OSMemoryBarrier)();

    return *pv;
}
#  define UNIXSTL_HAS_ATOMIC_READ

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_preadd(atomic_int_t volatile* pl, atomic_int_t n)
{
    return STLSOFT_NS_GLOBAL(OSAtomicAdd32Barrier)(n, stlsoft_const_cast(atomic_int_t*, pl));
}
#  define UNIXSTL_HAS_ATOMIC_PREADD

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_postadd(atomic_int_t volatile* pl, atomic_int_t n)
{
    return STLSOFT_NS_GLOBAL(OSAtomicAdd32Barrier)(n, stlsoft_const_cast(atomic_int_t*, pl)) - n;
}
#  define UNIXSTL_HAS_ATOMIC_POSTADD


# else
#  error Atomic integer operations not supported: see unixstl/synch/util/features.h for details
# endif /* ? */


/* ////////////////////////////////////////////////////////////////////// */

# if 0

STLSOFT_INLINE atomic_int_t atomic_read(atomic_int_t volatile* pv);

/* # define UNIXSTL_HAS_ATOMIC_READ */

STLSOFT_INLINE atomic_int_t atomic_write(atomic_int_t volatile* pv, atomic_int_t n)
{
    atomic_int_t    oldval;

    /* Note: the "xchg" instruction does not need a "lock" prefix */
#  ifdef STLSOFT_COMPILER_IS_GCC
    __asm__ __volatile__(   "xchgl %0, %1"      /* long (32-bit) xchg, from */
                        :   "=r"(oldval),   "=m"(*(pv))
                        :   "0"(n),         "m"(*(pv))
                        :   "memory");
#  else /* ? compiler */
    _asm
    {
        mov ecx, dword ptr [pv]
        mov eax, n
        xchg dword ptr [ecx], eax
        mov oldval, eax
    }
#  endif /* compiler */

    return oldval;
}

#endif

/* ////////////////////////////////////////////////////////////////////// */

#endif /* UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/atomic_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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
