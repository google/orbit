/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/cccap/sunpro.h
 *
 * Purpose:     Compiler feature discrimination for SunPro C / SunPro C++.
 *
 * Created:     24th April 2008
 * Updated:     22nd November 2013
 *
 * Thanks to:   Jonathan Wakely and Lars Ivar Igesund for help with
 *              getting STLSoft (and Pantheios) compatible with Solaris.
 *
 *              Austin Ziegler for pointing out __func__ support
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2008-2013, Matthew Wilson and Synesis Software
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


#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# error This file must not be included independently of stlsoft/stlsoft.h
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/** \file stlsoft/internal/cccap/sunpro.h
 *
 * Compiler feature discrimination for SunPro C / SunPro C++.
 * (\ref group__library__internal).
 */

#ifdef STLSOFT_INCL_H_STLSOFT_CCCAP_SUNPRO
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_SUNPRO */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT_CCCAP_SUNPRO_MAJOR      1
# define STLSOFT_VER_H_STLSOFT_CCCAP_SUNPRO_MINOR      0
# define STLSOFT_VER_H_STLSOFT_CCCAP_SUNPRO_REVISION   4
# define STLSOFT_VER_H_STLSOFT_CCCAP_SUNPRO_EDIT       9
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Helper definitions
 *
 * NOTE: These are #undef'd at the end of this file
 */

#ifdef __cplusplus
# define _STLSOFT_SUNPRO_VER_   __SUNPRO_CC
#else /* ? __cplusplus */
# define _STLSOFT_SUNPRO_VER_   __SUNPRO_C
#endif /* __cplusplus */

#define _STLSOFT_SUNPRO_VER_MAJOR   ((_STLSOFT_SUNPRO_VER_ & 0xff00) >> 8)
#define _STLSOFT_SUNPRO_VER_MINOR   ((_STLSOFT_SUNPRO_VER_ & 0x00f0) >> 4)

/* /////////////////////////////////////////////////////////////////////////
 * Compiler features
 */

#if (5 != _STLSOFT_SUNPRO_VER_MAJOR) || \
    (9 != _STLSOFT_SUNPRO_VER_MINOR)
# error The STLSoft libraries have only been verified with Sun Pro 5.9.x. Please contact Synesis Software
#endif /* major.minor */

/* ///////////////////////////////////////////////
 * Pre-processor / compiler
 */

/* Messaging
 */

/* #define STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT */

/* Support for #pragma once
 */

/* #define STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* Support for __FUNCTION__
 */

/* #define STLSOFT_CF_FUNCTION_SYMBOL_SUPPORT */

#define STLSOFT_CF_func_SYMBOL_SUPPORT

/* Variadic Macros
 *
 * Feature was buggy at one point, require 5.5 to be safe.
 */

#if _STLSOFT_SUNPRO_VER_ > 0x0550
# define STLSOFT_CF_SUPPORTS_VARIADIC_MACROS
#endif /* _STLSOFT_SUNPRO_VER_ */

/* ///////////////////////////////////////////////
 * Types
 */

/* bool */
#if defined(__cplusplus) && \
    defined(_BOOL)
# define STLSOFT_CF_NATIVE_BOOL_SUPPORT
#endif /* __cplusplus */

/* char (sign) */
/* #define STLSOFT_CF_CHAR_IS_UNSIGNED */

/* wchar_t */
#ifdef _WCHAR_T
# define STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#endif

/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////
 * Integral types
 *
 * The purpose of this section is to define the following types:
 *
 *  - 8-bit signed and unsigned integers
 *  - 16-bit signed and unsigned integers
 *  - 32-bit signed and unsigned integers
 *  - (optionally) 64-bit signed and unsigned integers
 *
 * and to define, where appropriate the following symbols (used for
 * overloading):
 *
 *  - STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
 *  - STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
 *  - STLSOFT_CF_INT_DISTINCT_INT_TYPE
 *  - STLSOFT_CF_LONG_DISTINCT_INT_TYPE
 *  - STLSOFT_CF_LONG_LONG_DISTINCT_INT_TYPE
 *
 * which indicate that a given type is not used in the size-specific types.
 */


