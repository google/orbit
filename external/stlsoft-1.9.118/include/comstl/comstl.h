/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/comstl.h
 *
 * Purpose:     Root header for the COMSTL libraries. Performs various compiler
 *              and platform discriminations, and definitions of types.
 *
 * Created:     15th January 2002
 * Updated:     22nd November 2013
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2013, Matthew Wilson and Synesis Software
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


#ifndef COMSTL_INCL_COMSTL_H_COMSTL
#define COMSTL_INCL_COMSTL_H_COMSTL
/** \brief Defined for backward-compatiblity
 *
 * This is the old header include guard, from when comstl.h was in the
 * STLSoft root include directory
 */
#define COMSTL_INCL_H_COMSTL

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_H_COMSTL_MAJOR       3
# define COMSTL_VER_COMSTL_H_COMSTL_MINOR       9
# define COMSTL_VER_COMSTL_H_COMSTL_REVISION    2
# define COMSTL_VER_COMSTL_H_COMSTL_EDIT        117
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file comstl/comstl.h
 *
 * \brief [C, C++; requires COM] The root header for the \ref group__project__comstl "COMSTL" project.
 */

/* /////////////////////////////////////////////////////////////////////////
 * COMSTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * The major version is denoted by the _COMSTL_VER_MAJOR preprocessor symbol.
 * A changes to the major version component implies that a dramatic change has
 * occurred in the libraries, such that considerable changes to source dependent
 * on previous versions would need to be effected.
 *
 * The minor version is denoted by the _COMSTL_VER_MINOR preprocessor symbol.
 * Changes to the minor version component imply that a significant change has
 * occurred to the libraries, either in the addition of new functionality or in
 * the destructive change to one or more components such that recomplilation and
 * code change may be necessitated.
 *
 * The revision version is denoted by the _COMSTL_VER_REVISION preprocessor
 * symbol. Changes to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the changes.
 *
 * In addition to the individual version symbols - _COMSTL_VER_MAJOR,
 * _COMSTL_VER_MINOR and _COMSTL_VER_REVISION - a composite symbol _COMSTL_VER
 * is defined, where the upper 8 bits are 0, bits 16-23 represent the major
 * component,  bits 8-15 represent the minor component, and bits 0-7 represent
 * the revision component.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies _COMSTL_VER_1_0_1.
 *
 * Thus the symbol _COMSTL_VER may be compared meaningfully with a specific
 * version symbol, e.g. #if _COMSTL_VER >= _COMSTL_VER_1_0_1
 */

/** \def _COMSTL_VER_MAJOR
 *
 * \brief The major version number of COMSTL
 *
 * This number is subject to increment when changes to COMSTL are so
 * substantial as to require wholesale changes to application code.
 */

/** \def _COMSTL_VER_MINOR
 *
 * \brief The minor version number of COMSTL
 *
 * This number is subject to increment when changes to COMSTL as so
 * substantial as to require wholesale changes to application code.
 */

/** \def _COMSTL_VER_REVISION
 *
 * \brief The revision version number of COMSTL
 */

