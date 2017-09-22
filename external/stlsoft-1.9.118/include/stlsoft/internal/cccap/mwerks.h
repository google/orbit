/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/cccap/mwerks.h
 *
 * Purpose:     Compiler feature discrimination for Metrowerks CodeWarrior.
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

/** \file stlsoft/internal/cccap/mwerks.h
 *
 * Compiler feature discrimination for Metrowerks CodeWarrior
 * (\ref group__library__internal).
 */

#ifdef STLSOFT_INCL_H_STLSOFT_CCCAP_MWERKS
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_MWERKS */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT_CCCAP_MWERKS_MAJOR       3
# define STLSOFT_VER_H_STLSOFT_CCCAP_MWERKS_MINOR       18
# define STLSOFT_VER_H_STLSOFT_CCCAP_MWERKS_REVISION    3
# define STLSOFT_VER_H_STLSOFT_CCCAP_MWERKS_EDIT        75
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

#define STLSOFT_CF_PRAGMA_ONCE_SUPPORT

/* Support for __FUNCTION__
 */

#if (__MWERKS__ & 0xFF00) >= 0x3000
# define STLSOFT_CF_FUNCTION_SYMBOL_SUPPORT
#else /* ? compiler */
 /* Not defined */
#endif /* compiler */

/* Variadic Macros
 */

#if (__MWERKS__ & 0xFF00) >= 0x3000
# define STLSOFT_CF_SUPPORTS_VARIADIC_MACROS
#else /* ? compiler */
 /* Not defined */
#endif /* compiler */

/* Types:
 */

/* bool */
#if __option(bool)
# define STLSOFT_CF_NATIVE_BOOL_SUPPORT
#else
  /* Not defined */
#endif /* __option(bool) */

/* char (sign) */
#if 0 /* Does not seem to work ?? */
#if __option(chars_unsigned)
# define STLSOFT_CF_CHAR_IS_UNSIGNED
#endif /* _CHAR_UNSIGNED */
#endif /* 0 */

/* wchar_t */
# if __option(wchar_type)
#  define STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
# else
  /* Not defined */
# endif /* __option(wchar_type) */

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
# error Use of CodeWarrior has not been verified on any operation system other than Win32. Please contact Synesis Software
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
#if __option(longlong)
# define STLSOFT_CF_64BIT_INT_IS_long_long
#endif /* __option(longlong) */
#ifdef STLSOFT_CF_64BIT_INT_IS_long_long
# define STLSOFT_CF_64BIT_INT_SUPPORT
# define STLSOFT_SI64_T_BASE_TYPE   signed      long long
# define STLSOFT_UI64_T_BASE_TYPE   unsigned    long long
#endif /* STLSOFT_CF_64BIT_INT_IS_long_long */

/* Member constants */
#define STLSOFT_CF_MEMBER_CONSTANT_SUPPORT

/* Static assertions */
#define STLSOFT_CF_STATIC_ASSERT_SUPPORT

/* RTTI support */
#if __option(RTTI)
# define STLSOFT_CF_RTTI_SUPPORT
#else
  /* Not defined */
#endif /* __option(RTTI) */

/* Exception support */
#if __option(exceptions) && \
    defined(__cplusplus)
# define STLSOFT_CF_EXCEPTION_SUPPORT
#else
  /* Not defined */
#endif /* __option(exceptions) && __cplusplus */

/*  */
#define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED

/* Namespace support */
/* #define _STLSOFT_NO_NAMESPACES */

#ifdef __cplusplus
# define STLSOFT_CF_NAMESPACE_SUPPORT
#endif /* __cplusplus */

#define STLSOFT_CF_ANONYMOUS_UNION_SUPPORT

#define STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID

/* Template support */
#if __option(ecplusplus)
  /* Not defined */
#else
# define STLSOFT_CF_TEMPLATE_SUPPORT
#endif /* __option(ecplusplus) */

/* #define STLSOFT_CF_TEMPLATE_TYPE_REQUIRED_IN_ARGS */

#define STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT

/* #define STLSOFT_CF_EXCEPTION_SPEC_EXPENSIVE */


#define STLSOFT_CF_THROW_BAD_ALLOC

#define STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT

#define STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT

#if (__MWERKS__ & 0xFF00) >= 0x3000
# define STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
#else /* ? compiler */
 /* Not defined */
#endif /* compiler */

#define STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT

#define STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED

#define STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

#define STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED

#define STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT

#define STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT

#define STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX

#define STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

#define STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT

#define STLSOFT_CF_std_NAMESPACE

#define STLSOFT_CF_std_char_traits_AVAILABLE

#define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_NON_TEMPLATE

#define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_TEMPLATE

#define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT

#define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT

/* #define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT */

#define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT

/* #define STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT */

/* #define STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

#define STLSOFT_CF_ADL_LOOKUP_SUPPORT

#define STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT

#define STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT

#define STLSOFT_CF_VENEER_SUPPORT

#define STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE

/* #define STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */

/* Shims are supported. */
/* #define STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED */

#define STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT

#define STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
#define STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT

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
# define __STLSOFT_CF_USE_cassert
#endif /* _STLSOFT_CUSTOM_ASSERT */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 */

#define STLSOFT_CF_CDECL_SUPPORTED

#if defined(_WIN32)

# define STLSOFT_CF_THISCALL_SUPPORTED
# define STLSOFT_CF_FASTCALL_SUPPORTED
# define STLSOFT_CF_STDCALL_SUPPORTED

# ifdef STLSOFT_CF_CDECL_SUPPORTED
#  define STLSOFT_CDECL             __cdecl
# endif /* STLSOFT_CF_CDECL_SUPPORTED */
# ifdef STLSOFT_CF_FASTCALL_SUPPORTED
#  define STLSOFT_FASTCALL          __fastcall
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */
# ifdef STLSOFT_CF_STDCALL_SUPPORTED
#  define STLSOFT_STDCALL           __stdcall
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */

#else /* ? UNIX */

# ifdef STLSOFT_CF_CDECL_SUPPORTED
#  define STLSOFT_CDECL
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

#endif /* !unix && !__unix__ */

/* /////////////////////////////////////////////////////////////////////////
 * Inline assembler
 */

#define STSLSOFT_INLINE_ASM_SUPPORTED
#if (__MWERKS__ & 0xFF00) >= 0x3000
# define STSLSOFT_ASM_IN_INLINE_SUPPORTED
#else /* ? compiler */
 /* Not defined */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * inline support
 */

#define STLSOFT_CF_C99_INLINE

/* /////////////////////////////////////////////////////////////////////////
 * Compiler warning suppression
 */

/* ///////////////////////////// end of file //////////////////////////// */
