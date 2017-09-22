/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/inetstl.h
 *
 * Purpose:     Root header for the InetSTL libraries. Performs various compiler
 *              and platform discriminations, and definitions of types.
 *
 * Created:     24th April 2004
 * Updated:     22nd November 2013
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2013, Matthew Wilson and Synesis Software
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


#ifndef INETSTL_INCL_INETSTL_H_INETSTL
#define INETSTL_INCL_INETSTL_H_INETSTL
#define INETSTL_INCL_H_INETSTL

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_H_INETSTL_MAJOR    3
# define INETSTL_VER_INETSTL_H_INETSTL_MINOR    6
# define INETSTL_VER_INETSTL_H_INETSTL_REVISION 2
# define INETSTL_VER_INETSTL_H_INETSTL_EDIT     54
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file inetstl/inetstl.h \brief [C, C++] The root header for the \ref group__project__inetstl "InetSTL" project. */

/* /////////////////////////////////////////////////////////////////////////
 * InetSTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * The major version is denoted by the _INETSTL_VER_MAJOR preprocessor symbol.
 * A changes to the major version component implies that a dramatic change has
 * occurred in the libraries, such that considerable changes to source dependent
 * on previous versions would need to be effected.
 *
 * The minor version is denoted by the _INETSTL_VER_MINOR preprocessor symbol.
 * Changes to the minor version component imply that a significant change has
 * occurred to the libraries, either in the addition of new functionality or in
 * the destructive change to one or more components such that recomplilation and
 * code change may be necessitated.
 *
 * The revision version is denoted by the _INETSTL_VER_REVISION preprocessor
 * symbol. Changes to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the changes.
 *
 * In addition to the individual version symbols - _INETSTL_VER_MAJOR,
 * _INETSTL_VER_MINOR and _INETSTL_VER_REVISION - a composite symbol _INETSTL_VER
 * is defined, where the upper 8 bits are 0, bits 16-23 represent the major
 * component,  bits 8-15 represent the minor component, and bits 0-7 represent
 * the revision component.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies _INETSTL_VER_1_0_1.
 *
 * Thus the symbol _INETSTL_VER may be compared meaningfully with a specific
 * version symbol, e.g. #if _INETSTL_VER >= _INETSTL_VER_1_0_1
 */

/** \def _INETSTL_VER_MAJOR
 * The major version number of InetSTL
 */

/** \def _INETSTL_VER_MINOR
 * The minor version number of InetSTL
 */

/** \def _INETSTL_VER_REVISION
 * The revision version number of InetSTL
 */

/** \def _INETSTL_VER
 * The current composite version number of InetSTL
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _INETSTL_VER_1_0_1      0x00010001  /*!< Version 1.0.1 */
# define _INETSTL_VER_1_0_2      0x00010002  /*!< Version 1.0.2 */
# define _INETSTL_VER_1_1_1      0x00010101  /*!< Version 1.1.1 */
# define _INETSTL_VER_1_1_2      0x00010102  /*!< Version 1.1.2 */
# define _INETSTL_VER_1_1_3      0x00010103  /*!< Version 1.1.3 */
# define _INETSTL_VER_1_1_4      0x00010104  /*!< Version 1.1.4 */
# define _INETSTL_VER_1_2_1      0x00010201  /*!< Version 1.2.1 (with STLSoft 1.9.1) */
# define _INETSTL_VER_1_2_2      0x00010202  /*!< Version 1.2.2 (with STLSoft 1.9.25) */
# define _INETSTL_VER_1_2_3      0x010203ff  /*!< Version 1.2.3 (with STLSoft 1.9.46) */
# define _INETSTL_VER_1_3_1      0x010301ff  /*!< Version 1.3.1 (with STLSoft 1.9.79) */
# define _INETSTL_VER_1_3_2      0x010302ff  /*!< Version 1.3.2 (with STLSoft 1.9.80) */
# define _INETSTL_VER_1_3_3      0x010303ff  /*!< Version 1.3.3 (with STLSoft 1.9.86) */
# define _INETSTL_VER_1_3_4      0x010304ff  /*!< Version 1.3.4 (with STLSoft 1.9.91) */
# define _INETSTL_VER_1_3_5      0x010305ff  /*!< Version 1.3.5 (with STLSoft 1.9.92) */
# define _INETSTL_VER_1_3_6      0x010306ff  /*!< Version 1.3.6 (with STLSoft 1.9.113) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _INETSTL_VER_MAJOR       1
#define _INETSTL_VER_MINOR       3
#define _INETSTL_VER_REVISION    6
#define _INETSTL_VER             _INETSTL_VER_1_3_6

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

