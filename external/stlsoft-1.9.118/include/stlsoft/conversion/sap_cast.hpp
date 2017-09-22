/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/sap_cast.hpp
 *
 * Purpose:     A cast operator function that casts between non void pointers of
 *              the same cv-qualification.
 *
 * Created:     25th February 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/conversion/sap_cast.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::sap_cast cast function
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_SAP_CAST_MAJOR      4
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_SAP_CAST_MINOR      0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_SAP_CAST_REVISION   2
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_SAP_CAST_EDIT       46
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if (   defined(STLSOFT_COMPILER_IS_GCC) && \
        __GNUC__ < 3) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1200) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
# define STLSOFT_NO_SAP_CAST
#endif /* compiler */

#if !defined(STLSOFT_NO_SAP_CAST) && \
    defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
# ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
#  include <stlsoft/util/constraints.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_BASE_TYPE_TRAITS
#  include <stlsoft/meta/base_type_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_BASE_TYPE_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF
#  include <stlsoft/meta/select_first_type_if.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF */
#endif /* !STLSOFT_NO_SAP_CAST && STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** \brief A cast operator function that casts between non void pointers of
 *   the same cv-qualification.
 *
 * \ingroup group__library__conversion
 *
 * The implementation of the operator uses compile-time enforcement of
 * various constraints to ensure that:
 *
 * - the FROM and TO types are both pointers
 * - no cv-qualifiers are stripped from the FROM type
 *
 * For example, this cast is allowed:
\code
int*          pi = . . .;
short const*  ps = stlsoft::sap_cast<short const*>(pi);
\endcode
 *
 * but this cast is not:
\code
int const*    pi = . . .;
short*        ps = stlsoft::sap_cast<short*>(pi);
\endcode
 *
 * \param from The pointer to cast from.
 *
 * \remarks The cast operator was inspired by an item in the
 *   Sutter/Alexandrescu coding standards book, and its names stands for
 *   Sutter-Alexandrescu-Pointer cast. The acronym is overloaded, since one
 *   might also be said to be a sap if one made injudicious use of the cast,
 *   due to its inherent dangers.
 */
#if defined(STLSOFT_NO_SAP_CAST)
# define    sap_cast    reinterpret_cast
#else /* ? STLSOFT_NO_SAP_CAST */
template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline TO sap_cast(FROM from)
{
# if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    (   !defined(STLSOFT_COMPILER_IS_BORLAND))
    // Both types must be pointer types
    STLSOFT_STATIC_ASSERT(0 != base_type_traits<FROM>::is_pointer);
    STLSOFT_STATIC_ASSERT(0 != base_type_traits<TO>::is_pointer);

    typedef ss_typename_type_k base_type_traits<FROM>::base_type    from_base_type;
    typedef ss_typename_type_k base_type_traits<TO>::base_type      to_base_type;

    // The intermediate type might be void *, void const*, void volatile * or
    // void const volatile *
    typedef ss_typename_type_k select_first_type_if<void const*
                                                ,   void*
                                                ,   base_type_traits<FROM>::is_const
                                                >::type     non_volatile_type;
    typedef ss_typename_type_k select_first_type_if<void const volatile*
                                                ,   void volatile*
                                                ,   base_type_traits<FROM>::is_const
                                                >::type     volatile_type;
    typedef ss_typename_type_k select_first_type_if<volatile_type
                                                ,   non_volatile_type
                                                ,   base_type_traits<FROM>::is_volatile
                                                >::type     pointer_type;

    // "static_cast" to void (const) (volatile) *
    pointer_type        pv  =   from;
# else /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
    void const volatile *p1 =   from;
    void                *pv =   const_cast<void*>(p1);
# endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

    // static_cast to destination type
    return static_cast<TO>(pv);
}
#endif /* STLSOFT_NO_SAP_CAST */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/sap_cast_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
