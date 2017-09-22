/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/cccap/watcom.h
 *
 * Purpose:     Compiler feature discrimination for Watcom C/C++.
 *
 * Created:     7th February 2003
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


#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# error This file must not be included independently of stlsoft/stlsoft.h
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/** \file stlsoft/internal/cccap/watcom.h
 *
 * Compiler feature discrimination for Watcom C/C++
 * (\ref group__library__internal).
 */

#ifdef STLSOFT_INCL_H_STLSOFT_CCCAP_WATCOM
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_WATCOM */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT_CCCAP_WATCOM_MAJOR       3
# define STLSOFT_VER_H_STLSOFT_CCCAP_WATCOM_MINOR       18
# define STLSOFT_VER_H_STLSOFT_CCCAP_WATCOM_REVISION    3
# define STLSOFT_VER_H_STLSOFT_CCCAP_WATCOM_EDIT        80
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Compiler features
 */

/* Messaging
 */

#define STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT

/* Support for #pragma once
 */

#if __WATCOMC__ >= 1200
# define STLSOFT_CF_PRAGMA_ONCE_SUPPORT
#endif /* __WATCOMC__ */

/* Support for __FUNCTION__
 */

#if __WATCOMC__ >= 1240
# define STLSOFT_CF_FUNCTION_SYMBOL_SUPPORT
#endif /* __WATCOMC__ */

/* Variadic Macros
 */

#if __WATCOMC__ >= 1240
# define STLSOFT_CF_SUPPORTS_VARIADIC_MACROS
#endif /* __WATCOMC__ */

/* Types:
 */

/* bool */
#ifdef __cplusplus
# define STLSOFT_CF_NATIVE_BOOL_SUPPORT
#endif /* __cplusplus */

/* char (sign) */
#ifndef __CHAR_SIGNED__
# define STLSOFT_CF_CHAR_IS_UNSIGNED
#endif /* !__CHAR_SIGNED__ */

/* wchar_t */
#if __WATCOMC__ >= 1200
# define STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#endif /* __WATCOMC__ */

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

#if defined(__LP64__)
# define _STLSOFT_SIZEOF_CHAR           (1)
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (8)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#elif defined(_WIN32) || \
      defined(_WIN64)
# define _STLSOFT_SIZEOF_CHAR           (1)
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (4)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#else /* ? data model */
# error Use of Watcom C/C++ has not been verified on any operation system other than Win32. Please contact Synesis Software
#endif /* data model */

/* 8-bit integer */
#define STLSOFT_CF_8BIT_INT_SUPPORT
#define STLSOFT_SI08_T_BASE_TYPE    signed      char
#define STLSOFT_UI08_T_BASE_TYPE    unsigned    char

/* 16-bit integer */
#define STLSOFT_CF_16BIT_INT_SUPPORT
#define STLSOFT_SI16_T_BASE_TYPE    signed      short
#define STLSOFT_UI16_T_BASE_TYPE    unsigned    short

/* 32-bit integer */
#define STLSOFT_CF_32BIT_INT_SUPPORT
#define STLSOFT_SI32_T_BASE_TYPE    signed      int
#define STLSOFT_UI32_T_BASE_TYPE    unsigned    int
#define STLSOFT_CF_LONG_DISTINCT_INT_TYPE

/* 64-bit integer */
#define STLSOFT_CF_64BIT_INT_SUPPORT
#define STLSOFT_CF_64BIT_INT_IS_long_long
#define STLSOFT_SI64_T_BASE_TYPE    signed      long long
#define STLSOFT_UI64_T_BASE_TYPE    unsigned    long long



/* Member constants */
/* #define STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */

/* Static assertions */
#define STLSOFT_CF_STATIC_ASSERT_SUPPORT

/* RTTI support */
#if defined(_CPPRTTI) || \
    defined(__CPPRTTI)
# define STLSOFT_CF_RTTI_SUPPORT
#else
 /* Not defined */
#endif /* __CPPUNWIND */

/* Exception support */
#if defined(_CPPUNWIND) || \
    defined(__CPPUNWIND)
# define STLSOFT_CF_EXCEPTION_SUPPORT
#else
 /* Not defined */
#endif /* __CPPUNWIND */

/*  */
#define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED

/* Namespace support */
 /* Watcom C/C++ gets confused when declaring ::stlsoft::X, so namespaces are
  * suppressed.
  */
#if __WATCOMC__ < 1240
# define _STLSOFT_NO_NAMESPACES
#endif /* __WATCOMC__ */

#ifdef __cplusplus
# define STLSOFT_CF_NAMESPACE_SUPPORT
#endif /* __cplusplus */

#define STLSOFT_CF_ANONYMOUS_UNION_SUPPORT