/** \def _COMSTL_VER
 *
 * \brief The current composite version number of COMSTL
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _COMSTL_VER_1_0_1      0x00010001  /*!< Version 1.0.1 */
# define _COMSTL_VER_1_1_1      0x00010101  /*!< Version 1.1.1 */
# define _COMSTL_VER_1_2_1      0x00010201  /*!< Version 1.2.1 */
# define _COMSTL_VER_1_2_2      0x00010202  /*!< Version 1.2.2 */
# define _COMSTL_VER_1_2_3      0x00010203  /*!< Version 1.2.3 */
# define _COMSTL_VER_1_2_4      0x00010204  /*!< Version 1.2.4 */
# define _COMSTL_VER_1_2_5      0x00010205  /*!< Version 1.2.5 */
# define _COMSTL_VER_1_3_1      0x00010301  /*!< Version 1.3.1 */
# define _COMSTL_VER_1_4_1      0x00010401  /*!< Version 1.4.1 */
# define _COMSTL_VER_1_5_1      0x00010501  /*!< Version 1.5.1 */
# define _COMSTL_VER_1_5_2      0x00010502  /*!< Version 1.5.2 */
# define _COMSTL_VER_1_5_3      0x00010503  /*!< Version 1.5.3 */
# define _COMSTL_VER_1_5_4      0x00010504  /*!< Version 1.5.4 */
# define _COMSTL_VER_1_6_1      0x00010601  /*!< Version 1.6.1 */
# define _COMSTL_VER_1_6_2      0x00010602  /*!< Version 1.6.2 */
# define _COMSTL_VER_1_7_1      0x00010701  /*!< Version 1.7.1 (with STLSoft 1.9.1) */
# define _COMSTL_VER_1_7_2      0x00010702  /*!< Version 1.7.2 (with STLSoft 1.9.8) */
# define _COMSTL_VER_1_7_3      0x00010703  /*!< Version 1.7.3 (with STLSoft 1.9.25) */
# define _COMSTL_VER_1_7_4      0x010704ff  /*!< Version 1.7.4 (with STLSoft 1.9.31) */
# define _COMSTL_VER_1_8_1      0x010801ff  /*!< Version 1.8.1 (with STLSoft 1.9.37) */
# define _COMSTL_VER_1_8_2      0x010802ff  /*!< Version 1.8.2 (with STLSoft 1.9.57) */
# define _COMSTL_VER_1_8_3      0x010803ff  /*!< Version 1.8.3 (with STLSoft 1.9.59) */
# define _COMSTL_VER_1_8_4      0x010804ff  /*!< Version 1.8.4 (with STLSoft 1.9.84) */
# define _COMSTL_VER_1_8_5      0x010805ff  /*!< Version 1.8.5 (with STLSoft 1.9.88) */
# define _COMSTL_VER_1_8_6      0x010806ff  /*!< Version 1.8.6 (with STLSoft 1.9.97) */
# define _COMSTL_VER_1_9_1      0x010901ff  /*!< Version 1.9.1 (with STLSoft 1.9.98) */
# define _COMSTL_VER_1_9_2      0x010902ff  /*!< Version 1.9.2 (with STLSoft 1.9.113) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _COMSTL_VER_MAJOR       1
#define _COMSTL_VER_MINOR       9
#define _COMSTL_VER_REVISION    2
#define _COMSTL_VER             _COMSTL_VER_1_9_2

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_H_OBJBASE
# define STLSOFT_INCL_H_OBJBASE
# include <objbase.h>
#endif /* !STLSOFT_INCL_H_OBJBASE */

#if defined(STLSOFT_COMPILER_IS_GCC) && \
    defined(_WIN32) && \
    (   defined(WIN32) || \
        defined(WIN64))
# ifndef STLSOFT_INCL_H_OLEAUTO
#  define STLSOFT_INCL_H_OLEAUTO
#  include <oleauto.h>
# endif /* !STLSOFT_INCL_H_OLEAUTO */
# ifndef STLSOFT_INCL_H_OAIDL
#  define STLSOFT_INCL_H_OAIDL
#  include <oaidl.h>
# endif /* !STLSOFT_INCL_H_OAIDL */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if !defined(_STLSOFT_VER) || \
    _STLSOFT_VER < 0x010971ff
# error This version of the COMSTL libraries requires STLSoft version 1.9.113, or later
#endif /* _STLSOFT_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the COMSTL libraries are
 *
 * Borland C++ 5.5, 5.51, 5.6
 * Digital Mars C/C++ 8.26 - 8.32
 * Metrowerks 2.4 & 3.0 (CodeWarrior 7.0 & 8.0)
 * Intel C/C++ 6.0
 * Visual C++ 4.2, 5.0, 6.0, 7.0
 */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
/* Borland C++ */
# if __BORLANDC__ < 0x0550
#  error Versions of Borland C++ prior to 5.5 are not supported by the COMSTL libraries
# endif /* __BORLANDC__ */

#elif defined(STLSOFT_COMPILER_IS_COMO)
/* Comeau C++ */

