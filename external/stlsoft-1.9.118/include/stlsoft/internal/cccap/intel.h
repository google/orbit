/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/cccap/intel.h
 *
 * Purpose:     Compiler feature discrimination for Intel C/C++.
 *
 * Created:     7th February 2003
 * Updated:     22nd November 2013
 *
 * Thanks:      To Derek Baikie for working on the
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

/** \file stlsoft/internal/cccap/intel.h
 *
 * Compiler feature discrimination for Intel C/C++
 * (\ref group__library__internal).
 */

#ifdef STLSOFT_INCL_H_STLSOFT_CCCAP_INTEL
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_INTEL */

#ifndef STLSOFT_COMPILER_IS_INTEL
# error This file has been erroneously included for a compiler other than Intel C/C++
#endif /* compiler */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define  STLSOFT_VER_H_STLSOFT_CCCAP_INTEL_MAJOR       3
# define  STLSOFT_VER_H_STLSOFT_CCCAP_INTEL_MINOR       18
# define  STLSOFT_VER_H_STLSOFT_CCCAP_INTEL_REVISION    3
# define  STLSOFT_VER_H_STLSOFT_CCCAP_INTEL_EDIT        80
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
 * Compiler features / compatibility
 */

#if defined(_MSC_VER) && \
    _MSC_VER < 1200
# error STLSoft does not support Intel C/C++ in compatibility mode with versions of Visual C++ less than 6.0
#endif

/* For this to work, -Qvc7.1 must be defined. */
#if __INTEL_COMPILER == 700 && \
    _MSC_VER == 1310
# define STLSOFT_INCL_H_STLSOFT_CCCAP_INTEL_7_1
#endif /* __INTEL_COMPILER == 700 && _MSC_VER == 1310 */

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

#if __INTEL_COMPILER >= 700
# define STLSOFT_PPF_pragma_once_SUPPORT
#endif /* compiler */

#if __INTEL_COMPILER >= 700
# define STLSOFT_PPF_COUNTER_SYMBOL_SUPPORT
#endif /* compiler */

#if __INTEL_COMPILER >= 700
# define STLSOFT_PPF_FUNCTION_SYMBOL_SUPPORT
#endif /* compiler */

/* TODO: check this
# define STLSOFT_PPF_VARIADIC_MACROS_SUPPORT
*/

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

#if !defined(_MSC_VER) || \
    defined(__BOOL_DEFINED)
# define STLSOFT_CF_BUILTIN_bool_SUPPORT
#endif /* !_MSC_VER || __BOOL_DEFINED */

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
#endif /* _MSC_VER >= 1020 */

/* /////////////////////////////////////////////////////////////////////////
 * Built-in type characteristics
 *
 * - char is unsigned
 * - wchar_t
 *    - synthesised
 *    - available
 */

#ifdef _CHAR_UNSIGNED
# define STLSOFT_CF_CHAR_IS_UNSIGNED
#endif /* _CHAR_UNSIGNED */

#if !defined(STLSOFT_CF_BUILTIN_wchar_t_SUPPORT) && \
    defined(_WCHAR_T_DEFINED)
# define STLSOFT_CF_wchar_t_IS_SYNTHESISED
#endif /* !STLSOFT_CF_BUILTIN_wchar_t_SUPPORT && _WCHAR_T_DEFINED */

/* /////////////////////////////////////////////////////////////////////////
 * Support for C/C++ language features
 *
 * - return void
 * - static assertions
 * - anonymous unions
 * - -ve % +ve => -ve result
 */

#if __INTEL_COMPILER >= 700
# define STLSOFT_CF_return_void_SUPPORT
#endif /* compiler */

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

/* #define STLSOFT_CF_C99_INLINE_SUPPORT */

#define STLSOFT_CUSTOM_C_INLINE     __inline

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
# if defined(_MSC_VER) && \
     _MSC_VER >= 1100
#  define STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
# endif /* _MSC_VER */
/* TODO: check this
# define STLSOFT_CF_EXCEPTION_SPEC_EXPENSIVE
*/
# if (  defined(_MSC_VER) && \
        _MSC_VER >= 1300) || \
     (  !defined(_MSC_VER) && \
        __INTEL_COMPILER >= 900)
