/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/functional/type_functionals.hpp
 *
 * Purpose:     Type-specific predicates and functions.
 *
 * Created:     14th June 2002
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


/** \file comstl/functional/type_functionals.hpp
 *
 * \brief [C++ only] Function classes for releasing instances of
 *   COM-specific resource types
 *   (\ref group__library__functional "Functional" Library).
 */

#ifndef COMSTL_INCL_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS
#define COMSTL_INCL_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS_MAJOR    4
# define COMSTL_VER_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS_MINOR    0
# define COMSTL_VER_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS_REVISION 2
# define COMSTL_VER_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS_EDIT     65
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
STLSOFT_COMPILER_IS_GCC:     __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */

#ifndef STLSOFT_INCL_FUNCTIONAL
# define STLSOFT_INCL_FUNCTIONAL
# include <functional>
#endif /* !STLSOFT_INCL_FUNCTIONAL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functionals
 */

/** \brief A function class that closes BSTRs
 *
 * \ingroup group__library__functional
 *
 * This function class closes BSTRs by applying
 * SysFreeString() to them
 */
// [[synesis:class:unary-functor: BSTR_close]]
struct BSTR_close
    : public comstl_ns_qual_std(unary_function)<BSTR, void>
{
    /// \brief Function call operator
    ///
    /// \param s The string to free
    void operator ()(BSTR s) const
    {
        ::SysFreeString(s);
    }
};

/** \brief A function class that closes OLE strings
 *
 * \ingroup group__library__functional
 *
 * This function class closes OLE strings (LPOLESTR) by returning
 * their memory back to the COM task allocator
 */
// [[synesis:class:unary-functor: olestring_close]]
struct olestring_close
    : public comstl_ns_qual_std(unary_function)<LPOLESTR, void>
{
    /// \brief Function call operator
    ///
    /// \param s The string to free
    void operator ()(LPOLESTR s) const
    {
        ::CoTaskMemFree(s);
    }
};

/** \brief A function class that closes task memory blocks
 *
 * \ingroup group__library__functional
 *
 * This function class closes task memory blocks by returning
 * their memory back to the COM task allocator
 */
// [[synesis:class:unary-functor: task_memory_close]]
struct task_memory_close
    : public comstl_ns_qual_std(unary_function)<void *, void>
{
    /// \brief Function call operator
    ///
    /// \param p The memory to free
    void operator ()(void *p) const
    {
        ::CoTaskMemFree(p);
    }
};

/** \brief A function class that closes VARIANTs
 *
 * \ingroup group__library__functional
 *
 * This function class closes VARIANTs by applying
 * VariantClear() to them
 */
// [[synesis:class:unary-functor: VARIANT_close]]
struct VARIANT_close
    : public comstl_ns_qual_std(unary_function)<VARIANT&, void>
{
    /// \brief Function call operator
    ///
    /// \param v The variant to free
    void operator ()(VARIANT &v) const
    {
        ::VariantClear(&v);
    }
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/type_functionals_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_FUNCTIONAL_HPP_TYPE_FUNCTIONALS */

/* ///////////////////////////// end of file //////////////////////////// */