#elif defined(STLSOFT_COMPILER_IS_DMC)
/* Digital Mars C/C++ */
# if __DMC__ < 0x0826
#  error Versions of Digital Mars C/C++ prior to 8.26 are not supported by the COMSTL libraries
# endif /* __DMC__ */

#elif defined(STLSOFT_COMPILER_IS_GCC)
/* Digital Mars C/C++ */
# if __GNUC__ < 3
#  error Versions of GNU C/C++ prior to 3 are not supported by the COMSTL libraries
# endif /* __GNUC__ */

#elif defined(STLSOFT_COMPILER_IS_INTEL)
/* Intel C++ */
# if (__INTEL_COMPILER < 600)
#  error Versions of Intel C++ prior to 6.0 are not supported by the COMSTL libraries
# endif /* __INTEL_COMPILER */

#elif defined(STLSOFT_COMPILER_IS_MWERKS)
/* Metrowerks C++ */
# if (__MWERKS__ & 0xFF00) < 0x2400
#  error Versions of Metrowerks CodeWarrior C++ prior to 7.0 are not supported by the COMSTL libraries
# endif /* __MWERKS__ */

#elif defined(STLSOFT_COMPILER_IS_WATCOM)
/* Watcom C/C++ */
# if (__WATCOMC__ < 1230)
#  error Versions of Watcom C/C++ prior to 12.3 (Open Watcom 1.3) are not supported by the COMSTL libraries
# endif /* __MWERKS__ */

#elif defined(STLSOFT_COMPILER_IS_MSVC)
/* Visual C++ */
# if _MSC_VER < 1020
#  error Versions of Visual C++ prior to 4.2 are not supported by the COMSTL libraries
# endif /* _MSC_VER */

#else /* ? compiler */
/* No recognised compiler */
# ifdef _STLSOFT_FORCE_ANY_COMPILER
#  define _COMSTL_COMPILER_IS_UNKNOWN
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Compiler is unknown to COMSTL")
#  endif /* STLSOFT_COMPILE_VERBOSE */
# else /* ? _STLSOFT_FORCE_ANY_COMPILER */
#  error Currently only Borland C++, Comeau, Digital Mars C/C++, Intel C/C++, Metrowerks CodeWarrior C/C++ and Visual C++ compilers are supported by the COMSTL libraries
# endif /* _STLSOFT_FORCE_ANY_COMPILER */
#endif /* compiler tag */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro comstl_assert provides standard debug-mode assert functionality.
 */

/** \brief Defines a runtime assertion
 *
 * \ingroup group__project__comstl__assertion_macros
 *
 * \param expr Must be non-zero, or an assertion will be fired
 */
#define COMSTL_ASSERT(expr)                 STLSOFT_ASSERT(expr)

/** \brief Defines a runtime assertion, with message
 *
 * \ingroup group__project__comstl__assertion_macros
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define COMSTL_MESSAGE_ASSERT(msg, expr)    STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \brief Defines a compile-time assertion
 *
 * \ingroup group__project__comstl__assertion_macros
 *
 * \param expr Must be non-zero, or compilation will fail
 */
#define COMSTL_STATIC_ASSERT(expr)          STLSOFT_STATIC_ASSERT(expr)



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define comstl_assert(expr)                COMSTL_ASSERT(expr)
# define comstl_message_assert(msg, expr)   COMSTL_MESSAGE_ASSERT(msg, expr)
# define comstl_static_assert(expr)         COMSTL_STATIC_ASSERT(expr)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler language feature support
 */

/* DCOM support
 *
 * Since some COM APIs are only available, at compile time, when DCOM is
 * explicitly discriminated (for Windows this is via _WIN32_WINNT >= 0x0400 or
 * presence of _WIN32_DCOM), this must be determined.
 *
 * Override and always support DCOM by defining _COMSTL_DCOM
 *
 * Prevent DCOM support by defining _COMSTL_NO_DCOM
 *
 */

#ifdef __COMSTL_CF_DCOM_SUPPORT
#undef __COMSTL_CF_DCOM_SUPPORT
#endif /* __COMSTL_CF_DCOM_SUPPORT */