/* Strict */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifndef STRICT
#  if defined(_INETSTL_STRICT) || \
      (   !defined(_INETSTL_NO_STRICT) && \
          !defined(NO_STRICT))
#   define STRICT 1
#  endif /* !NO_STRICT && !_INETSTL_NO_STRICT */
# endif /* STRICT */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(WIN32) || \
    defined(WIN64)

# define INETSTL_OS_IS_WINDOWS

# ifndef STLSOFT_INCL_H_WINDOWS
#  define STLSOFT_INCL_H_WINDOWS
#  include <windows.h>
# endif /* !STLSOFT_INCL_H_WINDOWS */
# ifndef STLSOFT_INCL_H_WININET
#  define STLSOFT_INCL_H_WININET
#  include <wininet.h>
# endif /* !STLSOFT_INCL_H_WININET */

#else /* ? OS */

#endif /* OS */


/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if _STLSOFT_VER < 0x010971ff
# error This version of the InetSTL libraries requires STLSoft version 1.9.113, or later
#endif /* _STLSOFT_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Sanity checks
 */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the InetSTL libraries are
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
#  error Versions of Borland C++ prior to 5.5 are not supported by the InetSTL libraries
# endif /* __BORLANDC__ */

#elif defined(STLSOFT_COMPILER_IS_COMO)
/* Comeau C++ */
# if __COMO_VERSION__ < 4300
#  error Versions of Comeau C++ prior to 4.3 are not supported by the InetSTL libraries
# endif /* __COMO_VERSION__ */

#elif defined(STLSOFT_COMPILER_IS_DMC)
/* Digital Mars C/C++ */
# if __DMC__ < 0x0826
#  error Versions of Digital Mars C/C++ prior to 8.26 are not supported by the InetSTL libraries
# endif /* __DMC__ */

#elif defined(STLSOFT_COMPILER_IS_GCC)
/* GNU C/C++ */
# if __GNUC__ < 2 || \
     (  __GNUC__ == 2 && \
        __GNUC_MINOR__ < 95)
#  error Versions of GNU C/C++ prior to 2.95 are not supported by the InetSTL libraries
# endif /* __GNUC__ */

#elif defined(STLSOFT_COMPILER_IS_INTEL)
/* Intel C++ */
# if (__INTEL_COMPILER < 600)
#  error Versions of Intel C++ prior to 6.0 are not supported by the InetSTL libraries
# endif /* __INTEL_COMPILER */

#elif defined(STLSOFT_COMPILER_IS_MWERKS)
/* Metrowerks C++ */
# if (__MWERKS__ & 0xFF00) < 0x2400
#  error Versions of Metrowerks CodeWarrior C++ prior to 7.0 are not supported by the InetSTL libraries
# endif /* __MWERKS__ */

#elif defined(STLSOFT_COMPILER_IS_MSVC)
/* Visual C++ */
# if _MSC_VER < 1020
#  error Versions of Visual C++ prior to 4.2 are not supported by the InetSTL libraries
# endif /* _MSC_VER */

#elif defined(STLSOFT_COMPILER_IS_VECTORC)
/* VectorC C/C++ */

#elif defined(STLSOFT_COMPILER_IS_WATCOM)
/* Watcom C/C++ */
# if (__WATCOMC__ < 1200)
#  error Versions of Watcom C/C++ prior to 12.0 are not supported by the InetSTL libraries
# endif /* __WATCOMC__ */

#else /* ? compiler */
/* No recognised compiler */
# ifdef _STLSOFT_FORCE_ANY_COMPILER
#  define _INETSTL_COMPILER_IS_UNKNOWN
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Compiler is unknown to InetSTL")
#  endif /* STLSOFT_COMPILE_VERBOSE */
# else /* ? _STLSOFT_FORCE_ANY_COMPILER */
#  error Currently only Borland C++, Digital Mars C/C++, Intel C/C++, Metrowerks CodeWarrior and Visual C++ compilers are supported by the InetSTL libraries
# endif /* _STLSOFT_FORCE_ANY_COMPILER */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro inetstl_assert provides standard debug-mode assert functionality.
 */

