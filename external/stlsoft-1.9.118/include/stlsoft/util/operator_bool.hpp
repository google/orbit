/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/operator_bool.hpp
 *
 * Purpose:     A robust and portable operator bool generator class.
 *
 * Created:     5th November 2003
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


/** \file stlsoft/util/operator_bool.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::operator_bool_generator
 *   class template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPERATOR_BOOL_MAJOR    4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPERATOR_BOOL_MINOR    0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPERATOR_BOOL_REVISION 1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPERATOR_BOOL_EDIT     36
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

/** \brief \def STLSOFT_DEFINE_OPERATOR_BOOL_TYPES(U, G, B)
 * Defines the types for a type-safe boolean operator
 *
 * \ingroup group__library__utility
 *
 * \param U The Unique type. This is usually the \c class_type member type of the
 * class for which we are providing the boolean operator, but it could be any type.
 * \param G The Generator type. This is the type you use to qualify your call
 * to translate() inside the boolean operator
 * \param B The Boolean type. This is the type of the boolean operator
 */

# define STLSOFT_DEFINE_OPERATOR_BOOL_TYPES(U, G, B)                                                \
                                                                                                    \
    typedef stlsoft_ns_qual(operator_bool_generator)<U>::class_type   G;                            \
    typedef G::return_type                                            B

/** \brief \def STLSOFT_DEFINE_OPERATOR_BOOL_TYPES(U, G, B)
 * Defines the types for a type-safe boolean operator, for use in templates
 *
 * \ingroup group__library__utility
 *
 * \param U The Unique type. This is usually the \c class_type member type of the
 * class for which we are providing the boolean operator, but it could be any type.
 * \param G The Generator type. This is the type you use to qualify your call
 * to translate() inside the boolean operator
 * \param B The Boolean type. This is the type of the boolean operator
 *
 * \note This is identical in form and purpose to STLSOFT_DEFINE_OPERATOR_BOOL_TYPES(),
 * except that it must be used for implementing operator boolean for templates
 */

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1300)

# define STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(U, G, B)                                              \
                                                                                                    \
    STLSOFT_DEFINE_OPERATOR_BOOL_TYPES(U, G, B)

#else /* ? compiler */

# define STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(U, G, B)                                              \
                                                                                                    \
    typedef ss_typename_param_k stlsoft_ns_qual(operator_bool_generator)<U>::class_type G;          \
    typedef ss_typename_param_k G::return_type                                          B

#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Template which provides the types and conversion operations for safe and
 * highly-portable "<code>operator bool() const</code>".
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
struct operator_bool_generator
{
public:
    typedef operator_bool_generator<T>  class_type;

#ifdef STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
    typedef int class_type::*return_type;

    /// Returns the value representing the true condition
    static return_type true_value()
    {
        return &class_type::i;
    }

private:
    int i;
public:
#else
    typedef class_type const* return_type;

    /// Returns the value representing the true condition
    static return_type true_value()
    {
        class_type  t;
        void        *p = static_cast<void*>(&t);

        return static_cast<return_type>(p);
    }
#endif // STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT

    /// Returns the value representing the false condition
    static return_type false_value()
    {
        return static_cast<return_type>(0);
    }

    /// Does the ternary operator for you
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k U>
    static return_type translate(U b)
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static return_type translate(ss_bool_t b)
#endif // STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    {
        return b ? true_value() : false_value();
    }

private:
    void operator delete(void*);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL */

/* ///////////////////////////// end of file //////////////////////////// */