#if !defined(_COMSTL_NO_DCOM) && \
    (   (   defined(_WIN32_WINNT ) && \
            (_WIN32_WINNT >= 0x0400)) || \
        defined(_WIN32_DCOM) || \
        defined(_COMSTL_DCOM))
# define __COMSTL_CF_DCOM_SUPPORT
#endif /* _WIN32_WINNT >= 0x0400 || _WIN32_DCOM */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The COMSTL components are contained within the comstl namespace. This is
 * usually an alias for stlsoft::comstl_project.
 *
 * When compilers support namespaces they are defined by default. They can be
 * undefined using a cascasing system, as follows:
 *
 * If _STLSOFT_NO_NAMESPACES is defined, then _COMSTL_NO_NAMESPACES is defined.
 *
 * If _COMSTL_NO_NAMESPACES is defined, then _COMSTL_NO_NAMESPACE is defined.
 *
 * If _COMSTL_NO_NAMESPACE is defined, then the COMSTL constructs are defined
 * in the global scope.
 *
 * If _STLSOFT_NO_NAMESPACES, _COMSTL_NO_NAMESPACES and _COMSTL_NO_NAMESPACE are
 * all undefined but the symbol _STLSOFT_NO_NAMESPACE is defined (whence the
 * namespace stlsoft does not exist), then the COMSTL constructs are defined
 * within the comstl namespace. The definition matrix is as follows:
 *
 * _STLSOFT_NO_NAMESPACE    _COMSTL_NO_NAMESPACE    comstl definition
 * ---------------------    --------------------    -----------------
 *  not defined              not defined             = stlsoft::comstl_project
 *  not defined              defined                 not defined
 *  defined                  not defined             comstl
 *  defined                  defined                 not defined
 *
 *
 *
 * The macro comstl_ns_qual() macro can be used to refer to elements in the
 * COMSTL libraries irrespective of whether they are in the
 * stlsoft::comstl_project (or comstl) namespace or in the global namespace.
 *
 * Furthermore, some compilers do not support the standard library in the std
 * namespace, so the comstl_ns_qual_std() macro can be used to refer to elements
 * in the COMSTL libraries irrespective of whether they are in the std namespace
 * or in the global namespace.
 */

/* No STLSoft namespaces means no COMSTL namespaces */
#ifdef _STLSOFT_NO_NAMESPACES
# define _COMSTL_NO_NAMESPACES
#endif /* _STLSOFT_NO_NAMESPACES */

/* No COMSTL namespaces means no comstl namespace */
#ifdef _COMSTL_NO_NAMESPACES
# define _COMSTL_NO_NAMESPACE
#endif /* _COMSTL_NO_NAMESPACES */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
/** \brief The <code class="namespace">comstl</code> namespace contains all components
 *   in the \ref group__project__comstl "COMSTL" project.
 *
 * By default, the <code>comstl</code> namespace is actually an alias for
 * the namespace <code>stlsoft::comstl_project</code>, which is where all
 * the \ref group__project__comstl "COMSTL" components actually reside. This
 * measure allows all components within the main the
 * \ref group__project__stlsoft "STLSoft" project (which are defined within
 * the <code>stlsoft</code> namespace) to be visible to all components
 * "within" the <code>comstl</code> namespace. (Otherwise, there would be a
 * whole lot of onerous qualification throughout the code of all
 * \ref group__projects "sub-projects".)
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_COMSTL_NO_NAMESPACE</code> are defined, all
 * \ref group__project__comstl "COMSTL" components will be defined in the
 * global namespace. Conversely, if the <code>_STLSOFT_NO_NAMESPACE</code>
 * symbol (not to be confused with the
 * <code>_STLSOFT_NO_NAMESPACES</code> symbol!) is defined - meaning that
 * all \ref group__project__stlsoft "main project" components are to be
 * defined in the global namespace, and <code>_COMSTL_NO_NAMESPACE</code>
 * is <b>not</b> defined, then all \ref group__project__comstl "COMSTL"
 * components will be defined within a bona fide <code>comstl</code>
 * namespace.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#else
stlsoft_ns_using(move_lhs_from_rhs)
#endif /* !_COMSTL_NO_NAMESPACE */

