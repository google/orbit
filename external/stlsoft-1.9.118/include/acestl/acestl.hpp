/* /////////////////////////////////////////////////////////////////////////
 * File:        acestl/acestl.hpp
 *
 * Purpose:     Root header for the ACESTL libraries. Performs various compiler
 *              and platform discriminations, and definitions of types.
 *
 * Created:     15th September 2004
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


#ifndef ACESTL_INCL_ACESTL_HPP_ACESTL
#define ACESTL_INCL_ACESTL_HPP_ACESTL

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ACESTL_VER_ACESTL_HPP_ACESTL_MAJOR     1
# define ACESTL_VER_ACESTL_HPP_ACESTL_MINOR     7
# define ACESTL_VER_ACESTL_HPP_ACESTL_REVISION  5
# define ACESTL_VER_ACESTL_HPP_ACESTL_EDIT      41
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file acestl/acestl.hpp
 *
 * \brief [C++ only] The root header for the \ref group__project__acestl "ACESTL" project.
 */

/* /////////////////////////////////////////////////////////////////////////
 * ACESTL version
 *
 * The libraries version information is comprised of major, minor and revision
 * components.
 *
 * Each release of the libraries will bear a different version, and that version
 * will also have its own symbol: Version 1.0.1 specifies _ACESTL_VER_1_0_1.
 */

/** \def _ACESTL_VER_MAJOR
 * The major version number of ACESTL
 *
 * A change to the major version component implies that a dramatic change
 * has occurred in the libraries, such that considerable changes to source
 * dependent on previous versions would need to be effected.
 */

/** \def _ACESTL_VER_MINOR
 * The minor version number of ACESTL
 *
 * A change to the minor version component imply that a significant change
 * has occurred to the libraries, either in the addition of new functionality
 * or in the destructive change to one or more components such that
 * recompilation and code change may be necessitated.
 */

/** \def _ACESTL_VER_REVISION
 * The revision version number of ACESTL
 *
 * A change to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the
 * changes.
 */

/** \def _ACESTL_VER
 * The current composite version number of ACESTL
 *
 * Thus the symbol _ACESTL_VER may be compared meaningfully with a specific
 * version symbol, e.g.# if _ACESTL_VER >= _ACESTL_VER_1_0_1
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _ACESTL_VER_0_9_1     0x00000901  /*!< Version 0.9.1 */
# define _ACESTL_VER_1_0_1     0x00010001  /*!< Version 1.0.1 */
# define _ACESTL_VER_1_0_2     0x00010002  /*!< Version 1.0.2 */
# define _ACESTL_VER_1_0_3     0x00010003  /*!< Version 1.0.3 */
# define _ACESTL_VER_1_0_4     0x00010004  /*!< Version 1.0.4 */
# define _ACESTL_VER_1_1_1     0x00010101  /*!< Version 1.1.1 (STLSoft 1.9.1) */
# define _ACESTL_VER_1_1_2     0x00010102  /*!< Version 1.1.2 (STLSoft 1.9.25) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _ACESTL_VER_MAJOR      1
#define _ACESTL_VER_MINOR      1
#define _ACESTL_VER_REVISION   2
#define _ACESTL_VER            _ACESTL_VER_1_1_2

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_ACE_H_LOG_MSG
# define STLSOFT_INCL_ACE_H_LOG_MSG
# include <ace/Log_Msg.h>
#endif /* !STLSOFT_INCL_ACE_H_LOG_MSG */
#ifndef STLSOFT_INCL_ACE_H_VERSION
# define STLSOFT_INCL_ACE_H_VERSION
# include <ace/Version.h>
#endif /* !STLSOFT_INCL_ACE_H_VERSION */

/* Intel is super pernickety about conversions, so we need to bring out the union_cast. */
#if defined(STLSOFT_COMPILER_IS_INTEL)
# ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_UNION_CAST
#  include <stlsoft/conversion/union_cast.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_UNION_CAST */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version compatibility
 */

