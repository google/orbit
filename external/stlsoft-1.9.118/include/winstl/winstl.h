/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/winstl.h
 *
 * Purpose:     Root header for the WinSTL libraries. Performs various compiler
 *              and platform discriminations, and definitions of types.
 *
 * Created:     15th January 2002
 * Updated:     1st April 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2014, Matthew Wilson and Synesis Software
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


#ifndef WINSTL_INCL_WINSTL_H_WINSTL
#define WINSTL_INCL_WINSTL_H_WINSTL
#define WINSTL_INCL_H_WINSTL    /*!< \brief Definition of previous include-guard symbol for winstl/winstl.h, for backwards compatibility. */

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_H_WINSTL_MAJOR       3
# define WINSTL_VER_WINSTL_H_WINSTL_MINOR       14
# define WINSTL_VER_WINSTL_H_WINSTL_REVISION    3
# define WINSTL_VER_WINSTL_H_WINSTL_EDIT        193
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file winstl/winstl.h
 *
 * \brief [C, C++] The root header for the \ref group__project__winstl "WinSTL" project.
 */

/* /////////////////////////////////////////////////////////////////////////
 * WinSTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * The major version is denoted by the _WINSTL_VER_MAJOR preprocessor symbol.
 * A changes to the major version component implies that a dramatic change has
 * occurred in the libraries, such that considerable changes to source dependent
 * on previous versions would need to be effected.
 *
 * The minor version is denoted by the _WINSTL_VER_MINOR preprocessor symbol.
 * Changes to the minor version component imply that a significant change has
 * occurred to the libraries, either in the addition of new functionality or in
 * the destructive change to one or more components such that recomplilation and
 * code change may be necessitated.
 *
 * The revision version is denoted by the _WINSTL_VER_REVISION preprocessor
 * symbol. Changes to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the changes.
 *
 * In addition to the individual version symbols - _WINSTL_VER_MAJOR,
 * _WINSTL_VER_MINOR and _WINSTL_VER_REVISION - a composite symbol _WINSTL_VER
 * is defined, where the upper 8 bits are 0, bits 16-23 represent the major
 * component,  bits 8-15 represent the minor component, and bits 0-7 represent
 * the revision component.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies _WINSTL_VER_1_0_1.
 *
 * Thus the symbol _WINSTL_VER may be compared meaningfully with a specific
 * version symbol, e.g. #if _WINSTL_VER >= _WINSTL_VER_1_0_1
 */

/** \def _WINSTL_VER_MAJOR
 * \brief The major version number of the \ref group__project__winstl project "WinSTL" project
 */

/** \def _WINSTL_VER_MINOR
 * \brief The minor version number of the \ref group__project__winstl project "WinSTL" project
 */

/** \def _WINSTL_VER_REVISION
 * \brief The revision version number of the \ref group__project__winstl project "WinSTL" project
 */

