/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/copy_functions.hpp
 *
 * Purpose:     String utility functions for copying.
 *
 * Created:     13th June 2006
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


/** \file stlsoft/string/copy_functions.hpp
 *
 * \brief [C++ only] String utility functions for copying
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS
#define STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS_MAJOR       1
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS_MINOR       0
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS_REVISION    2
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS_EDIT        7
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */

#ifdef STLSOFT_UNITTEST
# include <string.h>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

/** \brief Utility function for copying C-string contents into a caller
 *   supplied buffer, which may be NULL to measure the required extent.
 *
 * \ingroup group__library__string
 *
 * \param dest Pointer to a character buffer that will receive the
 *  contents. May be NULL, in which case the function returns \c cchSource.
 * \param cchDest The maximum number of characters to be written into
 *  \c dest.
 * \param src Pointer to character buffer whose contents will be copied
 *  into \c dest. May not be NULL.
 * \param cchSource The number of characters in \c src.
 *
 */
template <ss_typename_param_k C>
inline ss_size_t copy_contents(C *dest, ss_size_t cchDest, C const* src, ss_size_t cchSource)
{
    STLSOFT_ASSERT(NULL != src);

    if(NULL == dest)
    {
        return cchSource;
    }
    else
    {
        typedef C                       char_t;
        typedef stlsoft_char_traits<C>  traits_t;

        const ss_size_t cchContent  =   (cchSource < cchDest) ? cchSource : cchDest;

        traits_t::copy(dest, src, cchContent);

        if(cchContent < cchDest)
        {
            traits_t::assign(&dest[cchContent], cchDest - cchContent, 0);
        }

        return cchContent;
    }
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/copy_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
