/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/atlstl.hpp
 *
 * Purpose:     Root header for the ATLSTL libraries. Performs various compiler
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


#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
#define ATLSTL_INCL_ATLSTL_HPP_ATLSTL
#define ATLSTL_INCL_H_ATLSTL    /* Define the old header include guard */

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_HPP_ATLSTL_MAJOR     3
# define ATLSTL_VER_ATLSTL_HPP_ATLSTL_MINOR     5
# define ATLSTL_VER_ATLSTL_HPP_ATLSTL_REVISION  2
# define ATLSTL_VER_ATLSTL_HPP_ATLSTL_EDIT      107
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file atlstl/atlstl.hpp
 *
 * \brief [C++ only; requires ATL library] The root header for the \ref group__project__atlstl "ATLSTL" project.
 */

/* /////////////////////////////////////////////////////////////////////////
 * ATLSTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * The major version is denoted by the _ATLSTL_VER_MAJOR preprocessor symbol.
 * A changes to the major version component implies that a dramatic change has
 * occurred in the libraries, such that considerable changes to source dependent
 * on previous versions would need to be effected.
 *
 * The minor version is denoted by the _ATLSTL_VER_MINOR preprocessor symbol.
 * Changes to the minor version component imply that a significant change has
 * occurred to the libraries, either in the addition of new functionality or in
 * the destructive change to one or more components such that recomplilation and
 * code change may be necessitated.
 *
 * The revision version is denoted by the _ATLSTL_VER_REVISION preprocessor
 * symbol. Changes to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the changes.
 *
 * In addition to the individual version symbols - _ATLSTL_VER_MAJOR,
 * _ATLSTL_VER_MINOR and _ATLSTL_VER_REVISION - a composite symbol _ATLSTL_VER
 * is defined, where the upper 8 bits are 0, bits 16-23 represent the major
 * component,  bits 8-15 represent the minor component, and bits 0-7 represent
 * the revision component.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies _ATLSTL_VER_1_0_1.
 *
 * Thus the symbol _ATLSTL_VER may be compared meaningfully with a specific
 * version symbol, e.g. #if _ATLSTL_VER >= _ATLSTL_VER_1_0_1
 */

/** \def _ATLSTL_VER_MAJOR
 * The major version number of ATLSTL
 */

/** \def _ATLSTL_VER_MINOR
 * The minor version number of ATLSTL
 */

/** \def _ATLSTL_VER_REVISION
 * The revision version number of ATLSTL
 */

/** \def _ATLSTL_VER
 * The current composite version number of ATLSTL
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _ATLSTL_VER_1_0_1      0x00010001  /*!< Version 1.0.1 */
# define _ATLSTL_VER_1_1_1      0x00010101  /*!< Version 1.1.1 */
# define _ATLSTL_VER_1_2_1      0x00010201  /*!< Version 1.2.1 */
# define _ATLSTL_VER_1_2_2      0x00010202  /*!< Version 1.2.2 */
# define _ATLSTL_VER_1_2_3      0x00010203  /*!< Version 1.2.3 */
# define _ATLSTL_VER_1_2_4      0x00010204  /*!< Version 1.2.4 */
# define _ATLSTL_VER_1_2_5      0x00010205  /*!< Version 1.2.5 */
# define _ATLSTL_VER_1_2_6      0x00010206  /*!< Version 1.2.6 */
# define _ATLSTL_VER_1_2_7      0x00010207  /*!< Version 1.2.7 */
# define _ATLSTL_VER_1_2_8      0x00010208  /*!< Version 1.2.8 */
# define _ATLSTL_VER_1_3_1      0x00010301  /*!< Version 1.3.1 */
# define _ATLSTL_VER_1_4_1      0x00010401  /*!< Version 1.4.1 */
# define _ATLSTL_VER_1_4_2      0x00010402  /*!< Version 1.4.2 */
# define _ATLSTL_VER_1_4_3      0x00010403  /*!< Version 1.4.3 */
# define _ATLSTL_VER_1_4_4      0x00010404  /*!< Version 1.4.4 */
# define _ATLSTL_VER_1_5_1      0x00010501  /*!< Version 1.5.1 */
# define _ATLSTL_VER_1_5_2      0x00010502  /*!< Version 1.5.2 */
# define _ATLSTL_VER_1_6_1      0x00010601  /*!< Version 1.6.1 (with STLSoft 1.9.1) */
# define _ATLSTL_VER_1_6_2      0x00010602  /*!< Version 1.6.2 (with STLSoft 1.9.25) */
# define _ATLSTL_VER_1_6_3      0x010603ff  /*!< Version 1.6.3 (with STLSoft 1.9.113) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _ATLSTL_VER_MAJOR       1
#define _ATLSTL_VER_MINOR       6
#define _ATLSTL_VER_REVISION    3
#define _ATLSTL_VER             _ATLSTL_VER_1_6_3

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
# pragma warn -8013 /* Suppresses "Possible use of 'f' before definition in '. . .'" */
# pragma warn -8084 /* Suppresses "Suggest parentheses to clarify precedence in function 'f()'" */
#endif /* compiler */

