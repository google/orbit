/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/std/library_discriminator.hpp
 *
 * Purpose:     Discriminates between standard library implementations
 *
 * Created:     2nd January 2000
 * Updated:     13th December 2012
 *
 * Thanks:      To Gabor Fischer, for reporting problems with VC++ 9/10
 *              compatibility, and persisting in (re-)reporting it even when
 *              I was being a thickie and unable to reproduce it.              
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2000-2012, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/util/std/library_discriminator.hpp
 *
 * \brief [C++ only] Discrimination and identification of standard library
 *   implementations
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR
#define STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR_MAJOR       4
# define STLSOFT_VER_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR_MINOR       8
# define STLSOFT_VER_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR_REVISION    1
# define STLSOFT_VER_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR_EDIT        107
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#if !defined(STLSOFT_COMPILER_IS_WATCOM)
# include <iterator>    // required for detecting header include guards
#endif /* compiler */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER >= 1500
# include <functional>
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Warnings
 */

/* This is here temporarily, until a better solution can be found. */
#ifdef STLSOFT_COMPILER_IS_MSVC
# pragma warning(disable : 4097)    // suppresses: typedef-name 'identifier1' used as synonym for class-name 'identifier2'
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Standard library identification
 *
 * Currently recognised libraries are:          (identifying header include guards)
 *
 * 1. Dinkumware (shipping with Visual C++)     _ITERATOR_, _UTILITY_, _XSTDDEF_
 * 2. Metrowerks' MSL                           _ITERATOR, _MSLCONFIG (and __MSL_CPP__; well done Metrowerks!)
 * 3. STLport                                   _STLP_INTERNAL_ITERATOR_H (and _STLPORT_VERSION)
 * 4. HP/SGI, including HP/SGI/Comeau           __SGI_STL_INTERNAL_ITERATOR_H
 * 5. Gnu FSF's HP/SGI derivative               __GLIBCPP_INTERNAL_ITERATOR_H, _GLIBCXX_ITERATOR
 * 6. HP/RW                                     __RW_ITERATOR_H. __STD_RW_ITERATOR__
 * 7. Sun Pro/RW                                __STD_ITERATOR__
 * 8. Watcom (patch)                            STLSOFT_OW12_INCL_ITERATOR
 * 9. Watcom (none)                             STLSOFT_COMPILER_IS_WATCOM
 */

/* The inclusion of <iterator> results in the following inclusions when using one
 * of the dinkumware libraries:
 *
 * VC++ 4.2: use_ansi.h, utility { use_ansi.h, iosfwd { use_ansi.h, cstdio { stdio.h }, cstring { string.h }, cwchar { wchar.h }, xstddef { yvals.h, cstddef { stddef.h } } } }
 * VC++ 5:   utility { iosfwd { cstdio, cstring, cwchar, xstddef { yvals.h, cstddef } } }
 * VC++ 6:   utility { iosfwd { cstdio, cstring, cwchar, xstddef { yvals.h, cstddef } } }
 * VC++ 7.0: xutility { climits, utility { iosfwd { cstdio, cstring, cwchar, xstddef { yvals.h, cstddef } } }
 * VC++ 7.1: xutility { climits, utility { iosfwd { cstdio, cstring, cwchar, xstddef { yvals.h, cstddef } } }
 *
 */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE
# undef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE
#endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_BORLAND
# undef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_BORLAND
#endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_BORLAND */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC
# undef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC
#endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC */

