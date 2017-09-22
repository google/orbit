/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/synch/atomic_functions.h
 *
 * Purpose:     Definition of the atomic functions.
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


/** \file platformstl/synch/atomic_functions.h
 *
 * \brief [C, C++] Definition of the atomic functions
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS
#define PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS_MAJOR     2
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS_MINOR     3
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS_REVISION  2
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS_EDIT      29
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
#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES
# include <platformstl/synch/atomic_types.h>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_TYPES */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS
#  include <unixstl/synch/atomic_functions.h>
# endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS
#  include <winstl/synch/atomic_functions.h>
# endif /* !WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS */
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

/* /////////////////////////////////////////////////////////////////////////
 * Features
 */

#ifdef PLATFORMSTL_HAS_ATOMIC_PREINCREMENT
# undef PLATFORMSTL_HAS_ATOMIC_PREINCREMENT
#endif /* PLATFORMSTL_HAS_ATOMIC_PREINCREMENT */
#ifdef PLATFORMSTL_HAS_ATOMIC_PREDECREMENT
# undef PLATFORMSTL_HAS_ATOMIC_PREDECREMENT
#endif /* PLATFORMSTL_HAS_ATOMIC_PREDECREMENT */
#ifdef PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT
# undef PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT
#endif /* PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT */
#ifdef PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT
# undef PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT
#endif /* PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT */
#ifdef PLATFORMSTL_HAS_ATOMIC_INCREMENT
# undef PLATFORMSTL_HAS_ATOMIC_INCREMENT
#endif /* PLATFORMSTL_HAS_ATOMIC_INCREMENT */
#ifdef PLATFORMSTL_HAS_ATOMIC_DECREMENT
# undef PLATFORMSTL_HAS_ATOMIC_DECREMENT
#endif /* PLATFORMSTL_HAS_ATOMIC_DECREMENT */
#ifdef PLATFORMSTL_HAS_ATOMIC_READ
# undef PLATFORMSTL_HAS_ATOMIC_READ
#endif /* PLATFORMSTL_HAS_ATOMIC_READ */
#ifdef PLATFORMSTL_HAS_ATOMIC_WRITE
# undef PLATFORMSTL_HAS_ATOMIC_WRITE
#endif /* PLATFORMSTL_HAS_ATOMIC_WRITE */
#ifdef PLATFORMSTL_HAS_ATOMIC_PREADD
# undef PLATFORMSTL_HAS_ATOMIC_PREADD
#endif /* PLATFORMSTL_HAS_ATOMIC_PREADD */
#ifdef PLATFORMSTL_HAS_ATOMIC_POSTADD
# undef PLATFORMSTL_HAS_ATOMIC_POSTADD
#endif /* PLATFORMSTL_HAS_ATOMIC_POSTADD */

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

#  if defined(PLATFORMSTL_OS_IS_UNIX)

#   ifdef UNIXSTL_HAS_ATOMIC_PREINCREMENT
    using atomic_preincrement;
#    define PLATFORMSTL_HAS_ATOMIC_PREINCREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_PREINCREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_PREDECREMENT
    using atomic_predecrement;
#    define PLATFORMSTL_HAS_ATOMIC_PREDECREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_PREDECREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_POSTINCREMENT
    using atomic_postincrement;
#    define PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_POSTINCREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_POSTDECREMENT
    using atomic_postdecrement;
#    define PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_POSTDECREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_INCREMENT
    using atomic_increment;
#    define PLATFORMSTL_HAS_ATOMIC_INCREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_INCREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_DECREMENT
    using atomic_decrement;
#    define PLATFORMSTL_HAS_ATOMIC_DECREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_DECREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_READ
    using atomic_read;
#    define PLATFORMSTL_HAS_ATOMIC_READ
#   endif /* UNIXSTL_HAS_ATOMIC_READ */
#   ifdef UNIXSTL_HAS_ATOMIC_WRITE
    using atomic_write;
