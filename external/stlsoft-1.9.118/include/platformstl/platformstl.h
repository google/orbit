/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/platformstl.h
 *
 * Purpose:     Root header for the PlatformSTL C/C++ libraries. Performs
 *              platform discriminations, and definitions of types.
 *
 * Created:     20th March 2005
 * Updated:     12th September 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2011, Matthew Wilson and Synesis Software
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


#ifndef PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL
#define PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_H_PLATFORMSTL_MAJOR    1
# define PLATFORMSTL_VER_PLATFORMSTL_H_PLATFORMSTL_MINOR    14
# define PLATFORMSTL_VER_PLATFORMSTL_H_PLATFORMSTL_REVISION 1
# define PLATFORMSTL_VER_PLATFORMSTL_H_PLATFORMSTL_EDIT     40
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file platformstl/platformstl.h
 *
 * \brief [C, C++] Root header for the
 *  \ref group__project__platformstl "PlatformSTL" project for C
 *  compilation.
 */

/* /////////////////////////////////////////////////////////////////////////
 * PlatformSTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * The major version is denoted by the _PLATFORMSTL_VER_MAJOR preprocessor symbol.
 * A changes to the major version component implies that a dramatic change has
 * occurred in the libraries, such that considerable changes to source dependent
 * on previous versions would need to be effected.
 *
 * The minor version is denoted by the _PLATFORMSTL_VER_MINOR preprocessor symbol.
 * Changes to the minor version component imply that a significant change has
 * occurred to the libraries, either in the addition of new functionality or in
 * the destructive change to one or more components such that recomplilation and
 * code change may be necessitated.
 *
 * The revision version is denoted by the _PLATFORMSTL_VER_REVISION preprocessor
 * symbol. Changes to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the changes.
 *
 * In addition to the individual version symbols - _PLATFORMSTL_VER_MAJOR,
 * _PLATFORMSTL_VER_MINOR and _PLATFORMSTL_VER_REVISION - a composite symbol _PLATFORMSTL_VER
 * is defined, where the upper 8 bits are 0, bits 16-23 represent the major
 * component,  bits 8-15 represent the minor component, and bits 0-7 represent
 * the revision component.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies _PLATFORMSTL_VER_1_0_1.
 *
 * Thus the symbol _PLATFORMSTL_VER may be compared meaningfully with a specific
 * version symbol, e.g.# if _PLATFORMSTL_VER >= _PLATFORMSTL_VER_1_0_1
 */

/** \def _PLATFORMSTL_VER_MAJOR
 * The major version number of PlatformSTL
 */

/** \def _PLATFORMSTL_VER_MINOR
 * The minor version number of PlatformSTL
 */

/** \def _PLATFORMSTL_VER_REVISION
 * The revision version number of PlatformSTL
 */

/** \def _PLATFORMSTL_VER
 * The current composite version number of PlatformSTL
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _PLATFORMSTL_VER_1_0_1     0x00010001  /*!< Version 1.0.1 */
# define _PLATFORMSTL_VER_1_1_1     0x00010101  /*!< Version 1.1.1 */
# define _PLATFORMSTL_VER_1_1_2     0x00010102  /*!< Version 1.1.2 */
# define _PLATFORMSTL_VER_1_2_1     0x00010201  /*!< Version 1.2.1 */
# define _PLATFORMSTL_VER_1_3_1     0x00010301  /*!< Version 1.3.1 */
# define _PLATFORMSTL_VER_1_4_1     0x00010401  /*!< Version 1.4.1 */
# define _PLATFORMSTL_VER_1_4_2     0x00010402  /*!< Version 1.4.2 */
# define _PLATFORMSTL_VER_1_5_1     0x00010501  /*!< Version 1.5.1 (with STLSoft 1.9.1) */
# define _PLATFORMSTL_VER_1_6_1     0x00010601  /*!< Version 1.6.1 (with STLSoft 1.9.16) */
# define _PLATFORMSTL_VER_1_6_2     0x00010602  /*!< Version 1.6.2 (with STLSoft 1.9.25) */
# define _PLATFORMSTL_VER_1_7_1     0x00010701  /*!< Version 1.7.1 (with STLSoft 1.9.38) */
# define _PLATFORMSTL_VER_1_7_2     0x010702ff  /*!< Version 1.7.2 (with STLSoft 1.9.64) */
# define _PLATFORMSTL_VER_1_8_1     0x010801ff  /*!< Version 1.8.1 (with STLSoft 1.9.86) */
# define _PLATFORMSTL_VER_1_8_2     0x010802ff  /*!< Version 1.8.2 (with STLSoft 1.9.90) */
# define _PLATFORMSTL_VER_1_8_3     0x010803ff  /*!< Version 1.8.3 (with STLSoft 1.9.110) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _PLATFORMSTL_VER_MAJOR      1
#define _PLATFORMSTL_VER_MINOR      8
#define _PLATFORMSTL_VER_REVISION   3
#define _PLATFORMSTL_VER            _PLATFORMSTL_VER_1_8_3

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if _STLSOFT_VER < 0x01096eff
# error This version of the PlatformSTL libraries requires STLSoft version 1.9.100, or later. (www.stlsoft.org)
#endif /* STLSoft version */

