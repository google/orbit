/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/_undefs.h
 *
 * Purpose:     The #undefs for stlsoft/stlsoft.h.
 *
 * Created:     1st December 2003
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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

/** \file stlsoft/internal/_undefs.h
 *
 * The \#undefs for stlsoft/stlsoft.h.
 * (\ref group__library__internal)
 */

#ifdef STLSOFT_INCL_H_STLSOFT__UNDEFS
# error This file cannot be included more than once in any compilation unit
#endif /* STLSOFT_INCL_H_STLSOFT__UNDEFS */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT__UNDEFS_MAJOR    2
# define STLSOFT_VER_H_STLSOFT__UNDEFS_MINOR    8
# define STLSOFT_VER_H_STLSOFT__UNDEFS_REVISION 1
# define STLSOFT_VER_H_STLSOFT__UNDEFS_EDIT     27
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * #undefs
 */

/* Messaging
 *
 * Discriminated symbol is STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT
 */

#ifdef STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT
# define STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT
#endif /* STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT */

/* Template support.
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_SUPPORT
 */

#ifdef STLSOFT_CF_TEMPLATE_SUPPORT
# undef STLSOFT_CF_TEMPLATE_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_SUPPORT */

/* Variadic Macros
 *
 * Discriminated symbol is STLSOFT_CF_SUPPORTS_VARIADIC_MACROS
 */

#ifdef STLSOFT_CF_SUPPORTS_VARIADIC_MACROS
# define STLSOFT_CF_SUPPORTS_VARIADIC_MACROS
#endif /* STLSOFT_CF_SUPPORTS_VARIADIC_MACROS */

/* Exception signature support.
 *
 * Discriminated symbol is STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
 */
#ifdef STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
# undef STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
#endif /* STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT */

/* Native bool support.
 *
 * Discriminated symbol is STLSOFT_CF_NATIVE_BOOL_SUPPORT
 */
#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
# undef STLSOFT_CF_NATIVE_BOOL_SUPPORT
#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */

/* Native / typedef'd wchar_t support.
 *
 * Discriminated symbols are STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
 *                           STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
 *
 * Implementation symbol is STLSOFT_NATIVE_WCHAR_T
 */
#ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
# undef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

#ifdef STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
# undef STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
#endif /* STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT */

#ifdef STLSOFT_NATIVE_WCHAR_T
# undef STLSOFT_NATIVE_WCHAR_T
#endif /* STLSOFT_NATIVE_WCHAR_T */

/* 8-bit, 16-bit, 32-bit, 64-bit type support
 */

#ifdef STLSOFT_CF_8BIT_INT_SUPPORT
# undef STLSOFT_CF_8BIT_INT_SUPPORT
#endif /* STLSOFT_CF_8BIT_INT_SUPPORT */

#ifdef STLSOFT_SI08_T_BASE_TYPE
# undef STLSOFT_SI08_T_BASE_TYPE
#endif /* STLSOFT_SI08_T_BASE_TYPE */
#ifdef STLSOFT_UI08_T_BASE_TYPE
# undef STLSOFT_UI08_T_BASE_TYPE
#endif /* STLSOFT_UI08_T_BASE_TYPE */

#ifdef STLSOFT_CF_16BIT_INT_SUPPORT
# undef STLSOFT_CF_16BIT_INT_SUPPORT
#endif /* STLSOFT_CF_16BIT_INT_SUPPORT */

#ifdef STLSOFT_SI16_T_BASE_TYPE
# undef STLSOFT_SI16_T_BASE_TYPE
#endif /* STLSOFT_SI16_T_BASE_TYPE */
#ifdef STLSOFT_UI16_T_BASE_TYPE
# undef STLSOFT_UI16_T_BASE_TYPE
#endif /* STLSOFT_UI16_T_BASE_TYPE */