/** \def _WINSTL_VER
 * \brief The current composite version number of the \ref group__project__winstl project "WinSTL" project
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _WINSTL_VER_1_0_1      0x00010001  /*!< Version 1.0.1 */
# define _WINSTL_VER_1_0_2      0x00010002  /*!< Version 1.0.2 */
# define _WINSTL_VER_1_1_1      0x00010101  /*!< Version 1.1.1 */
# define _WINSTL_VER_1_2_1      0x00010201  /*!< Version 1.2.1 */
# define _WINSTL_VER_1_3_1      0x00010301  /*!< Version 1.3.1 */
# define _WINSTL_VER_1_3_2      0x00010302  /*!< Version 1.3.2 */
# define _WINSTL_VER_1_3_3      0x00010303  /*!< Version 1.3.3 */
# define _WINSTL_VER_1_3_4      0x00010304  /*!< Version 1.3.4 */
# define _WINSTL_VER_1_3_5      0x00010305  /*!< Version 1.3.5 */
# define _WINSTL_VER_1_3_6      0x00010306  /*!< Version 1.3.6 */
# define _WINSTL_VER_1_3_7      0x00010307  /*!< Version 1.3.7 */
# define _WINSTL_VER_1_4_1      0x00010401  /*!< Version 1.4.1 */
# define _WINSTL_VER_1_5_1      0x00010501  /*!< Version 1.5.1 */
# define _WINSTL_VER_1_5_2      0x00010502  /*!< Version 1.5.2 */
# define _WINSTL_VER_1_6_1      0x00010601  /*!< Version 1.6.1 */
# define _WINSTL_VER_1_6_2      0x00010602  /*!< Version 1.6.2 */
# define _WINSTL_VER_1_6_3      0x00010603  /*!< Version 1.6.3 */
# define _WINSTL_VER_1_6_4      0x00010604  /*!< Version 1.6.4 */
# define _WINSTL_VER_1_6_5      0x00010605  /*!< Version 1.6.5 */
# define _WINSTL_VER_1_7_1      0x00010701  /*!< Version 1.7.1 */
# define _WINSTL_VER_1_8_1      0x00010801  /*!< Version 1.8.1 */
# define _WINSTL_VER_1_8_2      0x00010802  /*!< Version 1.8.2 */
# define _WINSTL_VER_1_9_1      0x00010901  /*!< Version 1.9.1 (with STLSoft 1.9.1) */
# define _WINSTL_VER_1_9_2      0x00010902  /*!< Version 1.9.2 (with STLSoft 1.9.7) */
# define _WINSTL_VER_1_9_3      0x00010903  /*!< Version 1.9.3 (with STLSoft 1.9.25) */
# define _WINSTL_VER_1_9_4      0x010904ff  /*!< Version 1.9.4 (with STLSoft 1.9.30) */
# define _WINSTL_VER_1_9_5      0x010905ff  /*!< Version 1.9.5 (with STLSoft 1.9.31) */
# define _WINSTL_VER_1_9_6      0x010906ff  /*!< Version 1.9.6 (with STLSoft 1.9.32) */
# define _WINSTL_VER_1_10_1     0x010a01ff  /*!< Version 1.10.1 (with STLSoft 1.9.37) */
# define _WINSTL_VER_1_10_2     0x010a02ff  /*!< Version 1.10.2 (with STLSoft 1.9.42) */
# define _WINSTL_VER_1_10_3     0x010a03ff  /*!< Version 1.10.3 (with STLSoft 1.9.82) */
# define _WINSTL_VER_1_10_4     0x010a04ff  /*!< Version 1.10.4 (with STLSoft 1.9.84) */
# define _WINSTL_VER_1_10_5     0x010a05ff  /*!< Version 1.10.5 (with STLSoft 1.9.88) */
# define _WINSTL_VER_1_10_6     0x010a06ff  /*!< Version 1.10.6 (with STLSoft 1.9.90) */
# define _WINSTL_VER_1_10_7     0x010a07ff  /*!< Version 1.10.7 (with STLSoft 1.9.91) */
# define _WINSTL_VER_1_11_1     0x010b01ff  /*!< Version 1.11.1 (with STLSoft 1.9.93) */
# define _WINSTL_VER_1_11_2     0x010b02ff  /*!< Version 1.11.2 (with STLSoft 1.9.100) */
# define _WINSTL_VER_1_11_3     0x010b03ff  /*!< Version 1.11.3 (with STLSoft 1.9.101) */
# define _WINSTL_VER_1_11_4     0x010b04ff  /*!< Version 1.11.4 (with STLSoft 1.9.105) */
# define _WINSTL_VER_1_11_5     0x010b05ff  /*!< Version 1.11.5 (with STLSoft 1.9.107) */
# define _WINSTL_VER_1_11_6     0x010b06ff  /*!< Version 1.11.6 (with STLSoft 1.9.108) */
# define _WINSTL_VER_1_11_7     0x010b07ff  /*!< Version 1.11.7 (with STLSoft 1.9.109) */
# define _WINSTL_VER_1_11_8     0x010b08ff  /*!< Version 1.11.8 (with STLSoft 1.9.113) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _WINSTL_VER_MAJOR       1
#define _WINSTL_VER_MINOR       11
#define _WINSTL_VER_REVISION    8
#define _WINSTL_VER             _WINSTL_VER_1_11_8

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

/* Strict */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifndef STRICT
#  if defined(_WINSTL_STRICT) || \
      (   !defined(_WINSTL_NO_STRICT) && \
          !defined(NO_STRICT))
#   define STRICT 1
#  endif /* !NO_STRICT && !_WINSTL_NO_STRICT */
# endif /* STRICT */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER == 1100
# include <wtypes.h>    /* This is here to fix a thoroughly inexplicable VC 5 bug */
#endif /* compiler */

#if defined(STLSOFT_COMPILER_IS_GCC) && \
    defined(_WIN32) && \
    (   defined(WIN32) || \
        defined(WIN64))

# ifndef STLSOFT_INCL_H_BASETYPS
#  define STLSOFT_INCL_H_BASETYPS
#  include <basetyps.h>
# endif /* !STLSOFT_INCL_H_BASETYPS */
# ifndef STLSOFT_INCL_H_WTYPES
#  define STLSOFT_INCL_H_WTYPES
#  include <wtypes.h>
# endif /* !STLSOFT_INCL_H_WTYPES */
# ifndef STLSOFT_INCL_H_OLEAUTO
#  define STLSOFT_INCL_H_OLEAUTO
#  include <oleauto.h>
# endif /* !STLSOFT_INCL_H_OLEAUTO */
# ifndef STLSOFT_INCL_H_OAIDL
#  define STLSOFT_INCL_H_OAIDL
#  include <oaidl.h>
# endif /* !STLSOFT_INCL_H_OAIDL */
#endif /* compiler */

#ifndef STLSOFT_INCL_H_WINDOWS
# define STLSOFT_INCL_H_WINDOWS
# include <windows.h>   /* Windows base header */
#endif /* !STLSOFT_INCL_H_WINDOWS */

/* Intel is super pernickety about conversions, so we need to bring out the union_cast. */
#if defined(STLSOFT_COMPILER_IS_INTEL) && \
    defined(__cplusplus)
# ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_UNION_CAST
#  include <stlsoft/conversion/union_cast.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_UNION_CAST */
#endif /* compiler */

#ifndef STLSOFT_INCL_H_STDLIB
# define STLSOFT_INCL_H_STDLIB
# include <stdlib.h> /* for MAX_PATH (CygWin) */
#endif /* !STLSOFT_INCL_H_STDLIB */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if !defined(_STLSOFT_VER) || \
    _STLSOFT_VER < 0x010971ff
# error This version of the WinSTL libraries requires STLSoft version 1.9.113, or later
#endif /* _STLSOFT_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Operating System
 */

#ifdef WINSTL_OS_IS_WIN32
# undef WINSTL_OS_IS_WIN32
#endif /* WINSTL_OS_IS_WIN32 */

