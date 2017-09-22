/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/cccap/msvc.h
 *
 * Purpose:     Compiler feature discrimination for Visual C++.
 *
 * Created:     7th February 2003
 * Updated:     22nd November 2013
 *
 * Thanks:      To Cláudio Albuquerque for working on the
 *              Win64-compatibility.
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

/** \file stlsoft/internal/cccap/msvc.h
 *
 * Compiler feature discrimination for Visual C++
 * (\ref group__library__internal).
 */

#ifdef STLSOFT_INCL_H_STLSOFT_CCCAP_MSVC
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_MSVC */

#ifndef STLSOFT_COMPILER_IS_MSVC
# error This file has been erroneously included for a compiler other than Visual C++
#endif /* compiler */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT_CCCAP_MSVC_MAJOR     3
# define STLSOFT_VER_H_STLSOFT_CCCAP_MSVC_MINOR     25
# define STLSOFT_VER_H_STLSOFT_CCCAP_MSVC_REVISION  2
# define STLSOFT_VER_H_STLSOFT_CCCAP_MSVC_EDIT      123
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Structure:
 *
 * - auto-generation and compatibility
 * - predefined macros extensions
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
 * - quality assurance features
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
 * Predefined macros extensions
 */

/*
 * _MSC_FULL_VER is _MSC_VER * multiplying_factor + build-number
 *
 * For _MSC_VER >= 1400, the multiplying_factor is 100000; for earlier
 * versions (1200 - 1310) it's 10000. For 1100-, it is not defined.
 */

#ifdef _MSC_FULL_VER
# define STLSOFT_MSVC_VER                   _MSC_FULL_VER
#else /* ? _MSC_FULL_VER */
# if _MSC_VER < 1200
#  define STLSOFT_MSVC_VER                  (_MSC_VER * 10000)
# else
#  define STLSOFT_MSVC_VER                  (_MSC_VER * 100000)
# endif
#endif /* _MSC_FULL_VER */

/* /////////////////////////////////////////////////////////////////////////
 * Preprocessor features
 *
 * - #pragma message
 * - #pragma once
 * - __COUNTER__
 * - __FUNCTION__
 * - variadic macros
 */

#define STLSOFT_PPF_pragma_message_SUPPORT

#if _MSC_VER >= 900
# define STLSOFT_PPF_pragma_once_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_PPF_COUNTER_SYMBOL_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_PPF_FUNCTION_SYMBOL_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1400
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

#ifdef __BOOL_DEFINED
# define STLSOFT_CF_BUILTIN_bool_SUPPORT
#endif /* __BOOL_DEFINED */

#ifdef _NATIVE_WCHAR_T_DEFINED
# define STLSOFT_CF_BUILTIN_wchar_t_SUPPORT
#endif /* _NATIVE_WCHAR_T_DEFINED */

/*
# define STLSOFT_CF_BUILTIN_long_long_SUPPORT
*/

#define STLSOFT_CF_BUILTIN___int8_SUPPORT

#define STLSOFT_CF_BUILTIN___int16_SUPPORT

#define STLSOFT_CF_BUILTIN___int32_SUPPORT

#if _MSC_VER >= 1020
# define STLSOFT_CF_BUILTIN___int64_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1400
# define STLSOFT_CF_BUILTIN_long_long_SUPPORT
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Built-in type characteristics
 *
 * - char is unsigned
 * - wchar_t
 *    - synthesised
 *    - available
 */

#ifdef _CHAR_UNSIGNED
# define STLSOFT_CF_char_IS_UNSIGNED
#endif /* _CHAR_UNSIGNED */

#if !defined(STLSOFT_CF_BUILTIN_wchar_t_SUPPORT) && \
    defined(_WCHAR_T_DEFINED)
# define STLSOFT_CF_wchar_t_IS_SYNTHESISED
#endif /* !STLSOFT_CF_BUILTIN_wchar_t_SUPPORT && _WCHAR_T_DEFINED */

