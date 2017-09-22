/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/functional/procedure_adaptors.hpp
 *
 * Purpose:     Contains the adaptors to allow functions to be used as procedures in algorithms.
 *
 * Created:     13th June 1999
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/functional/procedure_adaptors.hpp
 *
 * \brief [C++ only] Function classes that adapt non-member procedures (and
 *   handle different calling conventions)
 *   (\ref group__library__functional "Functional" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS
#define STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS_MAJOR       2
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS_MINOR       0
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS_REVISION    2
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS_EDIT        14
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_FUNCTIONAL
# define STLSOFT_INCL_FUNCTIONAL
# include <functional>
#endif /* !STLSOFT_INCL_FUNCTIONAL */

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

/** \brief Adapts a unary function into a unary procedure - one in which the return
 * type of the function call operator is void
 *
 * \ingroup group__library__functional
 */
// [[synesis:class:function-class:unary-function: unary_procedure_adaptor<T<F>>]]
template <ss_typename_param_k F>
struct unary_procedure_adaptor
    : public stlsoft_ns_qual_std(unary_function)<   ss_typename_type_k F::argument_type
                                                ,   void>
{
private:
    typedef F                                   adapted_function_type;
public:
    typedef ss_typename_type_k F::argument_type argument_type;
    typedef void                                result_type;

public:
    unary_procedure_adaptor(adapted_function_type func)
        : m_func(func)
    {}

public:
    void operator ()(argument_type arg) const
    {
        static_cast<void>(m_func(arg));
    }

private:
    adapted_function_type   m_func;
};

/** \brief Adapts a biary function into a unary procedure - one in which the return
 * type of the function call operator is void
 *
 * \ingroup group__library__functional
 */
// [[synesis:class:function-class:unary-function: binary_procedure_adaptor<T<F>>]]
template <ss_typename_param_k F>
struct binary_procedure_adaptor
    : public std::binary_function<  ss_typename_type_k F::first_argument_type
                                ,   ss_typename_type_k F::second_argument_type
                                ,   void>
{
private:
    typedef F                                           adapted_function_type;
public:
    typedef ss_typename_type_k F::first_argument_type   first_argument_type;
    typedef ss_typename_type_k F::second_argument_type  second_argument_type;
    typedef void                                        result_type;

public:
    binary_procedure_adaptor(adapted_function_type func)
        : m_func(func)
    {}

public:
    void operator ()(first_argument_type arg1, second_argument_type arg2) const
    {
        static_cast<void>(m_func(arg1, arg2));
    }

private:
    adapted_function_type   m_func;
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

/** \brief Creator function for the unary_procedure_adaptor
 *
 * \ingroup group__library__functional
 */
template <ss_typename_param_k F>
inline unary_procedure_adaptor<F> adapt_unary_procedure(F func)
{
    return unary_procedure_adaptor<F>(func);
}

/** \brief Creator function for the binary_procedure_adaptor
 *
 * \ingroup group__library__functional
 */
template <ss_typename_param_k F>
inline binary_procedure_adaptor<F> adapt_binary_procedure(F func)
{
    return binary_procedure_adaptor<F>(func);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_PROCEDURE_ADAPTORS */

/* ///////////////////////////// end of file //////////////////////////// */