#ifdef WINSTL_OS_IS_WIN64
# undef WINSTL_OS_IS_WIN64
#endif /* WINSTL_OS_IS_WIN64 */

#if defined(WIN64) || \
    defined(_WIN64)
# if !defined(WIN64)
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Win64 platform targeted, as indicated by definition of _WIN64, but WIN64 is not defined: adjust your project/make settings to define WIN64")
#  endif /* STLSOFT_COMPILE_VERBOSE */
# endif /* !WIN64 */
# define WINSTL_OS_IS_WIN64
# if !defined(_WIN64)
#  error Compiling with WIN64 defined and _WIN64 not defined. Are you using the wrong makefile?
# endif /* !_WIN64 */
#elif defined(WIN32)
# define WINSTL_OS_IS_WIN32
#else /* ? WIN?? */
# error WinSTL is only compatible with Win32 and Win64
#endif /* WIN?? */

/* /////////////////////////////////////////////////////////////////////////
 * Architecture
 */

#ifdef WINSTL_ARCH_IS_X86
# undef WINSTL_ARCH_IS_X86
#endif /* WINSTL_ARCH_IS_X86 */

#ifdef WINSTL_ARCH_IS_IA64
# undef WINSTL_ARCH_IS_IA64
#endif /* WINSTL_ARCH_IS_IA64 */

#ifdef WINSTL_ARCH_IS_X64
# undef WINSTL_ARCH_IS_X64
#endif /* WINSTL_ARCH_IS_X64 */

#if defined(_M_IA64)
# define WINSTL_ARCH_IS_IA64
#elif defined(_M_X64) || \
      defined(_M_AMD64)
# define WINSTL_ARCH_IS_X64
#elif defined(_M_IX86)
# define WINSTL_ARCH_IS_X86
#else /* ? _M_?? */
# error WinSTL is only compatible with x86, IA64 and x64 architectures
#endif /* _M_?? */

/* /////////////////////////////////////////////////////////////////////////
 * Proper C++ casting
 */

#ifdef __cplusplus
# undef     INVALID_HANDLE_VALUE
/** \def INVALID_HANDLE_VALUE
 * \brief A C++-only redefinition of this \#define which uses reinterpret_cast to
 *  avoid C-style cast warnings.
 */
# if defined(STLSOFT_COMPILER_IS_INTEL)
#  define   INVALID_HANDLE_VALUE        stlsoft_ns_qual(union_cast)<HANDLE>(winstl_ns_qual(ws_sptrint_t)(-1))
# else /* ? compiler */
#  define   INVALID_HANDLE_VALUE        reinterpret_cast<HANDLE>(-1)
# endif /* compiler */

/** \def MAKEINTRESOURCEA
 * \brief A C++-only redefinition of this \#define which uses C++ cast operators to
 *  avoid C-style cast warnings.
 */
# undef     MAKEINTRESOURCEA
# if defined(_WIN64) || \
     defined(_Wp64)
#  define   MAKEINTRESOURCEA(i)         reinterpret_cast<LPSTR>(static_cast<ULONG_PTR>(static_cast<WORD>(i)))
# else /* ? width */
#  define   MAKEINTRESOURCEA(i)         reinterpret_cast<LPSTR>(static_cast<ULONG>(static_cast<WORD>(i)))
# endif /* width */

/** \def MAKEINTRESOURCEW
 * \brief A C++-only redefinition of this \#define which uses C++ cast operators to
 *  avoid C-style cast warnings.
 */
# undef     MAKEINTRESOURCEW
# if defined(_WIN64) || \
     defined(_Wp64)
#  define   MAKEINTRESOURCEW(i)         reinterpret_cast<LPWSTR>(static_cast<ULONG_PTR>(static_cast<WORD>(i)))
# else /* ? width */
#  define   MAKEINTRESOURCEW(i)         reinterpret_cast<LPWSTR>(static_cast<ULONG>(static_cast<WORD>(i)))
# endif /* width */

/** \def MAKEINTRESOURCE
 * \brief A C++-only redefinition of this \#define which uses C++ cast operators to
 *  avoid C-style cast warnings.
 */
# undef     MAKEINTRESOURCE
# if defined(_WIN64) || \
     defined(_Wp64)
#  define   MAKEINTRESOURCE(i)          reinterpret_cast<LPTSTR>(static_cast<ULONG_PTR>(static_cast<WORD>(i)))
# else /* ? width */
#  define   MAKEINTRESOURCE(i)          reinterpret_cast<LPTSTR>(static_cast<ULONG>(static_cast<WORD>(i)))
# endif /* width */

/** \def MAKELANGID
 * \brief A C++-only redefinition of this \#define which uses C++ cast operators to
 *  avoid C-style cast warnings.
 */
# undef     MAKELANGID
# define    MAKELANGID(p, s)            ((static_cast<DWORD>(static_cast<WORD>(s)) << 10) | static_cast<WORD>(p))


/* # undef     LOWORD */
/* # define    LOWORD(l)                   static_cast<WORD>(static_cast<DWORD>(l) & 0xffff) */

/** \def INVALID_FILE_SIZE
 * \brief A C++-only redefinition of this \#define which uses C++ cast operators to
 *  avoid C-style cast warnings.
 */
# undef     INVALID_FILE_SIZE
# define    INVALID_FILE_SIZE           static_cast<DWORD>(0xFFFFFFFF)


/** \def INVALID_SET_FILE_POINTER
 * \brief A C++-only redefinition of this \#define which uses C++ cast operators to
 *  avoid C-style cast warnings.
 */