/** \def comstl_ns_qual(x)
 * Qualifies with <b>comstl::</b> if COMSTL is using namespaces or, if not, does not qualify
 */

/** \def comstl_ns_using(x)
 * Declares a using directive (with respect to <b>comstl</b>) if COMSTL is using namespaces or, if not, does nothing
 */

#ifndef _COMSTL_NO_NAMESPACE
# define comstl_ns_qual(x)          ::comstl::x
# define comstl_ns_using(x)         using ::comstl::x;
#else /* ? _COMSTL_NO_NAMESPACE */
# define comstl_ns_qual(x)          x
# define comstl_ns_using(x)
#endif /* !_COMSTL_NO_NAMESPACE */

/** \def comstl_ns_qual_std(x)
 * Qualifies with <b>std::</b> if COMSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def comstl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if COMSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define comstl_ns_qual_std(x)      ::std::x
# define comstl_ns_using_std(x)     using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define comstl_ns_qual_std(x)      x
# define comstl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Language agnostic macros.
 */

/** \def COMSTL_ITF_CALL(p)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>p->lpVtbl</b> for C compilation, and to <b>p</b> in C++
 *
 * \see COMSTL_ITF_THIS, COMSTL_ITF_THIS0, COMSTL_IID_2_REF, COMSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define COMSTL_ITF_CALL(p)         (p)
#else /* ? __cplusplus */
# define COMSTL_ITF_CALL(p)         (p)->lpVtbl
#endif /* __cplusplus */

/** \def COMSTL_ITF_THIS(p)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>p,</b> for C compilation, and to nothing in C++
 *
 * \see COMSTL_ITF_CALL, COMSTL_ITF_THIS0, COMSTL_IID_2_REF, COMSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define COMSTL_ITF_THIS(p)
#else /* ? __cplusplus */
# define COMSTL_ITF_THIS(p)         (p),
#endif /* __cplusplus */

/** \def COMSTL_ITF_THIS0(p)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>p</b> for C compilation, and to nothing in C++
 *
 * \see COMSTL_ITF_CALL, COMSTL_ITF_THIS, COMSTL_IID_2_REF, COMSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define COMSTL_ITF_THIS0(p)
#else /* ? __cplusplus */
# define COMSTL_ITF_THIS0(p)        (p)
#endif /* __cplusplus */

/** \def COMSTL_IID_2_REF(iid)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>&iid</b> for C compilation, and to <b>iid</b> in C++
 *
 * \see COMSTL_ITF_CALL, COMSTL_ITF_THIS, COMSTL_ITF_THIS0, COMSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define COMSTL_IID_2_REF(iid)        (iid)
#else /* ? __cplusplus */
# define COMSTL_IID_2_REF(iid)        (&(iid))
#endif /* __cplusplus */

/** \def COMSTL_REF_2_PTR(iid)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>iid</b> for C compilation, and to <b>&iid</b> in C++
 *
 * \see COMSTL_ITF_CALL, COMSTL_ITF_THIS, COMSTL_ITF_THIS0, COMSTL_PTR_2_REF
 */

#if defined(__cplusplus)
# define COMSTL_REF_2_PTR(iid)        (&(iid))
#else /* ? __cplusplus */
# define COMSTL_REF_2_PTR(iid)        (iid)
#endif /* __cplusplus */

/** \def COMSTL_PTR_2_REF(iid)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief Resolves to <b>piid</b> for C compilation, and to <b>*piid</b> in C++
 *
 * \see COMSTL_ITF_CALL, COMSTL_ITF_THIS, COMSTL_ITF_THIS0, COMSTL_REF_2_PTR
 */