#    define PLATFORMSTL_HAS_ATOMIC_WRITE
#   endif /* UNIXSTL_HAS_ATOMIC_WRITE */
#   ifdef UNIXSTL_HAS_ATOMIC_PREADD
    using atomic_preadd;
#    define PLATFORMSTL_HAS_ATOMIC_PREADD
#   endif /* UNIXSTL_HAS_ATOMIC_PREADD */
#   ifdef UNIXSTL_HAS_ATOMIC_POSTADD
    using atomic_postadd;
#    define PLATFORMSTL_HAS_ATOMIC_POSTADD
#   endif /* UNIXSTL_HAS_ATOMIC_POSTADD */

#  elif defined(PLATFORMSTL_OS_IS_WINDOWS)
   /* OS: Win32 */
#   if defined(UNIXSTL_ARCH_IS_X86) || \
       defined(UNIXSTL_ARCH_IS_IA64) || \
       defined(UNIXSTL_ARCH_IS_X64)
    /* Arch: i386 */
    using atomic_preincrement_up;
    using atomic_predecrement_up;
    using atomic_postincrement_up;
    using atomic_postdecrement_up;
    using atomic_increment_up;
    using atomic_decrement_up;
    using atomic_write_up;
    using atomic_read_up;
    using atomic_postadd_up;
    using atomic_preadd_up;
    using atomic_preincrement_smp;
    using atomic_predecrement_smp;
    using atomic_postincrement_smp;
    using atomic_postdecrement_smp;
    using atomic_increment_smp;
    using atomic_decrement_smp;
    using atomic_write_smp;
    using atomic_read_smp;
    using atomic_postadd_smp;
    using atomic_preadd_smp;
    using atomic_preincrement;
    using atomic_predecrement;
    using atomic_postincrement;
    using atomic_postdecrement;
    using atomic_increment;
    using atomic_decrement;
    using atomic_write;
    using atomic_read;
    using atomic_postadd;
    using atomic_preadd;

#    define PLATFORMSTL_HAS_ATOMIC_PREINCREMENT
#    define PLATFORMSTL_HAS_ATOMIC_PREDECREMENT
#    define PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT
#    define PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT
#    define PLATFORMSTL_HAS_ATOMIC_INCREMENT
#    define PLATFORMSTL_HAS_ATOMIC_DECREMENT
#    define PLATFORMSTL_HAS_ATOMIC_READ
#    define PLATFORMSTL_HAS_ATOMIC_WRITE
#    define PLATFORMSTL_HAS_ATOMIC_PREADD
#    define PLATFORMSTL_HAS_ATOMIC_POSTADD

#   else /* ? arch */
#    error Not valid for processors other than Intel
#   endif /* arch */
#  else /* ? operating system */
#   error Operating system not discriminated
#  endif /* operating system */
# else /* ? global */
 /* Source atomic functions are defined within the global namespace. */

#  if defined(PLATFORMSTL_OS_IS_UNIX)

#   ifdef UNIXSTL_HAS_ATOMIC_PREINCREMENT
    using implementation::platform_stl_::atomic_preincrement;
#    define PLATFORMSTL_HAS_ATOMIC_PREINCREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_PREINCREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_PREDECREMENT
    using implementation::platform_stl_::atomic_predecrement;
#    define PLATFORMSTL_HAS_ATOMIC_PREDECREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_PREDECREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_POSTINCREMENT
    using implementation::platform_stl_::atomic_postincrement;
#    define PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_POSTINCREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_POSTDECREMENT
    using implementation::platform_stl_::atomic_postdecrement;
#    define PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_POSTDECREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_INCREMENT
    using implementation::platform_stl_::atomic_increment;
#    define PLATFORMSTL_HAS_ATOMIC_INCREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_INCREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_DECREMENT
    using implementation::platform_stl_::atomic_decrement;
#    define PLATFORMSTL_HAS_ATOMIC_DECREMENT
#   endif /* UNIXSTL_HAS_ATOMIC_DECREMENT */
#   ifdef UNIXSTL_HAS_ATOMIC_READ
    using implementation::platform_stl_::atomic_read;
