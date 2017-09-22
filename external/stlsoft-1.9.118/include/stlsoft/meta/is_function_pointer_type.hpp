/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/is_function_pointer_type.hpp
 *
 * Purpose:     is_function_pointer_type meta class.
 *
 * Created:     4th November 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/meta/is_function_pointer_type.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::is_function_pointer_type meta class
 *  template
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE
#define STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE_MAJOR    1
# define STLSOFT_VER_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE_MINOR    1
# define STLSOFT_VER_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE_REVISION 3
# define STLSOFT_VER_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE_EDIT     10
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES
# include <stlsoft/meta/n_types.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES */
#ifdef STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF
#  include <stlsoft/meta/select_first_type_if.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF */
#endif /* STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#ifndef STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_
# include <stlsoft/meta/util/meta_.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_UTIL_HPP_META_ */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

template <ss_typename_param_k R>
one_type is_function_pointer_type_func(R (*)());

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        >
one_type is_function_pointer_type_func(R (*)(A0));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
one_type is_function_pointer_type_func(R (*)(A0, A1));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        ,   ss_typename_param_k A35
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        ,   ss_typename_param_k A35
        ,   ss_typename_param_k A36
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        ,   ss_typename_param_k A35
        ,   ss_typename_param_k A36
        ,   ss_typename_param_k A37
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        ,   ss_typename_param_k A35
        ,   ss_typename_param_k A36
        ,   ss_typename_param_k A37
        ,   ss_typename_param_k A38
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        ,   ss_typename_param_k A35
        ,   ss_typename_param_k A36
        ,   ss_typename_param_k A37
        ,   ss_typename_param_k A38
        ,   ss_typename_param_k A39
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38, A39));

template<   ss_typename_param_k R
        ,   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        ,   ss_typename_param_k A2
        ,   ss_typename_param_k A3
        ,   ss_typename_param_k A4
        ,   ss_typename_param_k A5
        ,   ss_typename_param_k A6
        ,   ss_typename_param_k A7
        ,   ss_typename_param_k A8
        ,   ss_typename_param_k A9
        ,   ss_typename_param_k A10
        ,   ss_typename_param_k A11
        ,   ss_typename_param_k A12
        ,   ss_typename_param_k A13
        ,   ss_typename_param_k A14
        ,   ss_typename_param_k A15
        ,   ss_typename_param_k A16
        ,   ss_typename_param_k A17
        ,   ss_typename_param_k A18
        ,   ss_typename_param_k A19
        ,   ss_typename_param_k A20
        ,   ss_typename_param_k A21
        ,   ss_typename_param_k A22
        ,   ss_typename_param_k A23
        ,   ss_typename_param_k A24
        ,   ss_typename_param_k A25
        ,   ss_typename_param_k A26
        ,   ss_typename_param_k A27
        ,   ss_typename_param_k A28
        ,   ss_typename_param_k A29
        ,   ss_typename_param_k A30
        ,   ss_typename_param_k A31
        ,   ss_typename_param_k A32
        ,   ss_typename_param_k A33
        ,   ss_typename_param_k A34
        ,   ss_typename_param_k A35
        ,   ss_typename_param_k A36
        ,   ss_typename_param_k A37
        ,   ss_typename_param_k A38
        ,   ss_typename_param_k A39
        ,   ss_typename_param_k A40
        >
one_type is_function_pointer_type_func(R (*)(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31, A32, A33, A34, A35, A36, A37, A38, A39, A40));

two_type is_function_pointer_type_func(...);

/** \brief Traits type used to determine whether the given type is a
 *   function pointer.
 *
 * \ingroup group__library__meta
 */
template <ss_typename_param_k T>
struct is_function_pointer_type
{
    typedef T   test_type;

private:
    static T    t;
public:
    enum { value = sizeof(is_function_pointer_type_func(t)) == sizeof(one_type) };

#ifdef STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF
    typedef ss_typename_type_k select_first_type_if<yes_type, no_type, value>::type     type;
#endif /* STLSOFT_META_HAS_SELECT_FIRST_TYPE_IF */
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE */

/* ///////////////////////////// end of file //////////////////////////// */
