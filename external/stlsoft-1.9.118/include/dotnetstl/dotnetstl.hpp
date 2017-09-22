/* /////////////////////////////////////////////////////////////////////////
 * File:        dotnetstl/dotnetstl.hpp
 *
 * Purpose:     Root header for the .netSTL libraries. Performs various compiler
 *              and platform discriminations, and definitions of types.
 *
 * Created:     12th May 2003
 * Updated:     22nd November 2013
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2013, Matthew Wilson and Synesis Software
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


#ifndef DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL
#define DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL
#define DOTNETSTL_INCL_H_DOTNETSTL

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define DOTNETSTL_VER_DOTNETSTL_HPP_DOTNETSTL_MAJOR    3
# define DOTNETSTL_VER_DOTNETSTL_HPP_DOTNETSTL_MINOR    5
# define DOTNETSTL_VER_DOTNETSTL_HPP_DOTNETSTL_REVISION 2
# define DOTNETSTL_VER_DOTNETSTL_HPP_DOTNETSTL_EDIT     52
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file dotnetstl/dotnetstl.hpp
 *
 * \brief [C++ only; requires C++.NET compiler] The root header for the \ref group__project__dotnetstl ".netSTL" project.
 */

/* /////////////////////////////////////////////////////////////////////////
 * .netSTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * The major version is denoted by the DOTNETSTL_VER_MAJOR preprocessor symbol.
 * A changes to the major version component implies that a dramatic change has
 * occurred in the libraries, such that considerable changes to source dependent
 * on previous versions would need to be effected.
 *
 * The minor version is denoted by the DOTNETSTL_VER_MINOR preprocessor symbol.
 * Changes to the minor version component imply that a significant change has
 * occurred to the libraries, either in the addition of new functionality or in
 * the destructive change to one or more components such that recomplilation and
 * code change may be necessitated.
 *
 * The revision version is denoted by the DOTNETSTL_VER_REVISION preprocessor
 * symbol. Changes to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the changes.
 *
 * In addition to the individual version symbols - DOTNETSTL_VER_MAJOR,
 * DOTNETSTL_VER_MINOR and DOTNETSTL_VER_REVISION - a composite symbol DOTNETSTL_VER
 * is defined, where the upper 8 bits are 0, bits 16-23 represent the major
 * component,  bits 8-15 represent the minor component, and bits 0-7 represent
 * the revision component.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies DOTNETSTL_VER_1_0_1.
 *
 * Thus the symbol DOTNETSTL_VER may be compared meaningfully with a specific
 * version symbol, e.g. #if DOTNETSTL_VER >= DOTNETSTL_VER_1_0_1
 */

/** \def DOTNETSTL_VER_MAJOR
 * The major version number of .netSTL
 */

/** \def DOTNETSTL_VER_MINOR
 * The minor version number of .netSTL
 */

/** \def DOTNETSTL_VER_REVISION
 * The revision version number of .netSTL
 */

/** \def DOTNETSTL_VER
 * The current composite version number of .netSTL
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define DOTNETSTL_VER_1_0_1      0x00010001  /*!< Version 1.0.1 */
# define DOTNETSTL_VER_1_1_1      0x00010101  /*!< Version 1.1.1 */
# define DOTNETSTL_VER_1_1_2      0x00010102  /*!< Version 1.1.2 */
# define DOTNETSTL_VER_1_2_1      0x00010201  /*!< Version 1.2.1 */
# define DOTNETSTL_VER_1_2_2      0x00010202  /*!< Version 1.2.2 */
# define DOTNETSTL_VER_1_2_3      0x00010203  /*!< Version 1.2.3 */
# define DOTNETSTL_VER_1_3_1      0x00010301  /*!< Version 1.3.1 (with STLSoft 1.9.1) */
# define DOTNETSTL_VER_1_3_2      0x00010302  /*!< Version 1.3.2 (with STLSoft 1.9.25) */
# define DOTNETSTL_VER_1_3_3      0x010303ff  /*!< Version 1.3.3 (with STLSoft 1.9.113) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define DOTNETSTL_VER_MAJOR       1
#define DOTNETSTL_VER_MINOR       3
#define DOTNETSTL_VER_REVISION    3
#define DOTNETSTL_VER             DOTNETSTL_VER_1_3_3

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if !defined(_STLSOFT_VER) || \
    _STLSOFT_VER < 0x010971ff
# error This version of the .netSTL libraries requires STLSoft version 1.9.113, or later
#endif /* _STLSOFT_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the .netSTL libraries are
 *
 * Borland C++ 5.5, 5.51, 5.6
 * Digital Mars C/C++ 8.26 - 8.32
 * Metrowerks 2.4 & 3.0 (CodeWarrior 7.0 & 8.0)
 * Intel C/C++ 6.0
 * Visual C++ 5.0, 6.0, 7.0
 */