#ifdef STLSOFT_CF_32BIT_INT_SUPPORT
# undef STLSOFT_CF_32BIT_INT_SUPPORT
#endif /* STLSOFT_CF_32BIT_INT_SUPPORT */

#ifdef STLSOFT_SI32_T_BASE_TYPE
# undef STLSOFT_SI32_T_BASE_TYPE
#endif /* STLSOFT_SI32_T_BASE_TYPE */
#ifdef STLSOFT_UI32_T_BASE_TYPE
# undef STLSOFT_UI32_T_BASE_TYPE
#endif /* STLSOFT_UI32_T_BASE_TYPE */

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
# undef STLSOFT_CF_64BIT_INT_SUPPORT
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

#ifdef STLSOFT_SI64_T_BASE_TYPE
# undef STLSOFT_SI64_T_BASE_TYPE
#endif /* STLSOFT_SI64_T_BASE_TYPE */
#ifdef STLSOFT_UI64_T_BASE_TYPE
# undef STLSOFT_UI64_T_BASE_TYPE
#endif /* STLSOFT_UI64_T_BASE_TYPE */

/* Distinct type support
 *
 */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
# undef STLSOFT_CF_INT_DISTINCT_INT_TYPE
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
# undef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

/* #ifdef STLSOFT_CF_NATIVE_INT_8_ */

/* Compiler supports member constants.
 *
 * Discriminated symbol is STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
 */
#ifdef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
# undef STLSOFT_CF_MEMBER_CONSTANT_SUPPORT
#endif /* STLSOFT_CF_MEMBER_CONSTANT_SUPPORT */

/* Compiler supports static assert.
 *
 * Discriminated symbol is STLSOFT_CF_STATIC_ASSERT_SUPPORT
 */
#ifdef STLSOFT_CF_STATIC_ASSERT_SUPPORT
# undef STLSOFT_CF_STATIC_ASSERT_SUPPORT
#endif /* STLSOFT_CF_STATIC_ASSERT_SUPPORT */

/* Function signature requires full-qualification.
 *
 * Discriminated symbol is STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED
 */
#ifdef STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED
# undef STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */

/* RTTI support.
 *
 * Discriminated symbol is STLSOFT_CF_RTTI_SUPPORT
 */
#ifdef STLSOFT_CF_RTTI_SUPPORT
# undef STLSOFT_CF_RTTI_SUPPORT
#endif /* STLSOFT_CF_RTTI_SUPPORT */

/* Exception support.
 *
 * Discriminated symbol is STLSOFT_CF_EXCEPTION_SUPPORT
 */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# undef STLSOFT_CF_EXCEPTION_SUPPORT
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

/* Template class default fundamental type argument support
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
 *
 * Microsoft Visual C++ 4.2 does not support template default fundamental type arguments.
 */
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
# undef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */

/* Template class default class type argument support
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
 *
 * Microsoft Visual C++ 4.2 does not support template default class type arguments.
 */
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
# undef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */

/* Member functions can appear as template parameters
 *
 * Discriminated symbol is STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
 */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
# undef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

/* Member function template support.
 *
 * Discriminated symbol is STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
 */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
# undef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
#endif // STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT

/* Member constructor template support.
 *
 * Discriminated symbol is STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
 */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
# undef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

/* Member template range method support.
 *
 * Discriminated symbol is STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT
 */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT
# undef STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT
#endif // STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT

/* Member template class support.
 *
 * Discriminated symbol is STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT
 */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT
# undef STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CLASS_SUPPORT

/* Template specialisation syntax support
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX
 */
#ifdef STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX
# undef STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX
#endif /* STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX */

/* Template partial specialisation support.
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
 */
#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
# undef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
#endif // STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

/* Template out-of-class function specialisation support.
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
 */
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
# undef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */

/* Standard library STL elements in std namespace.
 *
 * Discriminated symbol is STLSOFT_CF_std_NAMESPACE
 */
