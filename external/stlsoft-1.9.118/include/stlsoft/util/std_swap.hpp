/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/std_swap.hpp
 *
 * Purpose:     Contains the std_swap() function.
 *
 * Created:     27th June 2005
 * Updated:     12th August 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/std_swap.hpp
 *
 * \brief [C++ only] Swap utility functions
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_STD_SWAP_MAJOR    1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_STD_SWAP_MINOR    2
# define STLSOFT_VER_STLSOFT_UTIL_HPP_STD_SWAP_REVISION 2
# define STLSOFT_VER_STLSOFT_UTIL_HPP_STD_SWAP_EDIT     19
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#if !defined(STLSOFT_CF_std_NAMESPACE)
# define STLSOFT_STD_SWAP_NO_USE_STD
#endif /* !STLSOFT_CF_std_NAMESPACE */
#if !defined(STLSOFT_STD_SWAP_NO_USE_STD)
# include <algorithm>
#endif /* STLSOFT_STD_SWAP_NO_USE_STD */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft_std_swap
{
#endif /* _STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// This is defined only when the std namespace is not.
#ifdef STLSOFT_STD_SWAP_NO_USE_STD
# ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft_nostd_util
{
# endif /* _STLSOFT_NO_NAMESPACE */

    template<ss_typename_param_k T>
    inline void swap(T& lhs, T& rhs)
    {
        T   tmp(lhs);

        lhs = rhs;
        rhs = tmp;
    }

# ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft_nostd_util
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* STLSOFT_STD_SWAP_NO_USE_STD */


/** \brief Calls std::swap on the arguments, but may also use
 *    Argument-Dependent Lookup) to use a specialised form.
 *
 * \ingroup group__library__utility
 *
 * \note For compilers, such as Open Watcom, that do not have a functional
 * standard library implementation, the swap() implementation from
 * stlsoft::stlsoft_nostd_util is used.
 *
 * \param lhs The left hand parameter
 * \param rhs The right hand parameter
 */
template<ss_typename_param_k T>
inline void std_swap(T& lhs, T& rhs)
{
    // Here we introduce the 'std' implementation, via a 'using namespace std'
    // directive, so that Argument-Dependent Lookup can have the best grab

#ifdef STLSOFT_STD_SWAP_NO_USE_STD
# ifndef _STLSOFT_NO_NAMESPACE
    using namespace stlsoft_nostd_util;
# endif /* !_STLSOFT_NO_NAMESPACE */
#else /* ? STLSOFT_STD_SWAP_NO_USE_STD */
# ifdef STLSOFT_CF_std_NAMESPACE
#  if ( ( defined(STLSOFT_COMPILER_IS_INTEL) && \
          defined(_MSC_VER)) || \
         defined(STLSOFT_COMPILER_IS_MSVC)) && \
      _MSC_VER < 1310
    using std::swap;
#  else /* ? compiler */
    using namespace std;
#  endif /* compiler */
# endif /* STLSOFT_CF_std_NAMESPACE */
#endif /* STLSOFT_STD_SWAP_NO_USE_STD */

    swap(lhs, rhs);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft_std_swap
#endif /* _STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
    /// Calls std::swap on the arguments, but may also use Argument-Dependent
    /// Lookup to use a specialised form
    ///
    /// \note For compilers, such as Open Watcom, that do not have a functional
    /// standard library implementation, the swap() implementation from
    /// stlsoft::stlsoft_nostd_util is used.
    ///
    /// \param lhs The left hand parameter
    /// \param rhs The right hand parameter
    template<ss_typename_param_k T>
    inline void std_swap(T& lhs, T& rhs)
    {
        // Defer to std_swap implementation in stlsoft_std_swap namespace
        //
        // This is to 'isolate' the selection of the swap() algorithm out of
        // the stlsoft namespace, so both stlsoft and std get equal byte at the
        // type resolution

        stlsoft_std_swap::std_swap(lhs, rhs);
    }

} // namespace stlsoft

#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

/* ///////////////////////////// end of file //////////////////////////// */
