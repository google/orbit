/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/cccap/gcc.h
 *
 * Purpose:     Compiler feature discrimination for GNU C/C++.
 *
 * Created:     7th February 2003
 * Updated:     22nd November 2013
 *
 * Thanks:      To Sergey Nikulov, for PowerPC (BSD) compatibility fixes
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

/** \file stlsoft/internal/cccap/gcc.h
 *
 * Compiler feature discrimination for GNU C/C++
 * (\ref group__library__internal).
 */

#ifdef STLSOFT_INCL_H_STLSOFT_CCCAP_GCC
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_GCC */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT_CCCAP_GCC_MAJOR      3
# define STLSOFT_VER_H_STLSOFT_CCCAP_GCC_MINOR      21
# define STLSOFT_VER_H_STLSOFT_CCCAP_GCC_REVISION   2
# define STLSOFT_VER_H_STLSOFT_CCCAP_GCC_EDIT       87
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Structure:
 *
 * - auto-generation and compatibility
 * - preprocessor features
 * - support for built-in types
 * - built-in type characteristics
 * - support for C/C++ language features
 * - support for C language features
 * - support for C++ language features - 1
 * - support for C++ language features - 2
 * - inline assembler
 * - calling convention
 * - integer sizes
 * - size-specific integer types
 * - still to-be-determined features
 * - assertions
 * - compiler warning suppression
 * - obsolete features
 */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Custom macros
 */

#ifdef __GNUC_PATCHLEVEL__
# define STLSOFT_INTERNAL_GCC_PATCHLEVEL_   __GNUC_PATCHLEVEL__
#else /* ? __GNUC_PATCHLEVEL__ */
# define STLSOFT_INTERNAL_GCC_PATCHLEVEL_   (0)
#endif /* __GNUC_PATCHLEVEL__ */

#define STLSOFT_GCC_VER                                             \
                                            ((__GNUC__ * 10000)     \
                                            +                       \
                                            (__GNUC_MINOR__ * 100)  \
                                            +                       \
                                            (STLSOFT_INTERNAL_GCC_PATCHLEVEL_ * 1))

/* /////////////////////////////////////////////////////////////////////////
 * Preprocessor features
 *
 * - #pragma message
 * - #pragma once
 * - __COUNTER__
 * - __FUNCTION__
 * - variadic macros
 */

/*
#define STLSOFT_PPF_pragma_message_SUPPORT
*/

#if __GNUC__ > 3 || \
    (   __GNUC__ == 3 && \
        __GNUC_MINOR__ >= 4)
# define STLSOFT_PPF_pragma_once_SUPPORT
#endif /* compiler */

#if __GNUC__ > 4 || \
    (   __GNUC__ == 4 && \
        __GNUC_MINOR__ >= 3)
# define STLSOFT_PPF_COUNTER_SYMBOL_SUPPORT
#endif /* compiler */

#if __GNUC__ >= 3
# define STLSOFT_PPF_FUNCTION_SYMBOL_SUPPORT
#endif /* compiler */

#if __GNUC__ > 3 || \
    (   __GNUC__ == 3 && \
        __GNUC_MINOR__ >= 4)
# define STLSOFT_PPF_VARIADIC_MACROS_SUPPORT
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Support for built-in types
 *
 * - bool
 * - wchar_t
 * - long long
 * - __int8
 * - __int16
 * - __int32
 * - __int64
 * - long double
 */

#ifdef __cplusplus
# define STLSOFT_CF_BUILTIN_bool_SUPPORT
#endif /* __cplusplus */

#if __GNUC__ > 3 || \
    (   __GNUC__ == 3 && \
        __GNUC_MINOR__ >= 2)
# define STLSOFT_CF_BUILTIN_wchar_t_SUPPORT
#endif /* 3.2+ */

#define STLSOFT_CF_BUILTIN_long_long_SUPPORT

#define STLSOFT_CF_BUILTIN___int8_SUPPORT

#define STLSOFT_CF_BUILTIN___int16_SUPPORT

#define STLSOFT_CF_BUILTIN___int32_SUPPORT

#define STLSOFT_CF_BUILTIN___int64_SUPPORT

/* /////////////////////////////////////////////////////////////////////////
 * Built-in type characteristics
 *
 * - char is unsigned
 * - wchar_t
 *    - synthesised
 *    - available
 */