#if !defined(_STLSOFT_VER_1_9_1_B41) || \
    _STLSOFT_VER < _STLSOFT_VER_1_9_1_B41
# error This version of the ACESTL libraries requires STLSoft version 1.9.1 beta 41, or later
#endif /* _STLSOFT_VER */

/* /////////////////////////////////////////////////////////////////////////
 * ACE version
 */

/** \def ACESTL_ACE_VERSION
 *
 * Composite version describing the version of ACE being compiled. The
 * upper 16 bits correspond to ACE_MAJOR_VERSION. The lower 16 bits
 * correspond to ACE_MINOR_VERSION.
 */
#define ACESTL_ACE_VERSION  ((ACE_MAJOR_VERSION << 16) | ACE_MINOR_VERSION)

/* /////////////////////////////////////////////////////////////////////////
 * Proper C++ casting
 */

#ifdef __cplusplus
# ifdef ACE_WIN32
#  undef     ACE_INVALID_HANDLE
#  if defined(STLSOFT_COMPILER_IS_INTEL)
#   define    ACE_INVALID_HANDLE            stlsoft_ns_qual(union_cast)<HANDLE>(-1)
#  else /* ? compiler */
#   define    ACE_INVALID_HANDLE            reinterpret_cast<HANDLE>(-1)
#  endif /* compiler */
# endif /* ACE_WIN32 */
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the ACESTL libraries are
 *
 */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro acestl_assert provides standard debug-mode assert functionality.
 */

/** \brief Defines a runtime assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 */
#define ACESTL_ASSERT(expr)                STLSOFT_ASSERT(expr)

/** \brief Defines a runtime assertion, with message
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#define ACESTL_MESSAGE_ASSERT(msg, expr)   STLSOFT_MESSAGE_ASSERT(msg, expr)

/** \brief Defines a compile-time assertion
 *
 * \ingroup group__library__macros__assertion
 *
 * \param expr Must be non-zero, or compilation will fail
 */
#define ACESTL_STATIC_ASSERT(expr)         STLSOFT_STATIC_ASSERT(expr)



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define acestl_assert(expr)                ACESTL_ASSERT(expr)
# define acestl_message_assert(msg, expr)   ACESTL_MESSAGE_ASSERT(msg, expr)
# define acestl_static_assert(expr)         ACESTL_STATIC_ASSERT(expr)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The ACESTL components are contained within the acestl namespace. This is
 * usually an alias for stlsoft::acestl_project,
 *
 * When compilers support namespaces they are defined by default. They can be
 * undefined using a cascasing system, as follows:
 *
 * If _STLSOFT_NO_NAMESPACES is defined, then _ACESTL_NO_NAMESPACES is defined.
 *
 * If _ACESTL_NO_NAMESPACES is defined, then _ACESTL_NO_NAMESPACE is defined.
 *
 * If _ACESTL_NO_NAMESPACE is defined, then the ACESTL constructs are defined
 * in the global scope.
 *
 * If _STLSOFT_NO_NAMESPACES, _ACESTL_NO_NAMESPACES and _ACESTL_NO_NAMESPACE are
 * all undefined but the symbol _STLSOFT_NO_NAMESPACE is defined (whence the
 * namespace stlsoft does not exist), then the ACESTL constructs are defined
 * within the acestl namespace. The definition matrix is as follows:
 *
 * _STLSOFT_NO_NAMESPACE    _ACESTL_NO_NAMESPACE   acestl definition
 * ---------------------    --------------------    -----------------
 *  not defined              not defined             = stlsoft::acestl_project
 *  not defined              defined                 not defined
 *  defined                  not defined             acestl
 *  defined                  defined                 not defined
 *
 *
 *
 * The macro acestl_ns_qual() macro can be used to refer to elements in the
 * ACESTL libraries irrespective of whether they are in the
 * stlsoft::acestl_project (or acestl) namespace or in the global namespace.
 *
 * Furthermore, some compilers do not support the standard library in the std
 * namespace, so the acestl_ns_qual_std() macro can be used to refer to elements
 * in the ACESTL libraries irrespective of whether they are in the std namespace
 * or in the global namespace.
 */