/* #define STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID */

/* Template support */
#define STLSOFT_CF_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_TYPE_REQUIRED_IN_ARGS

#ifdef __CPPUNWIND
# define STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
#else
  /* Not defined */
#endif /* __CPPUNWIND */

/* #define STLSOFT_CF_EXCEPTION_SPEC_EXPENSIVE */


/* #define STLSOFT_CF_THROW_BAD_ALLOC */

#if __WATCOMC__ >= 1200
# define STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
#endif /* __WATCOMC__ */

#if __WATCOMC__ >= 1270
# define STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
#endif /* __WATCOMC__ */

/* # define STLSOFT_CF_TEMPLATE_FUNCTION_FUNDAMENTAL_ARGUMENT_SUPPORT */

/* #define STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

/* #define STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */

/* #define STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED */

/* #define STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

/* #define STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

/* #define STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

/* #define STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT */

/* #define STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX */

/* #define STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

# ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
#  undef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
# endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */

#if __WATCOMC__ >= 1240
# define STLSOFT_CF_std_NAMESPACE
#endif /* __WATCOMC__ */

#define STLSOFT_CF_std_char_traits_AVAILABLE

/* #define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_NON_TEMPLATE */

/* #define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_TEMPLATE */

#define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT

#define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT

/* #define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

/* #define STLSOFT_CF_ADL_LOOKUP_SUPPORT */

/* #define STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT */

/* #define STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

#define STLSOFT_CF_VENEER_SUPPORT

#define STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE

/* #define STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */

#define STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED

#define STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT

#define STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
/* #define STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Quality assurance features
 */

#if defined(_STLSOFT_CUSTOM_ASSERT)
 /* You have defined the preprocessor symbol _STLSOFT_CUSTOM_ASSERT,
  * which stipulates that you will be providing your own assert. This
  * requires that you have defined _STLSOFT_CUSTOM_ASSERT() as a macro
  * taking 1 parameter (the condition to assert).
  *
  * Suppose you have a function _DisplayAssert(), which has the
  * following signature:
  *
  *   void _DisplayAssert(char const* file, int line, char const* expression);
  *
  * Presumably you would also have your own assert macro, say MY_ASSERT(),
  * defined as:
  *
  *   #define MY_ASSERT(expr) ((void)((!(expr)) ? ((void)(_DisplayAssert(__FILE__, __LINE__, #expr))) : ((void)0)))
  *
  * so you would simply need to define _STLSOFT_CUSTOM_ASSERT() in terms of
  * MY_ASSERT(), as in:
  *
  *  #define _STLSOFT_CUSTOM_ASSERT(expr)    MY_ASSERT(expr)
  *
  * You must also specify the include header name containing your
  * custom assertion declaration, in the preprocessor symbol
  * _STLSOFT_CUSTOM_ASSERT_INCLUDE
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
# define __STLSOFT_CF_ASSERT_INCLUDE_NAME       <assert.h>
# define STLSOFT_ASSERT(expr)                   assert(expr)
#endif /* _STLSOFT_CUSTOM_ASSERT */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 */

#define STLSOFT_CF_THISCALL_SUPPORTED
#define STLSOFT_CF_CDECL_SUPPORTED
#define STLSOFT_CF_STDCALL_SUPPORTED

#ifdef STLSOFT_CF_CDECL_SUPPORTED
# define STLSOFT_CDECL              __cdecl
#endif /* STLSOFT_CF_CDECL_SUPPORTED */
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
# define STLSOFT_STDCALL            __stdcall
#endif /* STLSOFT_CF_STDCALL_SUPPORTED */

/* /////////////////////////////////////////////////////////////////////////
 * Inline assembler
 */

#define STSLSOFT_INLINE_ASM_SUPPORTED
/* #define STSLSOFT_ASM_IN_INLINE_SUPPORTED - must use static */

/* /////////////////////////////////////////////////////////////////////////
 * inline support
 */

#define STLSOFT_CF_C99_INLINE

/* /////////////////////////////////////////////////////////////////////////
 * Compiler warning suppression
 */

#pragma warning 14  10 /* Suppresses "Warning! W014: no reference to symbol ..." */
#pragma warning 472 10 /* Suppresses "Warning! W472: expression with side effect in 'sizeof' discarded" */
#pragma warning 555 10 /* Suppresses "Warning! W555: expression for 'while' is always false" */
#pragma warning 594 10 /* Suppresses "Warning! W594: construct resolved as a declaration/type" */
#pragma warning 604 10 /* Suppresses "Warning! W604: must look ahead to determine whether construct is a declaration/type or an expression" */

/* ///////////////////////////// end of file //////////////////////////// */