/* /////////////////////////////////////////////////////////////////////////
 * Operating system identification
 */

#if defined(unix) || \
    defined(UNIX) || \
    defined(__unix__) || \
    defined(__unix)
# define PLATFORMSTL_OS_IS_UNIX
#elif defined(WIN64) || \
      defined(_WIN64) || \
      defined(__WIN64__)
# define PLATFORMSTL_OS_IS_WIN64
#elif defined(WIN32) || \
      defined(_WIN32) || \
      defined(__WIN32__)
# define PLATFORMSTL_OS_IS_WIN32
#else /* ? operating system */
# error Operating system not discriminated. Only UNIX, Win32 and Win64 are currently recognised by PlatformSTL
#endif /* operating system */

#if defined(PLATFORMSTL_OS_IS_WIN32) || \
    defined(PLATFORMSTL_OS_IS_WIN64)
# define PLATFORMSTL_OS_IS_WINDOWS
#endif /* PLATFORMSTL_OS_IS_WIN32 || PLATFORMSTL_OS_IS_WIN64 */

/* /////////////////////////////////////////////////////////////////////////
 * Platform-specific includes
 */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# include <unixstl/unixstl.h>
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# include <winstl/winstl.h>
#else /* ? operating system */
# error Operating system not discriminated. Only UNIX and Windows are currently recognised by PlatformSTL
#endif /* operating system */

/* /////////////////////////////////////////////////////////////////////////
 * Architecture
 */

#ifdef PLATFORMSTL_ARCH_IS_X86
# undef PLATFORMSTL_ARCH_IS_X86
#endif /* PLATFORMSTL_ARCH_IS_X86 */
#ifdef PLATFORMSTL_ARCH_IS_IA64
# undef PLATFORMSTL_ARCH_IS_IA64
#endif /* PLATFORMSTL_ARCH_IS_IA64 */
#ifdef PLATFORMSTL_ARCH_IS_X64
# undef PLATFORMSTL_ARCH_IS_X64
#endif /* PLATFORMSTL_ARCH_IS_X64 */

#ifdef PLATFORMSTL_ARCH_IS_INTEL
# undef PLATFORMSTL_ARCH_IS_INTEL
#endif /* PLATFORMSTL_ARCH_IS_INTEL */
#ifdef PLATFORMSTL_ARCH_IS_POWERPC
# undef PLATFORMSTL_ARCH_IS_POWERPC
#endif /* PLATFORMSTL_ARCH_IS_POWERPC */
#ifdef PLATFORMSTL_ARCH_IS_ALPHA
# undef PLATFORMSTL_ARCH_IS_ALPHA
#endif /* PLATFORMSTL_ARCH_IS_ALPHA */
#ifdef PLATFORMSTL_ARCH_IS_HPPA
# undef PLATFORMSTL_ARCH_IS_HPPA
#endif /* PLATFORMSTL_ARCH_IS_HPPA */
#ifdef PLATFORMSTL_ARCH_IS_SPARC
# undef PLATFORMSTL_ARCH_IS_SPARC
#endif /* PLATFORMSTL_ARCH_IS_SPARC */
#ifdef PLATFORMSTL_ARCH_IS_UNKNOWN
# undef PLATFORMSTL_ARCH_IS_UNKNOWN
#endif /* PLATFORMSTL_ARCH_IS_UNKNOWN */