/* No STLSoft namespaces means no ACESTL namespaces */
#ifdef _STLSOFT_NO_NAMESPACES
# define _ACESTL_NO_NAMESPACES
#endif /* _STLSOFT_NO_NAMESPACES */

/* No ACESTL namespaces means no acestl namespace */
#ifdef _ACESTL_NO_NAMESPACES
# define _ACESTL_NO_NAMESPACE
#endif /* _ACESTL_NO_NAMESPACES */

#ifndef _ACESTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::acestl */
/** \brief The <code class="namespace">acestl</code> namespace contains all components
 *   in the \ref group__project__acestl "ACESTL" project.
 *
 * By default, the <code>acestl</code> namespace is actually an alias for
 * the namespace <code>stlsoft::acestl_project</code>, which is where all
 * the \ref group__project__acestl "ACESTL" components actually reside. This
 * measure allows all components within the main the
 * \ref group__project__stlsoft "STLSoft" project (which are defined within
 * the <code>stlsoft</code> namespace) to be visible to all components
 * "within" the <code>acestl</code> namespace. (Otherwise, there would be a
 * whole lot of onerous qualification throughout the code of all
 * \ref group__projects "sub-projects".)
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_ACESTL_NO_NAMESPACE</code> are defined, all
 * \ref group__project__acestl "ACESTL" components will be defined in the
 * global namespace. Conversely, if the <code>_STLSOFT_NO_NAMESPACE</code>
 * symbol (not to be confused with the
 * <code>_STLSOFT_NO_NAMESPACES</code> symbol!) is defined - meaning that
 * all \ref group__project__stlsoft "main project" components are to be
 * defined in the global namespace, and <code>_ACESTL_NO_NAMESPACE</code>
 * is <b>not</b> defined, then all \ref group__project__acestl "ACESTL"
 * components will be defined within a bona fide <code>acestl</code>
 * namespace.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace acestl
{
# else
/* Define stlsoft::acestl_project */