# undef     INVALID_SET_FILE_POINTER
# define    INVALID_SET_FILE_POINTER    static_cast<DWORD>(0xFFFFFFFF)

#else /* ? __cplusplus */ 

# ifndef INVALID_SET_FILE_POINTER
#  define INVALID_SET_FILE_POINTER      stlsoft_c_cast(DWORD, -1)
# endif /* !INVALID_SET_FILE_POINTER */

# ifndef INVALID_FILE_ATTRIBUTES
#  define INVALID_FILE_ATTRIBUTES       stlsoft_c_cast(DWORD, -1)
# endif /* !INVALID_FILE_ATTRIBUTES */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Sanity checks
 *
 * Win32    -   must be compiled in context of Win32 API
 */

/* Must be Win32 api. */
#if !defined(WIN32) && \
    !defined(_WIN32)
# error The WinSTL libraries is currently only compatible with the Win32 API
#endif /* !WIN32 && !_WIN32 */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the WinSTL libraries are
 *
 * Borland C++ 5.5, 5.51, 5.6
 * Digital Mars C/C++ 8.26 - 8.32
 * Metrowerks 2.4 & 3.0 (CodeWarrior 7.0 & 8.0)
 * Intel C/C++ 6.0 & 7.0
 * Visual C++ 4.2, 5.0, 6.0, 7.0
 * Watcom C/C++ 11.0
 */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
/* Borland C++ */
# if __BORLANDC__ < 0x0550
#  error Versions of Borland C++ prior to 5.5 are not supported by the WinSTL libraries
# endif /* __BORLANDC__ */

#elif defined(STLSOFT_COMPILER_IS_COMO)
/* Comeau C++ */
# if __COMO_VERSION__ < 4300
#  error Versions of Comeau C++ prior to 4.3 are not supported by the WinSTL libraries
# endif /* __COMO_VERSION__ */

#elif defined(STLSOFT_COMPILER_IS_DMC)
/* Digital Mars C/C++ */
# if __DMC__ < 0x0826
#  error Versions of Digital Mars C/C++ prior to 8.26 are not supported by the WinSTL libraries
# endif /* __DMC__ */

#elif defined(STLSOFT_COMPILER_IS_GCC)
/* GNU C/C++ */
# if __GNUC__ < 3
#  error Versions of GNU C/C++ prior to 3.0 are not supported by the WinSTL libraries
# endif /* __GNUC__ */

#elif defined(STLSOFT_COMPILER_IS_INTEL)
/* Intel C++ */
# if (__INTEL_COMPILER < 600)
#  error Versions of Intel C++ prior to 6.0 are not supported by the WinSTL libraries
# endif /* __INTEL_COMPILER */

#elif defined(STLSOFT_COMPILER_IS_MWERKS)
/* Metrowerks C++ */
# if (__MWERKS__ & 0xFF00) < 0x2400
#  error Versions of Metrowerks CodeWarrior C++ prior to 7.0 are not supported by the WinSTL libraries
# endif /* __MWERKS__ */

#elif defined(STLSOFT_COMPILER_IS_MSVC)
/* Visual C++ */
# if _MSC_VER < 1020
#  error Versions of Visual C++ prior to 4.2 are not supported by the WinSTL libraries
# endif /* _MSC_VER */

#elif defined(STLSOFT_COMPILER_IS_VECTORC)
/* VectorC C/C++ */

#elif defined(STLSOFT_COMPILER_IS_WATCOM)
/* Watcom C/C++ */
# if (__WATCOMC__ < 1200)
#  error Versions of Watcom C/C++ prior to 12.0 are not supported by the WinSTL libraries
# endif /* __WATCOMC__ */

#else
/* No recognised compiler */
# ifdef _STLSOFT_FORCE_ANY_COMPILER
#  define _WINSTL_COMPILER_IS_UNKNOWN
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Compiler is unknown to WinSTL")
#  endif /* STLSOFT_COMPILE_VERBOSE */
# else
#  error Currently only Borland C++, Comeau, Digital Mars C/C++, GNU C++, Intel C/C++, Metrowerks CodeWarrior, Visual C++ and Watcom compilers are supported by the WinSTL libraries
# endif /* _STLSOFT_FORCE_ANY_COMPILER */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Features
 */

#ifdef WINSTL_UDATE_DEFINED
# undef WINSTL_UDATE_DEFINED
#endif /* WINSTL_UDATE_DEFINED */

#if defined(WINSTL_FORCE_UDATE) || \
    defined(STLSOFT_COMPILER_IS_BORLAND) || \
    (   defined(STLSOFT_COMPILER_IS_COMO) && \
        defined(_MSC_VER)) || \
    defined(STLSOFT_COMPILER_IS_DMC) || \
    defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_MWERKS) || \
    defined(STLSOFT_COMPILER_IS_MSVC)
# define WINSTL_UDATE_DEFINED
#endif /* WINSTL_FORCE_UDATE || compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro winstl_assert provides standard debug-mode assert functionality.
 */

/** \brief Defines an assertion construct for runtime verification.
 *
 * \param expr Must be non-zero, or an assertion will be fired
 *
 * \remarks By default this is defined to \ref STLSOFT_ASSERT. However, this
 *  can be overriden if a prior definition is encountered, allowing the
 *  runtime assertion of WinSTL components to use a different mechanism to
 *  those in the other \ref group__projects "projects".
 */
#ifndef WINSTL_ASSERT
# define WINSTL_ASSERT(expr)                STLSOFT_ASSERT(expr)
#endif /* !WINSTL_ASSERT */