#ifndef STLSOFT_INCL_H_ATLBASE
# define STLSOFT_INCL_H_ATLBASE
# include <atlbase.h>    // ATL base header
#endif /* !STLSOFT_INCL_H_ATLBASE */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
# pragma warn .8013 /* Suppresses "Possible use of 'f' before definition in '. . .'" */
# pragma warn .8084 /* Suppresses "Suggest parentheses to clarify precedence in function 'f()'" */
#endif /* compiler */

#ifdef STLSOFT_UNITTEST
extern CComModule _Module;

# include <stdio.h>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if !defined(_STLSOFT_VER) || \
    _STLSOFT_VER < 0x010971ff
# error This version of the ATLSTL libraries requires STLSoft version 1.9.113, or later
#endif /* _STLSOFT_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the ATLSTL libraries are
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
#  error Versions of Borland C++ prior to 5.5 are not supported by the ATLSTL libraries
# endif /* __BORLANDC__ */

#elif defined(STLSOFT_COMPILER_IS_DMC)
/* Digital Mars C/C++ */
# if __DMC__ < 0x0826
#  error Versions of Digital Mars C/C++ prior to 8.26 are not supported by the ATLSTL libraries
# endif /* __DMC__ */

#elif defined(STLSOFT_COMPILER_IS_INTEL)
/* Intel C++ */
# if (__INTEL_COMPILER < 600)
#  error Versions of Intel C++ prior to 6.0 are not supported by the ATLSTL libraries
# endif /* __INTEL_COMPILER */

#elif defined(STLSOFT_COMPILER_IS_MWERKS)
/* Metrowerks C++ */
# if (__MWERKS__ & 0xFF00) < 0x2400
#  error Versions of Metrowerks CodeWarrior C++ prior to 7.0 are not supported by the ATLSTL libraries
# endif /* __MWERKS__ */

#elif defined(STLSOFT_COMPILER_IS_MSVC)
/* Visual C++ */
# if _MSC_VER < 1100
#  error Versions of Visual C++ prior to 5.0 are not supported by the ATLSTL libraries
# endif /* _MSC_VER */

#else /* ? compiler */
/* No recognised compiler */
# ifdef _STLSOFT_FORCE_ANY_COMPILER
#  define _ATLSTL_COMPILER_IS_UNKNOWN
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Compiler is unknown to ATLSTL")
#  endif /* STLSOFT_COMPILE_VERBOSE */
# else /* ? _STLSOFT_FORCE_ANY_COMPILER */
#  error Currently only Borland C++, Digital Mars C/C++, Intel C/C++, Metrowerks CodeWarrior and Visual C++ compilers are supported by the ATLSTL libraries
# endif /* _STLSOFT_FORCE_ANY_COMPILER */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro atlstl_assert provides standard debug-mode assert functionality.
 */

/** \brief Defines a runtime assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 */
#define ATLSTL_ASSERT(expr)                 STLSOFT_ASSERT(expr)