#ifdef STLSOFT_CF_std_NAMESPACE
# undef STLSOFT_CF_std_NAMESPACE
#endif /* STLSOFT_CF_std_NAMESPACE */

/* std::char_traits available.
 *
 * Discriminated symbol is STLSOFT_CF_std_char_traits_AVAILABLE
 */
#ifdef STLSOFT_CF_std_char_traits_AVAILABLE
# undef STLSOFT_CF_std_char_traits_AVAILABLE
#endif /* STLSOFT_CF_std_char_traits_AVAILABLE */

/* explicit keyword support
 *
 * Discriminated symbol is STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT
 */
#ifdef STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT
# undef STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT */

/* mutable keyword support
 *
 * Discriminated symbol is STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
 */
#ifdef STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
# undef STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */

/* typename keyword support
 *
 * Discriminated symbols are STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT,
 * STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT,
 * STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT
 * STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT and
 * STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT
 *
 * The typename keyword is actually used for two distinct purposes: the
 * generic type placeholder in template parameter specifications, and the
 * stipulation to compilers that a particular template derived construct
 * is a type, rather than a member or operation.
 *
 * These two uses have varying support on different compilers, hence the
 * STLSoft libraries utilise the ss_typename_param_k pseudo keyword for the
 * first purpose, and the ss_typename_type_k pseudo keyword for the second.
 *
 * In addition, some compilers cannot handle the use of typename as a type
 * qualifier in a template default parameter, so we further define the keyword
 * ss_typename_type_def_k. And some cannot handle it in a constructor
 * initialiser list, for which ss_typename_type_mil_k is defined.
 */
#ifdef STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT
# undef STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT */

#ifdef STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT
# undef STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT */

#ifdef STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT
# undef STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT */

#ifdef STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT
# undef STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT */

#ifdef STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT
# undef STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT
#endif /* STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT */

/* Move constructor support
 *
 * Discriminated symbol is STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
 */
#ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
# undef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

/* Koening Lookup support
 *
 * Discriminated symbol is STLSOFT_CF_ADL_LOOKUP_SUPPORT
 */
#ifdef STLSOFT_CF_ADL_LOOKUP_SUPPORT
# undef STLSOFT_CF_ADL_LOOKUP_SUPPORT
#endif /* STLSOFT_CF_ADL_LOOKUP_SUPPORT */

/* Template template support
 *
 * Discriminated symbol is STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT
 */
#ifdef STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT
# undef STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT
#endif /* STLSOFT_CF_TEMPLATE_TEMPLATE_SUPPORT */


#ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
# undef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
#endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

#ifdef STLSOFT_CF_VENEER_SUPPORT
# undef STLSOFT_CF_VENEER_SUPPORT
#endif /* STLSOFT_CF_VENEER_SUPPORT */

#ifdef STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED
# undef STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED
#endif /* STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED */

#ifdef STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT
# undef STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT
#endif /* STLSOFT_CF_NEGATIVE_MODULUS_POSITIVE_GIVES_NEGATIVE_RESULT */

#ifdef STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
# undef STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
#endif /* STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT */

#ifdef STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT
# undef STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT
#endif /* STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT */

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
# undef STLSOFT_CF_FASTCALL_SUPPORTED
#endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
# undef STLSOFT_CF_STDCALL_SUPPORTED
#endif /* STLSOFT_CF_STDCALL_SUPPORTED */

#ifdef STSLSOFT_INLINE_ASM_SUPPORTED
# undef STSLSOFT_INLINE_ASM_SUPPORTED
#endif /* STSLSOFT_INLINE_ASM_SUPPORTED */

#ifdef STSLSOFT_ASM_IN_INLINE_SUPPORTED
# undef STSLSOFT_ASM_IN_INLINE_SUPPORTED
#endif /* STSLSOFT_ASM_IN_INLINE_SUPPORTED */

/* ///////////////////////////// end of file //////////////////////////// */
