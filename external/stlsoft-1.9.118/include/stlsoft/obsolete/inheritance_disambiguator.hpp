/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/inheritance_disambiguator.hpp
 *
 * Purpose:     Class to disambiguate between parent classes of same type.
 *
 * Created:     1st September 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/// \file stlsoft/obsolete/inheritance_disambiguator.hpp
///
/// Class to disambiguate between parent classes of same type.

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR_MAJOR      3
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR_MINOR      1
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR_REVISION   1
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR_EDIT       25
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
 * Classes
 */

// class inheritance_disambiguator

/** \brief This class is used to disambiguate base classes, thereby faciliating multiple inheritance from the same type
 *
 * \ingroup group__library__obsolete
 *
 * \param T The type to disambiguate
 * \param N A unique integer index used to disambiguate between the classes
 */
template <ss_typename_param_k T, int N>
class inheritance_disambiguator
    : public T
{
public:
    /// The disambiguated class type
    typedef T                               parent_class_type;
    enum { unique = N };
    /// The type of the current parameterisation
    typedef inheritance_disambiguator<T, N> class_type;

// Construction
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    // For compilers that support member templates, the following constructors
    // are provided.

    /// Default constructor
    inheritance_disambiguator()
    {}

    /// Single parameter constructor
    template <ss_typename_param_k N1>
    ss_explicit_k inheritance_disambiguator(N1 n1)
        : parent_class_type(n1)
    {}

    /// Two parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2>
    inheritance_disambiguator(N1 n1, N2 n2)
        : parent_class_type(n1, n2)
    {}

    /// Three parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3>
    inheritance_disambiguator(N1 n1, N2 n2, N3 n3)
        : parent_class_type(n1, n2, n3)
    {}

    /// Four parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4>
    inheritance_disambiguator(N1 n1, N2 n2, N3 n3, N4 n4)
        : parent_class_type(n1, n2, n3, n4)
    {}

    /// Five parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5>
    inheritance_disambiguator(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5)
        : parent_class_type(n1, n2, n3, n4, n5)
    {}

    /// Six parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5, ss_typename_param_k N6>
    inheritance_disambiguator(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5, N6 n6)
        : parent_class_type(n1, n2, n3, n4, n5, n6)
    {}

    /// Seven parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5, ss_typename_param_k N6, ss_typename_param_k N7>
    inheritance_disambiguator(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5, N6 n6, N7 n7)
        : parent_class_type(n1, n2, n3, n4, n5, n6, n7)
    {}

    /// Eight parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5, ss_typename_param_k N6, ss_typename_param_k N7, ss_typename_param_k N8>
    inheritance_disambiguator(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5, N6 n6, N7 n7, N8 n8)
        : parent_class_type(n1, n2, n3, n4, n5, n6, n7, n8)
    {}
#endif // STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_INHERITANCE_DISAMBIGUATOR */

/* ///////////////////////////// end of file //////////////////////////// */