/** \brief Defines a runtime assertion, with message
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define ATLSTL_MESSAGE_ASSERT(msg, expr)    STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \brief Defines a compile-time assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or compilation will fail
 */
#define ATLSTL_STATIC_ASSERT(expr)          STLSOFT_STATIC_ASSERT(expr)



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define atlstl_assert(expr)                ATLSTL_ASSERT(expr)
# define atlstl_message_assert(msg, expr)   ATLSTL_MESSAGE_ASSERT(msg, expr)
# define atlstl_static_assert(expr)         ATLSTL_STATIC_ASSERT(expr)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The ATLSTL components are contained within the atlstl namespace. This is
 * usually an alias for stlsoft::atlstl_project,
 *
 * When compilers support namespaces they are defined by default. They can be
 * undefined using a cascasing system, as follows:
 *
 * If _STLSOFT_NO_NAMESPACES is defined, then _ATLSTL_NO_NAMESPACES is defined.
 *
 * If _ATLSTL_NO_NAMESPACES is defined, then _ATLSTL_NO_NAMESPACE is defined.
 *
 * If _ATLSTL_NO_NAMESPACE is defined, then the ATLSTL constructs are defined
 * in the global scope.
 *
 * If _STLSOFT_NO_NAMESPACES, _ATLSTL_NO_NAMESPACES and _ATLSTL_NO_NAMESPACE are
 * all undefined but the symbol _STLSOFT_NO_NAMESPACE is defined (whence the
 * namespace stlsoft does not exist), then the ATLSTL constructs are defined
 * within the atlstl namespace. The definition matrix is as follows:
 *
 * _STLSOFT_NO_NAMESPACE    _ATLSTL_NO_NAMESPACE    atlstl definition
 * ---------------------    --------------------    -----------------
 *  not defined              not defined             = stlsoft::atlstl_project
 *  not defined              defined                 not defined
 *  defined                  not defined             atlstl
 *  defined                  defined                 not defined
 *
 *
 *
 * The macro atlstl_ns_qual() macro can be used to refer to elements in the
 * ATLSTL libraries irrespective of whether they are in the
 * stlsoft::atlstl_project (or atlstl) namespace or in the global namespace.
 *
 * Furthermore, some compilers do not support the standard library in the std
 * namespace, so the atlstl_ns_qual_std() macro can be used to refer to elements
 * in the ATLSTL libraries irrespective of whether they are in the std namespace
 * or in the global namespace.
 *
 *
 * The ATL libraries themselves may or may not be defined within the ATL
 * namespace, so the atlstl_ns_qual_stl() macro can be used to refer to
 * elements in the ATLSTL libraries irrespective of whether they are in the std
 * namespace or in the global namespace.
 */

/* No STLSoft namespaces means no ATLSTL namespaces */
#ifdef _STLSOFT_NO_NAMESPACES
# define _ATLSTL_NO_NAMESPACES
#endif /* _STLSOFT_NO_NAMESPACES */

/* No ATLSTL namespaces means no atlstl namespace */
#ifdef _ATLSTL_NO_NAMESPACES
# define _ATLSTL_NO_NAMESPACE
#endif /* _ATLSTL_NO_NAMESPACES */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::atlstl */
/** \brief The <code class="namespace">atlstl</code> namespace contains all components
 *   in the \ref group__project__atlstl "ATLSTL" project.
 *
 * By default, the <code>atlstl</code> namespace is actually an alias for
 * the namespace <code>stlsoft::atlstl_project</code>, which is where all
 * the \ref group__project__atlstl "ATLSTL" components actually reside. This
 * measure allows all components within the main the
 * \ref group__project__stlsoft "STLSoft" project (which are defined within
 * the <code>stlsoft</code> namespace) to be visible to all components
 * "within" the <code>atlstl</code> namespace. (Otherwise, there would be a
 * whole lot of onerous qualification throughout the code of all
 * \ref group__projects "sub-projects".)
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_ATLSTL_NO_NAMESPACE</code> are defined, all
 * \ref group__project__atlstl "ATLSTL" components will be defined in the
 * global namespace. Conversely, if the <code>_STLSOFT_NO_NAMESPACE</code>
 * symbol (not to be confused with the
 * <code>_STLSOFT_NO_NAMESPACES</code> symbol!) is defined - meaning that
 * all \ref group__project__stlsoft "main project" components are to be
 * defined in the global namespace, and <code>_ATLSTL_NO_NAMESPACE</code>
 * is <b>not</b> defined, then all \ref group__project__atlstl "ATLSTL"
 * components will be defined within a bona fide <code>atlstl</code>
 * namespace.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace atlstl
{
# else
/* Define stlsoft::atlstl_project */