#    define PLATFORMSTL_HAS_ATOMIC_READ
#   endif /* UNIXSTL_HAS_ATOMIC_READ */
#   ifdef UNIXSTL_HAS_ATOMIC_WRITE
    using implementation::platform_stl_::atomic_write;
#    define PLATFORMSTL_HAS_ATOMIC_WRITE
#   endif /* UNIXSTL_HAS_ATOMIC_WRITE */
#   ifdef UNIXSTL_HAS_ATOMIC_PREADD
    using implementation::platform_stl_::atomic_preadd;
#    define PLATFORMSTL_HAS_ATOMIC_PREADD
#   endif /* UNIXSTL_HAS_ATOMIC_PREADD */
#   ifdef UNIXSTL_HAS_ATOMIC_POSTADD
    using implementation::platform_stl_::atomic_postadd;
#    define PLATFORMSTL_HAS_ATOMIC_POSTADD
#   endif /* UNIXSTL_HAS_ATOMIC_POSTADD */

#  elif defined(PLATFORMSTL_OS_IS_WINDOWS)
   /* OS: Win32 */
#   if defined(WINSTL_ARCH_IS_X86) || \
       defined(WINSTL_ARCH_IS_IA64) || \
       defined(WINSTL_ARCH_IS_X64)
    /* Arch: i386 */
    using implementation::platform_stl_::atomic_preincrement_up;
    using implementation::platform_stl_::atomic_predecrement_up;
    using implementation::platform_stl_::atomic_postincrement_up;
    using implementation::platform_stl_::atomic_postdecrement_up;
    using implementation::platform_stl_::atomic_increment_up;
    using implementation::platform_stl_::atomic_decrement_up;
    using implementation::platform_stl_::atomic_write_up;
    using implementation::platform_stl_::atomic_read_up;
    using implementation::platform_stl_::atomic_postadd_up;
    using implementation::platform_stl_::atomic_preadd_up;
    using implementation::platform_stl_::atomic_preincrement_smp;
    using implementation::platform_stl_::atomic_predecrement_smp;
    using implementation::platform_stl_::atomic_postincrement_smp;
    using implementation::platform_stl_::atomic_postdecrement_smp;
    using implementation::platform_stl_::atomic_increment_smp;
    using implementation::platform_stl_::atomic_decrement_smp;
    using implementation::platform_stl_::atomic_write_smp;
    using implementation::platform_stl_::atomic_read_smp;
    using implementation::platform_stl_::atomic_postadd_smp;
    using implementation::platform_stl_::atomic_preadd_smp;
    using implementation::platform_stl_::atomic_preincrement;
    using implementation::platform_stl_::atomic_predecrement;
    using implementation::platform_stl_::atomic_postincrement;
    using implementation::platform_stl_::atomic_postdecrement;
    using implementation::platform_stl_::atomic_increment;
    using implementation::platform_stl_::atomic_decrement;
    using implementation::platform_stl_::atomic_write;
    using implementation::platform_stl_::atomic_read;
    using implementation::platform_stl_::atomic_postadd;
    using implementation::platform_stl_::atomic_preadd;

#    define PLATFORMSTL_HAS_ATOMIC_PREINCREMENT
#    define PLATFORMSTL_HAS_ATOMIC_PREDECREMENT
#    define PLATFORMSTL_HAS_ATOMIC_POSTINCREMENT
#    define PLATFORMSTL_HAS_ATOMIC_POSTDECREMENT
#    define PLATFORMSTL_HAS_ATOMIC_INCREMENT
#    define PLATFORMSTL_HAS_ATOMIC_DECREMENT
#    define PLATFORMSTL_HAS_ATOMIC_READ
#    define PLATFORMSTL_HAS_ATOMIC_WRITE
#    define PLATFORMSTL_HAS_ATOMIC_PREADD
#    define PLATFORMSTL_HAS_ATOMIC_POSTADD

#   else /* ? arch */
#    error Not valid for processors other than Intel
#   endif /* arch */
#  else /* ? operating system */
#   error Operating system not discriminated
#  endif /* operating system */
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

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_H_ATOMIC_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