namespace stlsoft
{

namespace acestl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#else
stlsoft_ns_using(move_lhs_from_rhs)
#endif /* !_ACESTL_NO_NAMESPACE */

/** \def acestl_ns_qual(x)
 * Qualifies with <b>acestl::</b> if ACESTL is using namespaces or, if not, does not qualify
 */

/** \def acestl_ns_using(x)
 * Declares a using directive (with respect to <b>acestl</b>) if ACESTL is using namespaces or, if not, does nothing
 */

#ifndef _ACESTL_NO_NAMESPACE
# define acestl_ns_qual(x)             ::acestl::x
# define acestl_ns_using(x)            using ::acestl::x;
#else /* ? _ACESTL_NO_NAMESPACE */
# define acestl_ns_qual(x)             x
# define acestl_ns_using(x)
#endif /* !_ACESTL_NO_NAMESPACE */

/** \def acestl_ns_qual_std(x)
 * Qualifies with <b>std::</b> if ACESTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def acestl_ns_using_std(x)
 * Declares a using directive (with respect to <b>std</b>) if ACESTL is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#ifdef STLSOFT_CF_std_NAMESPACE
# define acestl_ns_qual_std(x)         ::std::x
# define acestl_ns_using_std(x)        using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define acestl_ns_qual_std(x)         x
# define acestl_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The ACESTL uses a number of typedefs to aid in compiler-independence in the
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
typedef stlsoft_ns_qual(ss_size_t)          as_size_t;      //!< size
typedef stlsoft_ns_qual(ss_ptrdiff_t)       as_ptrdiff_t;   //!< ptr diff
typedef stlsoft_ns_qual(ss_streampos_t)     as_streampos_t; //!< streampos
typedef stlsoft_ns_qual(ss_streamoff_t)     as_streamoff_t; //!< streamoff

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef _ACESTL_NO_NAMESPACE
typedef as_char_a_t         char_a_t;           //!< Ansi char type
typedef as_char_w_t         char_w_t;           //!< Unicode char type
//typedef as_int8_t           int8_t;             //!< 8-bit integer
typedef as_sint8_t          sint8_t;            //!< 8-bit signed integer
typedef as_uint8_t          uint8_t;            //!< 8-bit unsigned integer
typedef as_int16_t          int16_t;            //!< 16-bit integer
typedef as_sint16_t         sint16_t;           //!< 16-bit signed integer
typedef as_uint16_t         uint16_t;           //!< 16-bit unsigned integer
typedef as_int32_t          int32_t;            //!< 32-bit integer
typedef as_sint32_t         sint32_t;           //!< 32-bit signed integer
typedef as_uint32_t         uint32_t;           //!< 32-bit unsigned integer
# ifdef STLSOFT_CF_64BIT_INT_SUPPORT
 typedef as_int64_t         int64_t;            //!< 64-bit integer
 typedef as_sint64_t        sint64_t;           //!< 64-bit signed integer
 typedef as_uint64_t        uint64_t;           //!< 64-bit unsigned integer
# endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
//typedef as_short_t          short_t;            //!< short integer
typedef as_int_t            int_t;              //!< integer
typedef as_sint_t           sint_t;             //!< signed integer
typedef as_uint_t           uint_t;             //!< unsigned integer
typedef as_long_t           long_t;             //!< long integer
//typedef as_byte_t           byte_t;             //!< Byte
typedef as_bool_t           bool_t;             //!< bool
# if !defined(STLSOFT_COMPILER_IS_DMC)
typedef as_streampos_t      streampos_t;        //!< streampos
typedef as_streamoff_t      streamoff_t;        //!< streamoff
# endif /* compiler */
#endif /* !_ACESTL_NO_NAMESPACE */

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
# define acestl_throw_0()                               stlsoft_throw_0()
# define acestl_throw_1(x1)                             stlsoft_throw_1(x1)
# define acestl_throw_2(x1, x2)                         stlsoft_throw_2(x1, x2)
# define acestl_throw_3(x1, x2, x3)                     stlsoft_throw_3(x1, x2, x3)
# define acestl_throw_4(x1, x2, x3, x4)                 stlsoft_throw_4(x1, x2, x3, x4)
# define acestl_throw_5(x1, x2, x3, x4, x5)             stlsoft_throw_5(x1, x2, x3, x4, x5)
# define acestl_throw_6(x1, x2, x3, x4, x5, x6)         stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
# define acestl_throw_7(x1, x2, x3, x4, x5, x6, x7)     stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
# define acestl_throw_8(x1, x2, x3, x4, x5, x6, x7, x8) stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ACESTL_NUM_ELEMENTS(ar)                        STLSOFT_NUM_ELEMENTS(ar)
# define acestl_num_elements(ar)                        ACESTL_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief [DEPRECATED] Destroys the given instance \c p of the given type (\c t and \c type)
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 */
#define acestl_destroy_instance(t, type, p)            STLSOFT_DESTROY_INSTANCE(t, type, p)

/** \brief [DEPRECATED] Generates an opaque type with the name \c _htype
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 */
#define acestl_gen_opaque(htype)                       STLSOFT_GEN_OPAQUE(htype)

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ACESTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace acestl
# else
} // namespace acestl_project
} // namespace stlsoft
namespace acestl = ::stlsoft::acestl_project;
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ACESTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !ACESTL_INCL_ACESTL_HPP_ACESTL */

/* ///////////////////////////// end of file //////////////////////////// */