#  define STLSOFT_CF_EXCEPTION_OPERATOR_NEW_THROWS_BAD_ALLOC
# endif /* VC++ 13+ / Intel C/C++ 9+ */
#endif /* _CPPUNWIND */

#ifdef _CPPRTTI
# define STLSOFT_CF_RTTI_SUPPORT
#endif /* _CPPRTTI */

#ifdef __cplusplus
# define STLSOFT_CF_NAMESPACE_SUPPORT
#endif /* __cplusplus */

/*
# define _STLSOFT_NO_NAMESPACES
*/

#define STLSOFT_CF_MEMBER_CONSTANT_SUPPORT

#define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT

#define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT

/*
# define STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT
*/

#define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT

#define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT

#if defined(STLSOFT_INCL_H_STLSOFT_CCCAP_INTEL_7_1) || \
    __INTEL_COMPILER >= 800
# define STLSOFT_CF_ADL_SUPPORT
#endif /* STLSOFT_INCL_H_STLSOFT_CCCAP_INTEL_7_1  || __INTEL_COMPILER >= 800 */

#define STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT

#ifdef _MSC_EXTENSIONS
# define STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
#endif /* _MSC_EXTENSIONS */

#if !defined(_MSC_VER) || \
    _MSC_VER >= 1300
# define STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
# define STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT
#endif /* !_MSC_VER || _MSC_VER >= 1300 */

/*
# define STLSOFT_CF_REQUIRE_RETURN_ALWAYS
*/

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

#define STLSOFT_CF_TEMPLATE_MEMBER_RANGE_FUNCTION_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_CTOR_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT

#define STLSOFT_CF_TEMPLATE_FUNDAMENTAL_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_TYPE_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_DEFAULT_FUNDAMENTAL_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_DEFAULT_TYPE_PARAMETER_SUPPORT

#define STLSOFT_CF_TEMPLATE_FUNCTION_TEMPLATE_REQUIRES_TEMPLATE_PARAMETERS_IN_FUNCTION_PARAMETER_LIST

#define STLSOFT_CF_TEMPLATE_MEMBER_FUNCTION_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_MEMBER_FUNCTION

#define STLSOFT_CF_TEMPLATE_CONSTRUCTOR_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_CONSTRUCTOR

/* TODO: check this
# define STLSOFT_CF_TEMPLATE_COPY_CONSTRUCTOR_TEMPLATE_OVERLOAD_DISCRIMINATED_AGAINST_NON_TEMPLATE_COPY_CONSTRUCTOR
*/

/* /////////////////////////////////////////////////////////////////////////
 * Inline assembler
 */

#if defined(_M_IX86)
# define STSLSOFT_INLINE_ASM_SUPPORTED
# define STSLSOFT_ASM_IN_INLINE_SUPPORTED
#elif defined(_M_IA64) || \
      defined(_M_X64)
 /* Inline assembler not supported for Intel C/C++ 64-bit compilers */
#endif /* arch */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 *
 * On x86 (Windows), the following are supported:
 *   - cdecl
 *   - fastcall
 *   - stdcall
 *
 * On IA64/x64 there is only a single calling convention. Calling convention
 * keywords are ignored.
 */

#if defined(_M_IX86)

# define STLSOFT_CF_THISCALL_SUPPORTED
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

#elif defined(unix) || \
      defined(__unix__)

# define STLSOFT_CF_CDECL_SUPPORTED

# define STLSOFT_CDECL

#else /* ? arch */
# error Only defined for the Intel x86, IA64 and x64 architectures
#endif /* arch */

/* /////////////////////////////////////////////////////////////////////////
 * Integer sizes
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
# error Use of Intel C/C++ has not been verified on any operation system other than Win32. Please contact Synesis Software
#endif /* data model */

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
#define STLSOFT_SI08_T_BASE_TYPE    signed      __int8
#define STLSOFT_UI08_T_BASE_TYPE    unsigned    __int8
#if _MSC_VER == 1200
# define STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
#endif /* _MSC_VER */

/* 16-bit integer */
#define STLSOFT_CF_16BIT_INT_SUPPORT
#define STLSOFT_SI16_T_BASE_TYPE    signed      __int16
#define STLSOFT_UI16_T_BASE_TYPE    unsigned    __int16
#if _MSC_VER == 1200
# define STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
#endif /* _MSC_VER */