/** \brief Defines a runtime assertion, with message
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define WINSTL_MESSAGE_ASSERT(msg, expr)    STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \def WINSTL_STATIC_ASSERT(expr)
 *
 * \brief Defines an assertion construct for compile-time verification.
 *
 * \param expr A compile-time evaluatable condition that must be non-zero, or compilation will fail.
 *
 * \remarks This is defined to \ref STLSOFT_STATIC_ASSERT.
 */
#define WINSTL_STATIC_ASSERT(expr)          STLSOFT_STATIC_ASSERT(expr)


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define winstl_assert(expr)                WINSTL_ASSERT(expr)
# define winstl_message_assert(msg, expr)   WINSTL_MESSAGE_ASSERT(msg, expr)
# define winstl_static_assert(expr)         WINSTL_STATIC_ASSERT(expr)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The WinSTL components are contained within the winstl namespace. This is
 * usually an alias for stlsoft::winstl_project,
 *
 * When compilers support namespaces they are defined by default. They can be
 * undefined using a cascasing system, as follows:
 *
 * If _STLSOFT_NO_NAMESPACES is defined, then _WINSTL_NO_NAMESPACES is defined.
 *
 * If _WINSTL_NO_NAMESPACES is defined, then _WINSTL_NO_NAMESPACE is defined.
 *
 * If _WINSTL_NO_NAMESPACE is defined, then the WinSTL constructs are defined
 * in the global scope.
 *
 * If _STLSOFT_NO_NAMESPACES, _WINSTL_NO_NAMESPACES and _WINSTL_NO_NAMESPACE are
 * all undefined but the symbol _STLSOFT_NO_NAMESPACE is defined (whence the
 * namespace stlsoft does not exist), then the WinSTL constructs are defined
 * within the winstl namespace. The definition matrix is as follows:
 *
 * _STLSOFT_NO_NAMESPACE    _WINSTL_NO_NAMESPACE    winstl definition
 * ---------------------    --------------------    -----------------
 *  not defined              not defined             = stlsoft::winstl_project
 *  not defined              defined                 not defined
 *  defined                  not defined             winstl
 *  defined                  defined                 not defined
 *
 *
 *
 * The macro winstl_ns_qual() macro can be used to refer to elements in the
 * WinSTL libraries irrespective of whether they are in the
 * stlsoft::winstl_project (or winstl) namespace or in the global namespace.
 *
 * Furthermore, some compilers do not support the standard library in the std
 * namespace, so the winstl_ns_qual_std() macro can be used to refer to elements
 * in the WinSTL libraries irrespective of whether they are in the std namespace
 * or in the global namespace.
 */

/* No STLSoft namespaces means no WinSTL namespaces */
#ifdef _STLSOFT_NO_NAMESPACES
# define _WINSTL_NO_NAMESPACES
#endif /* _STLSOFT_NO_NAMESPACES */

/* No WinSTL namespaces means no winstl namespace */
#ifdef _WINSTL_NO_NAMESPACES
# define _WINSTL_NO_NAMESPACE
#endif /* _WINSTL_NO_NAMESPACES */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
/** \brief The <code class="namespace">winstl</code> namespace contains all components
 *   in the \ref group__project__winstl "WinSTL" project.
 *
 * By default, the <code>winstl</code> namespace is actually an alias for
 * the namespace <code>stlsoft::winstl_project</code>, which is where all
 * the \ref group__project__winstl "WinSTL" components actually reside. This
 * measure allows all components within the main the
 * \ref group__project__stlsoft "STLSoft" project (which are defined within
 * the <code>stlsoft</code> namespace) to be visible to all components
 * "within" the <code>winstl</code> namespace. (Otherwise, there would be a
 * whole lot of onerous qualification throughout the code of all
 * \ref group__projects "sub-projects".)
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_WINSTL_NO_NAMESPACE</code> are defined, all
 * \ref group__project__winstl "WinSTL" components will be defined in the
 * global namespace. Conversely, if the <code>_STLSOFT_NO_NAMESPACE</code>
 * symbol (not to be confused with the
 * <code>_STLSOFT_NO_NAMESPACES</code> symbol!) is defined - meaning that
 * all \ref group__project__stlsoft "main project" components are to be
 * defined in the global namespace, and <code>_WINSTL_NO_NAMESPACE</code>
 * is <b>not</b> defined, then all \ref group__project__winstl "WinSTL"
 * components will be defined within a bona fide <code>winstl</code>
 * namespace.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#else
stlsoft_ns_using(move_lhs_from_rhs)
#endif /* !_WINSTL_NO_NAMESPACE */

/** \def winstl_ns_qual(x)
 * \brief Namespace qualification macro for
 *  \ref group__project__winstl "WinSTL" components that generates correct
 *  code regardless of whether the <code>winstl</code> namespace is being
 *  suppressed or enabled.
 *
 * If the <span class="code">winstl</span> namespace is being suppressed - by the
 * definition of <code>_WINSTL_NO_NAMESPACE</code> or
 * <code>_WINSTL_NO_NAMESPACES</code> or
 * <code>_STLSOFT_NO_NAMESPACES</code>; detectable by the presence of the
 * symbol <code>_WINSTL_NO_NAMESPACE</code> - then
 * <code>winstl_ns_using(x)</code> is equivalent to writing <code>x</code>.
 * If the <code>winstl</code> namespace is not being suppressed then
 * <code>winstl_ns_using(x)</code> is equivalent to writing <code>winstl::x</code>.
 *
 * For example, the following code will compile correctly if
 * <code>winstl</code> is suppressed or not:
\code
#include <winstl/error_desc.hpp>
#include <stdio.h>

int main()
{
  winstl_ns_qual(reg_key)   key(HKEY_CURRENT_USER, "SOFTWARE\\AcmeLib\\AcmeApp");

  return 0;
}
\endcode
 *
 * \remarks These macros are used throughout the <i>implementation</i> of the
 *  STLSoft libraries. However, we do not recommend their use in application
 *  code for the simple reason that they are ugly and do not represent a
 *  widely recognised idiom. Of course, if they suit your purposes, then by
 *  all means ...
 */