#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifdef UNIXSTL_ARCH_IS_X86
#  define PLATFORMSTL_ARCH_IS_X86
# endif /* UNIXSTL_ARCH_IS_X86 */
# ifdef UNIXSTL_ARCH_IS_IA64
#  define PLATFORMSTL_ARCH_IS_IA64
# endif /* UNIXSTL_ARCH_IS_IA64 */
# ifdef UNIXSTL_ARCH_IS_X64
#  define PLATFORMSTL_ARCH_IS_X64
# endif /* UNIXSTL_ARCH_IS_X64 */
# ifdef UNIXSTL_ARCH_IS_INTEL
#  define PLATFORMSTL_ARCH_IS_INTEL
# endif /* UNIXSTL_ARCH_IS_INTEL */
# ifdef UNIXSTL_ARCH_IS_POWERPC
#  define PLATFORMSTL_ARCH_IS_POWERPC
# endif /* UNIXSTL_ARCH_IS_POWERPC */
# ifdef UNIXSTL_ARCH_IS_ALPHA
#  define PLATFORMSTL_ARCH_IS_ALPHA
# endif /* UNIXSTL_ARCH_IS_ALPHA */
# ifdef UNIXSTL_ARCH_IS_HPPA
#  define PLATFORMSTL_ARCH_IS_HPPA
# endif /* UNIXSTL_ARCH_IS_HPPA */
# ifdef UNIXSTL_ARCH_IS_SPARC
#  define PLATFORMSTL_ARCH_IS_SPARC
# endif /* UNIXSTL_ARCH_IS_SPARC */
# ifdef UNIXSTL_ARCH_IS_UNKNOWN
#  define PLATFORMSTL_ARCH_IS_UNKNOWN
# endif /* UNIXSTL_ARCH_IS_UNKNOWN */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifdef WINSTL_ARCH_IS_X86
#  define PLATFORMSTL_ARCH_IS_X86
# endif /* WINSTL_ARCH_IS_X86 */
# ifdef WINSTL_ARCH_IS_IA64
#  define PLATFORMSTL_ARCH_IS_IA64
# endif /* WINSTL_ARCH_IS_IA64 */
# ifdef WINSTL_ARCH_IS_X64
#  define PLATFORMSTL_ARCH_IS_X64
# endif /* WINSTL_ARCH_IS_X64 */
#else /* ? operating system */
#endif /* operating system */

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
#ifndef PLATFORMSTL_ASSERT
# define PLATFORMSTL_ASSERT(expr)             STLSOFT_ASSERT(expr)
#endif /* !PLATFORMSTL_ASSERT */

/** \brief Defines a runtime assertion, with message
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define PLATFORMSTL_MESSAGE_ASSERT(msg, expr) STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \def PLATFORMSTL_STATIC_ASSERT(expr)
 *
 * \brief Defines an assertion construct for compile-time verification.
 *
 * \param expr A compile-time evaluatable condition that must be non-zero, or compilation will fail.
 *
 * \remarks This is defined to \ref STLSOFT_STATIC_ASSERT.
 */
#define PLATFORMSTL_STATIC_ASSERT(expr)       STLSOFT_STATIC_ASSERT(expr)

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The PLATFORMSTL components are contained within the platformstl namespace. This is
 * usually an alias for stlsoft::platformstl_project,
 *
 * Since STLSoft supports the suspension of namespaces for its main and sub-projects
 * (except PlatformSTL) PlatformSTL must be able to work when STLSoft and/or
 * UNIXSTL and/or WinSTL constructs are defined in the global namespace.
 */

#if defined(__cplusplus)
# if defined(_STLSOFT_NO_NAMESPACES)
#  error PlatformSTL may not be compiled when _STLSOFT_NO_NAMESPACES is defined. Note: it _can_ be compiled in the presence of _STLSOFT_NO_NAMESPACE, or _UNIXSTL_NO_NAMESPACE, or _WINSTL_NO_NAMESPACE
# endif /* _STLSOFT_NO_NAMESPACES */

# if defined(_PLATFORMSTL_NO_NAMESPACES)
#  error Use of namespaces in PlatformSTL may not be suspended; _PLATFORMSTL_NO_NAMESPACES was detected
# endif /* _PLATFORMSTL_NO_NAMESPACES */