#if defined(__cplusplus)
# define COMSTL_PTR_2_REF(piid)       (*(piid))
#else /* ? __cplusplus */
# define COMSTL_PTR_2_REF(piid)       (piid)
#endif /* __cplusplus */


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifdef COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_
#  undef COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_
# endif /* COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#if !defined(COMSTL_ASSUME_VARIANT_UNION_FORCE_ARMS_HAVE_NAMES) && \
    defined(_FORCENAMELESSUNION) && \
    !defined(NONAMELESSUNION)
# define COMSTL_ASSUME_VARIANT_UNION_FORCE_ARMS_HAVE_NAMES
#endif /* !COMSTL_ASSUME_VARIANT_UNION_FORCE_ARMS_HAVE_NAMES) && _FORCENAMELESSUNION */


#if defined(COMSTL_ASSUME_VARIANT_UNION_FORCE_ARMS_HAVE_NAMES)
# define COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_
#elif defined(STLSOFT_COMPILER_IS_GCC)
   /* GCC has different definitions to the other compilers, so have to treat
    * differently
    */
# if defined(NONAMELESSUNION)
#  define COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_
# endif /* NONAMELESSUNION */
#else /* ? compiler */
   /* Other compilers use the MS headers, which test against __STDC__, 
    * _FORCENAMELESSUNION and NONAMELESSUNION
    */
# if (  __STDC__ && \
        !defined(_FORCENAMELESSUNION)) || \
     defined(NONAMELESSUNION)
#  define COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_
# endif /* (  __STDC__ && !_FORCENAMELESSUNION) || NONAMELESSUNION */
#endif /* compiler */

/** \def COMSTL_ACCESS_VARIANT_MEM_BYPTR(pvar, mem)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief 
 */
# if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
     !defined(COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_)
# define COMSTL_ACCESS_VARIANT_MEM_BYPTR(pvar, mem)     (pvar)->mem
# else /* ? COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */
#  define COMSTL_ACCESS_VARIANT_MEM_BYPTR(pvar, mem)    (pvar)->__VARIANT_NAME_1.__VARIANT_NAME_2.__VARIANT_NAME_3.mem
# endif /* COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */

/** \def COMSTL_ACCESS_VARIANT_MEM_BYREF(var, mem)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * Used to provide access to the \c mem member of a \c VARIANT structure in
 * a language-independent manner
 *
 * Usage:
<pre>
  VARIANT v;

  COMSTL_ACCESS_VARIANT_MEM_BYREF(v, lVal) = 10;
</pre>
 */
#define COMSTL_ACCESS_VARIANT_MEM_BYREF(var, mem)       COMSTL_ACCESS_VARIANT_MEM_BYPTR(&(var), mem)


/** \def COMSTL_ACCESS_VARIANT_vt_BYPTR(pvar)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief 
 */

# if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
     !defined(COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_)
#  define COMSTL_ACCESS_VARIANT_vt_BYPTR(pvar)          (pvar)->vt
# else /* ? COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */
#  define COMSTL_ACCESS_VARIANT_vt_BYPTR(pvar)          (pvar)->__VARIANT_NAME_1.__VARIANT_NAME_2.vt
# endif /* COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */

/** \def COMSTL_ACCESS_VARIANT_vt_BYREF(var)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * Used to provide access to the \c vt member of a \c VARIANT structure in
 * a language-independent manner
 *
 * Usage:
<pre>
  VARIANT v;

  COMSTL_ACCESS_VARIANT_vt_BYREF(v) = VT_I4;
</pre>
 */
#define COMSTL_ACCESS_VARIANT_vt_BYREF(var)             COMSTL_ACCESS_VARIANT_vt_BYPTR(&(var))


/** \def COMSTL_ACCESS_VARIANT_decVal_BYPTR(pvar)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief 
 */
# if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
     !defined(COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_)
# define COMSTL_ACCESS_VARIANT_decVal_BYPTR(pvar)       (pvar)->decVal
# else /* ? COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */
#  define COMSTL_ACCESS_VARIANT_decVal_BYPTR(pvar)      (pvar)->__VARIANT_NAME_1.decVal
# endif /* COMSTL_VARIANT_UNION_ARMS_HAVE_NAMES_ */