#if defined(STLSOFT_COMPILER_IS_MSVC)
/* Visual C++ */
# if _MSC_VER < 1300
#  error Versions of Visual C++ prior to 7.0 are not supported by the .netSTL libraries
# endif /* compiler */

#else /* ? compiler */
/* No recognised compiler */
# ifdef _STLSOFT_FORCE_ANY_COMPILER
#  define DOTNETSTL_COMPILER_IS_UNKNOWN
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("Compiler is unknown to .netSTL")
#  endif /* STLSOFT_COMPILE_VERBOSE */
# else /* ? _STLSOFT_FORCE_ANY_COMPILER */
#  error Currently only Visual C++ compilers are supported by the .netSTL libraries
# endif /* _STLSOFT_FORCE_ANY_COMPILER */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * More includes
 */

//#if _MSC_VER >= 1300
# using <mscorlib.dll>
//#else /* ? _MSC_VER */
//# using <system.dll>
//#endif /* _MSC_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro dotnetstl_assert provides standard debug-mode assert functionality.
 */

/** \brief Defines a runtime assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 */
#define DOTNETSTL_ASSERT(expr)                  STLSOFT_ASSERT(expr)

/** \brief Defines a runtime assertion, with message
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define DOTNETSTL_MESSAGE_ASSERT(msg, expr)     STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \brief Defines a compile-time assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or compilation will fail
 */
#define DOTNETSTL_STATIC_ASSERT(expr)           STLSOFT_STATIC_ASSERT(expr)


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define dotnetstl_assert(expr)                 DOTNETSTL_ASSERT(expr)
# define dotnetstl_message_assert(msg, expr)    DOTNETSTL_MESSAGE_ASSERT(msg, expr)
# define dotnetstl_static_assert(expr)          DOTNETSTL_STATIC_ASSERT(expr)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The .netSTL components are contained within the dotnetstl namespace. This is
 * usually an alias for stlsoft::dotnetstl_project,
 *
 * When compilers support namespaces they are defined by default. They can be
 * undefined using a cascasing system, as follows:
 *
 * DOTNETSTL_NO_NAMESPACES may not be defined.
 *
 * If _STLSOFT_NO_NAMESPACE is defined (whence the namespace stlsoft does not
 * exist), then the .netSTL constructs are defined within the dotnetstl
 * namespace. The definition matrix is as follows:
 *
 * _STLSOFT_NO_NAMESPACE     dotnetstl definition
 * ---------------------     -----------------
 *  not defined               = stlsoft::dotnetstl_project
 *  defined                   dotnetstl
 *
 * The macro dotnetstl_ns_qual() macro can be used to refer to elements in the
 * .netSTL libraries irrespective of whether they are in the
 * stlsoft::dotnetstl_project or dotnetstl namespaces.
 *
 * Furthermore, some compilers do not support the standard library in the std
 * namespace, so the dotnetstl_ns_qual_std() macro can be used to refer to elements
 * in the .netSTL libraries irrespective of whether they are in the std namespace
 * or in the global namespace.
 */

/* Cannot define .netSTL entities within the global namespace */
#if defined(DOTNETSTL_NO_NAMESPACE)
# error .netSTL constructs cannot be defined to be in the global namespace
#endif /* DOTNETSTL_NO_NAMESPACE */


#ifdef _STLSOFT_NO_NAMESPACE
/* There is no stlsoft namespace, so must define ::dotnetstl */
/** \brief The dotnetstl namespace - \c dotnetstl (aliased to \c stlsoft::dotnetstl_project) - is
 * the namespace for the .netSTL project.
 */