#ifdef __CHAR_UNSIGNED__
# define STLSOFT_CF_char_IS_UNSIGNED
#endif /* __CHAR_UNSIGNED__ */

#ifdef STLSOFT_CF_BUILTIN_wchar_t_SUPPORT
# define STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#endif /* STLSOFT_CF_BUILTIN_wchar_t_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Support for C/C++ language features
 *
 * - return void
 * - static assertions
 * - anonymous unions
 * - -ve % +ve => -ve result
 */

#define STLSOFT_CF_return_void_SUPPORT

#define STLSOFT_CF_STATIC_ASSERT_SUPPORT

#define STLSOFT_CF_ANONYMOUS_UNION_SUPPORT

#define STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT

/* /////////////////////////////////////////////////////////////////////////
 * Support for C language features
 *
 * - inline
 *    - C99 inline keyword
 *    - compiler-specific keyword
 */

#if __GNUC__ >= 4
# define STLSOFT_CF_C99_INLINE
#else /* ? compiler */
# define STLSOFT_CUSTOM_C_INLINE        __inline__
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Support for C++ language features - 1
 *
 * - exceptions
 *    - exception signatures
 *    - exception signatures expense
 *    - operator new throws bad_alloc
 * - RTTI
 * - namespaces
 *    - STLSoft namespace(s)?
 * - member constants
 * - explicit keyword
 * - mutable keyword
 * - template keyword
 * - typename keyword
 *    - in a template parameter
 *    - type disambiguation inside template bodies
 *    - disambiguation in default template arguments
 *    - type disambiguation inside initialiser lists in class template constructors
 *    - type disambiguation the return types in templates
 * - argument-dependent lookup
 * - static array-size determination
 * - empty-derived optimisation
 *    -
 * - empty-base optimisation
 *    -
 * - move constructor support
 * - operators
 *    - operator bool implemented as pointer-to-member
 *    - operator ! implemented as pointer-to-member
 */

#ifdef __cplusplus
# ifdef __EXCEPTIONS
#  define STLSOFT_CF_EXCEPTION_SUPPORT
#  define STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
#  define STLSOFT_CF_THROW_BAD_ALLOC
# endif /* __EXCEPTIONS */
#endif /* __cplusplus */

#ifdef __cplusplus
# define STLSOFT_CF_RTTI_SUPPORT
#endif /* __cplusplus */

#ifdef __cplusplus
# define STLSOFT_CF_NAMESPACE_SUPPORT
#endif /* __cplusplus */

/* #define _STLSOFT_NO_NAMESPACES */

#define STLSOFT_CF_MEMBER_CONSTANT_SUPPORT

#define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT

#define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT

#if __GNUC__ > 3 || \
    (   __GNUC__ == 3 && \
        __GNUC_MINOR__ >= 4)
# define STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT
#endif /* compiler */

#define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT

/* #define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT */

#define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT

#define STLSOFT_CF_ADL_SUPPORT

#if __GNUC__ >= 3
# define STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
#endif /* compiler */

/* #define STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

#define STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
#define STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT

/* /////////////////////////////////////////////////////////////////////////
 * Support for C++ language features - 2
 *
 * - templates
 *    - specialisation syntax (template <>)
 *    - partial specialisation
 *    - function template
 *    - class template
 *    - member class template
 *    - member function template
 *    - member range function template
 *    - constructor template
 *    - template template
 *    - class template fundamental argument
 *    - class template type argument
 *    - class template member function argument
 *    - class template default fundamental argument
 *    - class template default type argument
 *    - function template parameter list requires template parameter
 *    - member function template overload is properly discriminated against
 *      other non-template member function
 *    - constructor template overload is properly discriminated against
 *      other non-template constructor
 *    - copy-constructor template overload is properly discriminated against
 *      other non-template copy-constructor
 */

#define STLSOFT_CF_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX_SUPPORT

#define STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

#define STLSOFT_CF_TEMPLATE_FUNCTION_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_CLASS_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_MEMBER_CLASS_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_TEMPLATE_SUPPORT

#if __GNUC__ >= 3
# define STLSOFT_CF_TEMPLATE_MEMBER_RANGE_FUNCTION_TEMPLATE_SUPPORT
#endif /* compiler */