/** \def COMSTL_ACCESS_VARIANT_decVal_BYREF(var, mem)
 *
 * \ingroup group__project__comstl__language_agnostic_macros
 *
 * \brief 
 */
#define COMSTL_ACCESS_VARIANT_decVal_BYREF(var)         COMSTL_ACCESS_VARIANT_decVal_BYPTR(&(var))

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The COMSTL uses a number of typedefs to aid in compiler-independence in the
 * libraries' main code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

typedef stlsoft_ns_qual(ss_char_a_t)        cs_char_a_t;    /*!< Ansi char type             */
typedef stlsoft_ns_qual(ss_char_w_t)        cs_char_w_t;    /*!< Unicode char type          */
typedef OLECHAR                             cs_char_o_t;    /*!< COM char type              */
typedef stlsoft_ns_qual(ss_sint8_t)         cs_sint8_t;     /*!< 8-bit signed integer       */
typedef stlsoft_ns_qual(ss_uint8_t)         cs_uint8_t;     /*!< 8-bit unsigned integer     */
typedef stlsoft_ns_qual(ss_int16_t)         cs_int16_t;     /*!< 16-bit integer             */
typedef stlsoft_ns_qual(ss_sint16_t)        cs_sint16_t;    /*!< 16-bit signed integer      */
typedef stlsoft_ns_qual(ss_uint16_t)        cs_uint16_t;    /*!< 16-bit unsigned integer    */
typedef stlsoft_ns_qual(ss_int32_t)         cs_int32_t;     /*!< 32-bit integer             */
typedef stlsoft_ns_qual(ss_sint32_t)        cs_sint32_t;    /*!< 32-bit signed integer      */
typedef stlsoft_ns_qual(ss_uint32_t)        cs_uint32_t;    /*!< 32-bit unsigned integer    */
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef stlsoft_ns_qual(ss_int64_t)         cs_int64_t;     /*!< 64-bit integer             */
typedef stlsoft_ns_qual(ss_sint64_t)        cs_sint64_t;    /*!< 64-bit signed integer      */
typedef stlsoft_ns_qual(ss_uint64_t)        cs_uint64_t;    /*!< 64-bit unsigned integer    */
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef stlsoft_ns_qual(ss_int_t)           cs_int_t;       /*!< integer                    */
typedef stlsoft_ns_qual(ss_sint_t)          cs_sint_t;      /*!< signed integer             */
typedef stlsoft_ns_qual(ss_uint_t)          cs_uint_t;      /*!< unsigned integer           */
typedef stlsoft_ns_qual(ss_long_t)          cs_long_t;      /*!< long                       */
#if defined(__cplusplus)
typedef stlsoft_ns_qual(ss_bool_t)          cs_bool_t;      /*!< bool                       */
#endif /* __cplusplus */
typedef DWORD                               cs_dword_t;     /*!< dword                      */
typedef stlsoft_ns_qual(ss_size_t)          cs_size_t;      /*!< size                       */
typedef stlsoft_ns_qual(ss_ptrdiff_t)       cs_ptrdiff_t;   /*!< ptr diff                   */
typedef stlsoft_ns_qual(ss_streampos_t)     cs_streampos_t; /*!< streampos                  */
typedef stlsoft_ns_qual(ss_streamoff_t)     cs_streamoff_t; /*!< streamoff                  */
#ifdef _WIN64
typedef stlsoft_ns_qual(ss_sint64_t)        cs_sptrint_t;
typedef stlsoft_ns_qual(ss_uint64_t)        cs_uptrint_t;
#else /* ? _WIN64 */
typedef stlsoft_ns_qual(ss_sint32_t)        cs_sptrint_t;
typedef stlsoft_ns_qual(ss_uint32_t)        cs_uptrint_t;
#endif /* _WIN64 */