/** \def winstl_ns_using(x)
 * \brief Declares a using directive (with respect to <b>winstl</b>) if WinSTL is using namespaces or, if not, does nothing
 */

#ifndef _WINSTL_NO_NAMESPACE
# define winstl_ns_qual(x)          ::winstl::x
# define winstl_ns_using(x)         using ::winstl::x;
#else
# define winstl_ns_qual(x)          x
# define winstl_ns_using(x)
#endif /* !_WINSTL_NO_NAMESPACE */

/** \def winstl_ns_qual_std(x)
 * \brief Qualifies with <b>std::</b> if WinSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify.
 *
 * \see \ref page__namespace_suppression
 */

/** \def winstl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if WinSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing.
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define winstl_ns_qual_std(x)      ::std::x
# define winstl_ns_using_std(x)     using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define winstl_ns_qual_std(x)      x
# define winstl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Language agnostic macros.
 */

/** \def WINSTL_ITF_CALL(p)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>p->lpVtbl</b> for C compilation, and to <b>p</b> in C++
 *
 * \see WINSTL_ITF_THIS, WINSTL_ITF_THIS0, WINSTL_IID_2_REF, WINSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define WINSTL_ITF_CALL(p)         (p)
#else /* ? __cplusplus */
# define WINSTL_ITF_CALL(p)         (p)->lpVtbl
#endif /* __cplusplus */

/** \def WINSTL_ITF_THIS(p)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>p,</b> for C compilation, and to nothing in C++
 *
 * \see WINSTL_ITF_CALL, WINSTL_ITF_THIS0, WINSTL_IID_2_REF, WINSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define WINSTL_ITF_THIS(p)
#else /* ? __cplusplus */
# define WINSTL_ITF_THIS(p)         (p),
#endif /* __cplusplus */

/** \def WINSTL_ITF_THIS0(p)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>p</b> for C compilation, and to nothing in C++
 *
 * \see WINSTL_ITF_CALL, WINSTL_ITF_THIS, WINSTL_IID_2_REF, WINSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define WINSTL_ITF_THIS0(p)
#else /* ? __cplusplus */
# define WINSTL_ITF_THIS0(p)        (p)
#endif /* __cplusplus */

/** \def WINSTL_IID_2_REF(iid)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>&iid</b> for C compilation, and to <b>iid</b> in C++
 *
 * \see WINSTL_ITF_CALL, WINSTL_ITF_THIS, WINSTL_ITF_THIS0, WINSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define WINSTL_IID_2_REF(iid)        (iid)
#else /* ? __cplusplus */
# define WINSTL_IID_2_REF(iid)        (&(iid))
#endif /* __cplusplus */

/** \def WINSTL_REF_2_PTR(iid)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>iid</b> for C compilation, and to <b>&iid</b> in C++
 *
 * \see WINSTL_ITF_CALL, WINSTL_ITF_THIS, WINSTL_ITF_THIS0, WINSTL_REF_2_REF
 */

#if defined(__cplusplus)
# define WINSTL_REF_2_PTR(iid)        (&(iid))
#else /* ? __cplusplus */
# define WINSTL_REF_2_PTR(iid)        (iid)
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The WinSTL uses a number of typedefs to aid in compiler-independence in the
 * libraries' main code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

