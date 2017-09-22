/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/std/iterator_category_limiters.hpp (derived in part from stlsoft/iterators/filter_iterator.hpp)
 *
 * Purpose:     Meta classes to limit iterator categories.
 *
 * Created:     3rd January 2006
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


/** \file stlsoft/util/std/iterator_category_limiters.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::min_iterator_category
 *   iterator category utility class template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS
#define STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS_MAJOR      1
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS_MINOR      0
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS_REVISION   2
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS_EDIT       8
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
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

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

template<   ss_typename_param_k C1
        ,   ss_typename_param_k C2
        >
struct min_iterator_category;

#define STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(c1, c2, cr)  \
                                                                    \
    STLSOFT_TEMPLATE_SPECIALISATION                                 \
    struct min_iterator_category<c1, c2>                            \
    {                                                               \
        typedef cr      iterator_category;                          \
    }

STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::input_iterator_tag,         std::input_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::forward_iterator_tag,       std::input_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::bidirectional_iterator_tag, std::input_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::random_access_iterator_tag, std::input_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::input_iterator_tag,         std::forward_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::forward_iterator_tag,       std::forward_iterator_tag, std::forward_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::bidirectional_iterator_tag, std::forward_iterator_tag, std::forward_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::random_access_iterator_tag, std::forward_iterator_tag, std::forward_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::input_iterator_tag,         std::bidirectional_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::forward_iterator_tag,       std::bidirectional_iterator_tag, std::forward_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::bidirectional_iterator_tag, std::bidirectional_iterator_tag, std::bidirectional_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::random_access_iterator_tag, std::bidirectional_iterator_tag, std::bidirectional_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::input_iterator_tag,         std::random_access_iterator_tag, std::input_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::forward_iterator_tag,       std::random_access_iterator_tag, std::forward_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::bidirectional_iterator_tag, std::random_access_iterator_tag, std::bidirectional_iterator_tag);
STLSOFT_ITER_CAT_LIMITER_DEFINE_SPECIALISATION(std::random_access_iterator_tag, std::random_access_iterator_tag, std::random_access_iterator_tag);

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_CATEGORY_LIMITERS */

/* ///////////////////////////// end of file //////////////////////////// */