/* Includes: iterator { mslconfig, cstddef, iosfwd, msl_utility } */
#ifdef STLSOFT_CF_STD_LIBRARY_IS_MSL
# undef STLSOFT_CF_STD_LIBRARY_IS_MSL
#endif /* STLSOFT_CF_STD_LIBRARY_IS_MSL */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_STLPORT
# undef STLSOFT_CF_STD_LIBRARY_IS_STLPORT
#endif /* STLSOFT_CF_STD_LIBRARY_IS_STLPORT */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_HP_SGI
# undef STLSOFT_CF_STD_LIBRARY_IS_HP_SGI
#endif /* STLSOFT_CF_STD_LIBRARY_IS_HP_SGI */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_HP_SGI_GNU
# undef STLSOFT_CF_STD_LIBRARY_IS_HP_SGI_GNU
#endif /* STLSOFT_CF_STD_LIBRARY_IS_HP_SGI_GNU */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_HP_RW
# undef STLSOFT_CF_STD_LIBRARY_IS_HP_RW
#endif /* STLSOFT_CF_STD_LIBRARY_IS_HP_RW */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW
# undef STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW
#endif /* STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_WATCOM_NONE
# undef STLSOFT_CF_STD_LIBRARY_IS_WATCOM_NONE
#endif /* STLSOFT_CF_STD_LIBRARY_IS_WATCOM_NONE */

#ifdef STLSOFT_CF_STD_LIBRARY_IS_WATCOM_PATCH
# undef STLSOFT_CF_STD_LIBRARY_IS_WATCOM_PATCH
#endif /* STLSOFT_CF_STD_LIBRARY_IS_WATCOM_PATCH */

#if defined(_STLPORT_VERSION) && \
      defined(_STLP_INTERNAL_ITERATOR_H)
 /* STLport */
# ifdef STLSOFT_COMPILE_VERBOSE
#  pragma message("Standard library is STLport")
# endif /* STLSOFT_COMPILE_VERBOSE */
# define STLSOFT_CF_STD_LIBRARY_IS_STLPORT
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "STLport"
#elif defined(_ITERATOR_) && \
      defined(_UTILITY_) && \
      defined(_XSTDDEF_)
 /* Dinkumware */
# define STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE
# if defined(STLSOFT_COMPILER_IS_INTEL) || \
     defined(STLSOFT_COMPILER_IS_MSVC)
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Standard library is Dinkumware (VC++)")
#  endif /* STLSOFT_COMPILE_VERBOSE */
#  define STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC
#  define STLSOFT_CF_STD_LIBRARY_NAME_STRING            "Dinkumware (VC++)"
# elif defined(STLSOFT_COMPILER_IS_BORLAND)
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Standard library is Dinkumware (Borland)")
#  endif /* STLSOFT_COMPILE_VERBOSE */
#  define STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_BORLAND
#  define STLSOFT_CF_STD_LIBRARY_NAME_STRING            "Dinkumware (Borland)"
# else /* ? compiler */
#  error STLSoft does not currently recognise combination of any compilers except Borland, Intel and Microsoft with the Dinkumware libraries.
# endif /* compiler */
#elif defined(_ITERATOR) && \
      defined(_MSLCONFIG) && \
      defined(__MSL_CPP__)
 /* MSL */
# ifdef STLSOFT_COMPILE_VERBOSE
#  pragma message("Standard library is MSL")
# endif /* STLSOFT_COMPILE_VERBOSE */
# define STLSOFT_CF_STD_LIBRARY_IS_MSL
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "MSL"
#elif defined(__SGI_STL_INTERNAL_ITERATOR_H)
# ifdef STLSOFT_COMPILE_VERBOSE
#  pragma message("Standard library is HP/SGI")
# endif /* STLSOFT_COMPILE_VERBOSE */
# define STLSOFT_CF_STD_LIBRARY_IS_HP_SGI
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "HP/SGI"
#elif defined(__GLIBCPP_INTERNAL_ITERATOR_H) || \
      defined(_GLIBCXX_ITERATOR)
 /* HP/SGI/GnuFSF */
# ifdef STLSOFT_COMPILE_VERBOSE
#  pragma message("Standard library is HP/SGI/GnuFSF")
# endif /* STLSOFT_COMPILE_VERBOSE */
# define STLSOFT_CF_STD_LIBRARY_IS_HP_SGI_GNU
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "HP/SGI/GnuFSF"
#elif defined(__RW_ITERATOR_H) && \
      defined(__STD_RW_ITERATOR__)
 /* HP/RW */