typedef stlsoft_ns_qual(ss_char_a_t)        ws_char_a_t;    /*!< Ansi char type */
typedef stlsoft_ns_qual(ss_char_w_t)        ws_char_w_t;    /*!< Unicode char type */
typedef stlsoft_ns_qual(ss_sint8_t)         ws_sint8_t;     /*!< 8-bit signed integer */
typedef stlsoft_ns_qual(ss_uint8_t)         ws_uint8_t;     /*!< 8-bit unsigned integer */
typedef stlsoft_ns_qual(ss_int16_t)         ws_int16_t;     /*!< 16-bit integer */
typedef stlsoft_ns_qual(ss_sint16_t)        ws_sint16_t;    /*!< 16-bit signed integer */
typedef stlsoft_ns_qual(ss_uint16_t)        ws_uint16_t;    /*!< 16-bit unsigned integer */
typedef stlsoft_ns_qual(ss_int32_t)         ws_int32_t;     /*!< 32-bit integer */
typedef stlsoft_ns_qual(ss_sint32_t)        ws_sint32_t;    /*!< 32-bit signed integer */
typedef stlsoft_ns_qual(ss_uint32_t)        ws_uint32_t;    /*!< 32-bit unsigned integer */
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef stlsoft_ns_qual(ss_int64_t)         ws_int64_t;     /*!< 64-bit integer */
typedef stlsoft_ns_qual(ss_sint64_t)        ws_sint64_t;    /*!< 64-bit signed integer */
typedef stlsoft_ns_qual(ss_uint64_t)        ws_uint64_t;    /*!< 64-bit unsigned integer */
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
# ifdef _WIN64
typedef LONG_PTR                            ws_intptr_t;    /*!< integer capable of holding pointer without loss */
typedef LONG_PTR                            ws_sintptr_t;   /*!< signed integer capable of holding pointer without loss */
typedef ULONG_PTR                           ws_uintptr_t;   /*!< unsigned integer capable of holding pointer without loss */
# else /* ? _WIN64 */
typedef LONG                                ws_intptr_t;    /*!< integer capable of holding pointer without loss */
typedef LONG                                ws_sintptr_t;   /*!< signed integer capable of holding pointer without loss */
typedef ULONG                               ws_uintptr_t;   /*!< unsigned integer capable of holding pointer without loss */
# endif /* _WIN64 */
typedef stlsoft_ns_qual(ss_int_t)           ws_int_t;       /*!< integer */
typedef stlsoft_ns_qual(ss_sint_t)          ws_sint_t;      /*!< signed integer */
typedef stlsoft_ns_qual(ss_uint_t)          ws_uint_t;      /*!< unsigned integer */
typedef stlsoft_ns_qual(ss_long_t)          ws_long_t;      /*!< long */
typedef stlsoft_ns_qual(ss_byte_t)          ws_byte_t;      /*!< Byte */
#if defined(__cplusplus)
typedef stlsoft_ns_qual(ss_bool_t)          ws_bool_t;      /*!< bool */
#endif /* __cplusplus */
typedef DWORD                               ws_dword_t;     /*!< dword */
typedef stlsoft_ns_qual(ss_size_t)          ws_size_t;      /*!< size */
typedef stlsoft_ns_qual(ss_ptrdiff_t)       ws_ptrdiff_t;   /*!< ptr diff */
typedef stlsoft_ns_qual(ss_streampos_t)     ws_streampos_t; /*!< streampos */
typedef stlsoft_ns_qual(ss_streamoff_t)     ws_streamoff_t; /*!< streamoff */
#ifdef WINSTL_OS_IS_WIN64
typedef stlsoft_ns_qual(ss_sint64_t)        ws_sptrint_t;
typedef stlsoft_ns_qual(ss_uint64_t)        ws_uptrint_t;
#else /* ? WINSTL_OS_IS_WIN64 */
typedef stlsoft_ns_qual(ss_sint32_t)        ws_sptrint_t;
typedef stlsoft_ns_qual(ss_uint32_t)        ws_uptrint_t;
#endif /* WINSTL_OS_IS_WIN64 */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef _WINSTL_NO_NAMESPACE
typedef ws_char_a_t         char_a_t;           /*!< Ansi char type */
typedef ws_char_w_t         char_w_t;           /*!< Unicode char type */
/* typedef ws_int8_t           int8_t; */             /*!< 8-bit integer */
typedef ws_sint8_t          sint8_t;            /*!< 8-bit signed integer */
typedef ws_uint8_t          uint8_t;            /*!< 8-bit unsigned integer */
typedef ws_int16_t          int16_t;            /*!< 16-bit integer */
typedef ws_sint16_t         sint16_t;           /*!< 16-bit signed integer */
typedef ws_uint16_t         uint16_t;           /*!< 16-bit unsigned integer */
typedef ws_int32_t          int32_t;            /*!< 32-bit integer */
typedef ws_sint32_t         sint32_t;           /*!< 32-bit signed integer */
typedef ws_uint32_t         uint32_t;           /*!< 32-bit unsigned integer */
# ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef ws_int64_t          int64_t;            /*!< 64-bit integer */
typedef ws_sint64_t         sint64_t;           /*!< 64-bit signed integer */
typedef ws_uint64_t         uint64_t;           /*!< 64-bit unsigned integer */
# endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
/* typedef ws_short_t          short_t; */            /*!< short integer */
typedef ws_int_t            int_t;              /*!< integer */
typedef ws_sint_t           sint_t;             /*!< signed integer */
typedef ws_uint_t           uint_t;             /*!< unsigned integer */
typedef ws_long_t           long_t;             /*!< long integer */
typedef ws_byte_t           byte_t;             /*!< Byte */
#if defined(__cplusplus)
typedef ws_bool_t           bool_t;             /*!< bool */
#endif /* __cplusplus */
typedef ws_dword_t          dword_t;            /*!< dword */
# if !defined(STLSOFT_COMPILER_IS_DMC)
typedef ws_streampos_t      streampos_t;        /*!< streampos */
typedef ws_streamoff_t      streamoff_t;        /*!< streamoff */
# endif /* compiler */
typedef ws_sptrint_t        sptrint_t;
typedef ws_uptrint_t        uptrint_t;
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Constants
 */

/** \def WINSTL_CONST_NT_MAX_PATH
 *
 * \brief Defines the number of maximum length of a path specification on
 *  Windows NT: 4 + 32767.
 *
 * Windows NT supports so-called "long names" in the Unicode variants (e.g.
 * <code>CreateFileW()</code>) of many of its file-system API functions. The
 * maximum length of a long name is 32767, and a long name must be prefixed
 * with the 4-character sequence <code>\\\\\?\\</code>. This constant can be
 * used when there is a need to allocate the maximum possible size for a
 * file system path.
 */
#define WINSTL_CONST_NT_MAX_PATH            (4 + 32767)

/** \def WINSTL_CONST_MAX_PATH
 *
 */
