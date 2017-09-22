/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/inert.hpp
 *
 * Purpose:     Contains the inert class.
 *
 * Created:     16th January 2002
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


/** \file stlsoft/util/inert.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::inert class
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_INERT
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_INERT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INERT_MAJOR      4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INERT_MINOR      0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INERT_REVISION   3
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INERT_EDIT       40
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

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

/** \brief This class is almost completely unusable, and is used as a placeholder or as
 * a name remover
 *
 * \ingroup group__library__utility
 */
// class inert
class inert
{
// Construction
public:
    /// Default constructor
    inert()
    {}
    /// Destructor
    ~inert() stlsoft_throw_0()
    {}

// Not to be implemented
private:
    inert(inert const& rhs);
    inert const& operator =(inert const& rhs);

#ifndef new
    void *operator new(ss_size_t ) stlsoft_throw_0()
    {
        return 0;
    }
    void operator delete(void*)
    {}
#endif /* !new */

    int operator !() const;
#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
    operator bool() const;
#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */
    operator void *() const;
    void *operator &() const;
#if defined(STLSOFT_COMPILER_IS_GCC) || \
    defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
    struct _{};
    _ *operator ->() const;
#else /* ? compiler */
    struct {} *operator ->() const;
#endif /* compiler */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k T>
    operator T &();
    template <ss_typename_param_k T>
    operator T const& () const;
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_INERT */

/* ///////////////////////////// end of file //////////////////////////// */