/* /////////////////////////////////////////////////////////////////////////
 * Support for C/C++ language features
 *
 * - nullptr (C++11)
 * - return void
 * - static assertions
 * - anonymous unions
 * - -ve % +ve => -ve result
 *
 * nullptr (C++11)
 * ---------------
 *
 * nullptr keyword is recognised, represented by definition of the
 * preprocessor symbol STLSOFT_CF_BUILTIN_nullptr_SUPPORT
 *
 *
 * static assertions
 * -----------------
 *
 * Two questions:
 *
 * 1. Are STLSoft-style static assertions (see stlsoft/stlsoft.h)
 * supported by the compiler? This is indicated by the definition of the
 * preprocessor symbol STLSOFT_CF_STATIC_ASSERT_SUPPORT
 *
 * 2. Is the C++11 static_assert keyword supported? This is indicated by the
 * definition of the preprocessor symbol STLSOFT_CF_static_assert_SUPPORT
 */

#if _MSC_VER >= 1600
# define STLSOFT_CF_BUILTIN_nullptr_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_return_void_SUPPORT
#endif /* compiler */

#define STLSOFT_CF_STATIC_ASSERT_SUPPORT

#if _MSC_VER >= 1600
# define STLSOFT_CF_static_assert_SUPPORT
#endif /* compiler */

#define STLSOFT_CF_ANONYMOUS_UNION_SUPPORT

#define STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT

/* /////////////////////////////////////////////////////////////////////////
 * Support for C language features
 *
 * - inline
 *    - C99 inline keyword
 *    - compiler-specific keyword
 */

/* #define STLSOFT_CF_C99_INLINE_SUPPORT */

#define STLSOFT_CUSTOM_C_INLINE             __inline

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

#ifdef _CPPUNWIND
# define STLSOFT_CF_EXCEPTION_SUPPORT
# if _MSC_VER >= 1100
#  define STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
# endif /* compiler */
/*
# define STLSOFT_CF_EXCEPTION_SPEC_EXPENSIVE
*/
# if _MSC_VER >= 1300
#  define STLSOFT_CF_EXCEPTION_OPERATOR_NEW_THROWS_BAD_ALLOC
# endif /* compiler */
#endif /* _CPPUNWIND */

#ifdef _CPPRTTI
# define STLSOFT_CF_RTTI_SUPPORT
#endif /* _CPPRTTI */

#ifdef __cplusplus
# if _MSC_VER >= 1020
#  define STLSOFT_CF_NAMESPACE_SUPPORT
# endif /* compiler */
#endif /* __cplusplus */

#if _MSC_VER < 1100
 /* Since Visual C++ 4.2 and earlier do not correctly support using declarations
  * when applied to templates, it makes the use of namespaces with templates
  * too painful to use, so namespaces are suppressed.
  */
# define _STLSOFT_NO_NAMESPACES
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1200
# define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT
#endif /* compiler */

/*
# define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT
*/

#if _MSC_VER >= 1300
# define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1310
# define STLSOFT_CF_ADL_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
#endif /* compiler */

#if defined(_MSC_EXTENSIONS) && \
    _MSC_VER < 1310
# define STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
#endif /* _MSC_EXTENSIONS && _MSC_VER < 1310 */

#if _MSC_VER >= 1300
# define STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
# define STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT
#endif /* compiler */

#if _MSC_VER <= 1200
# define STLSOFT_CF_REQUIRE_RETURN_ALWAYS
#endif /* compiler */

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

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1310
# define STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_FUNCTION_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_CLASS_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_MEMBER_CLASS_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1200
# define STLSOFT_CF_TEMPLATE_MEMBER_RANGE_FUNCTION_TEMPLATE_SUPPORT
#endif /* compiler */

/* Although VC++ 5.0 does work with this in isolated cases, in practice it experiences
 * too many internal compiler errors, or compiler lock-ups, to make it tolerable
 */
#if _MSC_VER > 1100
# define STLSOFT_CF_TEMPLATE_CTOR_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_FUNDAMENTAL_PARAMETER_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_TYPE_PARAMETER_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1310
# define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_PARAMETER_SUPPORT
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_DEFAULT_FUNDAMENTAL_PARAMETER_SUPPORT
#endif /* compiler */