# ifdef STLSOFT_COMPILE_VERBOSE
#  pragma message("Standard library is HP/RW")
# endif /* STLSOFT_COMPILE_VERBOSE */
# define STLSOFT_CF_STD_LIBRARY_IS_HP_RW
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "HP/SGI/RW"
#elif defined(STLSOFT_COMPILER_IS_SUNPRO) && \
      defined(__STD_ITERATOR__)
 /* Sun Pro/RW */
# define STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "SunPro/RW"
#elif defined(STLSOFT_OW12_INCL_ITERATOR)
 /* Watcom (patch) */
# define STLSOFT_CF_STD_LIBRARY_IS_WATCOM_PATCH
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "STLSoft Watcom Patch"
#elif defined(STLSOFT_COMPILER_IS_WATCOM)
 /* Watcom (none) */
# define STLSOFT_CF_STD_LIBRARY_IS_WATCOM_NONE
# define STLSOFT_CF_STD_LIBRARY_NAME_STRING             "<no standard library with Open Watcom>"
#else /* ? */
# error Standard library implementation not recognised
#endif /* various "unique" macros */

/* Detecting presence of Dinkumware is easy (as shown above). The fun is in
 * differentiating between versions of the library, because there is no
 * version information contained in any of the headers, until version 7
 * of the compiler.
 *
 * _STCONS               in      5, 6, 7.0, 7.1, 8,  9,    10
 * _TEMPLATE_MEMBER      in            7.0
 * _TEMPLATE             in            7.0
 * _MESG                 in            7.0, 7.1, 8,  9,    10
 * _HAS_EXCEPTIONS       in            7.0, 7.1, 8,  9,    10
 * _EMPTY_ARGUMENT       in                      8,  9,    10
 * _THROWS               in                          9,    10
 * _IS_YES               in                          (9),  10
 *
 * _CPPLIB_VER           in      -  -  310  313  405 503/5 520
 *
 * _XTREE_ is in 6, 7.0, 7.1, but not in 4.2, 5. Of course, this means including it, which is a PITA!
 *
 * _INC_ASSERT (from assert.h) is in 6, but not in 4.2, 5, 7.0, 7.1. This
 * one is even dodgier than the rest, since if _STLSOFT_NO_ASSERT is
 * specified, asserts will not even be included.
 */

#ifdef STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION
# undef STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION
#endif /* STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION */

#define STLSOFT_CF_DINKUMWARE_VC_VERSION_UNKNOWN        (0x0000)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_4_2            (0x0402)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_5_0            (0x0500)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_6_0            (0x0600)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0            (0x0700)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1            (0x0701)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_8_0            (0x0800)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_9_0            (0x0900)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_10_0           (0x0a00)
#define STLSOFT_CF_DINKUMWARE_VC_VERSION_11_0           (0x0b00)

#ifdef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC
# if defined(STLSOFT_COMPILER_IS_INTEL) && \
     defined(unix)
  /* Ok to have Intel + Dinkumware without VC++ on Linux */
# elif defined(__BORLANDC__)
# elif defined(_MSC_VER)
# else /* ? compiler */
#  error When the Dinkumware-VC library is used, STLSoft requires that Visual C++ or a compatible compiler (e.g. DMC++, Comeau, CodeWarrior, Intel) is used
# endif /* compiler */

# if defined(_CPPLIB_VER)
#  if _CPPLIB_VER < 300
#   error Dinkumware C++ Library version unrecognised

#  elif _CPPLIB_VER <= 310
  /* Version 7.0 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 7.0")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0

#  elif _CPPLIB_VER <= 313
  /* Version 7.1 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 7.1")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

#  elif _CPPLIB_VER <= 405
  /* Version 8.0 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 8.0")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_8_0

#  elif _CPPLIB_VER <= 505
  /* Version 9.0 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 9.0")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_9_0

#  elif _CPPLIB_VER <= 520
  /* Version 10.0 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 10.0")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_10_0

#  elif _CPPLIB_VER <= 540
  /* Version 11.0 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 11.0")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_11_0

#  else
#   error Dinkumware C++ Library version unrecognised: are you using a version of VC++ later than 10.0?

#  endif /* _CPPLIB_VER */