/* 32-bit integer */
#define STLSOFT_CF_32BIT_INT_SUPPORT
#define STLSOFT_SI32_T_BASE_TYPE    signed      __int32
#define STLSOFT_UI32_T_BASE_TYPE    unsigned    __int32
#if _MSC_VER == 1200
# define STLSOFT_CF_INT_DISTINCT_INT_TYPE
#endif /* _MSC_VER */
#define STLSOFT_CF_LONG_DISTINCT_INT_TYPE

/* 64-bit integer */
#define STLSOFT_CF_64BIT_INT_SUPPORT
#define STLSOFT_CF_64BIT_INT_IS___int64
#define STLSOFT_SI64_T_BASE_TYPE    signed      __int64
#define STLSOFT_UI64_T_BASE_TYPE    unsigned    __int64

/* /////////////////////////////////////////////////////////////////////////
 * Still to-be-determined features
 */

#if defined(_MSC_VER)
# if _MSC_VER >= 1300
  /* Even though VC 7.0 and 7.1 provide a native wchar_t type, that is __wchar_t,
   * it is not compatible with their libraries (which use the typedef wchar_t),
   * so we cannot use it.
   *
   * wchar_t itself may be used, when _NATIVE_WCHAR_T_DEFINED is defined
   */
#  ifdef _NATIVE_WCHAR_T_DEFINED
#   define STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#  elif defined(_WCHAR_T_DEFINED)
#   define STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
#  else
   /* Not defined */
#   define STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
#  endif /* _WCHAR_T_DEFINED */
# else
  /* Previous versions do not have a native type, but do have the typedef wchar_t
   * when _WCHAR_T_DEFINED is defined
   */
#  if defined(_WCHAR_T_DEFINED)
#   define STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
#  else
   /* Not defined */
#  endif /* _WCHAR_T_DEFINED */
# endif /* _MSC_VER */
#endif /* _MSC_VER */

#define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED

#define STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT

#define STLSOFT_CF_std_NAMESPACE

#define STLSOFT_CF_std_char_traits_AVAILABLE

#define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_NON_TEMPLATE
#define STLSOFT_CF_PARENT_TYPES_CAN_BE_USED_IN_TEMPLATE

#define STLSOFT_CF_VENEER_SUPPORT

#if !defined(_ATL_MIN_CRT)
# define STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE
#endif /* !_ATL_MIN_CRT */

/*
# define STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR
*/
/*
# define STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED
*/

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
 //#define   __STLSOFT_CF_USE_cassert
# ifdef _MSC_VER
#  if defined(__cplusplus) && \
      _MSC_VER >= 1200
#   include <new> /* Used to ensure that the headers don't get confused between <new> and
                   * <crtdbg.h>, which results in balking on exception specifications on op new
                   */
#  endif /* C++ && _MSC_VER >= 1300 */
#  define __STLSOFT_CF_ASSERT_INCLUDE_NAME      <crtdbg.h>
#  define STLSOFT_ASSERT(expr)                  _ASSERTE(expr)
# else /* _MSC_VER */
#  define __STLSOFT_CF_ASSERT_INCLUDE_NAME      <assert.h>
#  define STLSOFT_ASSERT(expr)                  assert(expr)
# endif /* _MSC_VER */
#endif /* _STLSOFT_CUSTOM_ASSERT */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler warning suppression
 */

#ifdef _DEBUG
/* Suppresses: "expression has no effect" */
# pragma warning(disable : 174)
#endif /* _DEBUG */

/* Suppresses: "controlling expression is constant" */
#pragma warning(disable : 279)

#ifdef _DEBUG
/* Suppresses: "value copied to temporary, reference to temporary used" */
# pragma warning(disable : 383)
#endif /* _DEBUG */

/* Suppresses: "operands are evaluated in unspecified order" */
#pragma warning(disable : 981)

/* /////////////////////////////////////////////////////////////////////////
 * Obsolete features
 */

#include <stlsoft/internal/cccap/obsolete.h>

/* ///////////////////////////// end of file //////////////////////////// */