/** \brief Defines a runtime assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 */
#define INETSTL_ASSERT(expr)                STLSOFT_ASSERT(expr)

/** \brief Defines a runtime assertion, with message
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define INETSTL_MESSAGE_ASSERT(msg, expr)   STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \brief Defines a compile-time assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or compilation will fail
 */
#define INETSTL_STATIC_ASSERT(expr)         STLSOFT_STATIC_ASSERT(expr)



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define inetstl_assert(expr)               INETSTL_ASSERT(expr)
# define inetstl_message_assert(msg, expr)  INETSTL_MESSAGE_ASSERT(msg, expr)
# define inetstl_static_assert(expr)        INETSTL_STATIC_ASSERT(expr)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The InetSTL components are contained within the inetstl namespace. This is
 * usually an alias for stlsoft::inetstl_project,
 *
 * When compilers support namespaces they are defined by default. They can be
 * undefined using a cascasing system, as follows:
 *
 * If _STLSOFT_NO_NAMESPACES is defined, then _INETSTL_NO_NAMESPACES is defined.
 *
 * If _INETSTL_NO_NAMESPACES is defined, then _INETSTL_NO_NAMESPACE is defined.
 *
 * If _INETSTL_NO_NAMESPACE is defined, then the InetSTL constructs are defined
 * in the global scope.
 *
 * If _STLSOFT_NO_NAMESPACES, _INETSTL_NO_NAMESPACES and _INETSTL_NO_NAMESPACE are
 * all undefined but the symbol _STLSOFT_NO_NAMESPACE is defined (whence the
 * namespace stlsoft does not exist), then the InetSTL constructs are defined
 * within the inetstl namespace. The definition matrix is as follows:
 *
 * _STLSOFT_NO_NAMESPACE    _INETSTL_NO_NAMESPACE    inetstl definition
 * ---------------------    --------------------    -----------------
 *  not defined              not defined             = stlsoft::inetstl_project
 *  not defined              defined                 not defined
 *  defined                  not defined             inetstl
 *  defined                  defined                 not defined
 *
 *
 *
 * The macro inetstl_ns_qual() macro can be used to refer to elements in the
 * InetSTL libraries irrespective of whether they are in the
 * stlsoft::inetstl_project (or inetstl) namespace or in the global namespace.
 *
 * Furthermore, some compilers do not support the standard library in the std
 * namespace, so the inetstl_ns_qual_std() macro can be used to refer to elements
 * in the InetSTL libraries irrespective of whether they are in the std namespace
 * or in the global namespace.
 */

/* No STLSoft namespaces means no InetSTL namespaces */
#ifdef _STLSOFT_NO_NAMESPACES
# define _INETSTL_NO_NAMESPACES
#endif /* _STLSOFT_NO_NAMESPACES */

/* No InetSTL namespaces means no inetstl namespace */
#ifdef _INETSTL_NO_NAMESPACES
# define _INETSTL_NO_NAMESPACE
#endif /* _INETSTL_NO_NAMESPACES */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::inetstl */
/** \brief The <code class="namespace">inetstl</code> namespace contains all components
 *   in the \ref group__project__inetstl "InetSTL" project.
 *
 * By default, the <code>inetstl</code> namespace is actually an alias for
 * the namespace <code>stlsoft::inetstl_project</code>, which is where all
 * the \ref group__project__inetstl "InetSTL" components actually reside. This
 * measure allows all components within the main the
 * \ref group__project__stlsoft "STLSoft" project (which are defined within
 * the <code>stlsoft</code> namespace) to be visible to all components
 * "within" the <code>inetstl</code> namespace. (Otherwise, there would be a
 * whole lot of onerous qualification throughout the code of all
 * \ref group__projects "sub-projects".)
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_INETSTL_NO_NAMESPACE</code> are defined, all
 * \ref group__project__inetstl "InetSTL" components will be defined in the
 * global namespace. Conversely, if the <code>_STLSOFT_NO_NAMESPACE</code>
 * symbol (not to be confused with the
 * <code>_STLSOFT_NO_NAMESPACES</code> symbol!) is defined - meaning that
 * all \ref group__project__stlsoft "main project" components are to be
 * defined in the global namespace, and <code>_INETSTL_NO_NAMESPACE</code>
 * is <b>not</b> defined, then all \ref group__project__inetstl "InetSTL"
 * components will be defined within a bona fide <code>inetstl`</code>
 * namespace.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace inetstl
{
# else
/* Define stlsoft::inetstl_project */