/* Although VC++ 5.0 does work with this in isolated cases, in practice it experiences
 * too many internal compiler errors, or compiler lock-ups, to make it tolerable
 */
#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_DEFAULT_TYPE_PARAMETER_SUPPORT
#endif /* compiler */

#if _MSC_VER < 1100
# define STLSOFT_CF_TEMPLATE_FUNCTION_TEMPLATE_REQUIRES_TEMPLATE_PARAMETERS_IN_FUNCTION_PARAMETER_LIST
#endif /* compiler */

#if _MSC_VER >= 1300
# define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_MEMBER_FUNCTION
#endif /* compiler */

#if _MSC_VER >= 1310
# define STLSOFT_CF_TEMPLATE_CONSTRUCTOR_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_CONSTRUCTOR
#endif /* compiler */

/* This caters for the case where a copy constructor and a template copy constructor -
 * as in the case of an allocator class template - may not overload.
 */
#if _MSC_VER >= 1300
# define STLSOFT_CF_TEMPLATE_COPY_CONSTRUCTOR_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_COPY_CONSTRUCTOR
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Inline assembler
 */

#if defined(_M_IX86)
# define STSLSOFT_INLINE_ASM_SUPPORTED
# define STSLSOFT_ASM_IN_INLINE_SUPPORTED
#elif defined(_M_IA64) || \
      defined(_M_X64)
 /* VC++ 64-bit compilers do not support inline assembler for. */
#endif /* arch */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 *
 * On x86, the following are supported:
 *   - cdecl
 *   - fastcall
 *   - stdcall
 *
 * On IA64/x64 there is only a single calling convention. Calling convention
 * keywords are ignored.
 */

#define STLSOFT_CF_THISCALL_SUPPORTED

#if defined(_M_IX86)

# define STLSOFT_CF_CDECL_SUPPORTED
# ifndef _MANAGED
#  define STLSOFT_CF_FASTCALL_SUPPORTED
# endif /* !_MANAGED */
# define STLSOFT_CF_STDCALL_SUPPORTED

# define STLSOFT_CDECL                  __cdecl
# ifndef _MANAGED
#  define STLSOFT_FASTCALL              __fastcall
# endif /* !_MANAGED */
# define STLSOFT_STDCALL                __stdcall

#elif defined(_M_IA64) || \
      defined(_M_X64)

# define STLSOFT_CF_CDECL_SUPPORTED

# define STLSOFT_CDECL

#else /* ? arch */
# error Only defined for the Intel x86, IA64 and x64 architectures
#endif /* arch */

/* /////////////////////////////////////////////////////////////////////////
 * Integer sizes
 */

#define _STLSOFT_SIZEOF_CHAR            (1)
#define _STLSOFT_SIZEOF_SHORT           (2)
#define _STLSOFT_SIZEOF_INT             (4)
#define _STLSOFT_SIZEOF_LONG            (4)
#if _MSC_VER >= 1400
# define _STLSOFT_SIZEOF_LONG_LONG      (8)
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Size-specific integer types
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
#if _MSC_VER >= 1020
# define STLSOFT_SI08_T_BASE_TYPE   signed      __int8
# define STLSOFT_UI08_T_BASE_TYPE   unsigned    __int8
# if _MSC_VER == 1200
#  define STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
# endif /* compiler */
#else /* ? compiler */
# define STLSOFT_SI08_T_BASE_TYPE   signed      char
# define STLSOFT_UI08_T_BASE_TYPE   unsigned    char
#endif /* compiler */

/* 16-bit integer */
#define STLSOFT_CF_16BIT_INT_SUPPORT
#if _MSC_VER >= 1020
# define STLSOFT_SI16_T_BASE_TYPE   signed      __int16
# define STLSOFT_UI16_T_BASE_TYPE   unsigned    __int16
# if _MSC_VER == 1200
#  define STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
# endif /* compiler */
#else /* ? compiler */
# define STLSOFT_SI16_T_BASE_TYPE   signed      short
# define STLSOFT_UI16_T_BASE_TYPE   unsigned    short
#endif /* compiler */