#define STLSOFT_CF_TEMPLATE_CTOR_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_FUNDAMENTAL_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_TYPE_PARAMETER_SUPPORT

/* #define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_PARAMETER_SUPPORT */

#define STLSOFT_CF_TEMPLATE_DEFAULT_FUNDAMENTAL_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_DEFAULT_TYPE_PARAMETER_SUPPORT

/* #define STLSOFT_CF_TEMPLATE_FUNCTION_TEMPLATE_REQUIRES_TEMPLATE_PARAMETERS_IN_FUNCTION_PARAMETER_LIST */

#define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_MEMBER_FUNCTION

#if __GNUC__ >= 3
# define STLSOFT_CF_TEMPLATE_CONSTRUCTOR_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_CONSTRUCTOR
#endif /* compiler */

/* #define STLSOFT_CF_TEMPLATE_COPY_CONSTRUCTOR_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_COPY_CONSTRUCTOR */

/* /////////////////////////////////////////////////////////////////////////
 * Inline assembler
 */

/* #define STSLSOFT_INLINE_ASM_SUPPORTED */
/* #define STSLSOFT_ASM_IN_INLINE_SUPPORTED */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 */

#define STLSOFT_CF_CDECL_SUPPORTED

#if defined(WIN32)

# define STLSOFT_CF_THISCALL_SUPPORTED

# ifdef STLSOFT_CF_CDECL_SUPPORTED
#  define STLSOFT_CDECL                 __cdecl
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

# if __GNUC__ > 2
#  define STLSOFT_CF_FASTCALL_SUPPORTED
#  define STLSOFT_FASTCALL              __fastcall
# endif /* __GNUC__ > 2 */

# define STLSOFT_CF_STDCALL_SUPPORTED
# define STLSOFT_STDCALL                __stdcall


# if (   __GNUC__ < 3 || \
        (   __GNUC__ == 3 && \
            __GNUC_MINOR__ < 4))
#  define    STLSOFT_CF_CALLING_CONVENTION_OUTSIDE_BRACE_REQUIRED
# endif /* version */

#else /* ? OS */

# ifdef STLSOFT_CF_CDECL_SUPPORTED
#  define STLSOFT_CDECL
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

#endif /* !unix && !__unix__ */

/* /////////////////////////////////////////////////////////////////////////
 * Integer sizes
 */

#define _STLSOFT_SIZEOF_CHAR            (1)

#if defined(__ILP64__) || \
    defined(_ILP64)
# error Currently the STLSoft libraries are not compatible with the ILP64 memory model
#elif defined(__LP64__) || \
      defined(_LP64)
 /* LP64 */
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (8)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#elif defined(__LLP64__) || \
      defined(_WIN32) || \
      defined(_WIN64) || \
      defined(__i386__) || \
      defined(__i386) || \
      defined(__POWERPC__) || \
      defined(__ppc)  || \
      defined(__ppc__) || \
      defined(_ARCH_PPC) || \
      defined(__PPC__) || \
      defined(__powerpc__)
 /* LLP64 */
# define _STLSOFT_SIZEOF_SHORT          (2)
# define _STLSOFT_SIZEOF_INT            (4)
# define _STLSOFT_SIZEOF_LONG           (4)
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#else /* ? data model */
# error Use of GCC has not been verified with any memory model other than LP64 and LLP64. Please contact Synesis Software
#endif /* data model */

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
 * Still to-be-determined features
 */

#define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED

/* Template support */

/* #define STLSOFT_CF_EXCEPTION_SPEC_EXPENSIVE */

#define STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT

#define STLSOFT_CF_std_NAMESPACE

#if __GNUC__ >= 3
# define STLSOFT_CF_std_char_traits_AVAILABLE
#endif /* compiler */

#define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_NON_TEMPLATE

#define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_TEMPLATE

#if __GNUC__ >= 3
# define STLSOFT_CF_VENEER_SUPPORT
#endif /* compiler */

#define STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE

#define STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR

/* Shims are supported?
 */
/* #define STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED */

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
 * Compiler warning suppression
 */

/* /////////////////////////////////////////////////////////////////////////
 * Obsolete features
 */

#include <stlsoft/internal/cccap/obsolete.h>

/* ///////////////////////////// end of file //////////////////////////// */