namespace dotnetstl
{
# else
/* Define stlsoft::dotnetstl_project */

namespace stlsoft
{

namespace dotnetstl_project
{

#endif /* _STLSOFT_NO_NAMESPACE */

/** \def dotnetstl_ns_qual(x)
 * Qualifies with <b>dotnetstl::</b> if .netSTL is using namespaces or, if not, does not qualify
 */

/** \def dotnetstl_ns_using(x)
 * Declares a using directive (with respect to <b>dotnetstl</b>) if .netSTL is using namespaces or, if not, does nothing
 */

#define dotnetstl_ns_qual(x)            ::dotnetstl::x
#define dotnetstl_ns_using(x)           using ::dotnetstl::x;

/** \def dotnetstl_ns_qual_std(x)
 * Qualifies with <b>std::</b> if .netSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def dotnetstl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if .netSTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define dotnetstl_ns_qual_std(x)       ::std::x
# define dotnetstl_ns_using_std(x)      using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define dotnetstl_ns_qual_std(x)       x
# define dotnetstl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * CLR / VC++ versions
 */

#if defined(_MSC_VER) && \
    _MSC_VER >= 1400
# define DOTNETSTL_HAT_SYNTAX_SUPPORT
#else /* ? _MSC_VER */
#endif /* _MSC_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The .netSTL uses a number of typedefs to aid in compiler-independence in the
 * libraries' main code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

typedef stlsoft_ns_qual(ss_char_a_t)        ds_char_a_t;    //!< Ansi char type
typedef stlsoft_ns_qual(ss_char_w_t)        ds_char_w_t;    //!< Unicode char type
typedef stlsoft_ns_qual(ss_sint8_t)         ds_sint8_t;     //!< 8-bit signed integer
typedef stlsoft_ns_qual(ss_uint8_t)         ds_uint8_t;     //!< 8-bit unsigned integer
typedef stlsoft_ns_qual(ss_int16_t)         ds_int16_t;     //!< 16-bit integer
typedef stlsoft_ns_qual(ss_sint16_t)        ds_sint16_t;    //!< 16-bit signed integer
typedef stlsoft_ns_qual(ss_uint16_t)        ds_uint16_t;    //!< 16-bit unsigned integer
typedef stlsoft_ns_qual(ss_int32_t)         ds_int32_t;     //!< 32-bit integer
typedef stlsoft_ns_qual(ss_sint32_t)        ds_sint32_t;    //!< 32-bit signed integer
typedef stlsoft_ns_qual(ss_uint32_t)        ds_uint32_t;    //!< 32-bit unsigned integer
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef stlsoft_ns_qual(ss_int64_t)         ds_int64_t;     //!< 64-bit integer
typedef stlsoft_ns_qual(ss_sint64_t)        ds_sint64_t;    //!< 64-bit signed integer
typedef stlsoft_ns_qual(ss_uint64_t)        ds_uint64_t;    //!< 64-bit unsigned integer
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef stlsoft_ns_qual(ss_int_t)           ds_int_t;       //!< integer
typedef stlsoft_ns_qual(ss_sint_t)          ds_sint_t;      //!< signed integer
typedef stlsoft_ns_qual(ss_uint_t)          ds_uint_t;      //!< unsigned integer
typedef stlsoft_ns_qual(ss_long_t)          ds_long_t;      //!< long
typedef stlsoft_ns_qual(ss_bool_t)          ds_bool_t;      //!< bool
typedef stlsoft_ns_qual(ss_size_t)          ds_size_t;      //!< size
typedef stlsoft_ns_qual(ss_ptrdiff_t)       ds_ptrdiff_t;   //!< ptr diff
typedef stlsoft_ns_qual(ss_streampos_t)     ds_streampos_t; //!< streampos
typedef stlsoft_ns_qual(ss_streamoff_t)     ds_streamoff_t; //!< streamoff

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Values
 *
 * Since the boolean type may not be supported natively on all compilers, the
 * values of true and false may also not be provided. Hence the values of
 * ds_true_v and ds_false_v are defined, and are used in all code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define ds_true_v       ss_true_v
#define ds_false_v      ss_false_v

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/* /////////////////////////////////////////////////////////////////////////
 * Code modification macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/* Exception signatures. */
# define dotnetstl_throw_0()                                stlsoft_throw_0()
# define dotnetstl_throw_1(x1)                              stlsoft_throw_1(x1)
# define dotnetstl_throw_2(x1, x2)                          stlsoft_throw_2(x1, x2)
# define dotnetstl_throw_3(x1, x2, x3)                      stlsoft_throw_3(x1, x2, x3)
# define dotnetstl_throw_4(x1, x2, x3, x4)                  stlsoft_throw_4(x1, x2, x3, x4)
# define dotnetstl_throw_5(x1, x2, x3, x4, x5)              stlsoft_throw_5(x1, x2, x3, x4, x5)
# define dotnetstl_throw_6(x1, x2, x3, x4, x5, x6)          stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
# define dotnetstl_throw_7(x1, x2, x3, x4, x5, x6, x7)      stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
# define dotnetstl_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)  stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define DOTNETSTL_NUM_ELEMENTS(ar)                         STLSOFT_NUM_ELEMENTS(ar)
# define dotnetstl_num_elements(ar)                         DOTNETSTL_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [DEPRECATED] Destroys the given instance \c p of the given type (\c t and \c type)
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 */
#define dotnetstl_destroy_instance(t, type, p)              STLSOFT_DESTROY_INSTANCE(t, type, p)

/** \brief [DEPRECATED] Generates an opaque type with the name \c _htype
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 */
#define dotnetstl_gen_opaque(htype)                         STLSOFT_GEN_OPAQUE(htype)

/* ////////////////////////////////////////////////////////////////////// */

#ifndef DOTNETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace dotnetstl
# else
} // namespace dotnetstl_project
} // namespace stlsoft
namespace dotnetstl = ::stlsoft::dotnetstl_project;
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !DOTNETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL */

/* ///////////////////////////// end of file //////////////////////////// */