# if defined(_PLATFORMSTL_NO_NAMESPACE)
#  error Use of namespaces in PlatformSTL may not be suspended; _PLATFORMSTL_NO_NAMESPACE was detected
# endif /* _PLATFORMSTL_NO_NAMESPACE */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifdef UNIXSTL_NO_NAMESPACE
#  define PLATFORMSTL_NO_PLATFORM_NAMESPACE
# endif /* UNIXSTL_NO_NAMESPACE */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifdef WINSTL_NO_NAMESPACE
#  define PLATFORMSTL_NO_PLATFORM_NAMESPACE
# endif /* WINSTL_NO_NAMESPACE */
#else /* ? operating system */
# error Operating system not discriminated. Only UNIX and Windows are currently recognised by PlatformSTL
#endif /* operating system */

#endif /* __cplusplus */

#if !defined(__cplusplus)
 /* Nothing defined in C */
#elif defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::platformstl */
/** \brief The <code class="namespace">platformstl</code> namespace contains all components
 *   in the \ref group__project__platformstl "PlatformSTL" project.
 *
 * By default, the <code>platformstl</code> namespace is actually an alias for
 * the namespace <code>stlsoft::platformstl_project</code>, which is where all
 * the \ref group__project__platformstl "PlatformSTL" components actually reside. This
 * measure allows all components within the main the
 * \ref group__project__stlsoft "STLSoft" project (which are defined within
 * the <code>stlsoft</code> namespace) to be visible to all components
 * "within" the <code>platformstl</code> namespace. (Otherwise, there would be a
 * whole lot of onerous qualification throughout the code of all
 * \ref group__projects "sub-projects".)
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_PLATFORMSTL_NO_NAMESPACE</code> are defined, all
 * \ref group__project__platformstl "PlatformSTL" components will be defined in the
 * global namespace. Conversely, if the <code>_STLSOFT_NO_NAMESPACE</code>
 * symbol (not to be confused with the
 * <code>_STLSOFT_NO_NAMESPACES</code> symbol!) is defined - meaning that
 * all \ref group__project__stlsoft "main project" components are to be
 * defined in the global namespace, and <code>_PLATFORMSTL_NO_NAMESPACE</code>
 * is <b>not</b> defined, then all \ref group__project__platformstl "PlatformSTL"
 * components will be defined within a bona fide <code>platformstl</code>
 * namespace.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace platformstl
{
#else
/* Define stlsoft::platformstl_project */