#ifndef _STLSOFT_NO_NAMESPACE
typedef cs_char_a_t         char_a_t;           /*!< Ansi char type             */
typedef cs_char_w_t         char_w_t;           /*!< Unicode char type          */
typedef cs_char_o_t         char_o_t;           /*!< COM char type              */
typedef cs_sint8_t          sint8_t;            /*!< 8-bit signed integer       */
typedef cs_uint8_t          uint8_t;            /*!< 8-bit unsigned integer     */
typedef cs_int16_t          int16_t;            /*!< 16-bit integer             */
typedef cs_sint16_t         sint16_t;           /*!< 16-bit signed integer      */
typedef cs_uint16_t         uint16_t;           /*!< 16-bit unsigned integer    */
typedef cs_int32_t          int32_t;            /*!< 32-bit integer             */
typedef cs_sint32_t         sint32_t;           /*!< 32-bit signed integer      */
typedef cs_uint32_t         uint32_t;           /*!< 32-bit unsigned integer    */
# ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef cs_int64_t          int64_t;            /*!< 64-bit integer             */
typedef cs_sint64_t         sint64_t;           /*!< 64-bit signed integer      */
typedef cs_uint64_t         uint64_t;           /*!< 64-bit unsigned integer    */
# endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef cs_int_t            int_t;              /*!< integer                    */
typedef cs_sint_t           sint_t;             /*!< signed integer             */
typedef cs_uint_t           uint_t;             /*!< unsigned integer           */
typedef cs_long_t           long_t;             /*!< long integer               */
#if defined(__cplusplus)
typedef cs_bool_t           bool_t;             /*!< bool                       */
#endif /* __cplusplus */
# if !defined(STLSOFT_COMPILER_IS_DMC)
typedef cs_dword_t          dword_t;            /*!< dword                      */
typedef cs_streampos_t      streampos_t;        /*!< streampos                  */
typedef cs_streamoff_t      streamoff_t;        /*!< streamoff                  */
# endif /* compiler */
typedef cs_sptrint_t        sptrint_t;
typedef cs_uptrint_t        uptrint_t;
#endif /* !_STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Values
 *
 * Since the boolean type may not be supported natively on all compilers, the
 * values of true and false may also not be provided. Hence the values of
 * cs_true_v and cs_false_v are defined, and are used in all code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define cs_true_v       ss_true_v
#define cs_false_v      ss_false_v

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Constants
 */

#ifdef __cplusplus
const cs_size_t COMSTL_CCH_GUID     =   38;
#else /* ? __cplusplus */
# define COMSTL_CCH_GUID            (38u)
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Code modification macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/* Exception signatures. */
#define comstl_throw_0()                                stlsoft_throw_0()
#define comstl_throw_1(x1)                              stlsoft_throw_1(x1)
#define comstl_throw_2(x1, x2)                          stlsoft_throw_2(x1, x2)
#define comstl_throw_3(x1, x2, x3)                      stlsoft_throw_3(x1, x2, x3)
#define comstl_throw_4(x1, x2, x3, x4)                  stlsoft_throw_4(x1, x2, x3, x4)
#define comstl_throw_5(x1, x2, x3, x4, x5)              stlsoft_throw_5(x1, x2, x3, x4, x5)
#define comstl_throw_6(x1, x2, x3, x4, x5, x6)          stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
#define comstl_throw_7(x1, x2, x3, x4, x5, x6, x7)      stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
#define comstl_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)  stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_NUM_ELEMENTS(ar)                        STLSOFT_NUM_ELEMENTS(ar)
# define comstl_num_elements(ar)                        COMSTL_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [DEPRECATED] Destroys the given instance \c p of the given type (\c t and \c type)
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 */
#define comstl_destroy_instance(t, type, p)             STLSOFT_DESTROY_INSTANCE(t, type, p)

/** \brief [DEPRECATED] Generates an opaque type with the name \c _htype
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 */
#define comstl_gen_opaque(htype)                        STLSOFT_GEN_OPAQUE(htype)

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace comstl */
# else
} /* namespace comstl_project */
} /* namespace stlsoft */
namespace comstl = ::stlsoft::comstl_project;
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* COMSTL_INCL_COMSTL_H_COMSTL */

/* ///////////////////////////// end of file //////////////////////////// */
