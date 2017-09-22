/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/common/string_concatenation_flags.hpp
 *
 * Purpose:     Common flags for use with string_concatenator_iterator and
 *              c_string_concatenator_iterator.
 *
 * Created:     31st July 2010
 * Updated:     31st July 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/iterators/common/string_concatenation_flags.hpp
 *
 * \brief [C++ only] Common flags for use with string_concatenator_iterator and
 *   c_string_concatenator_iterator.
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS
#define STLSOFT_INCL_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS_MAJOR    1
# define STLSOFT_VER_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS_MINOR    0
# define STLSOFT_VER_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS_REVISION 1
# define STLSOFT_VER_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS_EDIT     1
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
 * Flags
 */

struct string_concatenation_flags
{
    enum
    {
        /// Causes string concatenation to always insert the separator, even
        /// between empty elements
        AlwaysSeparate  =   0x0001,
    };
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS */

/* ///////////////////////////// end of file //////////////////////////// */