/* 32-bit integer */
#define STLSOFT_CF_32BIT_INT_SUPPORT
#if _MSC_VER >= 1020
# define STLSOFT_SI32_T_BASE_TYPE   signed      __int32
# define STLSOFT_UI32_T_BASE_TYPE   unsigned    __int32
# if _MSC_VER == 1200
#  define STLSOFT_CF_INT_DISTINCT_INT_TYPE
# endif /* compiler */
# define STLSOFT_CF_LONG_DISTINCT_INT_TYPE
#else /* ? compiler */
# define STLSOFT_SI32_T_BASE_TYPE   signed      long
# define STLSOFT_UI32_T_BASE_TYPE   unsigned    long
# define STLSOFT_CF_INT_DISTINCT_INT_TYPE
#endif /* compiler */

/* 64-bit integer */
#if _MSC_VER >= 1020
# define STLSOFT_CF_64BIT_INT_SUPPORT
# define STLSOFT_CF_64BIT_INT_IS___int64
# define STLSOFT_SI64_T_BASE_TYPE   signed      __int64
# define STLSOFT_UI64_T_BASE_TYPE   unsigned    __int64
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Still to-be-determined features
 */

/* wchar_t */
#if _MSC_VER >= 1300
 /* Even though VC 7.0 and 7.1 provide a native wchar_t type, that is __wchar_t,
  * it is not compatible with their libraries (which use the typedef wchar_t),
  * so we cannot use it.
  *
  * wchar_t itself may be used, when _NATIVE_WCHAR_T_DEFINED is defined
  */
# ifdef _NATIVE_WCHAR_T_DEFINED
#  define STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
/* #  define STLSOFT_NATIVE_WCHAR_T            __wchar_t */
# elif defined(_WCHAR_T_DEFINED)
#  define STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
# else
  /* Not defined */
#  define STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
# endif /* _WCHAR_T_DEFINED */
#else
 /* Previous versions do not have a native type, but do have the typedef wchar_t
  * when _WCHAR_T_DEFINED is defined
  */
# if defined(_WCHAR_T_DEFINED)
#  define STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
# endif /* _WCHAR_T_DEFINED */
#endif /* compiler */

/*  */
#if (   _MSC_VER >= 1100 && \
        _MSC_VER < 1310) || \
    _MSC_VER >= 1500
# define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED
#endif /* compiler */

#if _MSC_VER >= 1100
# define STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
#else
  /* Not supported */
# ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
#  undef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
# endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
#endif /* compiler */


#if _MSC_VER >= 1100
# define STLSOFT_CF_std_NAMESPACE
#endif /* compiler */

#define STLSOFT_CF_std_char_traits_AVAILABLE

#if _MSC_VER >= 1100
# define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_NON_TEMPLATE
# define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_TEMPLATE
#endif /* compiler */



#define STLSOFT_CF_VENEER_SUPPORT

#if !defined(_ATL_MIN_CRT)
# define STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE
#endif /* !_ATL_MIN_CRT */

/*
# define STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR
*/

#if _MSC_VER < 1100
# define STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED
#endif /* compiler */


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
# else /* ? _STLSOFT_CUSTOM_ASSERT_INCLUDE */
#  error You must define _STLSOFT_CUSTOM_ASSERT_INCLUDE along with _STLSOFT_CUSTOM_ASSERT()
# endif /* !_STLSOFT_CUSTOM_ASSERT_INCLUDE */
#else /* ? _STLSOFT_CUSTOM_ASSERT */
# define __STLSOFT_CF_ASSERT_SUPPORT
# define STLSOFT_CF_ASSERT_SUPPORT
 /* #define   __STLSOFT_CF_USE_cassert */
# define __STLSOFT_CF_ASSERT_INCLUDE_NAME       <crtdbg.h>
# define STLSOFT_ASSERT(expr)                   _ASSERTE(expr)
#endif /* _STLSOFT_CUSTOM_ASSERT */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler warning suppression
 */

/* Suppresses: "'identifier' : has bad storage class" */
#pragma warning(disable : 4042)