# else /* ? _CPPLIB_VER */
  /* Versions 4.2, 5.0 and 6.0 ? */

#  if \
     !defined(_STCONS) && \
     !defined(_TEMPLATE_MEMBER) && \
     !defined(_TEMPLATE) && \
     !defined(_MESG) && \
     !defined(_HAS_EXCEPTIONS) && \
     !defined(_EMPTY_ARGUMENT) && \
     !defined(_THROWS) && \
     !defined(_IS_YES)
  /* Version 4.2 */
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 4.2")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_4_2

#  elif\
     defined(_STCONS) && \
     !defined(_TEMPLATE_MEMBER) && \
     !defined(_TEMPLATE) && \
     !defined(_MESG) && \
     !defined(_HAS_EXCEPTIONS) && \
     !defined(_EMPTY_ARGUMENT) && \
     !defined(_THROWS) && \
     !defined(_IS_YES)
  /* Versions 5 or 6
   *
   * Need to skip out of the STLSoft namespace, then #include <xtree>, skip back in and then
   * test for
   */

#   if _MSC_VER < 1100 || \
       _MSC_VER > 1200
#    error Dinkumware library version discrimination failed
#   endif /* 1100 <= _MSC_VER < = 1200 */

#   ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#   endif /* _STLSOFT_NO_NAMESPACE */
#   include <xtree>
#   if defined(_XTREE_)
  /* Version 6 */
#    ifdef STLSOFT_COMPILE_VERBOSE
#     pragma message("  Dinkumware version 6")
#    endif /* STLSOFT_COMPILE_VERBOSE */
#    define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION     STLSOFT_CF_DINKUMWARE_VC_VERSION_6_0
#   elif defined(_TREE_)
  /* Version 5 */
#    ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version 5")
#    endif /* STLSOFT_COMPILE_VERBOSE */
#    define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION     STLSOFT_CF_DINKUMWARE_VC_VERSION_5_0
#   else
#    error Does not appear to be either the VC5 or VC6 Dinkumware library
#   endif /* _XTREE_ */
#   ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#   endif /* _STLSOFT_NO_NAMESPACE */
#  else
#   ifdef STLSOFT_COMPILE_VERBOSE
#    pragma message("  Dinkumware version unknown")
#   endif /* STLSOFT_COMPILE_VERBOSE */
#   define STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION      STLSOFT_CF_DINKUMWARE_VC_VERSION_UNKNOWN
#  endif /* VC++ version */
# endif /* _CPPLIB_VER */
#endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC */

/* /////////////////////////////////////////////////////////////////////////
 * Tested compatibilities
 */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_BORLAND) || \
    defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC)
# ifndef STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE
#  error STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE should be defined if one of the STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_xxxx symbols is defined
# endif /* !STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE */
#endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_xxxx */

/* /////////////////////////////////////////////////////////////////////////
 * Random access iterator support
 */

// This is all some hideous kludge caused by Dinkumware's standard library's
// failure to leave behind any definitive discriminatable vestige of its
// presence.

#ifdef _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES
# undef _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES
#endif /* !_STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES */

#ifdef _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES_1300
# undef _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES_1300
#endif /* !_STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES_1300 */

/* Detect whether Dinkumware "may" be present
 *
 * Discriminated symbol is _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES
 */
#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    (   STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_6_0 || \
        STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0)
# define _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES
#endif /* _MSC_VER && _MSC_VER == 1300 */

#if defined(_STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES) && \
    defined(_DEPRECATED) && \
    defined(_HAS_TEMPLATE_PARTIAL_ORDERING) && \
    defined(_CPPLIB_VER)
# define _STLSOFT_CF_MIGHT_BE_DINKUMWARE_MS_NAUGHTIES_1300
#endif /*  */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR */

/* ///////////////////////////// end of file //////////////////////////// */