#if defined(_MAX_PATH)
# define WINSTL_CONST_MAX_PATH              _MAX_PATH
#elif defined(__CYGWIN__)
# define WINSTL_CONST_MAX_PATH              (260)
#else
# error _MAX_PATH not defined, and not CygWin compiler
#endif

#ifdef __cplusplus

/** \brief C++ constant equivalent to \ref WINSTL_CONST_NT_MAX_PATH.
 */
const ws_size_t CONST_NT_MAX_PATH       =   WINSTL_CONST_NT_MAX_PATH;

/** \brief C++ constant equivalent to \ref WINSTL_CONST_MAX_PATH.
 */
const ws_size_t CONST_MAX_PATH          =   WINSTL_CONST_MAX_PATH;

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Values
 *
 * Since the boolean type may not be supported natively on all compilers, the
 * values of true and false may also not be provided. Hence the values of
 * ws_true_v and ws_false_v are defined, and are used in all code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define ws_true_v       ss_true_v
#define ws_false_v      ss_false_v

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/* /////////////////////////////////////////////////////////////////////////
 * Code modification macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/* Exception signatures. */
# define winstl_throw_0()                               stlsoft_throw_0()
# define winstl_throw_1(x1)                             stlsoft_throw_1(x1)
# define winstl_throw_2(x1, x2)                         stlsoft_throw_2(x1, x2)
# define winstl_throw_3(x1, x2, x3)                     stlsoft_throw_3(x1, x2, x3)
# define winstl_throw_4(x1, x2, x3, x4)                 stlsoft_throw_4(x1, x2, x3, x4)
# define winstl_throw_5(x1, x2, x3, x4, x5)             stlsoft_throw_5(x1, x2, x3, x4, x5)
# define winstl_throw_6(x1, x2, x3, x4, x5, x6)         stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
# define winstl_throw_7(x1, x2, x3, x4, x5, x6, x7)     stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
# define winstl_throw_8(x1, x2, x3, x4, x5, x6, x7, x8) stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_NUM_ELEMENTS(ar)                        STLSOFT_NUM_ELEMENTS(ar)
# define winstl_num_elements(ar)                        WINSTL_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [DEPRECATED] Destroys the given instance \c p of the given type (\c t and \c type)
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 */
#define winstl_destroy_instance(t, type, p)             STLSOFT_DESTROY_INSTANCE(t, type, p)

/** \brief [DEPRECATED] Generates an opaque type with the name \c _htype
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 */
#define winstl_gen_opaque(htype)                        STLSOFT_GEN_OPAQUE(htype)

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifdef __cplusplus
/** \brief Safely converts a <code>BOOL</code> to a <code>bool</code>.
 *
 * \param bVal The instance of <code>BOOL</code> to be converted to <code>bool</code>.
 *
 * \retval false If <code>bVal</code> is 0.
 * \retval true If <code>bVal</code> is non-0.
 *
 * \note In C compilation, this is defined as a functionally equivalent macro.
 *
 * \remark Because all compilers (that we know about) represent a
 *  <code>bool</code> as one byte and <code>BOOL</code> (which is a typedef
 *  from <code>int</code>) as more than one byte, safely converting from
 *  <code>BOOL</code> to <code>bool</code> has to involve a runtime test
 *  against 0 that cannot be inlined by the compiler. Consequently, use of
 *  BOOL2bool() should be avoided where possible, and extensive use of it
 *  in your code base likely represents misuse. (<b>See</b> section 13.4.2
 *  of <a href = "http://www.imperfectcplusplus.com/" target="blank">Imperfect C++</a>
 *  for a discussion of these issues.)
 */
#ifdef _Wp64
inline bool BOOL2bool(ws_int_t bVal)
#else /* ? _Wp64 */
inline bool BOOL2bool(BOOL bVal)
#endif /* _Wp64 */
{
    return bVal != FALSE;
}
/** \brief Safely converts a <code>bool</code> to a <code>BOOL</code>.
 *
 * \param bVal The instance of <code>bool</code> to be converted to <code>BOOL</code>.
 *
 * \retval 0 If <code>bVal</code> is <code>false</code>.
 * \retval 1 If <code>bVal</code> is <code>true</code>.
 *
 * \note In C compilation, this is defined as a functionally equivalent macro.
 *
 * \remark Although use of the <code>BOOL</code> type indicates that a
 *  <b>"false"</b> condition is represented by a value of 0, and all other values
 *  represent <b>"true"</b> it is not the case that all client code is implemented
 *  as such. In order to avoid fruitless debates with users of STLSoft who
 *  may be unaware of this convention, or unwilling or unable to accept it,
 *  bool2BOOL() implemented to ensure that the return value is only ever 0 or 1.
 *  Consequently, use of this function should be avoided where possible, and
 *  extensive use of it in your code base likely represents misuse.
 *  (<b>See</b> section 13.4.2 of
 *  <a href = "http://www.imperfectcplusplus.com/" target="blank">Imperfect C++</a>
 *  for a discussion of these issues.)
 */
inline BOOL bool2BOOL(bool bVal)
{
    return bVal != false;
}
#else
# define BOOL2bool(bVal)            stlsoft_static_cast(bool, ((bVal) != FALSE))
# define bool2BOOL(bVal)            stlsoft_static_cast(BOOL, ((bVal) != false))
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace winstl_project */
} /* namespace stlsoft */
namespace winstl = ::stlsoft::winstl_project;
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_H_WINSTL */

/* ///////////////////////////// end of file //////////////////////////// */