namespace stlsoft
{

namespace platformstl_project
{
#endif /* _STLSOFT_NO_NAMESPACE */

/** \def platformstl_ns_qual(x)
 * Qualifies with <b>platformstl::</b> if PlatformSTL is using namespaces or, if not, does not qualify
 */

/** \def platformstl_ns_using(x)
 * Declares a using directive (with respect to <b>platformstl</b>) if PlatformSTL is using namespaces or, if not, does nothing
 */

#ifndef _STLSOFT_NO_NAMESPACE
# define platformstl_ns_qual(x)         ::platformstl::x
# define platformstl_ns_using(x)        using ::platformstl::x;
#else /* ? _STLSOFT_NO_NAMESPACE */
# define platformstl_ns_qual(x)         x
# define platformstl_ns_using(x)
#endif /* !_STLSOFT_NO_NAMESPACE */

/** \def platformstl_ns_qual_std(x)
 * Qualifies with <b>std::</b> if PlatformSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def platformstl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if PlatformSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define platformstl_ns_qual_std(x)     ::std::x
# define platformstl_ns_using_std(x)    using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define platformstl_ns_qual_std(x)     x
# define platformstl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#if !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) && \
    defined(__cplusplus)

# if defined(PLATFORMSTL_OS_IS_UNIX)
typedef unixstl_ns_qual(us_char_a_t)        char_a_t;    /*!< Ansi char type */
typedef unixstl_ns_qual(us_char_w_t)        char_w_t;    /*!< Unicode char type */
typedef unixstl_ns_qual(us_sint8_t)         sint8_t;     /*!< 8-bit signed integer */
typedef unixstl_ns_qual(us_uint8_t)         uint8_t;     /*!< 8-bit unsigned integer */
typedef unixstl_ns_qual(us_int16_t)         int16_t;     /*!< 16-bit integer */
typedef unixstl_ns_qual(us_sint16_t)        sint16_t;    /*!< 16-bit signed integer */
typedef unixstl_ns_qual(us_uint16_t)        uint16_t;    /*!< 16-bit unsigned integer */
typedef unixstl_ns_qual(us_int32_t)         int32_t;     /*!< 32-bit integer */
typedef unixstl_ns_qual(us_sint32_t)        sint32_t;    /*!< 32-bit signed integer */
typedef unixstl_ns_qual(us_uint32_t)        uint32_t;    /*!< 32-bit unsigned integer */
#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef unixstl_ns_qual(us_int64_t)         int64_t;     /*!< 64-bit integer */
typedef unixstl_ns_qual(us_sint64_t)        sint64_t;    /*!< 64-bit signed integer */
typedef unixstl_ns_qual(us_uint64_t)        uint64_t;    /*!< 64-bit unsigned integer */
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef unixstl_ns_qual(us_int_t)           int_t;       /*!< integer */
typedef unixstl_ns_qual(us_sint_t)          sint_t;      /*!< signed integer */
typedef unixstl_ns_qual(us_uint_t)          uint_t;      /*!< unsigned integer */
typedef unixstl_ns_qual(us_long_t)          long_t;      /*!< long */
#ifdef __cplusplus
typedef unixstl_ns_qual(us_bool_t)          bool_t;      /*!< bool */
#endif /* __cplusplus */
typedef unixstl_ns_qual(us_streampos_t)     streampos_t; /*!< streampos */
typedef unixstl_ns_qual(us_streamoff_t)     streamoff_t; /*!< streamoff */
# elif defined(PLATFORMSTL_OS_IS_WINDOWS)
typedef winstl_ns_qual(ws_char_a_t)         char_a_t;    /*!< Ansi char type */
typedef winstl_ns_qual(ws_char_w_t)         char_w_t;    /*!< Unicode char type */
typedef winstl_ns_qual(ws_sint8_t)          sint8_t;     /*!< 8-bit signed integer */
typedef winstl_ns_qual(ws_uint8_t)          uint8_t;     /*!< 8-bit unsigned integer */
typedef winstl_ns_qual(ws_int16_t)          int16_t;     /*!< 16-bit integer */
typedef winstl_ns_qual(ws_sint16_t)         sint16_t;    /*!< 16-bit signed integer */
typedef winstl_ns_qual(ws_uint16_t)         uint16_t;    /*!< 16-bit unsigned integer */
typedef winstl_ns_qual(ws_int32_t)          int32_t;     /*!< 32-bit integer */
typedef winstl_ns_qual(ws_sint32_t)         sint32_t;    /*!< 32-bit signed integer */
typedef winstl_ns_qual(ws_uint32_t)         uint32_t;    /*!< 32-bit unsigned integer */
#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef winstl_ns_qual(ws_int64_t)          int64_t;     /*!< 64-bit integer */
typedef winstl_ns_qual(ws_sint64_t)         sint64_t;    /*!< 64-bit signed integer */
typedef winstl_ns_qual(ws_uint64_t)         uint64_t;    /*!< 64-bit unsigned integer */
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef winstl_ns_qual(ws_int_t)            int_t;       /*!< integer */
typedef winstl_ns_qual(ws_sint_t)           sint_t;      /*!< signed integer */
typedef winstl_ns_qual(ws_uint_t)           uint_t;      /*!< unsigned integer */
typedef winstl_ns_qual(ws_long_t)           long_t;      /*!< long */
#ifdef __cplusplus
typedef winstl_ns_qual(ws_bool_t)           bool_t;      /*!< bool */
#endif /* __cplusplus */
typedef winstl_ns_qual(ws_streampos_t)      streampos_t; /*!< streampos */
typedef winstl_ns_qual(ws_streamoff_t)      streamoff_t; /*!< streamoff */
# else /* ? operating system */
# error Operating system not discriminated. Only UNIX and Win32 are currently recognised by PlatformSTL
# endif /* operating system */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION && __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#if !defined(__cplusplus)
 /* Nothing defined in C */
#elif defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace platformstl */
#else
} /* namespace platformstl_project */
} /* namespace stlsoft */
namespace platformstl = ::stlsoft::platformstl_project;
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_H_PLATFORMSTL */

/* ///////////////////////////// end of file //////////////////////////// */