namespace stlsoft
{

namespace inetstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#else
stlsoft_ns_using(move_lhs_from_rhs)
#endif /* !_INETSTL_NO_NAMESPACE */

/** \def inetstl_ns_qual(x)
 * Qualifies with <b>inetstl::</b> if InetSTL is using namespaces or, if not, does not qualify
 */

/** \def inetstl_ns_using(x)
 * Declares a using directive (with respect to <b>inetstl</b>) if InetSTL is using namespaces or, if not, does nothing
 */

#ifndef _INETSTL_NO_NAMESPACE
# define inetstl_ns_qual(x)          ::inetstl::x
# define inetstl_ns_using(x)         using ::inetstl::x;
#else /* ? _INETSTL_NO_NAMESPACE */
# define inetstl_ns_qual(x)          x
# define inetstl_ns_using(x)
#endif /* !_INETSTL_NO_NAMESPACE */

/** \def inetstl_ns_qual_std(x)
 * Qualifies with <b>std::</b> if InetSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def inetstl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if InetSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define inetstl_ns_qual_std(x)      ::std::x
# define inetstl_ns_using_std(x)     using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define inetstl_ns_qual_std(x)      x
# define inetstl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The InetSTL uses a number of typedefs to aid in compiler-independence in the
 * libraries' main code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

typedef stlsoft_ns_qual(ss_char_a_t)        is_char_a_t;    /*!< Ansi char type */
typedef stlsoft_ns_qual(ss_char_w_t)        is_char_w_t;    /*!< Unicode char type */
typedef stlsoft_ns_qual(ss_sint8_t)         is_sint8_t;     /*!< 8-bit signed integer */
typedef stlsoft_ns_qual(ss_uint8_t)         is_uint8_t;     /*!< 8-bit unsigned integer */
typedef stlsoft_ns_qual(ss_int16_t)         is_int16_t;     /*!< 16-bit integer */
typedef stlsoft_ns_qual(ss_sint16_t)        is_sint16_t;    /*!< 16-bit signed integer */
typedef stlsoft_ns_qual(ss_uint16_t)        is_uint16_t;    /*!< 16-bit unsigned integer */
typedef stlsoft_ns_qual(ss_int32_t)         is_int32_t;     /*!< 32-bit integer */
typedef stlsoft_ns_qual(ss_sint32_t)        is_sint32_t;    /*!< 32-bit signed integer */
typedef stlsoft_ns_qual(ss_uint32_t)        is_uint32_t;    /*!< 32-bit unsigned integer */
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
 typedef stlsoft_ns_qual(ss_int64_t)        is_int64_t;     /*!< 64-bit integer */
 typedef stlsoft_ns_qual(ss_sint64_t)       is_sint64_t;    /*!< 64-bit signed integer */
 typedef stlsoft_ns_qual(ss_uint64_t)       is_uint64_t;    /*!< 64-bit unsigned integer */
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef stlsoft_ns_qual(ss_int_t)           is_int_t;       /*!< integer */
typedef stlsoft_ns_qual(ss_sint_t)          is_sint_t;      /*!< signed integer */
typedef stlsoft_ns_qual(ss_uint_t)          is_uint_t;      /*!< unsigned integer */
typedef stlsoft_ns_qual(ss_long_t)          is_long_t;      /*!< long */
typedef stlsoft_ns_qual(ss_byte_t)          is_byte_t;      /*!< byte type */
#ifdef __cplusplus
typedef stlsoft_ns_qual(ss_bool_t)          is_bool_t;      /*!< Boolean type */
#endif /* __cplusplus */
# ifdef INETSTL_OS_IS_WINDOWS
typedef DWORD                               is_dword_t;     /*!< dword */
# endif /* INETSTL_OS_IS_WINDOWS */
typedef stlsoft_ns_qual(ss_size_t)          is_size_t;      /*!< size */
typedef stlsoft_ns_qual(ss_ptrdiff_t)       is_ptrdiff_t;   /*!< ptr diff */
typedef stlsoft_ns_qual(ss_streampos_t)     is_streampos_t; /*!< streampos */
typedef stlsoft_ns_qual(ss_streamoff_t)     is_streamoff_t; /*!< streamoff */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef _INETSTL_NO_NAMESPACE
typedef is_char_a_t         char_a_t;           /*!< Ansi char type */
typedef is_char_w_t         char_w_t;           /*!< Unicode char type */
typedef is_sint8_t          sint8_t;            /*!< 8-bit signed integer */
typedef is_uint8_t          uint8_t;            /*!< 8-bit unsigned integer */
typedef is_int16_t          int16_t;            /*!< 16-bit integer */
typedef is_sint16_t         sint16_t;           /*!< 16-bit signed integer */
typedef is_uint16_t         uint16_t;           /*!< 16-bit unsigned integer */
typedef is_int32_t          int32_t;            /*!< 32-bit integer */
typedef is_sint32_t         sint32_t;           /*!< 32-bit signed integer */
typedef is_uint32_t         uint32_t;           /*!< 32-bit unsigned integer */
# ifdef STLSOFT_CF_64BIT_INT_SUPPORT
 typedef is_int64_t         int64_t;            /*!< 64-bit integer */
 typedef is_sint64_t        sint64_t;           /*!< 64-bit signed integer */
 typedef is_uint64_t        uint64_t;           /*!< 64-bit unsigned integer */
# endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef is_int_t            int_t;              /*!< integer */
typedef is_sint_t           sint_t;             /*!< signed integer */
typedef is_uint_t           uint_t;             /*!< unsigned integer */
typedef is_long_t           long_t;             /*!< long integer */
typedef is_byte_t           byte_t;             /*!< byte type */
typedef is_bool_t           bool_t;             /*!< Boolean type */
# ifdef INETSTL_OS_IS_WINDOWS
typedef is_dword_t          dword_t;            /*!< dword */
# endif /* INETSTL_OS_IS_WINDOWS */
# if !defined(STLSOFT_COMPILER_IS_DMC)
typedef is_streampos_t      streampos_t;        /*!< streampos */
typedef is_streamoff_t      streamoff_t;        /*!< streamoff */
# endif /* compiler */
#endif /* !_INETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Values
 *
 * Since the boolean type may not be supported natively on all compilers, the
 * values of true and false may also not be provided. Hence the values of
 * is_true_v and is_false_v are defined, and are used in all code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define is_true_v       ss_true_v
#define is_false_v      ss_false_v

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Code modification macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/* Exception signatures. */
# define inetstl_throw_0()                               stlsoft_throw_0()
# define inetstl_throw_1(x1)                             stlsoft_throw_1(x1)
# define inetstl_throw_2(x1, x2)                         stlsoft_throw_2(x1, x2)
# define inetstl_throw_3(x1, x2, x3)                     stlsoft_throw_3(x1, x2, x3)
# define inetstl_throw_4(x1, x2, x3, x4)                 stlsoft_throw_4(x1, x2, x3, x4)
# define inetstl_throw_5(x1, x2, x3, x4, x5)             stlsoft_throw_5(x1, x2, x3, x4, x5)
# define inetstl_throw_6(x1, x2, x3, x4, x5, x6)         stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
# define inetstl_throw_7(x1, x2, x3, x4, x5, x6, x7)     stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
# define inetstl_throw_8(x1, x2, x3, x4, x5, x6, x7, x8) stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_NUM_ELEMENTS(ar)                        STLSOFT_NUM_ELEMENTS(ar)
# define inetstl_num_elements(ar)                        INETSTL_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [DEPRECATED] Destroys the given instance \c p of the given type (\c t and \c type)
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 */
#define inetstl_destroy_instance(t, type, p)             STLSOFT_DESTROY_INSTANCE(t, type, p)

/** \brief [DEPRECATED] Generates an opaque type with the name \c _htype
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 */
#define inetstl_gen_opaque(htype)                        STLSOFT_GEN_OPAQUE(htype)

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace inetstl */
# else
} /* namespace inetstl_project */
} /* namespace stlsoft */
namespace inetstl = ::stlsoft::inetstl_project;
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* INETSTL_INCL_INETSTL_H_INETSTL */

/* ///////////////////////////// end of file //////////////////////////// */
