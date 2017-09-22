/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/util/memory_exception_translation_policies.hpp
 *
 * Purpose:     Contains .
 *
 * Created:     2nd February 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file mfcstl/util/memory_exception_translation_policies.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::bad_alloc_throwing_policy
 *   and mfcstl::CMemoryException_throwing_policy exception translation
 *   policy classes
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES
#define MFCSTL_INCL_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES_MAJOR     1
# define MFCSTL_VER_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES_MINOR     0
# define MFCSTL_VER_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES_REVISION  5
# define MFCSTL_VER_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES_EDIT      12
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */

#ifndef STLSOFT_INCL_NEW
# define STLSOFT_INCL_NEW
# include <new>  // for std::bad_alloc
#endif /* !STLSOFT_INCL_NEW */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
/* Define stlsoft::mfcstl_project */

namespace stlsoft
{

namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Memory exception translation policy that ensures CMemoryException* is the thrown type in all cases
 *
 * \ingroup group__library__utility
 */
struct CMemoryException_throwing_policy
{
public:
    /// <i>Translates</i> a CMemoryException* into a CMemoryException*, by simply rethrowing via a <code>throw;</code> statement
    static void handle(CMemoryException *)
    {
        throw;
    }
    /// Translates a std::bad_alloc& into a CMemoryException*
    static void handle(mfcstl_ns_qual_std(bad_alloc) &)
    {
        AfxThrowMemoryException();
    }
};

/** \brief Memory exception translation policy that ensures std::bad_alloc is the thrown type in all cases
 *
 * \ingroup group__library__utility
 */
struct bad_alloc_throwing_policy
{
public:
    /// Translates a CMemoryException* into a std::bad_alloc&
    static void handle(CMemoryException *)
    {
        STLSOFT_THROW_X(mfcstl_ns_qual_std(bad_alloc)());
    }
    /// <i>Translates</i> a std::bad_alloc& into a std::bad_alloc&, by simply rethrowing via a <code>throw;</code> statement
    static void handle(mfcstl_ns_qual_std(bad_alloc) &)
    {
        throw;
    }
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* MFCSTL_INCL_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES */

/* ///////////////////////////// end of file //////////////////////////// */