#if defined(_LP64) || \
    defined(__LP64__)
# define _STLSOFT_SIZEOF_CHAR           (1)
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (8)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#else /* ? data model */
# define _STLSOFT_SIZEOF_CHAR           (1)
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (4)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#endif /* data model */


#if 0
#if defined(__sparcv9)
# error Use of Sun Pro has not yet been verified on 64-bit Sparc. Please contact Synesis Software
#elif defined(__amd64)
# error Use of Sun Pro has not yet been verified on x64. Please contact Synesis Software
# ifndef __LP64__
#  error LP64 memory model is assumed on x64. Please contact Synesis Software
# endif /* __LP64__ */
# define _STLSOFT_SIZEOF_CHAR           (1)
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (8)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#elif defined(__sparc) || \
      defined(__i386)
# define _STLSOFT_SIZEOF_CHAR           (1)
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (4)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#else /* ? data model */
# error Use of Sun Pro has not been verified on any operation system other than x86, x64 and Sparc. Please contact Synesis Software
#endif /* data model */
#endif /* 0 */

/* 8-bit integer */
#define STLSOFT_CF_8BIT_INT_SUPPORT
#define STLSOFT_SI08_T_BASE_TYPE        signed      char
#define STLSOFT_UI08_T_BASE_TYPE        unsigned    char

/* 16-bit integer */
#define STLSOFT_CF_16BIT_INT_SUPPORT
#define STLSOFT_SI16_T_BASE_TYPE        signed      short
#define STLSOFT_UI16_T_BASE_TYPE        unsigned    short

/* 32-bit integer */
#define STLSOFT_CF_32BIT_INT_SUPPORT
#define STLSOFT_SI32_T_BASE_TYPE        signed      int
#define STLSOFT_UI32_T_BASE_TYPE        unsigned    int
#define STLSOFT_CF_LONG_DISTINCT_INT_TYPE

/* 64-bit integer */
#define STLSOFT_CF_64BIT_INT_SUPPORT
#define STLSOFT_CF_64BIT_INT_IS_long_long
#define STLSOFT_SI64_T_BASE_TYPE        signed      long long
#define STLSOFT_UI64_T_BASE_TYPE        unsigned    long long

/* ///////////////////////////////////////////////
 * Language features
 */

/* Anonymous unions */
#ifdef __cplusplus
# define STLSOFT_CF_ANONYMOUS_UNION_SUPPORT
#endif /* __cplusplus */

/* Member constants */
#ifdef __cplusplus
# define STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
#endif /* __cplusplus */

/* Static assertions */
#define STLSOFT_CF_STATIC_ASSERT_SUPPORT

/* sign of modulus of negative numbers */
#define STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT

/* RTTI support */
#ifdef __cplusplus
# define STLSOFT_CF_RTTI_SUPPORT
#endif /* __cplusplus */

/* Exception support */
#ifdef __cplusplus
# define STLSOFT_CF_EXCEPTION_SUPPORT
#endif /* __cplusplus */

#ifdef __cplusplus
# define STLSOFT_CF_THROW_BAD_ALLOC
#endif /* __cplusplus */

/* Namespace support */
#ifdef __cplusplus
# define STLSOFT_CF_NAMESPACE_SUPPORT
#endif /* __cplusplus */

#ifdef __cplusplus
# define STLSOFT_CF_std_NAMESPACE
#endif /* __cplusplus */

/* return void */
#ifdef __cplusplus
# define STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID
#endif /* __cplusplus */

/* Template support */
#ifdef __cplusplus
# define STLSOFT_CF_TEMPLATE_SUPPORT
#endif /* __cplusplus */

/* #define STLSOFT_CF_TEMPLATE_TYPE_REQUIRED_IN_ARGS */

/*  */
/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT

/* !!! Assumed. Not yet verified !!! */ /* #define STLSOFT_CF_EXCEPTION_SPEC_EXPENSIVE */

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_std_char_traits_AVAILABLE

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_NON_TEMPLATE

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_TEMPLATE

#define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT

#define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT

/* !!! Assumed. Not yet verified !!! */ /* #define STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_ADL_LOOKUP_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_VENEER_SUPPORT

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR

/* Shims are supported?
 */
/* #define STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED */

/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
/* !!! Assumed. Not yet verified !!! */ #define STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT

/* /////////////////////////////////////////////////////////////////////////
 * Quality assurance features
 */

#if defined(_STLSOFT_CUSTOM_ASSERT)
 /* You have defined the preprocessor symbol _STLSOFT_CUSTOM_ASSERT,
  * which stipulates that you will be providing your own assert. This
  * requires that you have defined _STLSOFT_CUSTOM_ASSERT() as a macro
  * taking 1 parameter (the condition to assert).
  *
  * Suppose you have a function DisplayAssert_(), which has the
  * following signature:
  *
  *   void DisplayAssert_(char const* file, int line, char const* expression);
  *
  * Presumably you would also have your own assert macro, say MY_ASSERT(),
  * defined as:
  *
  *   #define MY_ASSERT(expr) ((void)((!(expr)) ? ((void)(DisplayAssert_(__FILE__, __LINE__, #expr))) : ((void)0)))
  *
  * so you would simply need to define _STLSOFT_CUSTOM_ASSERT() in terms of
  * MY_ASSERT(), as in:
  *
  *  #define _STLSOFT_CUSTOM_ASSERT(expr)    MY_ASSERT(expr)
  *
  * where
  */
# define __STLSOFT_CF_ASSERT_SUPPORT
# define STLSOFT_CF_ASSERT_SUPPORT
# define STLSOFT_ASSERT(expr)                   _STLSOFT_CUSTOM_ASSERT(expr)
# if defined(_STLSOFT_CUSTOM_ASSERT_INCLUDE)
#  define   __STLSOFT_CF_ASSERT_INCLUDE_NAME    _STLSOFT_CUSTOM_ASSERT_INCLUDE
# else
#  error You must define _STLSOFT_CUSTOM_ASSERT_INCLUDE along with _STLSOFT_CUSTOM_ASSERT()
# endif /* !_STLSOFT_CUSTOM_ASSERT_INCLUDE */
#else /* ? _STLSOFT_CUSTOM_ASSERT */
# define __STLSOFT_CF_ASSERT_SUPPORT
# define STLSOFT_CF_ASSERT_SUPPORT
/* # define   __STLSOFT_CF_USE_cassert */
# define __STLSOFT_CF_ASSERT_INCLUDE_NAME       <assert.h>
# define STLSOFT_ASSERT(expr)                   assert(expr)
#endif /* _STLSOFT_CUSTOM_ASSERT */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 */

#define STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_CDECL_SUPPORTED
# define STLSOFT_CDECL
#endif /* STLSOFT_CF_CDECL_SUPPORTED */

/* /////////////////////////////////////////////////////////////////////////
 * Inline assembler
 */

/* #define STSLSOFT_INLINE_ASM_SUPPORTED */
/* #define STSLSOFT_ASM_IN_INLINE_SUPPORTED */

/* /////////////////////////////////////////////////////////////////////////
 * inline support
 */

#define STLSOFT_CF_C99_INLINE

/* /////////////////////////////////////////////////////////////////////////
 * If <cwchar> gets included after <stdio.h> or <cstdio> then it fails to
 * declare mbstate_t and various wchar functions, so include it now to try
 * and reduce problem (too late if <stdio.h> has already been included.)
 * http://forum.java.sun.com/thread.jspa?messageID=10035724
 */

#ifdef STLSOFT_LINUX_STDIO_ORDER_BUG
# ifdef __cplusplus
#  include <cwchar>
# endif /* __cplusplus */
#endif /* STLSOFT_LINUX_STDIO_ORDER_BUG */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler warning suppression
 */

/* /////////////////////////////////////////////////////////////////////////
 * Helper definitions
 */

#undef _STLSOFT_SUNPRO_VER_
#undef _STLSOFT_SUNPRO_VER_MAJOR
#undef _STLSOFT_SUNPRO_VER_MINOR

/* ///////////////////////////// end of file //////////////////////////// */
