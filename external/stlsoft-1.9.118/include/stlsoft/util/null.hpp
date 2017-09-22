/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/null.hpp (stlsoft_null.h
 *
 * Purpose:     NULL_v template class. Include stlsoft_nulldef.h for NULL to be
 *              automatically the strong NULL.
 *
 * Created:     8th September 2002
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


/** \file stlsoft/util/null.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::NULL_v class
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_NULL
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_NULL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_NULL_MAJOR     4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_NULL_MINOR     0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_NULL_REVISION  1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_NULL_EDIT      50
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* _STLSOFT_NULL_v_DEFINED */

#ifdef _STLSOFT_NULL_v_DEFINED
# undef _STLSOFT_NULL_v_DEFINED
#endif /* _STLSOFT_NULL_v_DEFINED */

#define _STLSOFT_NULL_v_DEFINED

#if defined(STLSOFT_COMPILER_IS_DMC) && \
    __DMC__ < 0x0840
# undef _STLSOFT_NULL_v_DEFINED
#elif defined(STLSOFT_COMPILER_IS_MSVC) && \
      _MSC_VER < 1310
# undef _STLSOFT_NULL_v_DEFINED
#elif defined(STLSOFT_COMPILER_IS_WATCOM)
# undef _STLSOFT_NULL_v_DEFINED
#endif /* compiler */

/* _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT */

#ifdef _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT
# undef _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT
#endif /* _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT */

#define _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT

#if defined(STLSOFT_COMPILER_IS_GCC)
# undef _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT
#elif defined(STLSOFT_COMPILER_IS_MWERKS)
# undef _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT
#endif /* compiler */

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

#ifdef _STLSOFT_NULL_v_DEFINED

/** \brief Represents a type that can be an active replacement for NULL
 *
 * \ingroup group__library__utility
 *
 * This class can act as a replacement for the NULL macro, by being validly
 * assigned to or equated with pointer types only, as in
 *
 *   int   i = NULL; // error
 *   int   *p = NULL; // OK
 *
 *   if(i == NULL) {} // error
 *   if(NULL == i) {} // error
 *
 *   if(p == NULL) {} // OK
 *   if(NULL == p) {} // OK
 *
 *
 * When used via inclusion of the file stlsoft_nulldef.h, the macro NULL is
 * redefined as NULL_v(), such that expressions containing NULL will be valid
 * against pointers only.
 */
struct NULL_v
{
// Construction
public:
    /// Default constructor
    NULL_v()
    {}

/** \brief Static creation
 *
 * \ingroup group__library__utility
 */
public:
    static NULL_v create()
    {
        return NULL_v();
    }

// Conversion
public:
    /// Implicit conversion operator (convertible to any pointer type)
    template <ss_typename_param_k T>
    operator T *() const
    {
        return 0;
    }

#ifdef _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT
    /// Implicit conversion operator (convertible to any pointer type)
    template<   ss_typename_param_k T2
            ,   ss_typename_param_k C
            >
    operator T2 C::*() const
    {
        return 0;
    }
#endif /* _STLSOFT_NULL_v_DEFINED_PTR_TO_MEMBER_SUPPORT */

    /// Evaluates whether an instance of a type is null
    ///
    /// \param rhs A reference arbitrary type which will be compared to null
    template <ss_typename_param_k T>
    ss_bool_t equal(T const& rhs) const
    {
        return rhs == 0;
    }
    /// \deprecated
    template <ss_typename_param_k T>
    ss_bool_t equals(T const& rhs) const
    {
        return equal(rhs);
    }

// Not to be implemented
private:
    void operator &() const;

#if !defined(STLSOFT_COMPILER_IS_GCC)
    NULL_v(NULL_v const&);
    NULL_v const& operator =(NULL_v const&);
#endif /* compiler */
};

#if 0
/** \brief operator == for NULL_v and an arbitrary type
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
inline ss_bool_t operator ==(NULL_v const& lhs, T const& rhs)
{
    STLSOFT_STATIC_ASSERT(sizeof(rhs) == 4);

    return lhs.equal(rhs);
}

/** \brief operator == for an arbitrary type and NULL_v
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
inline ss_bool_t operator ==(T const& lhs, NULL_v const& rhs)
{
    STLSOFT_STATIC_ASSERT(sizeof(lhs) == 4);

    return rhs.equal(lhs);
}

/** \brief operator != for NULL_v and an arbitrary type
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
inline ss_bool_t operator !=(NULL_v const& lhs, T const& rhs)
{
    STLSOFT_STATIC_ASSERT(sizeof(rhs) == 4);

    return !lhs.equal(rhs);
}

/** \brief operator != for an arbitrary type and NULL_v
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
inline ss_bool_t operator !=(T const& lhs, NULL_v const& rhs)
{
    STLSOFT_STATIC_ASSERT(sizeof(lhs) == 4);

    return !rhs.equal(lhs);
}
#endif /* 0 */

#endif /* _STLSOFT_NULL_v_DEFINED */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/null_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_NULL */

/* ///////////////////////////// end of file //////////////////////////// */