namespace stlsoft
{

namespace atlstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#else
stlsoft_ns_using(move_lhs_from_rhs)
#endif /* !_ATLSTL_NO_NAMESPACE */

/** \def atlstl_ns_qual(x)
 * Qualifies with <b>atlstl::</b> if ATLSTL is using namespaces or, if not, does not qualify
 */

/** \def atlstl_ns_using(x)
 * Declares a using directive (with respect to <b>atlstl</b>) if ATLSTL is using namespaces or, if not, does nothing
 */

#ifndef _ATLSTL_NO_NAMESPACE
# define atlstl_ns_qual(x)          ::atlstl::x
# define atlstl_ns_using(x)         using ::atlstl::x;
#else /* ? _ATLSTL_NO_NAMESPACE */
# define atlstl_ns_qual(x)          x
# define atlstl_ns_using(x)
#endif /* !_ATLSTL_NO_NAMESPACE */

/** \def atlstl_ns_qual_std(x)
 * Qualifies with <b>std::</b> if ATLSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def atlstl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if ATLSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define atlstl_ns_qual_std(x)      ::std::x
# define atlstl_ns_using_std(x)     using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define atlstl_ns_qual_std(x)      x
# define atlstl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/** \def atlstl_ns_qual_atl(x)
 * Qualifies with <b>ATL::</b> if ATLSTL is being translated in the context of ATL being within the <b>ATL</b> namespace or, if not, does not qualify
 */

/** \def atlstl_ns_using_atl(x)
 * Declares a using directive (with respect to <b>ATL</b>) if ATLSTL is being translated in the context of ATL being within the <b>ATL</b> namespace or, if not, does nothing
 */

#ifndef ATL_NO_NAMESPACE
# define atlstl_ns_qual_atl(x)       ::ATL::x
# define atlstl_ns_using_atl(x)      using ::ATL::x;
#else /* ? ATL_NO_NAMESPACE */
# define atlstl_ns_qual_atl(x)       x
# define atlstl_ns_using_atl(x)
#endif /* !ATL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The ATLSTL uses a number of typedefs to aid in compiler-independence in the
 * libraries' main code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

typedef stlsoft_ns_qual(ss_char_a_t)        as_char_a_t;    //!< Ansi char type
typedef stlsoft_ns_qual(ss_char_w_t)        as_char_w_t;    //!< Unicode char type
typedef stlsoft_ns_qual(ss_sint8_t)         as_sint8_t;     //!< 8-bit signed integer
typedef stlsoft_ns_qual(ss_uint8_t)         as_uint8_t;     //!< 8-bit unsigned integer
typedef stlsoft_ns_qual(ss_int16_t)         as_int16_t;     //!< 16-bit integer
typedef stlsoft_ns_qual(ss_sint16_t)        as_sint16_t;    //!< 16-bit signed integer
typedef stlsoft_ns_qual(ss_uint16_t)        as_uint16_t;    //!< 16-bit unsigned integer
typedef stlsoft_ns_qual(ss_int32_t)         as_int32_t;     //!< 32-bit integer
typedef stlsoft_ns_qual(ss_sint32_t)        as_sint32_t;    //!< 32-bit signed integer
typedef stlsoft_ns_qual(ss_uint32_t)        as_uint32_t;    //!< 32-bit unsigned integer
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef stlsoft_ns_qual(ss_int64_t)         as_int64_t;     //!< 64-bit integer
typedef stlsoft_ns_qual(ss_sint64_t)        as_sint64_t;    //!< 64-bit signed integer
typedef stlsoft_ns_qual(ss_uint64_t)        as_uint64_t;    //!< 64-bit unsigned integer
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef stlsoft_ns_qual(ss_int_t)           as_int_t;       //!< integer
typedef stlsoft_ns_qual(ss_sint_t)          as_sint_t;      //!< signed integer
typedef stlsoft_ns_qual(ss_uint_t)          as_uint_t;      //!< unsigned integer
typedef stlsoft_ns_qual(ss_long_t)          as_long_t;      //!< long
typedef stlsoft_ns_qual(ss_bool_t)          as_bool_t;      //!< bool
typedef DWORD                               as_dword_t;     //!< dword
typedef stlsoft_ns_qual(ss_size_t)          as_size_t;      //!< size
typedef stlsoft_ns_qual(ss_ptrdiff_t)       as_ptrdiff_t;   //!< ptr diff
typedef stlsoft_ns_qual(ss_streampos_t)     as_streampos_t; //!< streampos
typedef stlsoft_ns_qual(ss_streamoff_t)     as_streamoff_t; //!< streamoff

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Values
 *
 * Since the boolean type may not be supported natively on all compilers, the
 * values of true and false may also not be provided. Hence the values of
 * as_true_v and as_false_v are defined, and are used in all code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define as_true_v       ss_true_v
#define as_false_v      ss_false_v

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/* /////////////////////////////////////////////////////////////////////////
 * Code modification macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/* Exception signatures. */
#define atlstl_throw_0()                                stlsoft_throw_0()
#define atlstl_throw_1(x1)                              stlsoft_throw_1(x1)
#define atlstl_throw_2(x1, x2)                          stlsoft_throw_2(x1, x2)
#define atlstl_throw_3(x1, x2, x3)                      stlsoft_throw_3(x1, x2, x3)
#define atlstl_throw_4(x1, x2, x3, x4)                  stlsoft_throw_4(x1, x2, x3, x4)
#define atlstl_throw_5(x1, x2, x3, x4, x5)              stlsoft_throw_5(x1, x2, x3, x4, x5)
#define atlstl_throw_6(x1, x2, x3, x4, x5, x6)          stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
#define atlstl_throw_7(x1, x2, x3, x4, x5, x6, x7)      stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
#define atlstl_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)  stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_NUM_ELEMENTS(ar)                        STLSOFT_NUM_ELEMENTS(ar)
# define atlstl_num_elements(ar)                        ATLSTL_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [DEPRECATED] Destroys the given instance \c p of the given type (\c t and \c type)
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 */
#define atlstl_destroy_instance(t, type, p)             STLSOFT_DESTROY_INSTANCE(t, type, p)

/** \brief [DEPRECATED] Generates an opaque type with the name \c _htype
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 */
#define atlstl_gen_opaque(htype)                        STLSOFT_GEN_OPAQUE(htype)

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** This function is needed since there are ambiguities between COM headers and
 * ATL headers
 */
#if _ATL_VER < 0x0700
inline as_bool_t InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return ( ((as_int32_t*) &rguid1)[0] == ((as_int32_t*) &rguid2)[0] &&
            ((as_int32_t*) &rguid1)[1] == ((as_int32_t*) &rguid2)[1] &&
            ((as_int32_t*) &rguid1)[2] == ((as_int32_t*) &rguid2)[2] &&
            ((as_int32_t*) &rguid1)[3] == ((as_int32_t*) &rguid2)[3]);
}
#endif /* _ATL_VER < 0x0700 */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/atlstl_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace atlstl
# else
} // namespace atlstl_project
} // namespace stlsoft
namespace atlstl = ::stlsoft::atlstl_project;
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* ATLSTL_INCL_ATLSTL_HPP_ATLSTL */

/* ///////////////////////////// end of file //////////////////////////// */