/* Suppresses: "typedef-name 'identifier1' used as synonym for class-name 'identifier2'" */
#pragma warning(disable : 4097)

/* Suppresses: "conditional expression is constant" */
#pragma warning(disable : 4127)

/* Suppresses: "qualifier applied to reference type ignored" */
#pragma warning(disable : 4181)

/* Suppresses: "'<function>' has C-linkage specified, but returns UDT '<udt>' which is incompatible with C" */
#if _MSC_VER < 1200
# pragma warning(disable : 4190)
#endif /* compiler */

/* Suppresses: "nonstandard extension used : nameless struct/union" */
#pragma warning(disable : 4201)

/* Suppresses: "nonstandard extension used : 'xxxx' keyword is reserved for future use" */
#if _MSC_VER < 1100
# pragma warning(disable : 4237)
#endif /* compiler */

/* Suppresses: "return type for 'identifier::operator ->' is not a UDT or reference to a UDT. Will produce errors if applied using infix notation" */
#if _MSC_VER < 1300
# pragma warning(disable : 4284)
#endif /* compiler */

/* Suppresses: "C++ Exception Specification ignored" */
#pragma warning(disable : 4290)

#if defined(_MSC_EXTENSIONS)
/* Suppresses: nonstandard extension used : 'argument' : conversion from 'X' to 'X&' */
# pragma warning(disable : 4239)
#endif /* _MSC_EXTENSIONS && _MSC_VER < 1310 */

/* Suppresses: "'' decorated name length exceeded, name was truncated" */
#pragma warning(disable : 4503)

#if _MSC_VER < 1300 && \
    !defined(STLSOFT_STRICT)
# pragma warning(disable : 4512)
#endif /* _MSC_VER < 1300 && STLSOFT_STRICT */

/* Suppresses: "unreferenced inline function has been removed" */
#pragma warning(disable : 4514)

#if _MSC_VER >= 1310
/* Suppresses: "expression before comma has no effect; expected expression with side-effect" */
# pragma warning(disable : 4548)

/* Suppresses: "#pragma warning : there is no warning number 'XXXX'" */
# pragma warning(disable: 4619)
#endif /* compiler */

/* Suppresses: "C++ language change: to explicitly specialize class template 'X' use the following syntax: template<> struct X<Y>" */
#if _MSC_VER < 1310
# pragma warning(disable : 4663)
#endif /* compiler */

/* Suppresses: "'function' : resolved overload was found by argument-dependent lookup" */
#if _MSC_VER >= 1310
# pragma warning(disable : 4675)
#endif /* compiler */

/* Suppresses: "function not expanded" */
#pragma warning(disable : 4710)

/* Suppresses: "identifier was truncated to '255' characters in the browser information" */
#pragma warning(disable : 4786)

#if _MSC_VER >= 1310
/* Suppresses: "'bytes' bytes padding added after member 'member'" */
# pragma warning(disable : 4820)
#endif /* compiler */

#if _MSC_VER < 1300
# ifdef __cplusplus
#  include <functional>  /* This is included so we get past the MSVC promotion of 4663 before we then disable it again */
# endif /* __cplusplus */
# pragma warning(disable : 4663)
# ifdef __cplusplus
#  include <utility>     /* This is included so we get past the MSVC promotion of 4284 before we then disable it again */
# endif /* __cplusplus */
# pragma warning(disable : 4284)
#endif /* compiler */

#if _MSC_VER < 1300 && \
    defined(__cplusplus) && \
    defined(_DEBUG)
# include <cstddef>
# ifdef _XSTDDEF_
#  undef _TRY_BEGIN
#  define _TRY_BEGIN    if(1) {
#  undef _CATCH
#  define _CATCH(x)     } else {
#  undef _CATCH_ALL
#  define _CATCH_ALL    } else {
#  undef _CATCH_END
#  define _CATCH_END    }
# endif /* _XSTDDEF_ */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Obsolete features
 */

#include <stlsoft/internal/cccap/obsolete.h>

/* ///////////////////////////// end of file //////////////////////////// */
